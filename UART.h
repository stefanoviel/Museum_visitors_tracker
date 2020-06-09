// per il PIC16F887: TX=> RC6   RX=> RC7

void UART_Init()
{    
    // bassa velocità (BRGH=0) baud-rate=9600: SPBRG = [(Fosc/9600)/64]-1 =12
    
    SPBRG=12; 
    SPBRGH=0X00;

    BAUDCTLbits.BRG16=0; //esclusione del SPBRGH
            
    //TXSTA
    TXSTAbits.TX9=0;  //8 bit transmission
    TXSTAbits.TXEN=1; //Trasmissione abilitata
    TXSTAbits.SYNC=0; //Modalità asincrona
    TXSTAbits.BRGH=0; //Bassa velocità del baud rate

    //RCSTA
    RCSTAbits.SPEN=1;   //Abilitazione della porta seriale
    RCSTAbits.RX9=0;    //Modalità 8 bit
    RCSTAbits.CREN=1;   //Abilitazione della ricezione
    RCSTAbits.ADDEN=0;  //Disabilitazione del "address detection"

    //Interrupt in ricezione
    RCIE=1;
    PEIE=1;

}

void UART_TxChar(char ch)
{
  while(!PIR1bits.TXIF);  // se TXIF è a 0 l'UART è occupata con un'altro carattere
                          // quindi bisogna aspettare
  TXREG=ch;  //carico sul registo per la trasmissione di un carattere
}

void UART_TxString(const char *str)    
{
  while(*str!='\0')
  {
      UART_TxChar(*str);
      str++;
  }
}

char UART_RxChar()
{
	if(OERR) 		// se ci sono errori 
    {
        CREN = 0; 	// riavvia la 
        CREN = 1; 	// periferica
    }
    
    while(RCIF==0);    // aspetta la completa ricezione
    RCIF=0;            // azzera il flag di ricezione
    return(RCREG);     // restituisci il carattere ricevuto
}

char UART_RxString(char *stringa)
{
    char ch;
    char len = 0;
    while(1)
    {
        ch=UART_RxChar();    //Recevi un carattere
        UART_TxChar(ch);     //Echo ricezione carattere

        if((ch=='\r') || (ch=='\n'))  //è premuto il tasto invio? Allora temina la stringa
        {                             
            stringa[len]=0;        
            break;                  
        }
        else if((ch=='\b') && (len!=0))
        {
            len--;    //Se viene premuto backspace, decrementare l'indice per rimuovere il vecchio carattere
        }
        else
        {
            stringa[len]=ch; //copia il carattere nella stringa e incrementa l'indice
            len++;
        }
    }
  return len;   
}


void UART_GotoNewLine()
{
    UART_TxChar('\r');//CR Carriage Return (ritorna all'inizio del rigo) 
    UART_TxChar('\n');//LF Line Feed (passa al rigo sotto)
}


void UART_TxInt(int val) 
{
    char sig=0;
    
    if(val<0)
    {
        sig=1;			// memorizza i segno
        val*=-1;        //converti a valore positivo       
    }    
    
    char cent=0, dec=0, uni=0;

 //scomposizione di 'val' in unità decine e centinaia
    
    while (val>=100) { 
        val-=100;
        cent++;
        }
    while (val>=10) {
        val-=10;
        dec++;
        }
    uni=val;
    
    if (cent==0 && dec==0) dec=32; // spengo 'dec' se 'cent' e 'dec' valgono '0'
        else dec+=48;
    
    if (cent==0) cent=32; // spengo 'cent' se vale '0'
        else cent+=48;
    
    uni+=48;
        
    if (sig==1)    UART_TxChar('-');   //Scrivi il segno meno    
    
    if (cent!=32)  UART_TxChar(cent);
    if (dec!=32)   UART_TxChar(dec);    
    UART_TxChar(uni);
}

void UART_TxDeci(long int val) 
{    
    char deci=0, cent=0, mill=0;

 //scomposizione di 'val' in decimi, centesimi e millesimi
    
    while (val>=100) { 
        val-=100;
        deci++;
        }
    while (val>=10) {
        val-=10;
        cent++;
        }
    mill=val;
    
    if (mill==0 && cent==0) cent=32; // spengo 'cent' se 'cent' e 'mill' valgono '0'
    else cent+=48;
    
    if (mill==0) mill=32; // spengo 'mill' se vale '0'
        else mill+=48;
    
    deci+=48;
 
    UART_TxChar(deci);
    if (cent!=32)  UART_TxChar(cent);  
    if (mill!=32)  UART_TxChar(mill);
}


void UART_TxFloat(float numb)
{
    int numbInt;

    numbInt = (int) numb;
    UART_TxInt(numbInt);

    UART_TxChar('.');
    
    if (numb<0) {
        numb*=-1;
        numbInt*=-1;
    }    
    

    numb = numb - numbInt;
    numb = numb * 1000;
        
    UART_TxDeci(numb);
}
