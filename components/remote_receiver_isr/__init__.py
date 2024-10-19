from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import spi
import esphome.config_validation as cv
from esphome.const import CONF_PIN, CONF_ID
from esphome.core import CORE, TimePeriod

CODEOWNERS = ["@swoboda1337"]
AUTO_LOAD = ["remote_base"]
CONF_ON_RAW = "on_raw"
CONF_SHORTEST = "shortest"
CONF_CAPTURE = "capture"
CONF_IDLE = "idle"

remote_receiver_isr_ns = cg.esphome_ns.namespace("remote_receiver_isr")
remote_receiver_isr = remote_receiver_isr_ns.class_("RemoteReceiverIsr", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(remote_receiver_isr),
        cv.Required(CONF_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_ON_RAW): automation.validate_automation(single=True),
        cv.Optional(CONF_CAPTURE, default=156): cv.int_range(min=0, max=1024),
        cv.Optional(CONF_SHORTEST, default="100us"): cv.All(
            cv.positive_time_period_microseconds,
            cv.Range(max=TimePeriod(microseconds=1000000000)),
        ),
        cv.Optional(CONF_IDLE, default="5000us"): cv.All(
            cv.positive_time_period_microseconds,
            cv.Range(max=TimePeriod(microseconds=1000000000)),
        ),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
    cg.add(var.set_shortest(config[CONF_SHORTEST]))
    cg.add(var.set_capture(config[CONF_CAPTURE]))
    cg.add(var.set_idle(config[CONF_IDLE]))
    if CONF_ON_RAW in config:
        await automation.build_automation(
            var.get_packet_trigger(),
            [(cg.std_vector.template(cg.int32), "x")],
            config[CONF_ON_RAW]
        )
