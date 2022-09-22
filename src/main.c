#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "esp32/rom/uart.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"

#define BUTTON 0

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;

void inicia_nvs()
{
  // Inicializa o NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void controle_temp()
{
  DHT11_init();
  // xTaskCreate(&envia_media_dht11, "Envia Media DHT11", 2098, NULL, 1, NULL);
  xTaskCreate(&envia_dados_sensor_dht11, "Envia DHT11", 2098, NULL, 1, NULL);
}

void modo_low_power()
{
  gpio_pad_select_gpio(BUTTON);
  gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
  gpio_wakeup_enable(BUTTON, GPIO_INTR_LOW_LEVEL);

  esp_sleep_enable_gpio_wakeup();

  while (true)
  {
    mqtt_client_stop();
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
    esp_light_sleep_start();

    if (rtc_gpio_get_level(BUTTON) == 0)
    {
      wifi_reconnect();
      if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
      {
        mqtt_client_restart();
      }

      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
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
    while (true)
    {
      controle_temp();
    }
#else
    ESP_LOGI("MODO", "Modo Bateria");
    xTaskCreate(&modo_low_power, "Modo Low Power", 4096, NULL, 1, NULL);
#endif
  }
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
