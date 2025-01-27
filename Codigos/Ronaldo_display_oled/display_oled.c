#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

//Definindo conexoes-------
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

#define BTN_A_PIN 5
//-------------------------

//Bloquinho de um texto no OLED-------------------------------------------------
void bloquinhoOLED(uint8_t *ssd, struct render_area *frame_area, char *text[], int count_of_text){

    // Limpa o display (zerando o buffer antes de desenhar o novo conteúdo)
    memset(ssd, 0, ssd1306_buffer_length);  // Zera o buffer do display

    int y = 0;
    for (uint i = 0; i < count_of_text; i++)
    {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 10;
    }
    render_on_display(ssd, frame_area);
    
}   
//-------------------------------------------------------------------------------

//Variaveis de controle------------------------
int A_state = 0;//Botao A esta pressionado?

char *textVerde[] = {
    "  SINAL ABERTO",
    " ",
    " ATRAVESSAR COM",
    "    CUIDADO"};


char *textAmarelo[] = {
    "    SINAL DE",
    "    ATENCAO",
    " ",
    "   PREPARE-SE"};


char *textVermelho[] = {
    " SINAL FECHADO",
    " ",
    "    AGUARDE"};

//---------------------------------------------

//Funcoes do semaforo----------------------------
void SinalAberto(){
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);   
}

void SinalAtencao(){
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void SinalFechado(){
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

int WaitWithRead(int timeMS){
    for(int i = 0; i < timeMS; i = i+100){
        A_state = !gpio_get(BTN_A_PIN);
        if(A_state == 1){
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}
//------------------------------------------------


int main()
{
    //Display Oled---------------------------------
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário

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

    // Zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Definindo o número de elementos para cada array de texto
    int count_text_verde = sizeof(textVerde) / sizeof(textVerde[0]);
    int count_text_amarelo = sizeof(textAmarelo) / sizeof(textAmarelo[0]);
    int count_text_vermelho = sizeof(textVermelho) / sizeof(textVermelho[0]);

    //---------------------------------------------

    //Leds do semaforo-----------------------------
    // INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // INICIANDO BOTÄO
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    //---------------------------------------------

restart:

    while(true){

        SinalFechado();
        bloquinhoOLED(ssd, &frame_area, textVermelho, count_text_vermelho);
        A_state = WaitWithRead(8000);   //espera com leitura do botäo

        if(A_state){//ALGUEM APERTOU O BOTAO - SAI DO SEMAFORO NORMAL
            
            
            SinalAtencao();
            bloquinhoOLED(ssd, &frame_area, textAmarelo, count_text_amarelo);
            sleep_ms(5000);

            
            SinalAberto();
            bloquinhoOLED(ssd, &frame_area, textVerde, count_text_verde);
            sleep_ms(10000);

        }
        else{//NINGUEM APERTOU O BOTAO - CONTINUA NO SEMAFORO NORMAL
                                      
            SinalAtencao();
            bloquinhoOLED(ssd, &frame_area, textAmarelo, count_text_amarelo);
            sleep_ms(2000);

            
            SinalAberto();
            bloquinhoOLED(ssd, &frame_area, textVerde, count_text_verde);
            sleep_ms(8000);
        }
                
    }

    return 0;
}
