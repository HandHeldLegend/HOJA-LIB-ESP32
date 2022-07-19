#ifndef MAIN_H
#define MAIN_H

#include "retroblue_api.h"
#include "driver/adc.h"

#include "esp_system.h"
#include "sdkconfig.h"


// tmp wifi dev
#include "esp_mac.h"
#include "esp_wifi.h"
#define WIFI_SSID           "RetroBlue_Config"
#define WIFI_PASS           ""
#define WIFI_CHANNEL        6
#define WIFI_MAX_STA_CONN  2

// tmp server dev
#include "esp_http_server.h"
#include "esp_netif.h"

#define WEBPAGE "<!DOCTYPE html>\
<html>\
<head>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
<style>\
.switch {\
  position: relative;\
  display: inline-block;\
  width: 60px;\
  height: 34px;\
}\
.switch input { \
  opacity: 0;\
  width: 0;\
  height: 0;\
}\
.slider {\
  position: absolute;\
  cursor: pointer;\
  top: 0;\
  left: 0;\
  right: 0;\
  bottom: 0;\
  background-color: #ccc;\
  -webkit-transition: .4s;\
  transition: .4s;\
}\
.slider:before {\
  position: absolute;\
  content: \"\";\
  height: 26px;\
  width: 26px;\
  left: 4px;\
  bottom: 4px;\
  background-color: white;\
  -webkit-transition: .4s;\
  transition: .4s;\
}\
input:checked + .slider {\
  background-color: #2196F3;\
}\
input:focus + .slider {\
  box-shadow: 0 0 1px #2196F3;\
}\
input:checked + .slider:before {\
  -webkit-transform: translateX(26px);\
  -ms-transform: translateX(26px);\
  transform: translateX(26px);\
}\
.slider.round {\
  border-radius: 34px;\
}\
.slider.round:before {\
  border-radius: 50%;\
}\
</style>\
</head>\
<body>\
<h2>Toggle Switch</h2>\
<label class=\"switch\">\
  <input type=\"checkbox\">\
  <span class=\"slider\"></span>\
</label>\
<label class=\"switch\">\
  <input type=\"checkbox\" checked>\
  <span class=\"slider\"></span>\
</label><br><br>\
<label class=\"switch\">\
  <input type=\"checkbox\">\
  <span class=\"slider round\"></span>\
</label>\
<label class=\"switch\">\
  <input type=\"checkbox\" checked>\
  <span class=\"slider round\"></span>\
</label>\
</body>\
</html>"

#endif
