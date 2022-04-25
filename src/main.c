
#include "stm8s.h"
#include "milis.h"
#include "stm8_hd44780.h"

#include "delay.h"
#include <stdio.h>
#include "stm8s_gpio.h"
#include "swi2c.h"
//#include "uart1.h"

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define LED_PORT GPIOD
#define LED_PIN  GPIO_PIN_4
#define LED_HIGH   GPIO_WriteHigh(LED_PORT, LED_PIN)
#define LED_LOW  GPIO_WriteLow(LED_PORT, LED_PIN)
#define LED_REVERSE GPIO_WriteReverse(LED_PORT, LED_PIN)

void setup(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(GPIOB,GPIO_PIN_5,GPIO_MODE_IN_PU_NO_IT); // nastavíme pin PG4 jako vstup s vnitřním pull-up rezistorem  v pospisech nesedi jmena pinu
    GPIO_Init(GPIOB,GPIO_PIN_4,GPIO_MODE_IN_PU_NO_IT); // nastavíme pin PG5 jako vstup s vnitřním pull-up rezistorem
    GPIO_Init(GPIOC,GPIO_PIN_3,GPIO_MODE_IN_PU_NO_IT); // nastavíme pin PG4 jako vstup s vnitřním pull-up rezistorem
    GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_IN_PU_NO_IT); // nastavíme pin PG5 jako vstup s vnitřním pull-up rezistorem
    GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_PU_NO_IT); // nastavíme pin PG4 jako vstup s vnitřním pull-up rezistorem

    GPIO_Init(GPIOA,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_SLOW);// relé
 
    lcd_init(); //inicializace LCD
    init_milis();
    swi2c_init();
    //init_uart1();
}

void cum_lcd(int h,int m,int s) {

    char text[16];
    lcd_gotoxy(8,1);
    sprintf(text, "%02d:%02d:%02d", h, m, s);
    lcd_puts(text);
    
}

int main(void){
    uint32_t time = 0;
    uint32_t rtime = 0;
    uint32_t otime = 0;
    uint32_t ss = 0;
    //uint32_t cum = 0;
    //char text[32];
    uint16_t h = 1;
    uint16_t m = 1;
    uint16_t s = 8;
    int pozice = 2; //vyjadřuje pozici měňění čísel 0 == h , 1 == m , 2 == s
    int cum = 1;
    int xenofobni = 1;
    int pog = 1;
    uint32_t sec = 0;

    uint8_t precteno[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t zapsano[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t err;
    
    char text[32];
    
    _Bool minuly_stav_on_tlacitka=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    _Bool minuly_stav_on_tlacitka2=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka2=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka2=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    _Bool minuly_stav_on_tlacitka3=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka3=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka3=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    _Bool minuly_stav_on_tlacitka4=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka4=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka4=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    _Bool minuly_stav_on_tlacitka5=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka5=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka5=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    _Bool minuly_stav_on_tlacitka51=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool minuly_stav_off_tlacitka51=0; // 0=tlačítko bylo minule uvolněné, 1=tlačítko bylo minule stisknuté 
    _Bool aktualni_stav_tlacitka51=0; // 0=tlačítko je uvolněné, 1= tlačítko je stisknuté

    setup();

     /*error=swi2c_read_buf(0b11010000,0x00,precteno,4) */
    zapsano[0] = 0x00;  // sekundy
    zapsano[1] = 0x54;  // minuty
    zapsano[2] = 0x22;  // hodiny
    

    // když tento řádek odkomentuješ, tak se čas zapíše do obvodu
    //swi2c_write_buf(0x68, 0x00, zapsano, 7);

    cum_lcd(h, m, s);
    while (1) {

        if (milis() - ss > 1111 /*&& BTN_PUSH*/) {    
            // čtu RTC
            err = swi2c_read_buf(0x68, 0x00, precteno, 7);
            

            lcd_gotoxy(8,0);
            sprintf(text,"%d%d:%d%d:%d%d ",       // v RTC obvodu je čas uložen v BCD ************************************************************
                    precteno[2] >> 4, precteno[2] & 0x0F,
                    precteno[1] >> 4, precteno[1] & 0x0F,
                    precteno[0] >> 4, precteno[0] & 0x0F);
            lcd_puts(text);

            ss = milis();
        }

        if (xenofobni == 1 ) {
            lcd_gotoxy(0,1);
            lcd_puts("x1");
            // tlacitko 1 / směr v pravo → /////////////////////////////////////////////////////////////////////////////////////////////// →
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOB,GPIO_PIN_5)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka=0; // ...tlačítko uvolněné
            }
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka==0 && aktualni_stav_tlacitka==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            lcd_gotoxy(0,0);
            lcd_puts("1");
            cum_lcd(h,m,s);
            if (pozice != 2) {
                pozice++;
            }
            else {
                pozice = 0;
            }
            }       
            minuly_stav_on_tlacitka = aktualni_stav_tlacitka; // přepíšeme minulý stav tlačítka aktuálním

            // tlacitko 2 /směr v levo ←//////////////////////////////////////////////////////////////////////////////////////////////////// ← hotovo až na divné problikávání jenom někdy 
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOB,GPIO_PIN_4)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka2=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka2=0; // ...tlačítko uvolněné
            }
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka2==0 && aktualni_stav_tlacitka2==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            lcd_gotoxy(0,0);
            lcd_puts("2");
            if (pozice != 0) {
                pozice--;
            }
            else {
                pozice = 2;
            }
            }
            minuly_stav_on_tlacitka2 = aktualni_stav_tlacitka2; // přepíšeme minulý stav tlačítka aktuálním

            // tlacitko 3 /směr nahoru ↑//////////////////////////////////////////////////////////////////////////////////////////////////// ↑
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOC,GPIO_PIN_3)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka3=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka3=0; // ...tlačítko uvolněné
            }
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka3==0 && aktualni_stav_tlacitka3==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            lcd_gotoxy(0,0);
            lcd_puts("3");
            if (pozice == 0) {
                if (h<23) {
                    h++;
                }
                else {
                    h = 0;
                }   
            }
            if (pozice == 1) {
                if (m<59) {
                    m++;
                }
                else {
                    m = 0;
                }   
            }
            if (pozice == 2) {
                if (s<59) {
                    s++;
                }
                else {
                    s = 0;
                }
            }
            }
            minuly_stav_on_tlacitka3 = aktualni_stav_tlacitka3; // přepíšeme minulý stav tlačítka aktuálním

            // tlacitko 4 /směr dolů ↓ //////////////////////////////////////////////////////////////////////////////////////////////////// ↓
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOD,GPIO_PIN_2)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka4=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka4=0; // ...tlačítko uvolněné
            }
            
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka4==0 && aktualni_stav_tlacitka4==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            lcd_gotoxy(0,0);
            lcd_puts("4");
            if (pozice == 0) {
                if (h<24 && h>0) {
                    h--;
                }
                else {
                    h = 23;
                }   
            }
            if (pozice == 1) {
                if (m<60 && m>0) {
                    m--;
                }
                else {
                    m = 59;
                }   
            }
            if (pozice == 2) {
                if (s<60 && s>0) {
                    s--;
                }
                else {
                    s = 59;
                }
            }

            
            }
            minuly_stav_on_tlacitka4 = aktualni_stav_tlacitka4; // přepíšeme minulý stav tlačítka aktuálním

            // tlacitko 5 /potvrzení ////////////////////////////////////////////////////////////////////////////////////////////////////
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOD,GPIO_PIN_3)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka5=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka5=0; // ...tlačítko uvolněné
            }
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka5==0 && aktualni_stav_tlacitka5==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            lcd_gotoxy(0,0);
            lcd_puts("5");

            xenofobni = 2;
            }
            minuly_stav_on_tlacitka5 = aktualni_stav_tlacitka5; // přepíšeme minulý stav tlačítka aktuálním

            char text[16];
            lcd_gotoxy(3,0);
            sprintf(text, "%d", pozice);
            lcd_puts(text);

            ///////////////////////////////////////////////////Blikání vybrané sekce
            if (pozice == 2 && milis() - time > 500 ) {
                if (cum == 1) {
                    lcd_gotoxy(14,1);
                    lcd_puts("  ");
                    cum = 0;
                    
                }
                else {
                    cum = 1;
                    cum_lcd(h, m, s);
                }
                
                time = milis();
            }
            
            if (pozice == 1 && milis() - time > 500 ) {
                if (cum == 1) {
                    lcd_gotoxy(11,1);
                    lcd_puts("  ");
                    cum = 0;
                    
                }
                else {
                    cum = 1;
                    cum_lcd(h, m, s);

                }
                
                time = milis();
            }

            if (pozice == 0 && milis() - time > 500 ) {
                if (cum == 1) {
                    lcd_gotoxy(8,1);
                    lcd_puts("  ");
                    cum = 0;
                    
                }
                else {
                    cum = 1;
                    cum_lcd(h, m, s);
                }
                
                time = milis();
            }
        }  
        if (xenofobni == 2 ) { 

            lcd_gotoxy(0,1);
            lcd_puts("x2");
            sec = (h * 3600) + (m * 60) + s;
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Odpočítávání s
            if(sec == 0 && milis() - rtime > 500){ 
                GPIO_WriteReverse(GPIOA,GPIO_PIN_3); 
                rtime = milis();
                
            }

            if(sec != 0){
                if(milis() - otime > 1000){ 
                sec--;
                otime = milis();
                }  
                
            } 
            s = sec % 60; /// zde je problém z %
            m = (sec % 3600) / 60; /// tady taky 
            h = sec / 3600;

            cum_lcd(h,m,s);

            // tlacitko 5 /potvrzení ////////////////////////////////////////////////////////////////////////////////////////////////////
            // zjistíme stav "ON" tlačítka
            if(GPIO_ReadInputPin(GPIOD,GPIO_PIN_3)==RESET){ // pokud je na vstupu od "ON" tlačítka log.0 tak...
            aktualni_stav_tlacitka51=1; // ...je tlačítko stisknuté
            }
            else{ // jinak je ...
            aktualni_stav_tlacitka51=0; // ...tlačítko uvolněné
            }
            // zjišťujeme jestli nastal "okamžik stisku"
            if(minuly_stav_on_tlacitka51==0 && aktualni_stav_tlacitka51==1){
            LED_REVERSE; // pokud ano, rozsvítíme LEDku
            xenofobni = 1;
            pog = 1;
        
            }
            minuly_stav_on_tlacitka51 = aktualni_stav_tlacitka51; // přepíšeme minulý stav tlačítka aktuálním   
        } 
    }

}
/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
