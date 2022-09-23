#ifndef NVS_HANDLER_H
#define NVS_HANDLER_H

#include "esp_system.h"

void inicia_nvs();
int32_t le_valor_nvs(char *chave);
void grava_valor_nvs(char *chave, int32_t valor);

#endif