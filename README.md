# esphome-components
Custom (light) components for ESPHome


## RGB-CT Light
Create a real Philips HUE Lightstrip look-a-like with ESPHome and RGB+CT lightstrips.

- Allows mixing rgb leds for color temperature outside of range of the white leds.
- Same behaviour as native Hue lights in HA: RGB and white combiend into single light.
- Can be 100% color matched with other HUE lights.

### Installation:
1. Copy custom_components folder into esphome config directory (e.g. /config/esphome).
2. Use configuration from below.

### Why use a light and float outputs instead of just float outputs?
This allows for support of FastLED devices combined with CT channels. ESPHome currently
has no way of providing outputs for FastLED devices. Usage of an internal light and 
float outputs allows for a greater variety of devices to be supported.

### Why not use a CWWW Light as well?
ESPHome has no way to currently directly set the float outputs once abstracted behind
a light states. Float outputs are required to individually control both CT channels.

**Example ESpHome config:**

```
light:
  - platform: rgb
    id: color_led
    internal: true
    red: pwm_r
    green: pwm_g
    blue: pwm_b

  - platform: rgbct
    name: "${device_name}: RGBCT Light"
    rgb_light: color_led
    warm_white: pwm_ww
    cold_white: pwm_cw
    cold_white_color_temperature: 6500 K
    warm_white_color_temperature: 2400 K

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
