#include "ota.h"
#include <string.h>
#include <sys/param.h>
#include "esp_system.h"
#include "esp_event.h"
//#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "errno.h"

#include <sys/stat.h>

#define BUFFSIZE 16384
#define HASH_LEN 32 /* SHA-256 digest length */

/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };

static const char *TAG = "ota statio";

esp_err_t ota_login_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;
    char ret[]=""
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
    httpd_resp_send(req, ret, sizeof(ret));
    return res;
}

esp_err_t ota_file_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;
    char ret[]=""
  "<!DOCTYPE html>\n"
  "<html>\n"
    "<head>\n"
      "<meta charset=\"UTF-8\">\n"
    "</head>\n"
    "<body>\n"
      "<script type=\"text/javascript\">\n"
// Achtung dieses Script wird mit /fileserver aufgerufen, daher muss hier /html/gethost.js verwendet werden!
      "</script>\n"
      "<script language=\"JavaScript\">\n"
        "function upload() {\n"
          "var upload_path = \"/ota/grava\";\n"
          "var fileInput = document.getElementById(\"newfile\").files;\n"
	  "if (fileInput.length == 0) {\n"
	    "alert(\"No file selected!\");\n"
	  "} else {\n"
	    "document.getElementById(\"newfile\").disabled = true;\n"
	    "document.getElementById(\"upload\").disabled = true;\n"

	    "var file = fileInput[0];\n"
	    "var xhttp = new XMLHttpRequest();\n"
	    "xhttp.onreadystatechange = function() {\n"
	      "if (xhttp.readyState == 4) {\n"
	        "if (xhttp.status == 200) {\n"
	          "document.open();\n"
		  "document.write(xhttp.responseText);\n"
		  "document.close();\n"
	        "} else if (xhttp.status == 0) {\n"
	          "alert(\"Server closed the connection abruptly!\");\n"
		  "UpdatePage(false);\n"
	        "} else {\n"
	          "alert(xhttp.status + \" Error!\\n\" + xhttp.responseText);\n"
                  "UpdatePage(false);\n"
                "}\n"
              "}\n"
            "};\n"
            "xhttp.open(\"POST\", upload_path, true);\n"
            "xhttp.send(file);\n"
          "}\n"
        "}\n"
      "</script>\n"

      "<h2>ESP32 OTA Firmware Upgrade</h2>\n"
      "<p><label for=\"newfile\">Select Firmware</label></p>\n"
      "<p><input id=\"newfile\" type=\"file\" style=\"width:100%;\"></p>\n"
      "<p><button id=\"upload\" type=\"button\" onclick=\"upload()\">Upload and Reboot</button></p>\n"

    "</body>\n"
  "</html>\n"
  ;
    httpd_resp_send(req, ret, sizeof(ret));
    return res;
}

esp_err_t ota_grava_handler(httpd_req_t* req) {
	    int remaining = req->content_len;
    ESP_LOGE(TAG, "File size : %d bytes", remaining);
    char *buf = (char*) malloc(BUFFSIZE);

    if (!buf) {
        ESP_LOGE(TAG, "Could not allocate memory!");
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "Could not allocate buffer memory!");
        return ESP_FAIL;
    }

    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA example");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);


    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    int binary_file_length = 0;

    // deal with all receive packet
    bool image_header_was_checked = false;
    int data_read;


    ESP_LOGI(TAG, "Receiving file : ...");

    int received = 0;

    while (remaining > 0) {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */

        if ((data_read = httpd_req_recv(req, ota_write_data, MIN(remaining, BUFFSIZE))) <= 0) {
            if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        if (image_header_was_checked == false) {
            esp_app_desc_t new_app_info;
            if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                // check current version with downloading
                memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                esp_app_desc_t running_app_info;
                if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                }

                const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                esp_app_desc_t invalid_app_info;
                if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                }

                // check current version with last invalid partition
                if (last_invalid_app != NULL) {
                    if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "New version is the same as invalid version.");
                        ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                        ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                        return 0;
                    }
                }

                image_header_was_checked = true;

                err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    return false;
                }
                ESP_LOGI(TAG, "esp_ota_begin succeeded");
            } else {
                ESP_LOGE(TAG, "received package is not fit len");
                return false;
            }
        }
        err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
        if (err != ESP_OK) {
            return false;
        }
        binary_file_length += data_read;
        ESP_LOGD(TAG, "Written image length %d", binary_file_length);

//////////////////////////////

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= data_read;
    }



    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));

    }

    ESP_LOGI(TAG, "File reception complete");

    httpd_resp_sendstr(req, "File uploaded successfully - reboot in 5s!");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_restart();
    return ESP_OK;
/*    esp_err_t res V= ESP_OK;
    char ret[]=""
  "<!DOCTYPE html>"
  "<html>"
    "<head>"
      "<meta charset=\"UTF-8\">"
    "</head>"
    "<body>"
      "<h3>Vai Gravar</h3>"
    "</body>"
  "</html>"
  ;
  ESP_LOGI(TAG, "Tamanho do corpo da requisição %d", req->content_len);
    httpd_resp_send(req, ret, sizeof(ret));
    return res;*/
}

static const httpd_uri_t ota_login_uri = {
    .uri       = "/ota/login",
    .method    = HTTP_GET,
    .handler   = ota_login_handler
};

static const httpd_uri_t ota_file_uri = {
    .uri       = "/ota/file",
    .method    = HTTP_GET,
    .handler   = ota_file_handler
};

static const httpd_uri_t ota_grava_uri = {
    .uri       = "/ota/grava",
    .method    = HTTP_POST,
    .handler   = ota_grava_handler,
    .user_ctx  = (void*) "Upload"
};

void ota_register_uri_handler(httpd_handle_t* server)
{
    httpd_register_uri_handler(*server, &ota_login_uri);
    httpd_register_uri_handler(*server, &ota_file_uri);
    httpd_register_uri_handler(*server, &ota_grava_uri);
}
