#include "ota.h"

esp_err_t ota_login_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;
    char login[]=""
  "<!DOCTYPE html>"
  "<html>"
    "<head>"
      "<meta charset=\"UTF-8\">"
    "</head>"
    "<body>"
      "<h3>Faça o login</h3>"
      "<form>"
      "<p>Usuario: <input></p>"
      "<p>Senha: <input></p>"
    "</body>"
  "</html>"
  ;
    httpd_resp_send(req, login, sizeof(login));
    return res;
}

static const httpd_uri_t ota_login_uri = {
    .uri       = "/ota",
    .method    = HTTP_GET,
    .handler   = ota_login_handler
};

void ota_register_uri_handler(httpd_handle_t* server)
{
    httpd_register_uri_handler(*server, &ota_login_uri);
}
