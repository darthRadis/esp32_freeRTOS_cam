#ifndef OTA_H
#define OTA_H

#include "esp_http_server.h"

void ota_register_uri_handler(httpd_handle_t*);

#endif
