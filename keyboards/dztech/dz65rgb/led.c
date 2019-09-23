#include "dz65rgb.h"

static PWMConfig pwmCFG = {
  0xFFFF,                               // PWM clock frequency
  256,                                  // PWM period (in ticks) 1S (1/10kHz=0.1mS 0.1ms*10000 ticks=1S)
  NULL,                                 // No Callback
  {
      {PWM_OUTPUT_DISABLED, NULL},
      {PWM_OUTPUT_ACTIVE_HIGH, NULL},   // Enable Channel 2
      {PWM_OUTPUT_DISABLED, NULL},
      {PWM_OUTPUT_DISABLED, NULL}
  },
  0,                                    // HW dependent
  0
};

void backlight_init_ports(void) {
  palSetPadMode(GPIOA, 4, PAL_MODE_ALTERNATE(2));   // Initialize pin A4 in alt mode 2
  pwmStart(&PWMD3, &pwmCFG);                        // Start the PWM driver on timer 3 using the PWM config
  backlight_set(0);                                 // Initialize the backlights to "off"
}

// See http://jared.geek.nz/2013/feb/linear-led-pwm
static uint16_t cie_lightness(uint16_t v) {
  if (v <= 5243) // if below 8% of max
    return v / 9; // same as dividing by 900%
  else {
    uint32_t y = (((uint32_t) v + 10486) << 8) / (10486 + 0xFFFFUL); // add 16% of max and compare
    // to get a useful result with integer division, we shift left in the expression above
    // and revert what we've done again after squaring.
    y = y * y * y >> 8;
    if (y > 0xFFFFUL) // prevent overflow
      return 0xFFFFU;
    else
      return (uint16_t) y;
  }
}

void backlight_set(uint8_t level) {
  uint32_t duty = (uint32_t)(cie_lightness(0xFFFF * (uint32_t) level / BACKLIGHT_LEVELS));
  if (level == 0) {
      // Turn backlight off
      // Disable channel 2 on PWM3
      pwmDisableChannel(&PWMD3, 1);
  } else {
    // Turn backlight on
    // Enable channel 2 on PWM3
    pwmEnableChannel(&PWMD3, 1, PWM_FRACTION_TO_WIDTH(&PWMD3,0xFFFF,duty));
  }
}
