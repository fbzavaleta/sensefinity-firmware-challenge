#ifndef APP_MAIN_H
#define APP_MAIN_H
#include "nrf_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "queue.h"

#define MS_5HZ 200 //5Hz = 200ms
#define MS_1HZ 1000 //1Hz = 1000ms
#define TIMEOUT_INACTIVITY_SEC 10

#define TEMP_HIGH_THRESHOLD 3.0
#define TEMP_LOW_THRESHOLD -3.0


//callback functions
void configure_device();
void gpioHandler(void* arg);
void blinkTimerCallback(TimerHandle_t xTimer);
void HysteresisTimerCallback(TimerHandle_t xTimer);

//tasks
void task_button(void *pvParameter);
void task_temperature_status();
void task_set_profile();


extern TaskHandle_t xTaskLedHandle;
extern TaskHandle_t xTaskhysteresisHandle;

extern SemaphoreHandle_t xSemaphore_button;
extern SemaphoreHandle_t xSemaphore_timeout;
extern SemaphoreHandle_t xSemaphore_hysteresis;

extern TimerHandle_t blinkTimer;
extern TimerHandle_t HysteresisTimer;

extern QueueHandle_t XQueueSelectedProfile;
extern QueueHandle_t XQueueSetProfile;

#endif