#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <esphome.h>

//Comment this line if you want to have your private
//config is a separete file (that have to be manually created) 
#define USE_EXTERNAL_DEFS

#ifdef USE_EXTERNAL_DEFS
  #include "personal_defs.h"
#else
  #define WIFI_PASS   "your_wifipass"
  #define WIFI_SSID   "your_wifissid"
  #define HA_USERNAME "your_ha_user"
  #define HA_PASS     "your_ha_pass"
  #define HA_IP       "your_ha_ip"
#endif


/************************    CAPACITIVE BUTTONS   **************************/
/*10k - 1MOhm resistor between pins are required*/
#define CAP_BT_1 4
#define CAP_BT_2 5
#define CAP_CLK  2

CapacitiveSensor cap_sensor_1 =  CapacitiveSensor(CAP_CLK,CAP_BT_1);
CapacitiveSensor cap_sensor_2 =  CapacitiveSensor(CAP_CLK,CAP_BT_2);

//------------------------ PWM -----------------------------
const unsigned int led_pin = 14;
//unsigned int pwm_index = 0;
//unsigned int pwm_n_values = 100;
//-------------------------------------------------------------

using namespace esphome;

Application::MakeLight abajour;

float brightness = 0;

void CapSense()
{
  //Using 20k ohms between the writer and reader pins.
  //For this resistance, the threshhold seems to be around 10, with max 60
  const long th = 180;

  static float brightness_prev = 0;
  const float step = 0.01;

  static uint16_t counter = 0;

  bool bt1 = false;
  bool bt2 = false;

  long v1 = cap_sensor_1.capacitiveSensor(30);
  long v2 = cap_sensor_2.capacitiveSensor(30);
  
  if (v1 >= th) bt1 = true;
  if (v2 >= th) bt2 = true;
  
  if (bt1){ 
    if (brightness < 1 - step){
      brightness += step;
    }
  }

  if (bt2){
    if (brightness >= step){
      brightness -= step;
    }
  }

  if (bt1 || bt2){
    abajour.state->set_immediately(light::LightColorValues::from_monochromatic(brightness));
    printf("%f\n\r",brightness);
  }

  if (counter++ == 50){
    if (brightness != brightness_prev){
      abajour.state->send_values();
      brightness_prev = brightness;
      printf("Updated remote.\n\r");
    }
    counter = 0;
  }
}

void setup()
{
  // Set console baud rate
  Serial.begin(115200);

  App.set_name("WiLight");
  App.init_log();

  App.init_wifi(WIFI_SSID, WIFI_PASS);
  auto *mqtt = App.init_mqtt(HA_IP, HA_USERNAME, HA_PASS);
  mqtt->set_topic_prefix("home/light/abajour");
  
  App.init_ota()->start_safe_mode();
  abajour = App.make_monochromatic_light("Abajour", App.make_esp8266_pwm_output(led_pin));

  App.setup();
}

void loop()
{
  App.loop();
  CapSense();
}