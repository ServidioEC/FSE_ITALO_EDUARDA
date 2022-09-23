#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"

#include "button.h"

#define BUTTON 0

extern xSemaphoreHandle conexaoWifiSemaphore;

void button_routine()
{
	gpio_pad_select_gpio(BUTTON);
	gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
	gpio_wakeup_enable(BUTTON, GPIO_INTR_LOW_LEVEL);

	esp_sleep_enable_gpio_wakeup();

	int vezes_apertado = 0;

	while (true)
	{
		mqtt_client_stop();
		wifi_stop();

		esp_light_sleep_start();

		if (rtc_gpio_get_level(BUTTON) == 0)
		{
			vezes_apertado++;
			wifi_reconnect();

			if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
			{
				mqtt_client_restart();
				printf("Botao apertado %d vezes\n", vezes_apertado);
			}
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}