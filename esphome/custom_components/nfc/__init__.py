import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from esphome import pins

AUTO_LOAD = ['text_sensor']

nfc_ns = cg.esphome_ns.namespace('nfc')
NFCComponent = nfc_ns.class_('NFCComponent', cg.Component)

CONF_READED_VALUE = "readed_value"
CONF_TX_PIN = "tx_pin"
CONF_RX_PIN = "rx_pin"
CONF_RESET_PIN = "reset_pin"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(NFCComponent),
    cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_TX_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_RX_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_READED_VALUE): text_sensor.text_sensor_schema(),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add_library("adafruit/Adafruit BusIO", None)
    cg.add_library("adafruit/Adafruit PN532", None)
#    cg.add_library("Wire", None)
#    cg.add_library("SPI", None)


    reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_reset_pin(reset_pin))
    
    tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
    cg.add(var.set_tx_pin(tx_pin))
    rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
    cg.add(var.set_rx_pin(rx_pin))

    if CONF_READED_VALUE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_READED_VALUE])
        cg.add(var.set_readed_value_sensor(sens))