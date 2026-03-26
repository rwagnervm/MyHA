#include "nfc.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace nfc {

static const char *const TAG = "nfc";
static const char *const NVS_NAMESPACE = "tags";

void NFCComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up NFC...");

  // Configuração do Serial e NFC
  this->my_serial_ = new HardwareSerial(1);
  
  int8_t rx = -1, tx = -1;
  if (this->rx_pin_ && this->rx_pin_->is_internal()) rx = ((InternalGPIOPin *) this->rx_pin_)->get_pin();
  if (this->tx_pin_ && this->tx_pin_->is_internal()) tx = ((InternalGPIOPin *) this->tx_pin_)->get_pin();

  ESP_LOGD(TAG, "Inicializando Serial1 com RX=%d, TX=%d", rx, tx);
  this->my_serial_->begin(115200, SERIAL_8N1, rx, tx);
  
  // Pequeno delay para estabilizar a serial
  delay(100);

  uint8_t reset_pin = 0;
  if (this->reset_pin_ != nullptr && this->reset_pin_->is_internal()) {
    reset_pin = ((InternalGPIOPin *) this->reset_pin_)->get_pin();
  }

  this->nfc_ = new Adafruit_PN532(reset_pin, this->my_serial_);
  // Não usar begin() pois ele chama Serial.begin() novamente e reseta os pinos no ESP32
  // Usamos wakeup() diretamente para acordar o chip mantendo a configuração da Serial
  this->nfc_->wakeup();
  
  // Aguarda o chip acordar
  delay(100);

  uint32_t versiondata = this->nfc_->getFirmwareVersion();
  if (versiondata) {
    ESP_LOGI(TAG, "Firmware Version: %lu.%lu.%lu.%lu", (versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF, versiondata & 0xFF);
  } else {
    ESP_LOGE(TAG, "PN532 não encontrado!");
    this->mark_failed();
    return;
  }

  // Configura para tentar poucas vezes antes de desistir (evita travar)
  this->nfc_->setPassiveActivationRetries(0x02);
  this->nfc_->SAMConfig();
}

void NFCComponent::loop() {
  // Define um intervalo de 200ms entre as verificações para não travar o ESP32
  const uint32_t INTERVAL_MS = 50;
  if (millis() - this->last_update_ < INTERVAL_MS) {
    return;
  }
  this->last_update_ = millis();
  ESP_LOGD(TAG, "-> Loop");

  bool success;
  
  // Buffer para receber a resposta do celular
  uint8_t response[255];
  uint8_t responseLength = sizeof(response);
  
  // --- O APDU MÁGICO ---
  // Este é o comando padrão ISO7816-4 "SELECT FILE"
  // Estamos procurando o AID (Application ID): F0 01 02 03 04 05 06
  uint8_t selectApdu[] = { 
    0x00, /* CLA (Class) */
    0xA4, /* INS (Instruction: Select) */
    0x04, /* P1  (Select by name) */
    0x00, /* P2  (First or only occurrence) */
    0x07, /* Lc  (Length of AID: 7 bytes) */
    0xF0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 /* O AID que vamos configurar no Android */
  };
  
  // 1. Tenta detectar se tem ALGO perto (Celular ou Tag)
  uint8_t uid_data[7];
  uint8_t uid_len;
  success = this->nfc_->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid_data, &uid_len);

  if(success) {
    ESP_LOGD(TAG, "-> Dispositivo detectado");
    
    std::string uid = format_hex(uid_data, uid_len);
    std::string payload = this->has_tag(uid);
    if (payload.empty()) {
      ESP_LOGI(TAG, "Tag não encontrada: %s", uid.c_str());
    } else {
      ESP_LOGI(TAG, "Tag Validada: %s", payload.c_str());
    }
    ESP_LOGD(TAG, "Tentando handshake...");   
    
    // 2. Envia o comando SELECT APDU para o celular
    success = this->nfc_->inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength);
    
    if(success) {
      ESP_LOGD(TAG, "--- RESPOSTA RECEBIDA DO ANDROID ---");
      std::string hex_resp = format_hex(response, responseLength);
      ESP_LOGD(TAG, "%s", hex_resp.c_str());
      
      if (this->readed_value_sensor_ != nullptr) {
        this->readed_value_sensor_->publish_state(hex_resp);
      }
      
    } else {
      ESP_LOGE(TAG, "-> Falha: Dispositivo não tem o App correto ou rejeitou.");
    }
  }
}

void NFCComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "NFC:");
  if (this->readed_value_sensor_ != nullptr) {
    LOG_TEXT_SENSOR("  ", "Readed Value", this->readed_value_sensor_);
  }
  LOG_PIN("  TX Pin: ", this->tx_pin_);
  LOG_PIN("  RX Pin: ", this->rx_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
}

void NFCComponent::add_tag(const std::string &id, const std::string &desc) {
  nvs_handle_t my_handle;
  esp_err_t err;

  // Abre o namespace "nfc_storage" no NVS
  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Erro (%s) ao abrir NVS handle!", esp_err_to_name(err));
    return;
  }

  // Grava a string
  err = nvs_set_str(my_handle, id.c_str(), desc.c_str());
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Falha ao gravar no NVS! Erro: %s", esp_err_to_name(err));
  } else {
    // Commit (salva efetivamente na flash)
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Falha ao fazer commit no NVS! Erro: %s", esp_err_to_name(err));
    } else {
      ESP_LOGI(TAG, "NVS Salvo: %s = %s", id.c_str(), desc.c_str());
    }
  }

  // Fecha o handle
  nvs_close(my_handle);
}

void NFCComponent::list_tags() {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Erro ao abrir NVS namespace '%s': %s", NVS_NAMESPACE, esp_err_to_name(err));
    return;
  }

  // Inicia o iterador para a partição "nvs" e o namespace especificado
  nvs_iterator_t it = nullptr;
  err = nvs_entry_find("nvs", NVS_NAMESPACE, NVS_TYPE_ANY, &it);
  while (err == ESP_OK) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    
    if (info.type == NVS_TYPE_STR) {
        size_t len = 0;
        // Primeiro pega o tamanho da string
        if (nvs_get_str(my_handle, info.key, NULL, &len) == ESP_OK) {
            char *buf = new char[len];
            // Depois pega o valor
            if (nvs_get_str(my_handle, info.key, buf, &len) == ESP_OK) {
                ESP_LOGI(TAG, "Namespace: %s | Key: %s | Value: %s", NVS_NAMESPACE, info.key, buf);
            }
            delete[] buf;
        }
    } else {
        ESP_LOGI(TAG, "Namespace: %s | Key: %s | Type: 0x%02x", NVS_NAMESPACE, info.key, info.type);
    }
    
    err = nvs_entry_next(&it);
  }
  nvs_close(my_handle);
}

std::string NFCComponent::has_tag(const std::string &id) {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
  if (err != ESP_OK) {
    return "";
  }

  size_t required_size;
  err = nvs_get_str(my_handle, id.c_str(), NULL, &required_size);
  
  std::string result = "";
  if (err == ESP_OK) {
    char *buf = new char[required_size];
    if (nvs_get_str(my_handle, id.c_str(), buf, &required_size) == ESP_OK) {
      result = buf;
    }
    delete[] buf;
  }
  
  nvs_close(my_handle);
  return result;
}

}  // namespace nfc
}  // namespace esphome