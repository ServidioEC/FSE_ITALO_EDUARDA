#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;

void controle_temp()
{
  DHT11_init();
  xTaskCreate(&envia_dados_sensor_dht11, "Envia DHT11", 2098, NULL, 1, NULL);
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
    while (true)
    {
      controle_temp();
    }
  }
}

void app_main(void)
{
  // Inicializa o NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  conexaoWifiSemaphore = xSemaphoreCreateBinary();
  conexaoMQTTSemaphore = xSemaphoreCreateBinary();
  wifi_start();

  xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
  xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
}
