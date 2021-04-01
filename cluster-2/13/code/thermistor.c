//skill 13
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "driver/i2c.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling

//initializing attenuation variables
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_atten_t atten6 = ADC_ATTEN_DB_6;
static const adc_unit_t unit = ADC_UNIT_1;

//thermistor monitor adc
static const adc_channel_t channel2 = ADC_CHANNEL_4; //GPIO32

static uint32_t thermistor()
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
        if (unit == ADC_UNIT_1)
        {
            adc_reading += adc1_get_raw((adc1_channel_t)channel2);
        }
        else
        {
            int raw;
            adc2_get_raw((adc2_channel_t)channel2, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw; //temperature outputted in raw
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    //calculate resistance across thermistor using voltage divider formula
    double resistance = (33000.0 / ((double)voltage / 1000)) - 10000.0;
    //convert resistance across thermistor to Kelvin
    double tKelvin = -(1 / ((log(10000.0 / resistance) / 3435.0) - (1 / 298.0)));
    //convert Kelvin to Celsius
    double tCelsius = (tKelvin - 273.15);
    return tCelsius;
}

//display sensor values
static void display_console()
{
    uint32_t temp;
    //continuously print sensor readings to the serial port
    while (1)
    {
        temp = thermistor();
        printf("Temperature is: %d degrees celsius\n", temp);
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2 seconds
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Default Vref\n");
    }
}

void app_main(void)
{

    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel2, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel2, atten6);
    }
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    xTaskCreate(display_console, "display_console", 4096, NULL, 5, NULL);
}
