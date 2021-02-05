#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <time.h>
#include <conio.h>
#include "uart.h"
#include "lcd.h"
#include "twi.h"
#include "dataflash.h"
#include <stdbool.h>
#include "ecc.h"

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

//// als vektor
#define NUMBER_OF_VERSIONS 2
#define MAX_LENGTH_OF_TEXT 976

struct qr_versions
{
	int modules[NUMBER_OF_VERSIONS];
	char level[NUMBER_OF_VERSIONS];
	int databits[NUMBER_OF_VERSIONS];
	int numeric[NUMBER_OF_VERSIONS];
	int alphanumeric[NUMBER_OF_VERSIONS];
	int alphanumericBits[NUMBER_OF_VERSIONS];
	int numericBits[NUMBER_OF_VERSIONS];
}
versions = {{57,21}, {'H', 'H'}, {976,72}, {288,17}, {174,10}, {11,9}, {12,10}};

//struct qr_version version1 = {21, 'H', 72, 17, 10, 9 , 10};

//struct qr_version versions[NUMBER_OF_VERSIONS] = {{57, 'H', 976, 288, 174, 11, 12}, {21, 'H', 72, 17, 10, 9 , 10}};

struct qr_version
{
	int modules;
	char level;
	int databits;
	int numeric;
	int alphanumeric;
	int alphanumericBits;
	int numericBits;
};

void demo_start(void);
int numberOfblocks(int mode, int size);
void textvalue_to_binary(int textvalue);

//mappen
int mappen(char input)
{
	if (input == '0' || input == '1' || input == '2' || input == '3' || input == '4' || input == '5' || input == '6' || input == '7' || input == '8' || input == '9')
	{
		return (int)input - 48;
	}
	else
	{
		switch (input)
		{
		case 'A':
			return 10;
			break;
		case 'B':
			return 11;
			break;
		case 'C':
			return 12;
			break;
		case 'D':
			return 13;
			break;
		case 'E':
			return 14;
			break;
		case 'F':
			return 15;
			break;
		case 'G':
			return 16;
			break;
		case 'H':
			return 17;
			break;
		case 'I':
			return 18;
			break;
		case 'J':
			return 19;
			break;
		case 'K':
			return 20;
			break;
		case 'L':
			return 21;
			break;
		case 'M':
			return 22;
			break;
		case 'N':
			return 23;
			break;
		case 'O':
			return 24;
			break;
		case 'P':
			return 25;
			break;
		case 'Q':
			return 26;
			break;
		case 'R':
			return 27;
			break;
		case 'S':
			return 28;
			break;
		case 'T':
			return 29;
			break;
		case 'U':
			return 30;
			break;
		case 'V':
			return 31;
			break;
		case 'W':
			return 32;
			break;
		case 'X':
			return 33;
			break;
		case 'Y':
			return 34;
			break;
		case 'Z':
			return 35;
			break;
		case ' ':
			return 36;
			break;
		case '$':
			return 37;
			break;
		case '%':
			return 38;
			break;
		case '*':
			return 39;
			break;
		case '+':
			return 40;
			break;
		case '-':
			return 41;
			break;
		case '.':
			return 42;
			break;
		case '/':
			return 43;
			break;
		case ':':
			return 44;
			break;
		default:
			printf("Keine gültige Eingabe");
		}
	}
}

///// Global variabels
bool numeric = true;
int new_binary_textvalue[8];
int usedPixels[58][58]; //2D-Array for checking if the desired pixel used or not
int counter1 = 0;
int counter2 = 0;

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
	LCD_PutString_P(PSTR("by Group 5 \r\n\n\n"));
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
	LCD_PutString_P(PSTR("271 binary numbers \r\n"));
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

	/////////////// Eingabe ////////////
	char *input = strtok(uart_nachricht,"~");

	/////////// Zusätzlich Abfrage der Version
	int used_ver = 1;
	struct qr_version version;
	
		version.modules = versions.modules[used_ver];
		version.level = versions.level[used_ver];
		version.databits = versions.databits[used_ver];
		version.numeric = versions.numeric[used_ver];
		version.alphanumeric = versions.alphanumeric[used_ver];
		version.alphanumericBits = versions.alphanumericBits[used_ver];
		version.numericBits = versions.numericBits[used_ver];

	///// Füllen des structs /////

	const int totalbits = version.databits;



	char input_split[MAX_LENGTH_OF_TEXT] = "";
	//////////

//######### Select mode	

	numeric = true;
	for(int i=0; i < strlen(input); i++)
	{
		switch (input[i])
		{
			case '0':
				break;
			case '1':
				break;
			case '2':
				break;
			case '3':
				break;	
			case '4':
				break;
			case '5':
				break;
			case '6':
				break;
			case '7':
				break;
			case '8':
				break;
			case '9':
				break;					
			default:
				numeric = false;
				break;
		}
	}
	//Version 10
	//Numeric 12 Bits
	//Alphanumeric 11 Bits

	if(numeric == true)
	{	
		int mode_start[] = {0, 0, 0, 1};
		/// dezTobinary
		/////////////////////////////
		int b2inary_start[version.numericBits];
		int q = strlen(input)/2;
		int rest = strlen(input) % 2;
		int i2 = 0;
		while(q != 0)
		{ 
			int help = q;
			q = q /2;
			rest = help % 2;
			i2++; 
		}
		for (int j = 0; j < version.numericBits; j++)
		{
			if(j < version.numericBits - i2 - 1)
			{
				b2inary_start[j] = 0;
			}
			else{
				int z  = j;
				int n = version.numericBits - z - 1;
				q = strlen(input)/2;
				rest = strlen(input) % 2;
				while(n!= 0)
				{
					int help = q;
					q = q /2;
					rest = help % 2;
					n--;
				}
			b2inary_start[z] = rest;    
			}
		}
		//////////////////////////////
		//int *b2inary_start = dezTobinary(12, strlen(input));

		//Concate 0001 + 12bit
		const int startlength = 4 + version.numericBits;
		int binaryQRstart[startlength];
		for(int x = 0; x < startlength; x++)
		{
			if (x < 4)
			{
				binaryQRstart[x] = mode_start[x];
			}
			else {
				binaryQRstart[x] = b2inary_start[x - 4];
			}
		}


		//Number of Elements 16
		//Divide binarynumber in blocks of 3
		const int blocknummern = numberOfblocks(3, strlen(input));
		int blocknumbers[blocknummern];
		int *helparray;
		const int sizeofBlocks = 11*blocknummern;
		int concated11bit[sizeofBlocks];
		for(int i = 0; i < numberOfblocks(3, strlen(input)) ; i++)
		{
			char string[3] = "";
			strcat(string, input[i*3]);
			if(strlen(input) - (i*3 + 1) > 0)
			{
				strcat(string, input[i*3 + 1]);
			}
			else if(strlen(input) - (i*3 + 2) > 0)
			{
				strcat(string, input[i*3 + 2]);
			}

			blocknumbers[i] = atoi(string);	

			// dezTobinary
			//////////////////////////////
			int binary11bit[11];
			int q1 = blocknumbers[i]/2;
			int rest1 = blocknumbers[i] % 2;
			int i1 = 0;
			while(q1 != 0)
			{ 
				int help = q1;
				q1 = q1 /2;
					rest1 = help % 2;
				i1++; 
			}
			for (int j = 0; j < 11; j++)
			{
				if(j < 11 - i1 - 1)
				{
					binary11bit[j] = 0;
				}
				else{
					int z  = j;
					int n = 11 - z - 1;
					q = blocknumbers[i]/2;
					rest = blocknumbers[i] % 2;
					while(n!= 0)
					{
						int help = q;
						q = q /2;
						rest = help % 2;
						n--;
					}
					binary11bit[z] = rest;    
				}
			}
			//////////////////////////////
			//int *binary11bit = dezTobinary(11, blocknumbers[i]);
			//helparray = concateArray(helparray, binary11bit, 11*i, 11);
			for(int j = 0; j < 11; j++)
			{
				concated11bit[j + 11*i] = binary11bit[j];
			}
		}

		int binarySequence[sizeofBlocks + startlength];
		// Concate binaryBlocks and binaryQRstart
		for(int y = 0; y < sizeofBlocks + startlength; y++)
		{
			if (y < startlength)
			{
				binarySequence[y] = binaryQRstart[y];
				char* bit = binarySequence[y] + '0';
         		strcat(input_split, &bit);
			}
			else {
				binarySequence[y] = concated11bit[y - startlength];
				char* bit = binarySequence[y] + '0';
         		strcat(input_split, &bit); 
			}
		}
	}
	else {
		int mode_start[] = {0, 0, 1, 0};

		/// dezTobinary
		///////////////////////////////
		int b2inary_start[version.alphanumericBits];
		int q = strlen(input)/2;
		int rest = strlen(input) % 2;
		int i2 = 0;
		while(q != 0)
		{ 
			int help = q;
			q = q /2;
			rest = help % 2;
			i2++; 
		}
		for (int j = 0; j < version.alphanumericBits; j++)
		{
			if(j < version.alphanumericBits - i2 - 1)
			{
				b2inary_start[j] = 0;
			}
			else{
				int z  = j;
				int n = version.alphanumericBits - z - 1;
				q = strlen(input)/2;
				rest = strlen(input) % 2;
				while(n!= 0)
				{
					int help = q;
					q = q /2;
					rest = help % 2;
					n--;
				}
				b2inary_start[z] = rest;    
			}
		}
		//////////////////////////////
		//int *b2inary_start = dezTobinary(11, strlen(input));

		//Concate 0010 + 11bit
		const int startlength = 4 + version.alphanumericBits;
		int binaryQRstart[startlength];
		for(int x = 0; x < startlength; x++)
		{
			if (x < 4)
			{
				binaryQRstart[x] = mode_start[x];
			}
			else {
				binaryQRstart[x] = b2inary_start[x - 4];
			}
		}

		//Number of Elements 15
		//Divide binarynumber in blocks of 2
		const int blocknummern = numberOfblocks(2, strlen(input));
		int blocknumbers[blocknummern];
		//int helparray;
		const int sizeofBlocks = 11*blocknummern;
		int concated11bit[sizeofBlocks];
		for(int i = 0; i < numberOfblocks(2, strlen(input)) ; i++)
		{
			int a =  mappen(input[i*2]); //function from bakir		
			int b = 0;
			if(strlen(input) - (i*2 + 1) > 0)
			{
				 b = mappen(input[i*2 + 1]);
			}
			blocknumbers[i] = a * 45 + b;

			// dezTobinary
			/////////////////////////////////
			int binary11bit[11];
			int q1 = blocknumbers[i]/2;
			int rest1 = blocknumbers[i] % 2;
			int i1 = 0;
			while(q1 != 0)
			{ 
				int help = q1;
				q1 = q1 /2;
					rest1 = help % 2;
				i1++; 
			}
			for (int j = 0; j < 11; j++)
			{
				if(j < 11 - i1 - 1)
				{
					binary11bit[j] = 0;
				}
				else{
					int z  = j;
					int n = 11 - z - 1;
					q = blocknumbers[i]/2;
					rest = blocknumbers[i] % 2;
					while(n!= 0)
					{
						int help = q;
						q = q /2;
						rest = help % 2;
						n--;
					}
					binary11bit[z] = rest;    
				}
			}
			////////////////////////////////////
			//int *binary11bit = dezTobinary(11, blocknumbers[i]);

			// Concate
			//helparray = concateArray(helparray, binary11bit, 11*i, 11);
			for(int j = 0; j < 11; j++)
			{
				concated11bit[j + 11*i] = binary11bit[j];
			}

		}

		int binarySequence[sizeofBlocks + startlength];
		//int *bitsequence = concateArray(binaryQRstart, helparray, 15, sizeofBlocks);
		// Concate binaryBlocks and binaryQRstart
		for(int y = 0; y < sizeofBlocks + startlength; y++)
		{
			if (y <startlength)
			{
				binarySequence[y] = binaryQRstart[y];
				char* bit = binarySequence[y] + '0';
				//////// DEbug
				/*LCD_GotoXY(0,0);
				char beistand[] = "";
				itoa(binarySequence[y], beistand, 10);
				LCD_PutString(&input_split);
				LCD_Update();
				wait_joy_button();
				LCD_Clear();*/
				/////////////

         		strcat(input_split, &bit);
			}
			else {
				binarySequence[y] = concated11bit[y - startlength]; 
				char* bit = binarySequence[y] + '0';
				////// DEbug
				/*LCD_GotoXY(0,0);
				char beistand[] = "";
				itoa(binarySequence[y], beistand, 10);
				LCD_PutString(&input_split);
				LCD_Update();
				wait_joy_button();
				LCD_Clear();*/

				///////////////////
         		strcat(input_split, &bit);
			}
		}
	}
//######### Split in Blocks ########### Replace eingabe[] with output from selectmode
	int databits = version.databits; //databits immmer > inputbits
	int inputbits = strlen(input_split);
	int bitdif = databits - inputbits;
	char *terminator = "";
	switch (bitdif)
	{
	case 1:
		terminator = "0";
		strcat(input_split, terminator);
		break;
	case 2:
		terminator = "00";
		strcat(input_split, terminator);
		break;
	case 3:
		terminator = "000";
		strcat(input_split, terminator);
		break;
	default:
		terminator = "0000";
		strcat(input_split, terminator);
		break;
	}
	inputbits = strlen(input_split);
	int block_length = strlen(input_split) % 8;
	int missing_bits = 8 - block_length;
	for (int i = 0; i < missing_bits; i++)
	{
		char* bit = '0';
		strcat(input_split, &bit);
	}
	int maxnum_of_blocks = databits / 8;
	char special1[] = "11101100";
	char special2[] = "00010001";
	int counter = 0;
	int blocks_length = strlen(input_split);
	while ((blocks_length / 8) <= maxnum_of_blocks)
	{
		if (counter == 0)
		{
			strcat(input_split, special1);
			counter = 1;
		}
		else if (counter == 1)
		{
			strcat(input_split, special2);
			counter = 0;
		}
		blocks_length = strlen(input_split);
	}
	//char blocks[] = "111000111000001011101001010010011100100011111001000001100100010100000101001000101111110100100001101100100110011101100011100111001011000001111001100011110100011110111101000101100101111101000110010010110001110111011110001111011000110101011000100011100010101000001000110000101111010011110111100101111001011011011100101000011101111001110100001110000000001010101011000011110111100011010010010111010000100101011010110110011101000101101011100101101101110100111100111111000011001111000010110101110110101010011001000010101101001100010110100110111001101110001011001101111000010000111011011110000111000110101111101001111001011111001000010100011010100001111101001110001101011110010110100000100011100001011111011001101111010001011110110110110111001011100001010110010101010011110110110111011110010001111110110101100110101111100101011011000011001011000111010111101010111100111010001000001101101010110110100110100110011101101110100101100011100000011011000001000111100100100001011000111010111";
	Backlight_LED(BL_BLUE_ON | BL_GREEN_ON | BL_RED_ON);
	LCD_Update();
	/*
	// Print input_split
	int print_counter = 0;
	for (int j = 0; j < 9; j++)
	{
		for (int g = 0; g < 8; g++)
		{
			LCD_GotoXY(g,j);
			LCD_PutChar(input_split[print_counter]);
			LCD_Update();
			print_counter++;
		}
	}
	wait_joy_button();
	LCD_Clear();
	*/
//######### Reed Solomon Algorithm
	/// still TODO : change sizeofblocks

	int total_length = version.databits;
	
	int				iterator;	
	int				s_msg;	
	int				MM;
	int				KK;
	int				NN;
	
    
    MM = 8;					// RS code over GF(2**MM)
	KK = total_length/8;				// KK = number of information symbols. KK must be <= 2**MM - 1
	NN = KK+28;            // Message + RS_code

	int				dezimal[NN];
	bytes_to_dec  (input_split, dezimal, total_length);
	
		
		
	
	unsigned char msg[total_length/8] ;    
	for (int z =0 ; z < total_length/8; z++ )
		{
			msg[z] = dezimal[z];  
		}


    unsigned char	data[NN];    
    initialize_ecc ();
    encode_data(msg, KK, data);
     
	
    
    // adds parity bytes to &data starting with KK to NN-1
	LCD_Clear();
	
	for (iterator=(total_length/8); iterator<NN; iterator++)
	{
		dezimal[iterator] = data[iterator];
	}
	///int a = 60;
	/*
	int dezimal_counter = 0;
	int xxx = 0;
	int yyy = 0;
	for (int i = 0; i < 26; i++)
	{
		LCD_GotoXY(xxx, yyy);
		char buffer[20];
		itoa(dezimal[i],buffer,10);
		LCD_PutString(buffer);
		xxx += 4;
		if (xxx == 20)
		{
			xxx = 0;
			yyy++;
		}
		
	}
	LCD_Update();
	wait_joy_button();
	*/

//######### After adding fec to textvalues -> REPLACE textvalues[] with RS output ##################
	//int textvalues[] = {32, 37, 39, 161, 32, 236, 17, 236, 17, 208, 147, 120, 235, 20, 36, 10, 42, 73, 162, 140, 142, 217, 162, 207, 0, 62};
	int sizeof_textvalues = NN;
	const int x = sizeof_textvalues;
	int binary_textvalues[x][8];
	int textvalue;
	for (int i = 0; i < sizeof_textvalues; i++)
	{		
		textvalue = dezimal[i];
		textvalue_to_binary(textvalue);
		for(int n = 0; n < 8; n++)
		{
			binary_textvalues[i][n] = new_binary_textvalue[n];

		}
	}

//######### Position placement for QR-Code #########################
	LCD_Clear();
	//----------i --> Spalte, j --> Zeile----------//
	uint8_t i;
	uint8_t j;
	LCD_GotoXY(1, 1); //Start point
	int qr_code_size = version.modules;
	//-------------Square top left--------------//
	for(i = 1; i <= 8; i++) {
		for(j = 1; j <= 8; j++) {
			LCD_DrawPixel(i, j, 1);
			usedPixels[i][j] = 1; 
			if (i == 8 || j == 8) { 
				LCD_DrawPixel(i,j,0); //white field around the exterior black square
			}
			if(i > 1 && i < 7) { 
				if(j > 1 && j < 7) {
					LCD_DrawPixel(i,j,0); //white field around the internal black square
				}
			}
			if(i > 2 && i < 6) { 
				if(j > 2 && j < 6) {
					LCD_DrawPixel(i,j,1); //internal black square
				}	
			}
		}
	}
	//-------------Sqaure top right-------------//
	for(i = (qr_code_size - 7); i <= qr_code_size; i++) {
		for(j = 1; j <= 8; j++) {
			LCD_DrawPixel(i, j, 1);
			usedPixels[i][j] = 1; 
			if (i == (qr_code_size - 7) || j == 8) {
				LCD_DrawPixel(i,j,0); //white field around the exterior black square
			}
			
			if(i > qr_code_size - 6 && i < qr_code_size) {
				if(j > 1 && j < 7){
					LCD_DrawPixel(i,j,0); //white field around the internal black square
				}
			}
			if(i > qr_code_size - 5 && i < qr_code_size - 1) {
				if(j > 2 && j < 6) {
					LCD_DrawPixel(i,j,1); //internal black square
				}	
			}
		}
	}
	//------------Square lower left-------------//
	for(i = 1; i <= 8; i++) {
		for(j = (qr_code_size - 7); j <= qr_code_size; j++) {
			LCD_DrawPixel(i, j, 1);
			usedPixels[i][j] = 1;
			if (i == 8 || j == (qr_code_size - 7)) {
				LCD_DrawPixel(i,j,0); //white field around the exterior black square
			}
			if(i > 1 && i < 7) {
				if(j > qr_code_size - 6 && j < qr_code_size){
					LCD_DrawPixel(i,j,0); //white field around the internal black square
				}
			}
			if (i > 2 && i < 6)
			{
				if (j > qr_code_size - 5 && j < qr_code_size - 1)
				{
					LCD_DrawPixel(i, j, 1); //internal black square
				}
			}
		}
	}
	//--------Connections between Corner--------//
	for (j = 9; j <= (qr_code_size - 8); j++)
	{
		usedPixels[7][j] = 1;
		if (j % 2 != 0)
		{
			LCD_DrawPixel(7, j, 1);
		}
		else
		{
			LCD_DrawPixel(7, j, 0);
		}
	}
	for (i = 9; i <= qr_code_size - 8; i++)
	{
		usedPixels[i][7] = 1;
		if (i % 2 != 0)
		{
			LCD_DrawPixel(i, 7, 1);
		}
		else
		{
			LCD_DrawPixel(i, 7, 0);
		}
	}

	//-----------one pixel lower left-----------//
	LCD_DrawPixel(9, qr_code_size - 7, 1);
	usedPixels[9][qr_code_size - 7] = 1;

	//------------------FehlerKorrektur—Maske--------------------//
	//-------------------LVL--> H, Maske-->1---------------------//
	//---------------------001011010001001-----------------------//
	int FK_Maske[15] = {0,0,1,0,1,1,0,1,0,0,0,1,0,0,1}; //Maske H,1
	
	int counterforFK_Maske = 0; //counter for reading the FK_Maske's elements
	//from bottom to top
	for (j = qr_code_size; j >= 1; j--)
	{

		if (j <= (qr_code_size - 7) && j >= 10 || j == 7)
		{
			continue; //jump over all the elements (10-50)between the corners and over the 7th pixel in y-axis
					  //from 10 - 50 (Zeile) do nothing and execute the next j value
		}

		LCD_DrawPixel(9, j, FK_Maske[counterforFK_Maske]); //draw the value of the array(FK_Maske)
		counterforFK_Maske++;
		usedPixels[9][j] = 1;
	}
	//from left to right
	counterforFK_Maske = 0; //reset the counter for using it again
	for (i = 1; i <= qr_code_size; i++)
	{
		if (i == 7 || i >= 9 && i <= (qr_code_size - 8))
		{
			continue; //jump over 9 - 49 (Spalten) and over the 7th pixel in the x-axis
		}
		LCD_DrawPixel(i, 9, FK_Maske[counterforFK_Maske]);
		counterforFK_Maske++;
		usedPixels[i][9] = 1;
	}

	//-------------in bearbeitung-----------//
	int kX = qr_code_size;
	int kY = qr_code_size;
	
	int counter3 = 2;

	for (kX; kX >= 2; kX -= 2)
	{
		if(counter3 % 2 == 0){
		for (int kYnew = kY; kYnew >= 1; kYnew--)
		{

			int kXnew = kX;

			if (usedPixels[kXnew][kYnew] == 1)
			{

			}
			else
			{
				LCD_DrawPixel(kXnew, kYnew, binary_textvalues[counter1][counter2]);

				if ((kYnew + kXnew) % 2 == 0)
				{
					if (binary_textvalues[counter1][counter2] == 1)
					{
						LCD_DrawPixel(kXnew, kYnew, 0);
					}
					else
					{
						LCD_DrawPixel(kXnew, kYnew, 1);
					}
				}
				counter2++;
				if(counter2 == 8)
				{
					counter2 = 0;
					counter1++;
				}
				if (counter1 == sizeof_textvalues)
				{
					counter1 = 0;
					counter2 = 0;
				}
				}
				kXnew--; // Nächste x in der gleichen Zeile

				if(usedPixels[kXnew][kYnew] == 1) {
				
				}
			    else {
					
			    LCD_DrawPixel(kXnew, kYnew, binary_textvalues[counter1][counter2]);
				if((kYnew + kXnew) % 2 == 0) {
                    if(binary_textvalues[counter1][counter2] == 1) {
                        LCD_DrawPixel(kXnew, kYnew, 0);
                    } else {
                        LCD_DrawPixel(kXnew, kYnew, 1);
                    }
				}
				counter2++;
				if (counter2 == 8)
				{
					counter2 = 0;
					counter1++;
				}
				if (counter1 == sizeof_textvalues)
				{
					counter1 = 0;
					counter2 = 0;
				}
			}
		}
		} else {
		for (int kYnew = 1; kYnew <= kY; kYnew++)
		{

			int kXnew = kX;

			if (usedPixels[kXnew][kYnew] == 1)
			{

			}
			else
			{
				LCD_DrawPixel(kXnew, kYnew, binary_textvalues[counter1][counter2]);

				if ((kYnew + kXnew) % 2 == 0)
				{
					if (binary_textvalues[counter1][counter2] == 1)
					{
						LCD_DrawPixel(kXnew, kYnew, 0);
					}
					else
					{
						LCD_DrawPixel(kXnew, kYnew, 1);
					}
				}
				counter2++;
				if (counter2 == 8)
				{
					counter2 = 0;
					counter1++;
				}
				if (counter1 == sizeof_textvalues)
				{
					counter1 = 0;
					counter2 = 0;
				}
			}
			kXnew--; // Nächste x in der gleichen Zeile

			if (usedPixels[kXnew][kYnew] == 1)
			{
			}
			else
			{
				LCD_DrawPixel(kXnew, kYnew, binary_textvalues[counter1][counter2]);
				if ((kYnew + kXnew) % 2 == 0)
				{
					if (binary_textvalues[counter1][counter2] == 1)
					{
						LCD_DrawPixel(kXnew, kYnew, 0);
					}
					else
					{
						LCD_DrawPixel(kXnew, kYnew, 1);
					}
				}
				counter2++;
				if (counter2 == 8)
				{
					counter2 = 0;
					counter1++;
				}
				if (counter1 == sizeof_textvalues)
				{
					counter1 = 0;
					counter2 = 0;
				}
			}
		}
		}
		counter3++;
	}

	if (kX == 1)
	{
		for (kY = qr_code_size; kY >= 1; kY--)
		{
			if (usedPixels[1][kY] == 1)
			{
				continue;
			}

			LCD_DrawPixel(1, kY, binary_textvalues[counter1][counter2]);
			if ((kY + 1) % 2 == 0)
			{
				if (binary_textvalues[counter1][counter2] == 1)
				{
					LCD_DrawPixel(1, kY, 0);
				}
				else
				{
					LCD_DrawPixel(1, kY, 1);
				}
			}
			counter2++;
			if (counter2 == 8)
			{
				counter2 = 0;
				counter1++;
			}
			if (counter1 == sizeof_textvalues)
			{
				counter1 = 0;
				counter2 = 0;
			}
		}
	}

	LCD_Update();
	wait_joy_button();

	// loop forever
	while (1)
		;
}


int numberOfblocks(int mode, int size)
{
	int numberOfblocks = 0;
	if (size % mode == 0)
	{
		numberOfblocks = size / mode;
	}
	else
	{
		numberOfblocks = size / mode + 1;
	}
	return numberOfblocks;
}

void bytes_to_dec(char block[], int dezimal[], int size)
{
	int faktor[size];
	int ascii;

	for (int x = 0; x < size; x++)
	{
		ascii = block[x];
		faktor[x] = ascii - 48;
	}
	for (int x = 0; x < size / 8; x++)
	{
		dezimal[x] = 128 * faktor[0 + 8 * x] + 64 * faktor[1 + 8 * x] + 32 * faktor[2 + 8 * x] + 16 * faktor[3 + 8 * x] + 8 * faktor[4 + 8 * x] + 4 * faktor[5 + 8 * x] + 2 * faktor[6 + 8 * x] + 1 * faktor[7 + 8 * x];
	}
}

void textvalue_to_binary(int textvalue)
{
	int dezimalzahl = textvalue;
	int binary_textvalue[8];
	int i;
	for (i = 0; dezimalzahl > 0; i++)
	{
		binary_textvalue[i] = dezimalzahl % 2;
		dezimalzahl = dezimalzahl / 2;
	}
	while (i < 8)
	{
		binary_textvalue[i] = 0;
		i++;
	}
	int counter = 0;
	for (; i > 0; i--)
	{
		new_binary_textvalue[counter] = binary_textvalue[i - 1];
		counter++;
	}
}
