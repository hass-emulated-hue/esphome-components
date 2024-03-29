#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"

namespace esphome {
namespace rgbcct {

class RGBCCTLightOutput : public light::LightOutput {
 public:
  void set_red(output::FloatOutput *red) { this->red_ = red; }
  void set_green(output::FloatOutput *green) { this->green_ = green; }
  void set_blue(output::FloatOutput *blue) { this->blue_ = blue; }
  void set_cold_white(output::FloatOutput *cold_white) { this->cold_white_ = cold_white; }
  void set_warm_white(output::FloatOutput *warm_white) { this->warm_white_ = warm_white; }
  void set_constant_brightness(bool constant_brightness) { this->constant_brightness_ = constant_brightness; }
  void set_cold_white_temperature(float cold_white_temperature) {
    this->cold_white_temperature_ = cold_white_temperature;
  }
  void set_warm_white_temperature(float warm_white_temperature) {
    this->warm_white_temperature_ = warm_white_temperature;
  }
  void set_max_warm_color_temperature(float warm_color_temperature) {
    // Warmer colors = > mireds
    this->max_color_temperature_ = warm_color_temperature;
  }
  void set_max_cold_color_temperature(float cold_color_temperature) {
    // Colder colors = < mireds
    this->min_color_temperature_ = cold_color_temperature;
  }
  void set_max_combined_white_level(float max_combined_white_level) {
    this->max_combined_white_level_ = max_combined_white_level;
  }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLOR_TEMPERATURE});
    traits.set_min_mireds(this->min_color_temperature_);
    traits.set_max_mireds(this->max_color_temperature_);
    return traits;
  }

  void write_state(light::LightState *state) override {
    float cwhite = 0.0, wwhite = 0.0;
    float red = state->current_values.get_red();
    float green = state->current_values.get_green();
    float blue = state->current_values.get_blue();
    float colorTemp = state->current_values.get_color_temperature();
    const float brightness = state->current_values.get_brightness() * state->current_values.get_state();
    const bool is_ct_mode = state->current_values.get_color_mode() == light::ColorMode::COLOR_TEMPERATURE;

    if (is_ct_mode) {
      // colortemp to rgb calculations based on the work of Tanner Helland
      // https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html

      // convert mired to kelvin
      float colorTempK = floor(1000000 / colorTemp);
      colorTempK = colorTempK / 100;  // divide by 100 for calculations

      if (colorTempK <= 66) {
        red = 255;
        green = colorTempK;
        green = 99.4708025861 * log(green) - 161.1195681661;
        green = green * 1.15;
        if (colorTempK <= 19) {
          blue = 0;
        } else {
          blue = colorTempK - 10;
          blue = 138.5177312231 * log(blue) - 305.0447927307;
        }
      } else {
        red = colorTempK - 60;
        red = 329.698727446 * pow(red, -0.1332047592);
        green = colorTempK - 60;
        green = 288.1221695283 * pow(green, -0.0755148492);
        blue = 255;
      }

      red = (clamp<float>(red, 0, 255) / 255.0f);
      green = (clamp<float>(green, 0, 255) / 255.0f);
      blue = (clamp<float>(blue, 0, 255) / 255.0f);

      const float ww_fraction =
          (colorTemp - this->cold_white_temperature_) / (this->warm_white_temperature_ - this->cold_white_temperature_);
      const float cw_fraction = 1.0f - ww_fraction;
      const float max_cw_ww = std::max(ww_fraction, cw_fraction);
      cwhite = (cw_fraction / max_cw_ww);
      wwhite = (ww_fraction / max_cw_ww);

      // // correct white levels
      if (colorTemp < this->cold_white_temperature_)
        cwhite = (1.0f - ((this->cold_white_temperature_ - colorTemp) / 100));
      else if (colorTemp > this->warm_white_temperature_)
        wwhite = (1.0f - ((colorTemp - this->warm_white_temperature_) / 100));
    }

    // apply gamma correction
    red = (red > 0.04045f) ? pow((red + 0.055f) / (1.0f + 0.055f), 2.4f) : (red / 12.92f);
    green = (green > 0.04045f) ? pow((green + 0.055f) / (1.0f + 0.055f), 2.4f) : (green / 12.92f);
    blue = (blue > 0.04045f) ? pow((blue + 0.055f) / (1.0f + 0.055f), 2.4f) : (blue / 12.92f);

    // apply brightness
    red = red * brightness;
    green = green * brightness;
    blue = blue * brightness;

    if (!constant_brightness_) {
      cwhite = cwhite * brightness;
      wwhite = wwhite * brightness;
    } else {
      // Just multiplying by cw_level / (cw_level + ww_level) would divide out the brightness information from the
      // cold_white and warm_white settings (i.e. cw=0.8, ww=0.4 would be identical to cw=0.4, ww=0.2), which breaks
      // transitions. Use the highest value as the brightness for the white channels (the alternative, using cw+ww/2,
      // reduces to cw/2 and ww/2, which would still limit brightness to 100% of a single channel, but isn't very
      // useful in all other aspects -- that behaviour can also be achieved by limiting the output power).

      // Prevent negatives from cancelling out, causing white to turn on when not intended
      wwhite = clamp<float>(wwhite, 0.0f, 1.0f);
      cwhite = clamp<float>(cwhite, 0.0f, 1.0f);
      const float sum = cwhite > 0 || wwhite > 0 ? cwhite + wwhite : 1;  // Don't divide by zero.
      // Obtain total brightness then apply max limiter
      const float white_level = std::min(this->max_combined_white_level_, std::max(cwhite, wwhite));
      cwhite = brightness * white_level * (cwhite / sum);
      wwhite = brightness * white_level * (wwhite / sum);
    }

    // actually set the new values
    red = clamp<float>(red, 0.0f, 1.0f);
    green = clamp<float>(green, 0.0f, 1.0f);
    blue = clamp<float>(blue, 0.0f, 1.0f);
    wwhite = clamp<float>(wwhite, 0.0f, 1.0f);
    cwhite = clamp<float>(cwhite, 0.0f, 1.0f);

    this->red_->set_level(red);
    this->green_->set_level(green);
    this->blue_->set_level(blue);
    this->warm_white_->set_level(wwhite);
    this->cold_white_->set_level(cwhite);
  }

 protected:
  output::FloatOutput *red_;
  output::FloatOutput *green_;
  output::FloatOutput *blue_;
  output::FloatOutput *cold_white_;
  output::FloatOutput *warm_white_;
  bool constant_brightness_;
  float cold_white_temperature_;
  float warm_white_temperature_;
  float red_correct_;
  float green_correct_;
  float blue_correct_;
  float cwhite_correct_;
  float wwhite_correct_;
  float brightness_correct_;
  float min_color_temperature_;
  float max_color_temperature_;
  float max_combined_white_level_;
};

}  // namespace rgbcct
}  // namespace esphome
