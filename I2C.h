

// Attesa della prevista fine della comunicazione I2C
static void I2C_fineRW()
{ 
    while ( (SEN == 1) || (RSEN == 1) || (PEN == 1) || (RCEN == 1) || (R_W == 1) );
    // aspetta che il modulo I2C completi l'operazione prevista e si disattivi
    // questi sono flag che rappresentano che c'è qualcosa in corso
}								



// Genera un bit di ACKnoledge positivo al termine della ricezione di qualunque byte
static void I2C_Ack()
{
	ACKDT = 0;            // bit di Acknowledge positivo
	ACKEN = 1;            // Ack bit abilitato
	while(ACKEN == 1);    // aspetta la fine trasmissione del bit ACK (automaticamente azzerato dall'hardware) 
}



// Genera un bit di ACKnoledge negativo al termine della ricezione di qualunque byte
static void I2C_NoAck()
{
	ACKDT = 1;            // Bit di Acknowledge negativo
	ACKEN = 1;            // Ack bit abilitato
	while(ACKEN == 1);    // Aspetta la fine trasmissione del bit NoACK (automaticamente azzerato dall'hardware)
}


void I2C_Restart()
{
	RSEN = 1;        // Reinizializza la cominicazione
	while(RSEN);     // aspetta il completamento dell'operazione
}


void I2C_Init()
{
	SSPSTAT = 0b10000000;   // Slew rate disabilitato 
	SSPCON =  0b00101000;    // SSPEN = 1 (accendola perificirca) , I2C Master mode, clock = FOSC/(4 * (SSPADD + 1)) 
	SSPADD = 19;             // 100Khz @ 8Mhz Fosc (vedi datasheet) 19 è il valore che mi fa uscire 100khz con un quarzo di 8mhz
}


// Genera la condizione start nell'I2C.
void I2C_Start()
{
	SEN = 1;              // avvia la condizione di start e aspetta che termini
	while(SEN == 1);      // questo bit viene automatocamente resettato quando lo stato di start è completato 
}


// Genera la condizione stop nell'I2C.
void I2C_Stop(void) 
{
	PEN = 1 ;             // avvia la condizione di stop e aspetta che termini
	while(PEN == 1);      // questo bit viene automatocamente resettato quando lo stato di start è completato 
}


/* Trasmissione di un byte con protocollo I2C.
 Viene inviato per primo il bit MSB e per ultimo l'LSB.
 Il dato è posto sul pin quando SCL è basso. */
void I2C_Write(char dato)
{
    SSPBUF = dato;            // copia il dato in SSPBUF per trasmetterlo 
    while(BF==1);             // aspetta lo svuotamento del buffer
    I2C_fineRW();             // aspetta il completamento dell'operazione
}


/* Ricezione di un byte con protocollo I2C.
 Viene ricevuto per primo il bit MSB e per ultimo l'LSB. */
char I2C_Read(char val_ACK)
{
	char  I2C_Data = 0x00;

	RCEN = 1;                   // Abilita la ricezione dati
	while(BF==0);               // Aspetta il completamento della ricezione
	I2C_Data = SSPBUF;          // Copia il dato ricevuto
	I2C_fineRW();               // Aspetta il completamento dell'operazione
	      
	if(val_ACK == 1)            
	{
		I2C_Ack();
	}
	else
	{
		I2C_NoAck();
	}

	return I2C_Data;            // Restituisci il byte ricevuto
}


