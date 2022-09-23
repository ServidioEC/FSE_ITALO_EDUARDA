#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "mqtt.h"
#include "nvs_handler.h"
#include "dht11.h"
#include "pwm.h"
#include "button.h"
#include "led.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;

void modo_enegia()
{
  config_pwm();
  xTaskCreate(&DHT11_routine, "Rotina do DHT11", 4096, NULL, 1, NULL);
}

void modo_low_power()
{
  xTaskCreate(&led_routine, "Rotina do led", 4096, NULL, 1, NULL);
  // xTaskCreate(&button_routine, "Rotina do botão", 4096, NULL, 1, NULL);
}

void conectadoWifi(void *params)
{
  while (true)
  {
    if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      // Processamento Internet
      mqtt_start();
    }
  }
}

void trataComunicacaoComServidor(void *params)
{
  if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {
#ifdef CONFIG_ENERGY_MODE
    ESP_LOGI("MODO", "Modo Energia");
    modo_enegia();
#else
    ESP_LOGI("MODO", "Modo Bateria");
    modo_low_power();
#endif
  }
  vTaskDelete(NULL);
}

void app_main(void)
{
  inicia_nvs();

  conexaoWifiSemaphore = xSemaphoreCreateBinary();
  conexaoMQTTSemaphore = xSemaphoreCreateBinary();

  wifi_start();

  xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
  xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
}
