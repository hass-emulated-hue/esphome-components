#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include <float.h>
#include <math.h>
#define LERP(a, b, c) (((b) - (a)) * (c) + (a))

typedef struct UVT
{
  double u;
  double v;
  double t;
} UVT;

double rt[31] = {/* reciprocal temperature (K) */
                 DBL_MIN, 10.0e-6, 20.0e-6, 30.0e-6, 40.0e-6, 50.0e-6,
                 60.0e-6, 70.0e-6, 80.0e-6, 90.0e-6, 100.0e-6, 125.0e-6,
                 150.0e-6, 175.0e-6, 200.0e-6, 225.0e-6, 250.0e-6, 275.0e-6,
                 300.0e-6, 325.0e-6, 350.0e-6, 375.0e-6, 400.0e-6, 425.0e-6,
                 450.0e-6, 475.0e-6, 500.0e-6, 525.0e-6, 550.0e-6, 575.0e-6,
                 600.0e-6};

UVT uvt[31] = {
    {0.18006, 0.26352, -0.24341},
    {0.18066, 0.26589, -0.25479},
    {0.18133, 0.26846, -0.26876},
    {0.18208, 0.27119, -0.28539},
    {0.18293, 0.27407, -0.30470},
    {0.18388, 0.27709, -0.32675},
    {0.18494, 0.28021, -0.35156},
    {0.18611, 0.28342, -0.37915},
    {0.18740, 0.28668, -0.40955},
    {0.18880, 0.28997, -0.44278},
    {0.19032, 0.29326, -0.47888},
    {0.19462, 0.30141, -0.58204},
    {0.19962, 0.30921, -0.70471},
    {0.20525, 0.31647, -0.84901},
    {0.21142, 0.32312, -1.0182},
    {0.21807, 0.32909, -1.2168},
    {0.22511, 0.33439, -1.4512},
    {0.23247, 0.33904, -1.7298},
    {0.24010, 0.34308, -2.0637},
    {0.24792, 0.34655, -2.4681}, /* Note: 0.24792 is a corrected value for the error found in W&S as 0.24702 */
    {0.25591, 0.34951, -2.9641},
    {0.26400, 0.35200, -3.5814},
    {0.27218, 0.35407, -4.3633},
    {0.28039, 0.35577, -5.3762},
    {0.28863, 0.35714, -6.7262},
    {0.29685, 0.35823, -8.5955},
    {0.30505, 0.35907, -11.324},
    {0.31320, 0.35968, -15.628},
    {0.32129, 0.36011, -23.325},
    {0.32931, 0.36038, -40.770},
    {0.33724, 0.36051, -116.45}};

/// XYT to ColorTemp corelation copyright Bruce Lindbloom
/// http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_T.html
float XYZtoColorTemp(float x, float y, float z)
{
  float us, vs, p, di, dm;
  int i;

  if ((x < 1.0e-20) && (y < 1.0e-20) && (z < 1.0e-20))
    return 0.0; /* protect against possible divide-by-zero failure */
  us = (4.0 * x) / (x + 15.0 * y + 3.0 * z);
  vs = (6.0 * y) / (x + 15.0 * y + 3.0 * z);
  dm = 0.0;
  for (i = 0; i < 31; i++)
  {
    di = (vs - uvt[i].v) - uvt[i].t * (us - uvt[i].u);
    if ((i > 0) && (((di < 0.0) && (dm >= 0.0)) || ((di >= 0.0) && (dm < 0.0))))
      break; /* found lines bounding (us, vs) : i-1 and i */
    dm = di;
  }
  if (i == 31)
    return 0.0; /* bad XYZ input, color temp would be less than minimum of 1666.7 degrees, or too far towards blue */
  di = di / sqrt(1.0 + uvt[i].t * uvt[i].t);
  dm = dm / sqrt(1.0 + uvt[i - 1].t * uvt[i - 1].t);
  p = dm / (dm - di); /* p = interpolation parameter, 0.0 : i-1, 1.0 : i */
  p = 1.0 / (LERP(rt[i - 1], rt[i], p));
  return p;
}

namespace esphome
{
namespace rgbct
{

class RGBCTLightOutput : public light::LightOutput
{
public:
  void set_rgb_light(light::LightState *rgb_light) { rgb_light_ = rgb_light; }
  void set_cold_white(output::FloatOutput *cold_white) { cold_white_ = cold_white; }
  void set_warm_white(output::FloatOutput *warm_white) { warm_white_ = warm_white; }
  void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }

  light::LightTraits get_traits() override
  {
    auto traits = light::LightTraits();
    traits.set_supports_brightness(true);
    traits.set_supports_rgb(true);
    traits.set_supports_rgb_white_value(false);
    traits.set_supports_color_temperature(true);
    traits.set_min_mireds(153); // home assistant minimum 153
    traits.set_max_mireds(500); // home assistant maximum 500
    return traits;
  }

  void set_rgb_light_(float red, float green, float blue, float brightness)
  {
    // set gamma from color
    this->rgb_light_->set_gamma_correct(0);
    if (red + green + blue == 0) {
      light::LightCall call = this->rgb_light_->turn_off();
      call.perform();
    } else {
      light::LightCall call = this->rgb_light_->turn_on();
      call.set_rgb(red, green, blue);
      // we are a pseudo layer, transition length must be 0
      // to properly display light levels
      // call.set_transition_length(0);
      call.set_brightness(brightness);
      call.perform();
    }
  }

  void write_state(light::LightState *state) override
  {
    float cwhite = 0.0, wwhite = 0.0;
    float red = state->current_values.get_red();
    float green = state->current_values.get_green();
    float blue = state->current_values.get_blue();
    float colorTemp = state->current_values.get_color_temperature();
    const float brightness = state->current_values.get_brightness() * state->current_values.get_state();
    const bool white_changed = this->last_color_temp_ != colorTemp;
    const bool color_changed = this->last_rgb_ != red + green + blue;

    if (white_changed || (this->white_active && !color_changed) || (red == 1.0 && green == 1.0 && blue == 1.0))
    {
      /// Set by color temperature / white level
      this->white_active = true;
      this->last_color_temp_ = colorTemp;
      
      // colortemp to rgb calculations based on the work of Tanner Helland
      // https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html

      // convert mired to kelvin
      float colorTempK = floor(1000000 / colorTemp);
      colorTempK = colorTempK / 100; // divide by 100 for calculations
      
      if( colorTempK <= 66 ){ 
        red = 255; 
        green = colorTempK;
        green = 99.4708025861 * log(green) - 161.1195681661;
        green = green * 1.15;
        if( colorTempK <= 19){
            blue = 0;
        } else {
            blue = colorTempK-10;
            blue = 138.5177312231 * log(blue) - 305.0447927307;
        }
      } else {
          red = colorTempK - 60;
          red = 329.698727446 * pow(red, -0.1332047592);
          green = colorTempK - 60;
          green = 288.1221695283 * pow(green, -0.0755148492 );
          blue = 255;
        }
      
      red = (clamp(red, 0, 255) / 255.0f);
      green = (clamp(green, 0, 255) / 255.0f);
      blue = (clamp(blue, 0, 255) / 255.0f);


      const float ww_fraction = (colorTemp - this->cold_white_temperature_) / (this->warm_white_temperature_ - this->cold_white_temperature_);
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
    else if (color_changed || !this->white_active)
    {
      // RGB color setting
      this->white_active = false;
      this->last_rgb_ = red + green + blue;

    }

    // apply gamma correction
    red = (red > 0.04045f) ? pow((red + 0.055f) / (1.0f + 0.055f), 2.4f) : (red / 12.92f);
    green = (green > 0.04045f) ? pow((green + 0.055f) / (1.0f + 0.055f), 2.4f) : (green / 12.92f);
    blue = (blue > 0.04045f) ? pow((blue + 0.055f) / (1.0f + 0.055f), 2.4f) : (blue / 12.92f);

    // apply brightness
    cwhite = cwhite * brightness;
    wwhite = wwhite * brightness;

    // actually set the new values
    red = clamp(red, 0.0f, 1.0f);
    green = clamp(green, 0.0f, 1.0f);
    blue = clamp(blue, 0.0f, 1.0f);
    wwhite = clamp(wwhite, 0.0f, 1.0f);
    cwhite = clamp(cwhite, 0.0f, 1.0f);

    this->set_rgb_light_(red, green, blue, brightness);
    this->warm_white_->set_level(wwhite);
    this->cold_white_->set_level(cwhite);
  }

protected:
  light::LightState *rgb_light_;
  output::FloatOutput *cold_white_;
  output::FloatOutput *warm_white_;
  float cold_white_temperature_;
  float warm_white_temperature_;
  float red_correct_;
  float green_correct_;
  float blue_correct_;
  float cwhite_correct_;
  float wwhite_correct_;
  float brightness_correct_;
  float last_color_temp_;
  float last_rgb_{0.0};
  bool white_active{false};
  void HSVtoRGB(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV)
  {
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;

    if (0 <= fHPrime && fHPrime < 1)
    {
      fR = fC;
      fG = fX;
      fB = 0;
    }
    else if (1 <= fHPrime && fHPrime < 2)
    {
      fR = fX;
      fG = fC;
      fB = 0;
    }
    else if (2 <= fHPrime && fHPrime < 3)
    {
      fR = 0;
      fG = fC;
      fB = fX;
    }
    else if (3 <= fHPrime && fHPrime < 4)
    {
      fR = 0;
      fG = fX;
      fB = fC;
    }
    else if (4 <= fHPrime && fHPrime < 5)
    {
      fR = fX;
      fG = 0;
      fB = fC;
    }
    else if (5 <= fHPrime && fHPrime < 6)
    {
      fR = fC;
      fG = 0;
      fB = fX;
    }
    else
    {
      fR = 0;
      fG = 0;
      fB = 0;
    }

    fR += fM;
    fG += fM;
    fB += fM;
  }
  static void RGBtoHSV(float &fR, float &fG, float fB, float &fH, float &fS, float &fV)
  {
    float fCMax = max(max(fR, fG), fB);
    float fCMin = min(min(fR, fG), fB);
    float fDelta = fCMax - fCMin;
    if (fDelta > 0)
    {
      if (fCMax == fR)
      {
        fH = 60 * (fmod(((fG - fB) / fDelta), 6));
      }
      else if (fCMax == fG)
      {
        fH = 60 * (((fB - fR) / fDelta) + 2);
      }
      else if (fCMax == fB)
      {
        fH = 60 * (((fR - fG) / fDelta) + 4);
      }
      if (fCMax > 0)
      {
        fS = fDelta / fCMax;
      }
      else
      {
        fS = 0;
      }

      fV = fCMax;
    }
    else
    {
      fH = 0;
      fS = 0;
      fV = fCMax;
    }

    if (fH < 0)
    {
      fH = 360 + fH;
    }
  }
};

} // namespace rgbct
} // namespace esphome
