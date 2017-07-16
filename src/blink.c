#include "blink.h"

void vBlinkTask(void *vpars){
  volatile int i, c=0, x=0, d=1;
  // Enable System LED
  RCC_APB2PeriphClockCmd(SYS_LED_RCC, ENABLE);
  GPIO_InitTypeDef sGPIOinit;
  sGPIOinit.GPIO_Mode = GPIO_Mode_Out_PP;
  sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
  sGPIOinit.GPIO_Pin = SYS_LED_PIN;
  GPIO_Init(SYS_LED_GPIO, &sGPIOinit);
  GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);


  while (1){
    if (GPIO_ReadOutputDataBit(SYS_LED_GPIO, SYS_LED_PIN)) GPIO_ResetBits(SYS_LED_GPIO, SYS_LED_PIN);
    else GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
    vTaskDelay(200);
  }
}

