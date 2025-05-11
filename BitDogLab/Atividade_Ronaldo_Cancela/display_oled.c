#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h" //joystick

#include "hardware/pio.h" //matriz ld
#include "hardware/clocks.h"
#include "matriz_led.h"// Biblioteca para controle da matriz de LEDs

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

#define BUTTON_A 5    // GPIO conectado ao Botão A
#define BUTTON_B 6    // GPIO conectado ao Botão B

#define LED_GREEN 11   // GPIO conectado ao terminal verde do LED RGB
#define LED_RED 13   // GPIO conectado ao terminal vermelho do LED RGB

#define JOYSTICK_BTN_PIN 22 // GP22 - Digital Input

int control = 0;
int menu = 0;

void ssd1306_draw_rect(uint8_t *ssd, int x, int y, int width, int height, bool set) {
    ssd1306_draw_line(ssd, x, y, x + width - 1, y, set); // topo
    ssd1306_draw_line(ssd, x, y + height - 1, x + width - 1, y + height - 1, set); // base
    ssd1306_draw_line(ssd, x, y, x, y + height - 1, set); // esquerda
    ssd1306_draw_line(ssd, x + width - 1, y, x + width - 1, y + height - 1, set); // direita
}


int or(bool button_a_state, bool button_b_state){

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

int not(bool button_a_state){

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

int main()
{
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário
    npInit(7); //matriz
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


    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);

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


    char *text[] = {
        "    (GR)",
        " ",
        "    (HO)",
        " ",
        "    (DI)",
        " ",
        "    (PT)"
    };

    int y = 0;
    for (uint i = 0; i < count_of(text); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);

    npClear(); // limpa tudo antes

    npSetLED(1, 255, 0, 0); // acende verde (R=255)
    npSetLED(2, 255, 0, 0); // acende verde (R=255)
    npSetLED(3, 255, 0, 0); // acende verde (R=255)
    npSetLED(4, 255, 0, 0); // acende verde (R=255)
  
    npWrite(); // envia os dados 

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;

    bool GR = false;
    bool HO = false;
    bool DI = false;
    bool PT = false;

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

        if (adc_y_raw > threshold_cima) {
            menu = (menu == 0) ? 3 : menu - 1;
        }
        if (adc_y_raw < threshold_baixo) {
            menu = (menu == 3) ? 0 : menu + 1;
        }

        memset(ssd, 0, ssd1306_buffer_length);

        int y_draw = 0;
        for (uint i = 0; i < count_of(text); i++) {
            ssd1306_draw_string(ssd, 5, y_draw, text[i]);
            y_draw += 8;
        }

        // Desenha quadrado ao redor do item selecionado
        int y_box = menu * 16;

        ssd1306_draw_rect(ssd, 0, y_box, 80, 10, true);

        render_on_display(ssd, &frame_area);
        
        static bool last_button_state = false;  // guarda o estado anterior do botão

        // Leitura do joystick como botão
        bool button_pressed = !gpio_get(JOYSTICK_BTN_PIN); // Botão pressionado = LOW

        // Exibir os valores
        // printf(button_pressed ? "Pressionado" : "Solto");

        switch(menu){
            case 0: 
                if (button_pressed && !last_button_state) {
                    count0++;
                    if (count0 % 2 == 0) {
                        npSetLED(4, 0, 255, 0); 
                        GR = true;
                    } else {
                        npSetLED(4, 255, 0, 0); 
                        GR = false;
                    }
                    npWrite();
                }
                break;
        
            case 1: 
                if (button_pressed && !last_button_state) {
                    count1++;
                    if (count1 % 2 == 0) {
                        npSetLED(3, 0, 255, 0);
                        HO = true;
                    } else {
                        npSetLED(3, 255, 0, 0);
                        HO = false;
                    }
                    npWrite();
                }
                break;
        
            case 2: 
                if (button_pressed && !last_button_state) {
                    count2++;
                    if (count2 % 2 == 0) {
                        npSetLED(2, 0, 255, 0);
                        DI = true;
                    } else {
                        npSetLED(2, 255, 0, 0);
                        DI = false;
                    }
                    npWrite();
                }
                break;
        
            case 3: 
                if (button_pressed && !last_button_state) {
                    count3++;
                    if (count3 % 2 == 0) {
                        npSetLED(1, 255, 0, 0);
                        PT = false;
                    } else {
                        npSetLED(1, 0, 255, 0);
                        PT = true;
                    }
                    npWrite();
                }
                break;
        }

        last_button_state = button_pressed;

        printf("GR: %d | HO: %d | DI: %d | PT: %d\n", GR, HO, DI, PT);

        if(!PT || (GR && HO && DI)){
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
