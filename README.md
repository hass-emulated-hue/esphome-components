# esphome-components
Custom (light) components for ESPHome


## RGB-CT Light
Create a real Philips HUE Lightstrip look-a-like with ESPHome and RGB+CT lightstrips.

- Allows mixing rgb leds for color temperature outside of range of the white leds.
- Same behaviour as native Hue lights in HA: RGB and white combiend into single light.
- Can be 100% color matched with other HUE lights.
- Also works with light bulbs

### Installation:
1. Copy custom_components folder into esphome config directory (e.g. /config/esphome).
2. Use configuration from below.

**Example ESpHome config:**

```
light:
  - platform: rgbct
    name: "${device_name}: RGBCT Light"
    red: pwm_r
    green: pwm_g
    blue: pwm_b
    warm_white: pwm_ww
    cold_white: pwm_cw
    cold_white_color_temperature: 6500 K
    warm_white_color_temperature: 2400 K
    # In Kelvin or Mireds
    max_warm_color_temperature: 500 mireds  # Optional defaults to 500 mireds
    max_cold_color_temperature: 153 mireds  # Optional defaults to 153 mireds

output:
  - platform: esp8266_pwm
    pin: GPIO13
    id: pwm_ww

  - platform: esp8266_pwm
    pin: GPIO12
    id: pwm_g
    max_power: 45%

  - platform: esp8266_pwm
    pin: GPIO14
    id: pwm_b
    max_power: 30%

  - platform: esp8266_pwm
    pin: GPIO5
    id: pwm_cw
    
  - platform: esp8266_pwm
    pin: GPIO4
    id: pwm_r
    
```

In the example above replace the correct values for the GPIO pins.
Adjust max_power setting for each channel to match your lightstrip.
Above settings were tested using a (very common) 24V RGB-CT 5050 ledstrip from Amazon/Ali.
Adjust the levels until the light exactly matches.

Adjust min/max color temperature of light as desired.
