
/*
 Definizione dell'indirizz del PCF8574 Remote 8-bit I/O Expander per I2C-bus.
 Rif. datasheet del PCF8574(A)
 */

#define PCF8574WriteMode  0b01001110   // Indirizzo del PCF8574 (0x27) + Write bit (bit LSB)


char	 LCD_BL_Status = 0;     // 1 per accendere, 0 per spegnere la retro illuminazione


char	 pin_RS;		//  I2C_BYTE.0 
char	 pin_RW;		//  I2C_BYTE.1
char	 pin_E;			//  I2C_BYTE.2
char	 pin_BL;		//  I2C_BYTE.3
char	 pin_D4;		//  I2C_BYTE.4
char	 pin_D5;		//  I2C_BYTE.5
char	 pin_D6;		//  I2C_BYTE.6
char	 pin_D7;		//  I2C_BYTE.7

//comandiamo a quattro linee siccome abbiamo solo 8 uscite del display
// questi sono tutti byte


char LCD_crea_Byte()  // questa è la funzione che mi prende 8 bit distinti e mettergli su un unico byte per serializzarlo
{
    char I2C_BYTE = 0x00;

    I2C_BYTE |= pin_RS   << 0;   //prendo solo il l'ultimo bit del primo byte e lo metto nella posizioen zero
    I2C_BYTE |= pin_RW   << 1;
    I2C_BYTE |= pin_E    << 2;
    I2C_BYTE |= pin_BL   << 3;
    I2C_BYTE |= pin_D4   << 4;
    I2C_BYTE |= pin_D5   << 5;
    I2C_BYTE |= pin_D6   << 6;
    I2C_BYTE |= pin_D7   << 7;

    return I2C_BYTE;
}


void Lcd_I2C_write(char val) 		// trasmissione del dato seriale
{
    I2C_Start();                    // Start comunicazione I2C
    I2C_Write(PCF8574WriteMode);     // Connetti al PCF8574 inviando l'idirizzo + write bit
    I2C_Write(val);                // Scrivi il dato  
    I2C_Stop();                     // Stop comunicazione I2C     
    
    // ho trasmesso il mio byte
    
}


void Lcd_I2C_Port(char a)
{
	if(a & 1)	pin_D4 = 1;
	else 	pin_D4 = 0;

	if(a & 2) 	pin_D5 = 1;    
	else	pin_D5 = 0;

	if(a & 4)	pin_D6 = 1;    
	else	pin_D6 = 0;

	if(a & 8)   pin_D7 = 1;    
	else    pin_D7 = 0;
}


void Lcd_I2C_Cmd(char cmd)  // passo un comando da inviare
{
 //   char comando;
	
	pin_RS = 0; 	
    pin_RW = 0;
    pin_E  = 0;
    pin_BL = LCD_BL_Status;
    // questi li forzo perchè devo inviare un comando
    
    Lcd_I2C_Port (cmd);
    
//    comando=LCD_crea_Byte();
    
    Lcd_I2C_write(LCD_crea_Byte());  // creo il byte da inviare lo scrivo 
    pin_E = 1;    // mando una seconda volta il byte tuttto uguale a prima, ma con l'enable a 1 il micro non si accorge che il byte è cambiato
    Lcd_I2C_write(LCD_crea_Byte());  // legge sul fronte di salita, quindi devo avere già il dato pronto sulla linea
     
    // ogni volta che comunico un byte devo trasmetterlo 3 volte 
    
    __delay_ms(4);    
    
    pin_E = 0;    
    Lcd_I2C_write(LCD_crea_Byte());	
	
}

void Lcd_Clear()             // Cancella LCD
{
	Lcd_I2C_Cmd(0);   // come prima si mandano i quattro bit meno significativi e poi quelli più significativi
	Lcd_I2C_Cmd(1);
}

void Lcd_Set_Cursor(char a, char b)     // Posiziona cursore
{                                       // a = riga ---  b = colonna
	char temp,z,y;
	if(a == 1)
	{
	  temp = 0x80 + b - 1;
		z = temp>>4;                // z = 4 bit piu' significativi
		y = temp & 0x0F;            // y = 4 bit meno significativi
		Lcd_I2C_Cmd(z);
		Lcd_I2C_Cmd(y);
	}
	else if(a == 2)
	{
		temp = 0xC0 + b - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_I2C_Cmd(z);
		Lcd_I2C_Cmd(y);
	}
}

void Lcd_Init()
{
   Lcd_I2C_Port(0x00);
    __delay_ms(20);
   Lcd_I2C_Cmd(0x03);
	__delay_ms(5);
   Lcd_I2C_Cmd(0x03);
	__delay_ms(10);
   Lcd_I2C_Cmd(0x03);
  
   Lcd_I2C_Cmd(0x02);    //LCD pilotato con 4 linee
  
   Lcd_I2C_Cmd(0x02);
   Lcd_I2C_Cmd(0x08);
  
   Lcd_I2C_Cmd(0x00);
   Lcd_I2C_Cmd(0x0C);
  
   Lcd_I2C_Cmd(0x00);
   Lcd_I2C_Cmd(0x06);
}

void Lcd_Write_Char(char a)
{
    char x,y;         // il carattere lo spezzo in due parti
    x = a&0x0F;        // x = 4 bit meno significativi
    y = a&0xF0;        // y = 4 bit piu' significativi
   
    pin_RS = 1;        // Invio a
    pin_RW = 0;
    pin_E  = 0;
    pin_BL = LCD_BL_Status;
    
    Lcd_I2C_Port (y>>4);    
    Lcd_I2C_write(LCD_crea_Byte());   // creo il byte, mando fuori, enable, mando furoi, enable = 0, mando fuori
    pin_E = 1;    
    Lcd_I2C_write(LCD_crea_Byte());
    
    pin_E = 0;    
    Lcd_I2C_write(LCD_crea_Byte());	
	
    Lcd_I2C_Port (x);    
    Lcd_I2C_write(LCD_crea_Byte());
    pin_E = 1;    
    Lcd_I2C_write(LCD_crea_Byte());
    
    pin_E = 0;    
    Lcd_I2C_write(LCD_crea_Byte());		
}


void Lcd_Write_String(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	   Lcd_Write_Char(a[i]);
}


void Lcd_Shift_Right()
{
	Lcd_I2C_Cmd(0x01);
	Lcd_I2C_Cmd(0x0C);
}


void Lcd_Shift_Left()
{
	Lcd_I2C_Cmd(0x01);
	Lcd_I2C_Cmd(0x08);
}

void Lcd_Write_Val(int val) 
{
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
    
    Lcd_Write_Char(cent);
    Lcd_Write_Char(dec);    
    Lcd_Write_Char(uni);        
 
}
