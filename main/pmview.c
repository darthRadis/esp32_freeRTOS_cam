#include "pmview.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <sys/param.h>
#include "esp_system.h"
#include "esp_event.h"
//#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "errno.h"
#include "pmstruct.h"
#include "analisys.h"

#include <sys/stat.h>

static const char *TAG = "pmview statio";
#define configUSE_TRACE_FACILITY        1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

esp_err_t pmview_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;
    ESP_LOGI(TAG, "inicio Relatório impresso");
    xSemaphoreTake( semAnalisys,portMAX_DELAY);
    int buf_len = strlen(analisys_resposta);
    httpd_resp_send(req, analisys_resposta, buf_len);
    xSemaphoreGive( semAnalisys);
    ESP_LOGI(TAG, "Relatório impresso");
    return res;
}

static const httpd_uri_t pmview_uri = {
    .uri       = "/pmview",
    .method    = HTTP_GET,
    .handler   = pmview_handler
};

void pmview_register_uri_handler(httpd_handle_t* server)
{
    httpd_register_uri_handler(*server, &pmview_uri);
}
