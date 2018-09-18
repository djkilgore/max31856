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

QueueHandle_t thermocouple_queue = NULL;

void receiver_task(void *pvParameter) {
    ESP_LOGI("task", "Created: thermocouple_task");
    QueueHandle_t *thermocouple_queue = pvParameter;
	
	max31856_cfg_t max31856;
    if(thermocouple_queue == NULL){
	    ESP_LOGI("queue", "Not Ready: thermocouple_queue");
        return;
    }
    while(1){
        xQueueReceive(thermocouple_queue,&max31856,(TickType_t )(200/portTICK_PERIOD_MS)); 
        printf("Struct Received on Queue:\nCold Junction: %0.2f\nTemperature: %0.2f\n", max31856.coldjunction_f, max31856.thermocouple_f);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}

void thermocouple_task(void *pvParameter) {
    ESP_LOGI("task", "Created: receiver_task");
    QueueHandle_t *thermocouple_queue = pvParameter;

    max31856_cfg_t max31856 = max31856_init();
    thermocouple_set_type(&max31856, MAX31856_TCTYPE_K);

    if(thermocouple_queue == NULL){
	    ESP_LOGI("queue", "Not Ready: thermocouple_queue");
        return;
    }
    while(1){
        thermocouple_read_fault(&max31856, true);
        thermocouple_read_coldjunction(&max31856);
        thermocouple_read_temperature(&max31856);
        xQueueSend(thermocouple_queue,&max31856,(TickType_t )0);
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

void app_main() {

    // Queue: Thermocouple Data -> Receiver
    thermocouple_queue = xQueueCreate(5, sizeof(struct max31856_cfg_t));
    if(thermocouple_queue != NULL){
	    ESP_LOGI("queue", "Created: thermocouple_queue");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        xTaskCreate(&thermocouple_task, "thermocouple_task", 2048, (void *)thermocouple_queue, 5, NULL);
        xTaskCreate(&receiver_task, "receiver_task", 2048, (void *)thermocouple_queue, 5, NULL);
    } else{
	    ESP_LOGI("queue", "Failed to Create: thermocouple_queue");
    }  
    
}
