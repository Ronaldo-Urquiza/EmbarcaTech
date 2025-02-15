#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <stdio.h>

QueueHandle_t dataQueue;

float read_onboard_temperature()
{
    const float conversion_factor = 3.3f / (1 << 12); // Conversão de 12 bits para tensão
    adc_select_input(4);                              // Seleciona o canal do sensor de temperatura interno (canal 4)

    uint16_t raw_value = adc_read();               // Lê o valor cru do ADC
    float voltage = raw_value * conversion_factor; // Converte o valor cru para tensão

    // Fórmula para converter a tensão em temperatura (Celsius)
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

void vSensorTask(void *pvParameters)
{
    float sensorData = 0;

    for (;;)
    {
        sensorData = read_onboard_temperature();
        xQueueSend(dataQueue, &sensorData, portMAX_DELAY); // Envia dados para a fila
        vTaskDelay(1000 / portTICK_PERIOD_MS);             // Simula coleta a cada 1s
    }
}

void vProcessingTask(void *pvParameters)
{
    float receivedData;
    static uint cnt = 0;
    static float avg = 0;

    for (;;)
    {
        if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY) == pdTRUE)
        {
            cnt++;
            avg += receivedData;
            if (cnt == 10)
            {
                cnt = 0;
                avg /= 10;
                printf("Media: %f\n", avg); // Processa o dado
                avg = 0;
            }
        }
    }
}

void setup(void)
{
    stdio_init_all();
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

int main()
{
    setup();

    dataQueue = xQueueCreate(5, sizeof(float)); // Fila com capacidade para 5 itens
    if (dataQueue != NULL)
    {
        xTaskCreate(vSensorTask, "Sensor Task", 128, NULL, 1, NULL);
        // Tarefas que usam printf precisam ter pilha maior, evite usar printf
        // melhor criar uma versão bufferizada com uma Queue :)
        xTaskCreate(vProcessingTask, "Processing Task", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1)
        ;
    return 0;
}
