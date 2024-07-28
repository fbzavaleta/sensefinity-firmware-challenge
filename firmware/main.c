#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_main.h"

static const char *TAG = "MAIN: ";


void app_main(void)
{
    configure_device();

    xSemaphore_button = xSemaphoreCreateBinary();
    xSemaphore_timeout = xSemaphoreCreateBinary();
    xSemaphore_hysteresis = xSemaphoreCreateBinary();

    blinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(MS_1HZ), pdTRUE, (void *)0, blinkTimerCallback);
    xTimerStart(blinkTimer, 0);

    HysteresisTimer = xTimerCreate("HysteresisTimer", pdMS_TO_TICKS(MS_1HZ), pdTRUE, (void *)0, HysteresisTimerCallback);
    xTimerStart(HysteresisTimer, 0);

	if( (XQueueSelectedProfile = xQueueCreate( 7, sizeof(int)) ) == NULL )
	{
		NRF_LOG_INFO( TAG, "error - nao foi possivel alocar XQueueSelectedProfile.\n" );
		return;
	}

	if( (XQueueSetProfile = xQueueCreate( 7, sizeof(int)) ) == NULL )
	{
		NRF_LOG_INFO( TAG, "error - nao foi possivel alocar XQueueSetProfile.\n" );
		return;
	}         

    if ((xTaskCreate(task_button, "task_button", 2048, NULL, 5, &xTaskLedHandle)) != pdTRUE)
    {
        NRF_LOG_INFO(TAG, "error - nao foi possivel alocar task_button.\n");
        return;
    }

    if ((xTaskCreate(task_set_profile, "task_set_profile", 2048, NULL, 5, NULL)) != pdTRUE)
    {
        NRF_LOG_INFO(TAG, "error - nao foi possivel alocar task_set_profile.\n");
        return;
    }

    if ((xTaskCreate(task_temperature_status, "task_temperature_status", 2048, NULL, 5, &xTaskhysteresisHandle)) != pdTRUE)
    {
        NRF_LOG_INFO(TAG, "error - nao foi possivel alocar task_temperature_status.\n");
        return;
    }    
}


int main(void)
{
    //init logging
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    // Call the freertos app
    app_main();

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (true)
    {
        // Enter sleep mode
        __WFE();
    }
}