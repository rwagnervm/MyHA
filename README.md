# My Home Assistant Setup

Este repositório contém as configurações e automações da minha residência utilizadas no **Home Assistant**, integrado a um amplo ecossistema de dispositivos **ESPHome**.

## 📌 Arquitetura & Integrações

O sistema baseia-se fortemente numa rede local construída em cima de dispositivos ESP8266/ESP32, orquestrados pelo Home Assistant usando a integração nativa da API ESPHome.

**Adicionalmente, algumas ferramentas e integrações notáveis são utilizadas:**

- **Frigate NVR:** Sistema de gravação de câmeras inteligente (NVR) com detecção de objetos por IA integrada ao Home Assistant. Gerencia os streams, gravações de eventos de segurança e detecção avançada de pessoas/veículos.
- **Google Assistant & Amazon Alexa:** Integrados no `configuration.yaml` para permitir que acionamentos da infraestrutura sejam realizados via entrada de voz.
- **MQTT:** Usado para reconhecimento e detecção de faces via ESP32-CAM (ESP WHO).
- **Painel de Alarme:** Configurado nativamente ("1311") para controle de segurança local aliado às automações.
- **Notificações Ricas:** Envio para smartphones (`mobile_app_s23`) contendo snapshots e ações executáveis diretas dos painéis de notificação (ex. Fechar o portão se o alarme disparar / Visualizar quem tocou a campainha).

## 🔌 Dispositivos ESPHome

As configurações primárias de automação dos equipamentos de hardware estão na pasta `/esphome`. O ecossistema é vasto e dividido por escopo:

### Controle de RF & Ambiente Central (Sala)

- **Luz Sala (`luz-sala.yaml`) / RF Bridge:**
  Além de comandar interruptores e relés, atua como bridge de RF (433MHz). Responsável por escutar e catalogar sensores passivos espalhados pela casa (sensores de porta e de movimento), decodificando e enviando para o Home Assistant.

### Acessos e Garagem

- **Portão Garagem (`portao-garagem.yaml`):**
  ESPHome conectado ao motor do portão operando atuadores usando relés com "Interlock" para proteção dupla.
  Possui decodificação RF embarcada, o que possibilita abri-lo e fechá-lo pelo controle de rádio do carro ("RF keys"). Possui `endstops` definindo `aberto/fechado` para atualização precisa de estado no aplicativo e notificação automática.

### Conforto Térmico e Controle IR Remoto (Ar-Condicionados e Ventiladores)

Dispositivos feitos para servir tanto na leitura de temperatura / botões físicos quanto para envio de sinais via Infravermelho.

- **Quarto da Paula (`ir-quarto-paula.yaml`):** Emissor infravermelho e leitor para protocolo Coolix. Permite controlar o ar condicionado pelo app simultaneamente ao controle de botões na parede ou interruptores do ventilador.
- **Quarto do Wagner (`ir-quarto-wagner.yaml`):** Emissor/receptor infravermelho de precisão focado na manipulação total de ar condicionado da Samsung (Componente Customizado via librarie `IRremoteESP8266`), incluindo leitura de cliques longos/curtos nos botões para ligar as luzes e ventiladores.

### Iluminação da Casa

Placas embutidas de Relés e/ou interruptores com regras de automação como ligar luzes ao detectar movimento (apenas a noite) ou paralelo virtual para poder ligar ou desligar um conjunto de luzes de um cômodo através de qualquer interruptor do cômodo.

### Monitores de Energia (Smart Meters)

- **Medidor Geral (`medidor-geral.yaml`):** Monitora o consumo de energia da casa com um pzem-004t.
- **Medidor Geladeira (`medidor-geladeira.yaml`):** Monitora o consumo individual deste eletrodoméstico.

## 🤖 Automações Inteligentes em Destaque

A pasta / arquivos do Core do HA trazem comodidades exclusivas:

1. **Luzes sincronizadas (Paralelo virtual)**: Banheiros possuem luminárias duplas. A automação liga ambas juntas detectando quem originou o evento para checar loops infinitos de recursão.
2. **Notificação de campainha**: O sistema captura um snapshot da câmera da rua para mostrar quem alguém tocou a campainha, repassando um JPG ao celular junto com notificações sonoras nas alexas da casa
3. **Ausência prolongada (Apaga Tudo)**: Notifica algum dispositivo foi equecido ligado quanto não tem ningém em casa e pergunta se deseja desligar.
4. **Notificação de estacionamento inteligente:** O sistema rastreia os atributos de geofence (`device_tracker` do S23). Ao definir o status final confirmando que acabou de "estacionar" (através da desconexão do blutooth do carro), envia as coordenadas do local onde o carro foi estacionado.
5. **Notificação de eventos do portão:** O sistema notifica enviando imagem de qualquer evento do portão.

## 🔒 Segurança

Por proteção à infraestrutura, todos os dados críticos e credenciais não se encontram dentro desse repositório de forma limpa. Eles foram extraídos valendo-se da tag de Macro `!secret`:

- Senhas Wi-Fi, OTA e Tokens das APIs ESPHome estão em `esphome/secrets.yaml`.
- Credenciais RTSP, Contas do Google Project ID e Senhas de Alarme localizam-se num arquivo local privado `secrets.yaml` raiz para que a infraestrutura se enxergue como código sem vazar dados.

## ⚙️ Integração Contínua (CI/CD) com GitHub Actions

Este repositório emprega fluxos automatizados através do arquivo localizado em `.github/workflows/ha.yml`. A cada commit, pull request e _cron schedule_ as seguintes metodologias são acionadas na nuvem para assegurar que o código esteja seguro e perfeitamente operacional antes de rodar localmente no servidor em produção:

- **Verificação Diária Preventiva (`cron: 0 12 * * *`):** Executado pontualmente todo dia ao meio-dia como um alarme preditivo. Sua finalidade mestre é testar constantemente o código estático atual e disparar avisos prévios se alguma integração ou automação quebrar nas versões novas (_stable_, _beta_ e _dev_) recém-lançadas do Home Assistant.
- **Home Assistant Configuration Check:** Monta uma infraestrutura virtual sem interface gráfica (valida o código paralelamente como _stable_, _beta_ e _dev_) para aferir a integridade estrutural de todas as `automations.yaml` e `configuration.yaml`.
- **yamllint & remarklint:** Verificam rigorosamente a formatação e formam barreiras contra lixo remanescente na indentação ou quebra de padrões estruturais dos arquivos `.yaml` e `.md`.
- **ESPHome Compiler Check:** Carrega a dependência pip nativa do ESPHome com Python 3.12 isolado e repassa todos os aparelhos `.yaml` presentes, compilando integralmente e antevendo vazamento de memória ram ou bibliotecas faltantes entre atualizações de versão.
- **Mocks de Segurança (.stubs):** Os testes em nuvem pública sobrevivem utilizando _mock files_ presentes na pasta de dotação `.stubs/` injetados paralelamente para realizar as validações do motor sem requerer os tokens, wi-fi e senhas do ambiente real.
