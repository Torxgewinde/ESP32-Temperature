/*******************************************************************************
#                                                                              #
# Using the ESP32 as (coarse) thermometer, an idea on how to replace           #
# temprature_sens_read()                                                       #
#                                                                              #
# Copyright (C) 2021 Tom Stöveken                                              #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
********************************************************************************/

#include <soc/rtc.h>

#define M1_CALPOINT1_CELSIUS 23.0f
#define M1_CALPOINT1_RAW 128253742.0f
#define M1_CALPOINT2_CELSIUS -20.0f
#define M1_CALPOINT2_RAW 114261758.0f

float readTemp1(bool printRaw = false) {
  uint64_t value = 0;
  int rounds = 100;

  for(int i=1; i<=rounds; i++) {
    value += rtc_clk_cal_ratio(RTC_CAL_RTC_MUX, 100);
    yield();
  }
  value /= (uint64_t)rounds;

  if(printRaw) {
    printf("%s: raw value is: %llu\r\n", __FUNCTION__, value);
  }

  return ((float)value - M1_CALPOINT1_RAW) * (M1_CALPOINT2_CELSIUS - M1_CALPOINT1_CELSIUS) / (M1_CALPOINT2_RAW - M1_CALPOINT1_RAW) + M1_CALPOINT1_CELSIUS;
}

#define M2_CALPOINT1_CELSIUS 23.0f
#define M2_CALPOINT1_RAW 163600.0f
#define M2_CALPOINT2_CELSIUS -20.0f
#define M2_CALPOINT2_RAW 183660.0f

float readTemp2(bool printRaw = false) {
  uint64_t value = rtc_time_get();
  delay(100);
  value = (rtc_time_get() - value);

  if(printRaw) {
    printf("%s: raw value is: %llu\r\n", __FUNCTION__, value);
  }

  return ((float)value*10.0 - M2_CALPOINT1_RAW) * (M2_CALPOINT2_CELSIUS - M2_CALPOINT1_CELSIUS) / (M2_CALPOINT2_RAW - M2_CALPOINT1_RAW) + M2_CALPOINT1_CELSIUS;
}

void setup() {
  Serial.begin(115200);
  Serial.print("===================================================\r\n");
  
  //configure RTC slow clock to internal oscillator, fast clock to XTAL divided by 4
  rtc_clk_slow_freq_set(RTC_SLOW_FREQ_RTC);
  rtc_clk_fast_freq_set(RTC_FAST_FREQ_XTALD4);
  
  //read CPU speed
  rtc_cpu_freq_config_t freq_config;
  rtc_clk_cpu_freq_get_config(&freq_config);
  //should be "0 -- 150000 -- 240", internal oscillator running at ~150kHz and CPU at 240 MHz
  printf("%d -- %d -- %d\r\n", (int)rtc_clk_slow_freq_get(), rtc_clk_slow_freq_get_hz(), freq_config.freq_mhz);
}

void loop() {
  // this method is not available in current ESP32 anymore:
  //float deprecatedMethod = (temprature_sens_read() - 32) / 1.8;
  
  //mapping is valid for this particular oscillator, yours likely deviate a bit
  printf("Results: ~ %.1f °C -- ~ %.1f °C\r\n", readTemp1(true), readTemp2(true));

  delay(1000);
}
