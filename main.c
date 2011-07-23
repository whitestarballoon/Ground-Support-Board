#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <i2c.h>

//#define BAUD 9600
//38400 Baud
#define MYUBRR 12
#define BUFFERSIZE 500

#define ERROR -1
#define I2CSTART -2
#define I2CSTOP -3
#define I2CACK -4
#define I2CNACK -5

#define getSDA (PINC >> PINC4)
#define getSCL (PINC >> PINC5)

#define GETLINEMAX 200

//Define functions
//======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
void getDebugDataFromI2CBus(uint8_t receiveDataLength, uint8_t *recieveData);
void printAsciiValues(void);
void termAttent(void);
uint8_t getLine(char *outputBuffer);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
//======================

//Globals
uint8_t sdaNew, sdaOld;
uint8_t sclNew, sclOld;

uint8_t dataState, dataBits;

uint8_t data, tempData;

int16_t buffer[BUFFERSIZE];

uint16_t start, end;

uint8_t termAttention = 0;

int main (void)
{	
    ioinit(); //Setup IO pins and defaults
    i2cSetTheDamnTWBRMyself(8);
	i2cSetLocalDeviceAddr(0b00001110, 0x00, 0, 0, 0);
	i2cSetSlaveReceiveHandler(getDebugDataFromI2CBus);

	printf_P(PSTR("I2C Sniffing is Alive\n\nPress '!' to get the terminal's attention\n\n"));
	
	sei();
	

    while(1)
    {
		if(termAttention == 0)
		{
			printAsciiValues();
		} else {
			termAttent();
		}
    }
   
    return(0);
}

void termAttent(void)
{
	cli();
	printf_P(PSTR("White Star Balloons\n\nGround Support Suite Version -1.0\n\n"
					"Caveats:\n - Right now, I can only Send Data, then immediately exit to allow others\n"
					"     to send me debug data after I tell them to do something.\n"
					" - I disable interrupts while I'm taking your input, so hurry up!\n"
					" - I hope your terminal only sends the \\r character for new lines!\n"
					" - DO NOT PRESS BACKSPACE\n\n"));
	printf_P(PSTR("You have my attention.  I've disabled interrupts, so this had better be good.\n"));
	printf_P(PSTR("Who Should I address? (Form: 0x00, 7  bit shifted left)\n"));
	char inputBuff[GETLINEMAX];
	uint8_t charsGot = getLine(inputBuff);
	if(charsGot == 1 && inputBuff[0] == 's')
	{
		printf_P(PSTR("You have discovered the Colonel's Secret Recipe!\n"
						"[1] Tell Flight Computer to dump Vars\n"
						"[2] Tell Flight Computer to reset Vars\n"));
		charsGot = getLine(inputBuff);
	} else {
		uint8_t address;
		uint8_t matches = sscanf(inputBuff, "%x", &address);
		if(matches != 1)
		{
			printf_P(PSTR("Error in parsing\n"));
			goto leave;
		}
		printf_P(PSTR("How Many Bytes you want?\n"));
		charsGot = getLine(inputBuff);
		uint8_t bytes;
		matches = sscanf(inputBuff, "%d", &bytes);
		if(matches != 1)
		{
			printf_P(PSTR("Error in parsing\n"));
			goto leave;
		}
		printf_P(PSTR("Bytes: %d\n"), bytes);
		uint8_t data[20];
		for(int i=0; i < bytes; i++)
		{
			printf_P(PSTR("Byte %d (0x00, hex):"), i);
			charsGot = getLine(inputBuff);
			sscanf(inputBuff, "%x", &data[i]);
			if(matches != 1)
			{
				printf_P(PSTR("Error in parsing\n"));
				goto leave;
			}
		}
		uint8_t status = i2cMasterSendNI(address, bytes, data);
		if(status != I2C_OK)
		{
			printf_P(PSTR("ERROR IN SENDING! Error Code: %d\n"), status);
		}
	}
	
	leave:
	termAttention = 0;
	sei();
	printf("Returning...\n");
}

uint8_t getLine(char *outputBuffer)
{
	int8_t bufferLoader=0;
	while((outputBuffer[bufferLoader] = (char)uart_getchar()) != '\r' && bufferLoader < GETLINEMAX)
	{
		uart_putchar(outputBuffer[bufferLoader], stdout);
		bufferLoader++;
	}
	printf("\n");
	return bufferLoader;
}

char ReceivedByte;

ISR(USART_RX_vect)
{
	ReceivedByte = UDR0; // Fetch the received byte value into the variable "ByteReceived" 
	
	UDR0 = ReceivedByte; // Echo back the received byte back to the computer
	
	if(ReceivedByte == '!')
	{
		termAttention = 1;
	}
}


void getDebugDataFromI2CBus(uint8_t receiveDataLength, uint8_t *recieveData)
{
	//printf("length: %d\n", receiveDataLength);
	if(recieveData[0] == 0x05)
	{
		int i = 1;
		for(i; i < receiveDataLength; i++)
		{
			buffer_add((uint16_t)recieveData[i]);
			//printf("1: %d 2: %c\n", i, recieveData[i]);
		}
	}
}

void printAsciiValues(void)
{
	
	int16_t print = buffer_get();
	if(print > -1)
	{
		printf("%c", (char)print);
	}
	
}

void ioinit (void)
{
    //1 = output, 0 = input
    DDRD = 0b11111110; //PORTD (RX on PD0)
    DDRC = 0b00000000;
    PORTC = 0b00110000;

    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    
    stdout = &mystdout; //Required for printf init
    
    i2cInit();
    /*PCICR |= _BV(1); //PCIE1, Enable Pin Change Interrups 8-14.
    PCMSK1 |= _BV(4) | _BV(5); // Turn on PCINT12 and 13.
    
    sdaOld = sdaNew = getSDA;
	sclOld = sclNew = getSCL;*/
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

void buffer_add(int16_t b)
{
	
	if ((end + 1) % BUFFERSIZE != start) {
                buffer[end] = b;
                end = (end + 1) % BUFFERSIZE;
    } else {
		printf("BUFFER FULL!\n");
	}
	
}

int16_t buffer_get(void)
{
        if (end != start) {        
                int16_t temp = buffer[start];
                start = (start + 1) % BUFFERSIZE;
                return(temp);
        }
        //otherwise, the buffer is empty; return an error code
        return -1;
}
