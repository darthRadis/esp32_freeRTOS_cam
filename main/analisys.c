#include "analisys.h"
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
#include "analisys.h"

#include <sys/stat.h>

static const char *TAG = "analisys statio";
#define configUSE_TRACE_FACILITY        1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

// Dimensions the buffer that the task being created will use as its stack.
// // NOTE: This is the number of bytes the stack will hold, not the number of
// // words as found in vanilla FreeRTOS.
#define STACK_SIZE 4096
//
// // Structure that will hold the TCB of the task being created.
StaticTask_t xTaskBuffer;
//
// // Buffer that the task being created will use as its stack. Note this is
// // an array of StackType_t variables. The size of StackType_t is dependent on
// // the RTOS port.
StackType_t xStack[ STACK_SIZE ];

void analisys_run(void* base){
  while(1){
    vTaskList(tracer);
    xSemaphoreTake( semAnalisys,portMAX_DELAY);
    int tam=0;
    sprintf(analisys_resposta,"<!DOCTYPE html>\n<html>\n <head>\n  <meta charset=\"UTF-8\">\n </head>\n <body>\n");
    tam = strlen(analisys_resposta);
    sprintf(analisys_resposta+tam,"<table>"); 
    tam = strlen(analisys_resposta);
    sprintf(analisys_resposta+tam,"<tr><th>Name</th><th>State</th><th>Priority</th><th>RunTime</th><th>Handler</th>\n"); 
    tam = strlen(analisys_resposta);
    int i=0;
    while(tracer[i]!='\0')
    {
      if(i)i++;
      sprintf(analisys_resposta+tam,"<tr><td>"); 
      tam = strlen(analisys_resposta);
      while(tracer[i]!='\n' && tracer[i]!='\0'){
        if(tracer[i]=='\t'){
          sprintf(analisys_resposta+tam,"</td><td>\n"); 
          tam = strlen(analisys_resposta);
          i++;
        }
        else
          analisys_resposta[tam++]=tracer[i++];
      }
      sprintf(analisys_resposta+tam,"</td></tr>"); 
      tam = strlen(analisys_resposta);
    }
    sprintf(analisys_resposta+tam,"</table></body></html>"); 
    xSemaphoreGive( semAnalisys);
    vTaskDelay(1);
  }
}

void init_analisys()
{
  TaskHandle_t analisysHandler=NULL;
  semAnalisys = xSemaphoreCreateMutex();
  analisysHandler= xTaskCreateStaticPinnedToCore(analisys_run,"Analisys",STACK_SIZE,(void *)1,25,xStack,&xTaskBuffer,1);
}
