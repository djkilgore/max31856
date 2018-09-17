/* Maxim 31856 Thermocouple IC Example

   This example code is under the MIT License.

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "max31856.h"

max31856_cfg_t max31856;

void thermocouple_task(void *pvParameter) {
    while(1) {

        // You can return data directly
        thermocouple_read_fault(&max31856);
        float cj_temp = thermocouple_read_coldjunction(&max31856);
        float tc_temp = thermocouple_read_temperature(&max31856);
        printf("Cold Junction Temp (C): %.2f\n", cj_temp);
        printf("Thermocouple Temp (C): %.2f\n", tc_temp);

        // Or pull the most recent data from the struct when you need it
        printf("Cold Junction Temp (F): %.2f\n", max31856.coldjunction_f);
        printf("Thermocouple Temp (F): %.2f\n",  max31856.thermocouple_f);
        printf("Fault Register: %02X\n",  max31856.fault);

        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
}

void app_main() {

    // Init SPI Device
    max31856 = max31856_init();

    // Set and Check Thermocouple Type
    thermocouple_set_type(&max31856, MAX31856_TCTYPE_K);
    thermocouple_get_type(&max31856);

    // Set Temperature Fault Limits
    //thermocouple_set_temperature_fault(&max31856, 0.0, 1300.0);

    // Start Task to Get Temperature and Check Faults
    xTaskCreate(&thermocouple_task, "thermocouple_task", 2048, NULL, 5, NULL);
    
}
