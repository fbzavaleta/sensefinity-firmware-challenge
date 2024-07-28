#include "app_main.h"
#include "./src/config.h"
#include "./src/lcd.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "app_error.h"
#include "nrfx_spim.h"
#include "app_util_platform.h"

TaskHandle_t xTaskLedHandle = NULL;
TaskHandle_t xTaskhysteresisHandle = NULL;

SemaphoreHandle_t xSemaphore_button = NULL;
SemaphoreHandle_t xSemaphore_timeout = NULL;
SemaphoreHandle_t xSemaphore_hysteresis = NULL;

QueueHandle_t XQueueSelectedProfile;
QueueHandle_t XQueueSetProfile;


TimerHandle_t blinkTimer;
TimerHandle_t HysteresisTimer;

// Global variables
static int ms_timer_count = 0;
static bool is_button_pressed = false;

static int sec_timer_count = 0;
static int stationary_temp_timeout = 0;
static bool is_stationary = false;

static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);

//timers asyncronous
void blinkTimerCallback(TimerHandle_t xTimer)
{

    if (is_button_pressed) 
    {
        ms_timer_count ++;
        if (ms_timer_count >= TIMEOUT_INACTIVITY_SEC)
        {
            xSemaphoreGive(xSemaphore_timeout);
            ms_timer_count = 0;
            is_button_pressed = false;
        }
    }
}

void HysteresisTimerCallback(TimerHandle_t xTimer)
{
    if (is_stationary) 
    {
        sec_timer_count ++;
        if (sec_timer_count >= stationary_temp_timeout)
        {
            xTaskNotifyGive(xTaskhysteresisHandle);
            sec_timer_count = 0;
            is_stationary = false;
        }
    }
}

//handler and functions
void gpio_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) 
{
    static TickType_t last_interrupt_time = 0;
    TickType_t interrupt_time = xTaskGetTickCountFromISR();

    // Debounce mechanism
    if ((interrupt_time - last_interrupt_time) > pdMS_TO_TICKS(TIMEOUT_INTERRUPT)) 
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Notify the LED task
        vTaskNotifyGiveFromISR(xTaskLedHandle, &xHigherPriorityTaskWoken);

        // Force a context switch if needed
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    last_interrupt_time = interrupt_time;
}


void alert_interrupt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin == ALERT_PIN)
    {
        // Handle the interrupt by reading the 0x01 register
        uint8_t reg_address = 0x01;
        uint8_t reg_value[2] = {0}; //** pointer to store the value read, allocate for 2 bytes

        nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(&reg_address, 1, reg_value, 2);
        ret_code_t err_code = nrfx_spim_xfer(&spi, &xfer_desc, 0);
        APP_ERROR_CHECK(err_code);

        // send semaphore to the hysteresis task
        xSemaphoreGive(xSemaphore_hysteresis);
    }
}

float sensor_read()
{
    //we have two registers to read the temperature
    uint8_t lsb_reg_address = 0x01; // LSB temperature register
    uint8_t msb_reg_address = 0x02; // MSB temperature register **
    uint8_t lsb_value = 0;
    uint8_t msb_value = 0;

    nrf_gpio_pin_set(SPI_CS_PIN); // Set the CS pin high **

    // Read the LSB register
    nrfx_spim_xfer_desc_t lsb_xfer_desc = NRFX_SPIM_XFER_TRX(&lsb_reg_address, 1, &lsb_value, 1);
    ret_code_t err_code = nrfx_spim_xfer(&spi, &lsb_xfer_desc, 0);
    APP_ERROR_CHECK(err_code);

    // Read the MSB register
    nrfx_spim_xfer_desc_t msb_xfer_desc = NRFX_SPIM_XFER_TRX(&msb_reg_address, 1, &msb_value, 1);
    err_code = nrfx_spim_xfer(&spi, &msb_xfer_desc, 0);
    APP_ERROR_CHECK(err_code);

    nrf_gpio_pin_clear(SPI_CS_PIN); // Clear the CS pin **

    //obtainign the temperature value in 16 bits    
    int16_t temperature_raw = (msb_value << 8) | lsb_value;

    return temperature_raw / 10.0;        
}

void sensor_init()
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.sck_pin  = SPI_SCK_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.ss_pin   = NRFX_SPIM_PIN_NOT_USED; //control manually **
    spi_config.frequency = NRF_SPIM_FREQ_1M;
    spi_config.mode = NRF_SPIM_MODE_1; //**CPOL: 0 CPHA: 1 sampling in the failing edge
    spi_config.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST;

    ret_code_t err_code_spi = nrfx_spim_init(&spi, &spi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code_spi);

    nrf_gpio_cfg_output(SPI_CS_PIN); // Configure the CS pin as output **
    nrf_gpio_pin_clear(SPI_CS_PIN); // Ensure CS starts low  **

}
void configure_device()
{
    init_lcd();
    sensor_init();

    //Button interrupt
    ret_code_t err_code;

    if (!nrfx_gpiote_is_init())
    {
        err_code = nrfx_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    //Configuration of the button as input with pull-up resistor and failling edge;
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrfx_gpiote_in_init(BUTTON_GPIO, &in_config, gpio_handler);
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_event_enable(BUTTON_GPIO, true);

    //configure the sensor interrupt
    nrfx_gpiote_in_config_t alert_in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    alert_in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrfx_gpiote_in_init(ALERT_PIN, &alert_in_config, alert_interrupt_handler);
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_event_enable(ALERT_PIN, true);    
}

void blink_lcd_before_status()
{
    for(;;)
    {
        if (xSemaphoreTake(xSemaphore_button, 0) == pdTRUE)
        {
            vTaskDelete(NULL);
        }
        blink_lcd(0, MS_5HZ, '\0');
    }
}

//tasks to interact with the system
void task_temperature_status()
{
    int profile_selected;
    float target_temperature = 0.0;

    for (;;)
    {
        if (xQueueReceive(XQueueSelectedProfile, &profile_selected, 0) == pdTRUE)
        {
            target_temperature = profiles[profile_selected].targetTemp;
            stationary_temp_timeout = profiles[profile_selected].settleTime;
        }


        if (xSemaphoreTake(xSemaphore_hysteresis, 0) == pdTRUE)
        {
            is_stationary = true;
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            float stable_temperature = sensor_read();
            float diff = target_temperature - stable_temperature;

            if (diff > TEMP_HIGH_THRESHOLD)
            {
                lcd_write_letter('H');
            }
            else if (diff < TEMP_LOW_THRESHOLD)
            {
                lcd_write_letter('L');
            }
            else
            {
                lcd_write_number(profile_selected);
            }            
        }           

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task_set_profile()
{
    int profile_selected;
    bool blink_control = false;
    for (;;)
    {
        if (xQueueReceive(XQueueSetProfile, &profile_selected, 0) == pdTRUE)
        {
            blink_control = true;                              
        }
        if (blink_control)
        {
            blink_lcd(profile_selected, MS_1HZ, '\0');
        }
        
        if (xSemaphoreTake(xSemaphore_timeout, 0) == pdTRUE)
        {
            xQueueSend(XQueueSelectedProfile, &profile_selected, 0);
            lcd_write_number(profile_selected);
            blink_control = false;
        }          
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task_button(void *pvParameter)
{
    int profile = 0;
    xTaskCreate( blink_lcd_before_status, "blink_lcd_before_status", 10000, NULL, 5, NULL );

    for (;;)
    {   
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        is_button_pressed = true;    
        ms_timer_count = 0;
        profile++;
        
        if (profile > MAX_PROFILES) {
            profile = 1;
        }
        xQueueSend(XQueueSetProfile, &profile, 0);

        if (xSemaphore_button != NULL)
        {
            xSemaphoreGive(xSemaphore_button);
        }        

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}