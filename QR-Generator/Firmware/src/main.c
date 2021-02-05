/*
* This a port of the QR-Code-generator (Copyright (c) Project Nayuki) by Group 5.
* UART input added and minor changes were done to the source code by Project Nayuki.
*
* Basically, this programm encodes a message received via UART and displays the equivalent QR-Code.
* It auto-detects size and encoding level depending on the input message.
* Message must fit 10-H standards at most given by LCD size.
*/
#include <util/delay.h>
#include "uart.h"
#include "lcd.h"
#include "twi.h"
#include "dataflash.h"
#include "qrcodegen.h"
#include "qrcodegen-demo.c"
#define wait_joy_button()      \
	{                          \
		LCD_GotoXY(20, 7);     \
		LCD_PutChar(0x10);     \
		LCD_Update();          \
		while (((PINA)&0x08))  \
			;                  \
		while (!((PINA)&0x08)) \
			;                  \
		_delay_ms(20);         \
		while (((PINA)&0x08))  \
			;                  \
	}


#define MAX_LENGTH_OF_TEXT 652


int main(void)
{
	// set PA3-PA7 as input and activated internal Pull-Up
	DDRA &= ~((1 << PINA3) | (1 << PINA4) | (1 << PINA5) | (1 << PINA6) | (1 << PINA7)); // Required for DMM Board 2013
	PORTA |= ((1 << PINA3) | (1 << PINA4) | (1 << PINA5) | (1 << PINA6) | (1 << PINA7)); // Required for DMM Board 2013

	// set PB0-PB3 as output
	DDRB = 0x0F;
	// set PB0-PB3 on high-level
	PORTB |= 0x0F; // Required for DMM Board DMM Board 2013

	LCD_Init();
	UART_Init();
	
	

	Backlight_LED(BL_GREEN_ON|BL_BLUE_ON|BL_RED_ON);

	
	LCD_Update();

	LCD_GotoXY(0, 0);
	LCD_PutString_P(PSTR("QR-Code generator \r\n"));
	LCD_PutString_P(PSTR("by Group 5 and \r\n"));
	LCD_PutString_P(PSTR("(c) Project Nayuki \r\n\n\n"));
	LCD_PutString_P(PSTR("Press any key to \r\n"));
	LCD_PutString_P(PSTR("continue "));
	wait_joy_button();
	LCD_Clear();
	
	LCD_GotoXY(0, 0);
	LCD_PutString_P(PSTR("Please send a message\r\n"));
	LCD_PutString_P(PSTR("via UART \r\n"));
	LCD_PutString_P(PSTR("Message must not \r\n"));
	LCD_PutString_P(PSTR("be longer than:\r\n"));
	LCD_PutString_P(PSTR("652 decimal numbers,\r\n"));
	LCD_PutString_P(PSTR("395 letters or\r\n"));
	LCD_PutString_P(PSTR("271 binary numbers\r\n"));
	LCD_PutString_P(PSTR("Enter ends a message"));
	LCD_Update();
	
	//Eingabe-UART
	int blu = 0;
	int bla = 0;
	char uart_nachricht[MAX_LENGTH_OF_TEXT];
	int uart_eingabe = UART_GetChar_Wait();
	while ( bla <MAX_LENGTH_OF_TEXT )
	{
		 uart_nachricht[bla]= '~';		// String to be encoded is filled with a character
		 ++bla;							// that is not supported determining the end of the message
		
	}
	
	while (uart_eingabe != 13)
	{
		uart_nachricht[blu] = uart_eingabe;
		++blu;
		uart_eingabe = UART_GetChar_Wait();
	}
	
	
	
	const char *text = strtok(uart_nachricht,"~");//only the input characters are passed			   
	
	
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_HIGH;
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
								   qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	
	LCD_Clear();

	printQr(qrcode);
	wait_joy_button();
	return 1;
	}
	