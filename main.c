//progetto museo

#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator: Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>                    //Header file generico
#define _XTAL_FREQ 8000000         //Specifico la frequenza dell'oscillatore
#include "I2C.h"
#include "LCD_I2C.h"
#include "UART.h"

void main(void) {

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;

    TRISA = 0b00000000; //Imposto out
    TRISB = 0b00001111; //Imposto 
    TRISC = 0b00000000;
    TRISD = 0b00000000; //Imposto tutti i pin di PORTD come output

    ANSEL = 0x00; //Imposto tutti i pin come ingressi digitali
    ANSELH = 0x00; //Imposto tutti i pin come ingressi digitali

    OPTION_REG = 0b10000101; // No pull-up PORTB,  Fosc/4,  PS=64
    INTCON = 0b11101000; // GIE = PIE = RBIE = TOIE = 1
    IOCB = 0b00000111; //interupt on-change su RB0 RB1 RB2
    TMR0 = 216;

    I2C_Init();
    Lcd_Init();

    char aperta = 0; //variabile porta ingresso sorveglianti
    char spegni = 0; //dopo quanto tempo devo spegnere la luce dell'ingresso dei sorveglianti
    int i = 0; //conteggio per visualizzazione temporizzata nomi sorveglianti
    char allarme; //se allarme = 1 non ci sono ancora visitatori nel museo, ma nessun sorvegliante
    int tempo = 1; //variabile per visualizzazione temporizzata nomi sorveglianti
    int cont = 0; //conta a quale barra della scheda d'identificazione siamo arrivati
    int codice[5]; //memorizza i valori del codice sulla scheda
    unsigned int n_persone = 0; //numero di persone all'interno del museo
    int var = 0;
    char Massimo = 0; //indica se il sorvegliante è in servizio
    char Decimo = 0;
    char Meridio = 0;

    while (1) {
        Lcd_Clear();
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("N. persone: "); //visulizzo il numero di persone 
        Lcd_Set_Cursor(1, 12);
        Lcd_Write_Val(n_persone);

        Lcd_Set_Cursor(2, 1);
        Lcd_Write_String("Sorv: "); //seconda riga visualizzo sorveglianti
        Lcd_Set_Cursor(2, 7);

        if (tempo == 1 && Massimo == 1) { //se Massimo è in servizio visualizzao massimo
            Lcd_Write_String("Massimo");
        } else if (tempo == 1 && Massimo == 0) { //la variabile tempo viene aggiornata ogni due secondi 
            tempo = 2; //se massimo non è presente passo a visualizzare quello dopo   
        }

        if (tempo == 2 && Decimo == 1) {
            Lcd_Write_String("Decimo");
        } else if (tempo == 2 && Decimo == 0) {
            tempo = 3;
        }

        if (tempo == 3 && Meridio == 1) {
            Lcd_Write_String("Meridio");
        } else if (tempo == 3 && Meridio == 0) {
            tempo = 1;
        }

        if (n_persone > 150)RA0 = 1; //se ci sono più di 150 persone attivo il led che 
                                     // simula il blocco dei tornelli

        if (n_persone <= 150) {
            if (Meridio || Decimo || Massimo) { //quando ci sono meno di 150 persone e
                RA0 = 0; // almeno un sorvegliante riapro i tornelli
            }
        }

        if (!Meridio && !Decimo && !Massimo) { //se non ci sono sorveglianti in servizio atttivo l'allarme
            allarme = 1;
            RA0 = 1;
        } else if (Meridio || Decimo || Massimo) {
            allarme = 0;
        }

        if (tempo == 3) { //ogni 6 secondi invio i dati con l'UART

            UART_RxString("VIS:");
            UART_TxInt(n_persone);
            UART_RxString("SOR:");
            int sorveglianti = 0;
            if (Meridio) sorveglianti++;
            if (Massimo) sorveglianti++;
            if (Decimo) sorveglianti++;
            UART_TxInt(sorveglianti);
        }
    }
}

void interrupt ISR(void) // Interrupt Service Routine
{

    if (INTCONbits.T0IE && INTCONbits.T0IF) // verifico che l'interrupt è stato 
    { // causato da un overflow del TMR0
        TMR0 = 216;
        INTCONbits.T0IF = 0; // Resetto il flag di interrupt per TMR0

        i++; //variabile per fare tempi più lunghi

        if (allarme) { //se l'allarme è attivo
            RD0 = !RD0; //frequenza di 400Hz sul buzzer
        } else {
            RD0 = 0;
        }

        if (i == spegni && aperta == 1) { //se sono passati 200 ms 
            RD1 = 0; //spengo la spia della porta aperta
            aperta = 0;
        }

        if (i == 1602) { //ogni due secondi cambio il nome
            i = 0;
            if (tempo >= 3) { //arrivato a 3 riparto da zero
                tempo = 0;
            } else {
                tempo++;
            }
        }
    }

    if (RBIF == 1 && RBIE == 1) { // interrupt on-change su portb
        RBIF = 0; // azzero il flag 

        if (RB0 == 1) { //una persona è entrata           

            n_persone++;

            while (RB0 == 1); // aspetto il rilascio         
        }

        if (RB1 == 1) { // una persona è uscita

            n_persone--;

            while (RB1 == 1); // aspetto il rilascio          
        }

        if (RB2 == 1) { // clock della scheda d'identificazione


            if (RB3 == 1) { //se la banda del codice è opaca
                codice[cont] = 1; //registro 1
            } else if (RB3 == 0) { //se è tra  sparente 
                codice[cont] = 0; //registro 0
            }

            cont++;

            if (cont == 5) { //quando ho registrato 5 numeri (tutto il codice)

                //controllo se il codice corrisponde ad un sorvegliante

                if (codice[0] == 1 && codice[1] == 1 && codice[2] == 1 && codice[3] == 1 && codice[4] == 0) {
                    Massimo = !Massimo; //massimo è uscito o rientrato
                    RD2 = !RD2;
                    RD1 = 1; //accendo la luce di porta aperta
                    if (i > 1469) {
                        spegni = i - 1469;
                    } else {
                        spegni = i + 133; //si spegne dopo 200ms
                    }
                    aperta = 1; //variabile memorizzare porta aperta
                }

                if (codice[0] == 1 && codice[1] == 1 && codice[2] == 0 && codice[3] == 1 && codice[4] == 0) {
                    Decimo = !Decimo;
                    RD3 = !RD3
                            RD1 = 1;
                    if (i > 1469) {
                        spegni = i - 1469;
                    } else {
                        spegni = i + 133; //si spegne dopo 200ms
                    }
                    aperta = 1;
                }

                if (codice[0] == 1 && codice[1] == 0 && codice[2] == 1 && codice[3] == 0 && codice[4] == 0) {
                    Meridio = !Meridio;
                    RD4 = !RD4;
                    RD1 = 1;
                    if (i > 1469) {
                        spegni = i - 1469;
                    } else {
                        spegni = i + 133; //si spegne dopo 200ms
                    }
                    aperta = 1;
                }

                cont = 0; //si ricomincia a creare un nuovo codice
            }

            while (RB2 == 1); // che passi tutta la barra della scheda
        }

    }   

}

