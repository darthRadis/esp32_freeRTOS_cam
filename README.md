# esp32-freeRTOS-cam

- wifi access point
- ota service
- jpeg/bmp capture, jpeg stream from camera
- onboard light as hacky light source
- based on esp-idf/fully modifiable

Need a file to config wifi (../../local.h)access with:
#define EXAMPLE_ESP_WIFI_SSID      "reserva"//"IntLab"//"VADER"//CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      "emergencia"//"labcoproj"//CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  4//CONFIG_ESP_MAXIMUM_RETRY

Baseado no seguinte github:
https://github.com/cubimon/esp32-cam-ota
