import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_PIN
from esphome import pins

AUTO_LOAD = ["text_sensor"]

rf433_ns = cg.esphome_ns.namespace("rf433")
RF433Component = rf433_ns.class_("RF433Component", cg.Component)

CONF_READED_VALUE = "readed_value"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RF433Component),
        cv.Required(CONF_PIN): cv.All(pins.internal_gpio_input_pin_schema),
        cv.Optional(CONF_READED_VALUE): text_sensor.text_sensor_schema(),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add_library("https://github.com/sui77/rc-switch.git", None)

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))

    if CONF_READED_VALUE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_READED_VALUE])
        cg.add(var.set_readed_value_sensor(sens))