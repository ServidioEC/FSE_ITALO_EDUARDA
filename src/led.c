#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "led.h"

#define LED 2

void led_routine()
{
  gpio_pad_select_gpio(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);

  int estado = 0;

  while (true)
  {
    gpio_set_level(LED, estado);
    estado = !estado;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}