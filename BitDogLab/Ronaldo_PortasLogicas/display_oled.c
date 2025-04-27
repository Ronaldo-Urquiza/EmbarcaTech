#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h" //joystick

#include "portas_logicas.h"

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B

#define LED_GREEN 11   // GPIO conectado ao terminal verde do LED RGB
#define LED_RED 13   // GPIO conectado ao terminal vermelho do LED RGB

int control = 0;
int menu = 0;

int and(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_and);

    int logicand = 0;

    if(button_a_state == true & button_b_state == true){
        logicand = 1;
    }
    else if(button_a_state == false & button_b_state == true){
        logicand = 0;
    }
    else if(button_a_state == true & button_b_state == false){
        logicand = 0;
    }else{
        logicand = 0;
    }

    control = logicand;
    return control;
};

int or(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_or);

    int logicor = 0;

    if(button_a_state == true & button_b_state == true){
        logicor = 1;
    }
    else if(button_a_state == false & button_b_state == true){
        logicor = 1;
    }
    else if(button_a_state == true & button_b_state == false){
        logicor = 1;
    }else{
        logicor = 0;
    }

    control = logicor;
    return control;
};

int not(ssd1306_t ssd_bm, bool button_a_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_not);

    int logicnot = 0;

    if(button_a_state == true){
        logicnot = 0;
    }
    else{
        logicnot = 1;
    }

    control = logicnot;
    return control;
};

int nand(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_nand);

    int logicnand = 0;

    if(button_a_state == true & button_b_state == true){
        logicnand = 0;
    }
    else if(button_a_state == false & button_b_state == true){
        logicnand = 1;
    }
    else if(button_a_state == true & button_b_state == false){
        logicnand = 1;
    }else{
        logicnand = 1;
    }

    control = logicnand;
    return control;
};

int nor(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_nor);

    int logicnor = 0;

    if(button_a_state == true & button_b_state == true){
        logicnor = 0;
    }
    else if(button_a_state == false & button_b_state == true){
        logicnor = 0;
    }
    else if(button_a_state == true & button_b_state == false){
        logicnor = 0;
    }else{
        logicnor = 1;
    }

    control = logicnor;
    return control;
};

int xor(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_xor);

    int logixor = 0;

    if(button_a_state == true & button_b_state == true){
        logixor = 0;
    }
    else if(button_a_state == false & button_b_state == true){
        logixor = 1;
    }
    else if(button_a_state == true & button_b_state == false){
        logixor = 1;
    }else{
        logixor = 0;
    }

    control = logixor;
    return control;
};

int xnor(ssd1306_t ssd_bm, bool button_a_state, bool button_b_state){

    ssd1306_draw_bitmap(&ssd_bm, bitmap_xnor);

    int logixnor = 0;

    if(button_a_state == true & button_b_state == true){
        logixnor = 1;
    }
    else if(button_a_state == false & button_b_state == true){
        logixnor = 0;
    }
    else if(button_a_state == true & button_b_state == false){
        logixnor = 0;
    }else{
        logixnor = 1;
    }

    control = logixnor;
    return control;
};


int main()
{
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário

    adc_init();// Incializa adc para o joystick

    // Configura os pinos GPIO 26 e 27 como entradas de ADC (alta impedância, sem resistores pull-up)
    adc_gpio_init(26);
    adc_gpio_init(27);

    // Configuração do GPIO do LED como saída
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, false);  // Inicialmente, o LED está apagado

    // Configuração do GPIO do LED como saída
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, false);  // Inicialmente, o LED está apagado


    // Configuração do GPIO do Botão A como entrada com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Configuração do GPIO do Botão B como entrada com pull-up interno
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    ssd1306_t ssd_bm;
    ssd1306_init_bm(&ssd_bm, 128, 64, false, 0x3C, i2c1);
    ssd1306_config(&ssd_bm);

    while(true) {

        // Seleciona a entrada ADC 0 (conectada ao eixo X do joystick)
        adc_select_input(1);
        // Lê o valor do ADC para o eixo X
        uint adc_x_raw = adc_read();
        
        // Seleciona a entrada ADC 1 (conectada ao eixo Y do joystick)
        adc_select_input(0);
        // Lê o valor do ADC para o eixo Y
        uint adc_y_raw = adc_read();

        const uint threshold_baixo = 1000;  // Ajuste para detectar "baixo" (valores baixos)
        const uint threshold_cima = 3000; // Ajuste para detectar "cima" (valores altos)

        // Verifica se o joystick foi para cima
        if (adc_y_raw > threshold_cima) {
            printf("Joystick foi para CIMA!\n");

            if(menu<=0){
                menu=6;
            }else{
                menu--;};
        }

        // Verifica se o joystick foi para baixo
        if (adc_y_raw < threshold_baixo) {
            printf("Joystick foi para BAIXO!\n");

            if(menu>6){
                menu = 0;
            }else{
                menu++;};
        }

        // Lê o estado do Botão A
        bool button_a_state = gpio_get(BUTTON_A);  // HIGH = solto, LOW = pressionado
        bool button_b_state = gpio_get(BUTTON_B);  // HIGH = solto, LOW = pressionado

        // Correção:
        button_a_state = button_a_state;
        button_b_state = button_b_state;

        switch (menu) {
            case 1:
                control = or(ssd_bm, button_a_state,button_b_state);
                break;

            case 2:
                control = not(ssd_bm, button_a_state);
                break;

            case 3:
                control = nand(ssd_bm, button_a_state,button_b_state);
                break;

            case 4:
                control = nor(ssd_bm, button_a_state,button_b_state);
                break;

            case 5:
                control = xor(ssd_bm, button_a_state,button_b_state);
                break;

            case 6:
                control = xnor(ssd_bm, button_a_state,button_b_state);
                break;

            default:
                control = and(ssd_bm, button_a_state,button_b_state);
                break;
        };
    
        if(control){
            gpio_put(LED_RED,0);
            gpio_put(LED_GREEN,1);
        }
        else{
            gpio_put(LED_GREEN,0);
            gpio_put(LED_RED,1);
        }

        sleep_ms(250);
    }

    return 0;
}
