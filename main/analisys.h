#ifndef ANALISYS_H
#define ANALISYS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"
#include <freertos/semphr.h>

char analisys_resposta[4096];
char tracer[1024];

SemaphoreHandle_t semAnalisys;

void init_analisys();

#endif
