import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_OUTPUT_ID, CONF_COLD_WHITE, \
    CONF_WARM_WHITE, CONF_COLD_WHITE_COLOR_TEMPERATURE, \
    CONF_WARM_WHITE_COLOR_TEMPERATURE

rgbct_ns = cg.esphome_ns.namespace('rgbct')
RGBCTLightOutput = rgbct_ns.class_('RGBCTLightOutput', light.LightOutput)
CONF_RGB_LIGHT = 'rgb_light'

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(RGBCTLightOutput),
    cv.Required(CONF_RGB_LIGHT): cv.use_id(light.LightState),
    cv.Required(CONF_COLD_WHITE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_WARM_WHITE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Required(CONF_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
})


def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)

    rgb_light = yield cg.get_variable(config[CONF_RGB_LIGHT])
    cg.add(var.set_rgb_light(rgb_light))

    cwhite = yield cg.get_variable(config[CONF_COLD_WHITE])
    cg.add(var.set_cold_white(cwhite))
    cg.add(var.set_cold_white_temperature(config[CONF_COLD_WHITE_COLOR_TEMPERATURE]))

    wwhite = yield cg.get_variable(config[CONF_WARM_WHITE])
    cg.add(var.set_warm_white(wwhite))
    cg.add(var.set_warm_white_temperature(config[CONF_WARM_WHITE_COLOR_TEMPERATURE]))

    rgb_led = yield cg.get_variable(config[CONF_RGB_LIGHT])
    # force the gamma correct to be 0 so it doesn't mess up our color values
    cg.add(rgb_led.set_gamma_correct(0))
    # force transition length to 0 for intermediary light
    cg.add(rgb_led.set_default_transition_length(0))
