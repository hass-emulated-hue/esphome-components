import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import (
    CONF_BLUE,
    CONF_GREEN,
    CONF_RED,
    CONF_OUTPUT_ID,
    CONF_COLD_WHITE,
    CONF_WARM_WHITE,
    CONF_COLD_WHITE_COLOR_TEMPERATURE,
    CONF_WARM_WHITE_COLOR_TEMPERATURE,
)

rgbct_ns = cg.esphome_ns.namespace("rgbct")
RGBCTLightOutput = rgbct_ns.class_("RGBCTLightOutput", light.LightOutput)

CONF_MAX_WARM_COLOR_TEMPERATURE = "max_warm_color_temperature"
CONF_MAX_COLD_COLOR_TEMPERATURE = "max_cold_color_temperature"
CONF_CONSTANT_BRIGHTNESS = "constant_brightness"
CONF_MAX_COMBINED_WHITE_LEVEL = "max_combined_white_level"

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(RGBCTLightOutput),
        cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
        cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
        cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_COLD_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_WARM_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
        cv.Required(CONF_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
        cv.Optional(
            CONF_MAX_WARM_COLOR_TEMPERATURE, default="500 mireds"
        ): cv.color_temperature,
        cv.Optional(
            CONF_MAX_COLD_COLOR_TEMPERATURE, default="153 mireds"
        ): cv.color_temperature,
        cv.Optional(CONF_CONSTANT_BRIGHTNESS, default=False): cv.boolean,
        cv.Optional(CONF_MAX_COMBINED_WHITE_LEVEL, default=1): cv.zero_to_one_float,
    }
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)

    red = yield cg.get_variable(config[CONF_RED])
    cg.add(var.set_red(red))

    green = yield cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_green(green))

    blue = yield cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_blue(blue))

    cwhite = yield cg.get_variable(config[CONF_COLD_WHITE])
    cg.add(var.set_cold_white(cwhite))
    cg.add(var.set_cold_white_temperature(config[CONF_COLD_WHITE_COLOR_TEMPERATURE]))

    wwhite = yield cg.get_variable(config[CONF_WARM_WHITE])
    cg.add(var.set_warm_white(wwhite))
    cg.add(var.set_warm_white_temperature(config[CONF_WARM_WHITE_COLOR_TEMPERATURE]))

    cg.add(var.set_max_warm_color_temperature(config[CONF_MAX_WARM_COLOR_TEMPERATURE]))
    cg.add(var.set_max_cold_color_temperature(config[CONF_MAX_COLD_COLOR_TEMPERATURE]))
    cg.add(var.set_constant_brightness(config[CONF_CONSTANT_BRIGHTNESS]))
    cg.add(var.set_max_combined_white_level(config[CONF_MAX_COMBINED_WHITE_LEVEL]))
