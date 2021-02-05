/********************************************************
 Author:					Manfred Langemann
 mailto:					Manfred.Langemann �t t-online.de
 Begin of project:			17.08.2012
 Latest version generated:	21.08.2012
 Filename:					Reed-Solomon.c
 Description:    			Main Programme
 Compiled with:				Visual Studio 2010

 Copyright (C) 2012 by Manfred Langemann

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

 ********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "Reed-Solomon.h"
/*
** These are declarations for private function
** Declaration for public functions are in Reed-Solomon.h
*/
int SetPP_RS (int MM, RS_Co_t* RS_Co);
int modnn (int x, int NN, int MM);
/*******************************************************************************
	Functions to be used for Reed-Solomon encoding and decoding are:

	Public functions:

		int Init_RS (int MM, int KK, RS_Co_t* RS_Co);
		void Clear_RS (RS_Co_t* RS_Co);
		int Encode_RS (unsigned char* data, unsigned char* bb,  RS_Co_t* RS_Co);
		int Decode_RS (unsigned char* data, unsigned char* eras_pos, int num_eras,  RS_Co_t* RS_Co);

		Typical application form:

			- Begin of program
			// Define the RS coding parameters MM and KK
			Init_RS (int MM, int KK, RS_Co_t* RS_Co);
			// Define a message in array 'data'
			Encode_RS (unsigned char* data, unsigned char* bb,  RS_Co_t* RS_Co);
			// Simulate here the erroneous transmission chain by introducing errors and erasures
			// Decode the erroneous message
			Decode_RS (unsigned char* data, unsigned char* eras_pos, int num_eras,  RS_Co_t* RS_Co);
			// Compare decoded message with original one --> should be identical
			:
			: Do this n times
			:
			// Define a message in array 'data'			
			Encode_RS (unsigned char* data, unsigned char* bb,  RS_Co_t* RS_Co);
			// Simulate here the erroneous transmission chain by introducing errors and erasures
			// Decode the erroneous message
			Decode_RS (unsigned char* data, unsigned char* eras_pos, int num_eras,  RS_Co_t* RS_Co);
			// Compare decoded message with original one --> should be identical
			// Clear created RS memory arrays
			Clear_RS (RS_Co_t* RS_Co);
			- End of program

		The parameters MM and KK specify the Reed-Solomon code parameters.
 
		Set MM to be the size of each code symbol in bits. The Reed-Solomon
		block size will then be NN = 2**M - 1 symbols. Supported values are 2 to 8
 
		Set KK to be the number of data symbols in each block, which must be
		less than the block size. The code will then be able to correct up
		to NN-KK erasures or (NN-KK)/2 errors, or combinations thereof with
		each error counting as two erasures.

	Private functions:

		int SetPP_RS (int MM, RS_Co_t* RS_Co);
		int modnn (int x, int NN, int MM);

--------------------------------------------------------------------------------
	A typical software example in C is:
--------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "Reed-Solomon.h"

int main (void)
	{
	int i;
	unsigned char msg[] =
"This is a test message for the Reed-Solomon (RS) encoder and decoder \
programm. This message will be RS encoded and transmitted. On transmission \
random noise errors occure, to be corrected by the RS decoder. \
This message is 239 chars long.";
	unsigned char data[255];
	unsigned char eras_pos[10];
	int eras_num;
	int MM;
	int KK;
	int	NN;
	RS_Co_t	RS_Co; // structure is defined in Reed-Solomon.h

// ** Set the RS(255, 239) coding parameters
	MM = 8;
	KK = 239;
	NN = (1 << MM) - 1;

	printf ("Implemented Forward Error Correction is:\n");
	printf (" - Reed Solomon (%d, %d)\n", NN, KK);
	printf (" - Number of bits per RS symbol = %d\n", MM);
	printf (" - Can correct %d erasures or %d errors,\n", NN-KK, (NN-KK)/2);
	printf ("   or combinations thereof with each error counting as two erasures\n\n");

// ** Initiate the RS coding parameters
	if (Init_RS (MM, KK, &RS_Co) != 0) return -1;
// ** Copy the message to an intermin buffer
	for (i=0;i<sizeof(msg);i++)
		{
		data[i] = msg[i];
		}
	printf ("The message is:\n\"%s\"\n", data);
// ** Encode the message
	Encode_RS (&data[0], &data[KK], &RS_Co);
// ** Now transmit the data.
// ** On transmission, errors occure. We simulate here 6 errors
// ** For simplicity we do not introduce here erasures (eras_num = 0)
	printf ("\nIntroducing 6 arbitrary erorrs in the message...\n", data);
	data[10] = 221;
	data[50] = 85;
	data[155] = 137;
	data[180] = 95;
	data[210] = 195;
	data[222] = 114;
	printf ("\nThe erroneous message is:\n\"%s\"\n", data);
// ** We 'receive' the erroneous message in the array data[255] and then decode it.
// ** We set eras_num=0, because we have not introduced erasures
	eras_num = 0;
	i = Decode_RS (&data[0], &eras_pos[0], eras_num,  &RS_Co);
// ** Free the allocated memories in the RS_Co structure
	Clear_RS (&RS_Co);
// ** Now the original message should be in the data buffer data[].
	data[KK] = 0; // A terminating NULL for the string, because destroyed by RS code words
	printf ("\nThe RS corrected message is:\n\"%s\"\n", data);
	printf ("\nPress any key to terminate the program ...\n");
	while (!_kbhit()) {}
	return 0;
	}
--------------------------------------------------------------------------------
*/
/*******************************************************************************
	Public Function:   Init_RS

	This function creates all RS parameters, as needed for the RS encoding and decoding.
	The function stores the created parmeters in the structure RS_Co for later use.
	During encoding and decoding by the functions Encode_RS and Decode_RS these
	parameters will not be changed.

	The function expects two input parameters for the definition of the RS code to be used.
	These are:
	- MM = Degree of Primitiv polynomial {2,...,8}, which is the RS code over GF(2**MM)
	- KK = Number of message bytes

	From MM we can derive the overall message+code length, which is NN = 1 << MM.

	These parameters define the Forward Error Correction (FEC) capability in terms of
	how many bytes can be corrected by and the number of bytes holding the information
	rate to be transmitted.
	
	As an example, with MM = 8 and KK = 239 we obtain the RS(255,239) code, allowing
	16 FEC words per 255 bytes to be transmitted. In this example we can correct
	16 (NN-KK) erasures or 8 errors (NN-KK)/2 or combinations thereof with each
	error counting as two erasures. See also:

		Consultative Committee for Space Data Systems (CCSDS)
		RECOMMENDATION FOR SPACE DATA SYSTEM STANDARDS
		TM SPACE DATA LINK PROTOCOL
		CCSDS 132.0-B-1
		BLUE BOOK
		September 2003
		http://public.ccsds.org/publications/BlueBooks.aspx

	For this example, the structure of the message to be transmitted is as follows:

	Byte position in array 
	0 1 2 3 .......... 238 239 240 ..... 255
	<--------------------> <--------------->
	 The orig. information	The addional
	 to be transmitted		RS code as
							created by
							Program

	In this case we can transmit messages with a length of 239 bytes followed
	by a code word length of 16 bytes, makes in total 255 bytes.

	This function shall be call once before the first use of the function Encode_RS,
	followed by Decode_RS. Don't forget to call the function Clear_RS at the end of the programm to
	delete all the allocated memories, as created within the function Init_RS.

	Between repeated calls of Encode_RS do not call the Init_RS function.
----------------------------------------------------------------------------------------

	Input parameter:

	- int		MM			MM = Degree of Primitiv polynomial {2,...,8},
							which is the RS code over GF(2**MM)
	- int		KK			KK = Number of message bytes
	- RS_Co_t	RS_Co		The RS code structure, created in RS_Init

	Output parameter:	None
	
	Return parameter:
	- int		 = 0	Function performed correctly
				 > 0	Error in function
*******************************************************************************/
int Init_RS (int MM, int KK, RS_Co_t* RS_Co, int NN)
	{
	int		i, j, mask;
/*
** Ckeck that MMx=RS code over GF(2**MM) has the correct value between 2 to 8
*/
	if (MM <2 || MM >8) return 1;
/*
** Check that KK=number of information symbols is <= 2**MM - 1
*/
	if (KK >= (1 << MM) - 1) return 2;
/*
** Fill the RS_Coef structure with the correct values
*/
	RS_Co->A0 = (1 << MM) - 1;
	RS_Co->B0 = 1;
	RS_Co->KK = KK;
	RS_Co->MM = MM;
	RS_Co->NN = NN;
/*
** Allocate sufficient memory for index computations
*/
	RS_Co->Alpha_to = (unsigned char*) malloc (sizeof (unsigned char) * (RS_Co->NN + 1));
	RS_Co->Index_of = (unsigned char*) malloc (sizeof (unsigned char) * (RS_Co->NN + 1));
	RS_Co->Gg       = (unsigned char*) malloc (sizeof (unsigned char) * (RS_Co->NN - RS_Co->KK + 1));
	RS_Co->Pp       = (unsigned char*) malloc (sizeof (unsigned char) * (RS_Co->MM + 1));
/*
** Create the Primitive polynomials in the structure array RS_Co->Pp
*/
	SetPP_RS (MM, RS_Co);
/*
** ====================================================================
   Now do the following:
   
   Generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**m)
   HARI's COMMENT: (4/13/94) alpha_to[] can be used as follows:
   
   Let @ represent the primitive element commonly called "alpha" that
   is the root of the primitive polynomial p(x). Then in GF(2^m), for any
   0 <= i <= 2^m-2,
        @^i = a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   where the binary vector (a(0),a(1),a(2),...,a(m-1)) is the representation
   of the integer "alpha_to[i]" with a(0) being the LSB and a(m-1) the MSB. Thus for
   example the polynomial representation of @^5 would be given by the binary
   representation of the integer "alpha_to[5]".
   
   Similarily, index_of[] can be used as follows:
   As above, let @ represent the primitive element of GF(2^m) that is
   the root of the primitive polynomial p(x). In order to find the power
   of @ (alpha) that has the polynomial representation
   a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   
   We consider the integer "i" whose binary representation with a(0) being LSB
   and a(m-1) MSB is (a(0),a(1),...,a(m-1)) and locate the entry
   "index_of[i]". Now, @^index_of[i] is that element whose polynomial 
    representation is (a(0),a(1),a(2),...,a(m-1)).
   
   NOTE:
   The element alpha_to[2^m-1] = 0 always signifying that the
   representation of "@^infinity" = 0 is (0,0,0,...,0).
   
   Similarily, the element index_of[0] = A0 always signifying
   that the power of alpha which has the polynomial representation
   (0,0,...,0) is "infinity".
** ====================================================================
*/
	mask = 1;
	RS_Co->Alpha_to[MM] = 0;
	for (i = 0; i < RS_Co->MM; i++)
		{
		RS_Co->Alpha_to[i] = mask;
		RS_Co->Index_of[RS_Co->Alpha_to[i]] = i;
/*
** If Pp[i] == 1 then, term @^i occurs in poly-repr of @^MM
*/
		if (RS_Co->Pp[i] != 0)
			RS_Co->Alpha_to[RS_Co->MM] ^= mask;	/* Bit-wise EXOR operation */
		mask <<= 1;	/* single left-shift */
		}
	RS_Co->Index_of[RS_Co->Alpha_to[RS_Co->MM]] = RS_Co->MM;
/*
** Have obtained poly-repr of @^MM. Poly-repr of @^(i+1) is given by
** poly-repr of @^i shifted left one-bit and accounting for any @^MM
** term that may occur when poly-repr of @^i is shifted.
*/
	mask >>= 1;
	for (i = RS_Co->MM + 1; i < RS_Co->NN; i++)
		{
		if (RS_Co->Alpha_to[i - 1] >= mask)
			RS_Co->Alpha_to[i] = RS_Co->Alpha_to[RS_Co->MM] ^ ((RS_Co->Alpha_to[i - 1] ^ mask) << 1);
		else
			RS_Co->Alpha_to[i] = RS_Co->Alpha_to[i - 1] << 1;
		RS_Co->Index_of[RS_Co->Alpha_to[i]] = i;
		}
	RS_Co->Index_of[0] = RS_Co->A0;
	RS_Co->Alpha_to[RS_Co->NN] = 0;
/*
** ====================================================================
**	Now do the following:
**	Obtain the generator polynomial of the TT-error correcting, length
**	NN=(2**MM -1) Reed Solomon code from the product of (X+@**(B0+i)), i = 0,
**	... ,(2*TT-1)
** 
**	Examples:
** 
**	If B0 = 1, TT = 1. deg(g(x)) = 2*TT = 2.
**	g(x) = (x+@) (x+@**2)
**
**	If B0 = 0, TT = 2. deg(g(x)) = 2*TT = 4.
**	g(x) = (x+1) (x+@) (x+@**2) (x+@**3)
*/
	RS_Co->Gg[0] = RS_Co->Alpha_to[RS_Co->B0];
/*
**	g(x) = (X+@**B0) initially
** ====================================================================
*/
	RS_Co->Gg[1] = 1;		

	for (i = 2; i <= RS_Co->NN - RS_Co->KK; i++)
		{
		RS_Co->Gg[i] = 1;
/*
**	Below multiply (Gg[0]+Gg[1]*x + ... +Gg[i]x^i) by
**	(@**(B0+i-1) + x)
*/
		for (j = i - 1; j > 0; j--)
			{
			if (RS_Co->Gg[j] != 0)
				RS_Co->Gg[j] = RS_Co->Gg[j - 1] ^ RS_Co->Alpha_to[modnn((RS_Co->Index_of[RS_Co->Gg[j]]) + RS_Co->B0 + i - 1, RS_Co->NN, RS_Co->MM)];
			else
				RS_Co->Gg[j] = RS_Co->Gg[j - 1];
			}
/*
**	Gg[0] can never be zero
*/
		RS_Co->Gg[0] = RS_Co->Alpha_to[modnn((RS_Co->Index_of[RS_Co->Gg[0]]) + RS_Co->B0 + i - 1, RS_Co->NN, RS_Co->MM)];
		}
/*
**	Convert Gg[] to index form for quicker encoding
*/
	for (i = 0; i <= RS_Co->NN - RS_Co->KK; i++)
		{
		RS_Co->Gg[i] = RS_Co->Index_of[RS_Co->Gg[i]];
		}

	return 0;
	}
/*******************************************************************************
	Public Function:   Clear_RS

	This function clears all allocated memories in the RS_Co structure.
	To be called, before termination of program.

	Input parameter:

	- RS_Co_t	RS_Co		The RS code structure, created in RS_Init

	Output parameter:	None
	
	Return parameter:	None

*******************************************************************************/
void Clear_RS (RS_Co_t* RS_Co)
	{
	if (RS_Co->Alpha_to != NULL) free (RS_Co->Alpha_to) ;
	if (RS_Co->Index_of != NULL) free (RS_Co->Index_of) ;
	if (RS_Co->Gg != NULL) free (RS_Co->Gg) ;
	if (RS_Co->Pp != NULL) free (RS_Co->Pp) ;

	return;
	}
/*******************************************************************************
	Public Function:   Encode_RS

	This function encodes the message to be transmitted according the
	RS parameters, as defined and created in the Init_RS function

	Take the string of symbols in data[i], i=0..(k-1) and encode
	systematically to produce NN-KK parity symbols in bb[0]..bb[NN-KK-1] data[]
	is input and bb[] is output in polynomial form. Encoding is done by using
	a feedback shift register with appropriate connections specified by the
	elements of Gg[], which was generated above. Codeword is
	c(X) = data(X)*X**(NN-KK)+ b(X)
 
 ------------------------------------------------------------------------------- 
	Input parameter:
	- unsigned char		data[(1<<NN)]		Pointer to buffer holding the information to be encoded
	- unsigned char		bb[(1<<NN)-KK]		Pointer to buffer where RS correction code shall
											be written, typically the next byte after
											the information to be transmitted
	- RS_Co_t			RS_Co				The RS code structure, created in RS_Init

	Output parameter:
	- unsigned char		data[(1<<NN)]		Here the RS code information is stored
	- unsigned char		bb[(1<<NN)-KK]		Here the function write the RS correction code

	Return parameter:
	- int		 = 0	Function performed correctly
				 > 0	Error in function
*******************************************************************************/
int Encode_RS (unsigned char* data, unsigned char* bb,  RS_Co_t* RS_Co)
	{
	int		i, j;
	int		feedback;

	for (i = RS_Co->NN-RS_Co->KK-1; i >= 0; i--)
		{
		bb[i] = 0;
		}

	for (i = RS_Co->KK - 1; i >= 0; i--)
		{
/*
** Check on correct main RS parameters
*/
		if (RS_Co->MM != 8)
			{
			if (data[i] > RS_Co->NN)
			return 1;	/* Illegal symbol */
			}

		feedback = RS_Co->Index_of[data[i] ^ bb[RS_Co->NN - RS_Co->KK - 1]];
		if (feedback != RS_Co->A0) /* feedback term is non-zero */
			{	
			for (j = RS_Co->NN - RS_Co->KK - 1; j > 0; j--)
				{
				if (RS_Co->Gg[j] != RS_Co->A0)
					bb[j] = bb[j - 1] ^ RS_Co->Alpha_to[modnn(RS_Co->Gg[j] + feedback, RS_Co->NN, RS_Co->MM)];
				else
					bb[j] = bb[j - 1];
				}
			bb[0] = RS_Co->Alpha_to[modnn(RS_Co->Gg[0] + feedback, RS_Co->NN, RS_Co->MM)];
			}
		else
			{
/*
** Feedback term is zero. encoder becomes a single-byte shifter
*/
			for (j = RS_Co->NN - RS_Co->KK - 1; j > 0; j--)
				{
				bb[j] = bb[j - 1];
				}
			bb[0] = 0;
			}
		}

	return 0;
	}
/*******************************************************************************
	Public Function:   Decode_RS

	This function decodes the erroneous message received after transmission
	according the RS parameters, as defined and created in the Init_RS function

	Performs ERRORS+ERASURES decoding of RS codes. If decoding is successful,
	writes the codeword into data[] itself. Otherwise data[] is unaltered.

	Return number of symbols corrected, or -1 if codeword is illegal
	or uncorrectable.
 
	First "num_eras" erasures are declared by the calling program. Then, the
	maximum # of errors correctable is floor((NN-KK-num_eras)/2).
	
	If the number of channel errors is not greater than "t_after_eras" the
	transmitted codeword will be recovered. Details of algorithm can be found
	in R. Blahut's "Theory ... of Error-Correcting Codes".
 
 ------------------------------------------------------------------------------- 
	Input parameter:
	- unsigned char		data[(1<<NN)]		Pointer to buffer holding the received
											and erroneous information
	- unsigned char		eras_pos[]			Pointer to buffer where RS correction code shall
											be written, typically the next byte after
											the information to be transmitted
	- int				num_eras			Number of erasure positions
	- RS_Co_t			RS_Co				The RS code structure, created in RS_Init

	Output parameter:
	- unsigned char		data[(1<<NN)]		Here the RS decoded information will be strored

	Return parameter:
	- int				>= 0				Number of detected and corrected information symbols (bytes)
						< 0					Error in function
*******************************************************************************/
int Decode_RS (unsigned char* data, unsigned char* eras_pos, int num_eras,  RS_Co_t* RS_Co)
	{
	int				deg_lambda, el, deg_omega;
	int				i, j, r;
	int				u, q, tmp, num1, num2, den, discr_r;
	int				syn_error, count;

	unsigned char	*recd;				// int recd[NN];
	unsigned char	*lambda, *s;		// int lambda[NN-KK + 1], s[NN-KK + 1];
										// Err+Eras Locator poly and syndrome poly
	unsigned char	*b, *t, *omega;		// int b[NN-KK + 1], t[NN-KK + 1], omega[NN-KK + 1];
	unsigned char	*root, *reg, *loc;	// root[NN-KK], reg[NN-KK + 1], loc[NN-KK];
/*
** Allocate sufficient memory for all arrays needed
*/
	recd =		(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN);
	lambda =	(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	s =			(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	b =			(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	t =			(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	omega =		(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	root =		(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK);
	reg =		(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK + 1);
	loc =		(unsigned char*) malloc (sizeof (unsigned char) * RS_Co->NN-RS_Co->KK);
/*
** data[] is in polynomial form, copy and convert to index form
*/
	for (i = RS_Co->NN-1; i >= 0; i--)
		{
		if (RS_Co->MM != 8)
			{
/*
** Illegal symbol
*/
			if (data[i] > RS_Co->NN)
				{
				count = -1;
				goto Ret;
				}
			}
		recd[i] = RS_Co->Index_of[data[i]];
		}
/*
** First form the syndromes; i.e., evaluate recd(x) at roots of g(x)
** namely @**(B0+i), i = 0, ... ,(NN-KK-1)
*/
	syn_error = 0;
	for (i = 1; i <= RS_Co->NN-RS_Co->KK; i++)
		{
		tmp = 0;
		for (j = 0; j < RS_Co->NN; j++)
			{
			if (recd[j] != RS_Co->A0)	/* recd[j] in index form */
				tmp ^= RS_Co->Alpha_to[modnn(recd[j] + (RS_Co->B0+i-1)*j, RS_Co->NN, RS_Co->MM)];
			}
/*
** Set flag if non-zero syndrome => * error
*/
		syn_error |= tmp;	
/*
** Store syndrome in index form 
*/
		s[i] = RS_Co->Index_of[tmp];
		}

	if (!syn_error)
		{
/*
** If syndrome is zero, data[] is a codeword and there are no
** errors to correct. So return data[] unmodified
*/
		printf ("No errors found\n");
		count = 0;
		goto Ret;
		}

	for (i=RS_Co->NN-RS_Co->KK-1; i>=0; i--)
		{
		lambda[1+i] = 0;
		}

	lambda[0] = 1;
	if (num_eras > 0)
		{
/*
** Init lambda to be the erasure locator polynomial
*/
		lambda[1] = RS_Co->Alpha_to[eras_pos[0]];
		for (i = 1; i < num_eras; i++)
			{
			u = eras_pos[i];
			for (j = i+1; j > 0; j--)
				{
				tmp = RS_Co->Index_of[lambda[j - 1]];
				if(tmp != RS_Co->A0)
					{
					lambda[j] ^= RS_Co->Alpha_to[modnn(u + tmp, RS_Co->NN, RS_Co->MM)];
					}
				}
			}
#ifdef DEBUG
/* 
** Find roots of the erasure location polynomial
*/
		for(i=1;i<=num_eras;i++)
			{
			reg[i] = RS_Co->Index_of[lambda[i]];
			}
		count = 0;
		for (i = 1; i <= RS_Co->NN; i++)
			{
			q = 1;
			for (j = 1; j <= num_eras; j++)
				{
				if (reg[j] != RS_Co->A0)
					{
					reg[j] = modnn(reg[j] + j, RS_Co->NN, RS_Co->MM);
					q ^= RS_Co->Alpha_to[reg[j]];
					}
				}
			if (!q) 
				{
/* 
** Store root and error location number indices
*/
				root[count] = i;
				loc[count] = RS_Co->NN - i;
				count++;
				}
			}
		if (count != num_eras)
			{
			printf("\n lambda(x) is WRONG\n");
			count = -2;
			goto Ret;
			}

		printf("\nErasure positions as determined by Reed-Solomon (roots of Eras Loc Poly):\n");
		for (i = 0; i < count; i++)
			{
			printf("%d ", loc[i]);
			}
		printf("\n");

#endif
	}
	for(i=0;i<RS_Co->NN-RS_Co->KK+1;i++)
		{
		b[i] = RS_Co->Index_of[lambda[i]];
		}
/*
** Begin Berlekamp-Massey algorithm to determine error+erasure
** locator polynomial
*/
	r = num_eras;
	el = num_eras;
	while (++r <= RS_Co->NN-RS_Co->KK)
		{
/* 
** r is the step number
** Compute discrepancy at the r-th step in poly-form
*/
		discr_r = 0;
		for (i = 0; i < r; i++)
			{
			if ((lambda[i] != 0) && (s[r - i] != RS_Co->A0))
				{
				discr_r ^= RS_Co->Alpha_to[modnn(RS_Co->Index_of[lambda[i]] + s[r - i], RS_Co->NN, RS_Co->MM)];
				}
			}
		discr_r = RS_Co->Index_of[discr_r];	/* Index form */
		if (discr_r == RS_Co->A0)
			{
/*
** 2 lines below: B(x) <-- x*B(x)
*/
			for (i=RS_Co->NN-RS_Co->KK-1; i>=0; i--)
				{
				b[1+i] = b[i];
				}
			b[0] = RS_Co->A0;
			}
		else 
			{
/*
** 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x)
*/
			t[0] = lambda[0];
			for (i = 0 ; i < RS_Co->NN-RS_Co->KK; i++)
				{
				if(b[i] != RS_Co->A0)
					{
					t[i+1] = lambda[i+1] ^ RS_Co->Alpha_to[modnn(discr_r + b[i], RS_Co->NN, RS_Co->MM)];
					}
				else
					{
					t[i+1] = lambda[i+1];
					}
				}

			if (2 * el <= r + num_eras - 1)
				{
				el = r + num_eras - el;
/*
** 2 lines below: B(x) <-- inv(discr_r) * lambda(x)
*/
				for (i = 0; i <= RS_Co->NN-RS_Co->KK; i++)
					{
					b[i] = (lambda[i] == 0) ? RS_Co->A0 : modnn(RS_Co->Index_of[lambda[i]] - discr_r + RS_Co->NN, RS_Co->NN, RS_Co->MM);
					}
				}
			else
				{
/* 
** 2 lines below: B(x) <-- x*B(x)
*/
				for (i=RS_Co->NN-RS_Co->KK-1; i>=0; i--)
					{
					b[1+i] = b[i];
					}
				b[0] = RS_Co->A0;
				}

			for (i=RS_Co->NN-RS_Co->KK+1-1; i>=0; i--)
				{
				lambda[i] = t[i];
				}
			}
		}
/*
** Convert lambda to index form and compute deg(lambda(x))
*/
	deg_lambda = 0;
	for (i=0;i<RS_Co->NN-RS_Co->KK+1;i++)
		{
		lambda[i] = RS_Co->Index_of[lambda[i]];
		if(lambda[i] != RS_Co->A0)
			deg_lambda = i;
		}
/*
** Find roots of the error+erasure locator polynomial. By Chien
*/
	for (i=RS_Co->NN-RS_Co->KK+1-1; i>=0; i--)
		{
		reg[i] = lambda[i];
		}
/*
** Number of roots of lambda(x) 
*/
	count = 0;

	for (i = 1; i <= RS_Co->NN; i++)
		{
		q = 1;
		for (j = deg_lambda; j > 0; j--)
			{
			if (reg[j] != RS_Co->A0)
				{
				reg[j] = modnn(reg[j] + j, RS_Co->NN, RS_Co->MM);
				q ^= RS_Co->Alpha_to[reg[j]];
				}
			}
		if (!q)
			{
/* 
* Store root (index-form) and error location number
*/
			root[count] = i;
			loc[count] = RS_Co->NN - i;
			count++;
			}
		}

#ifdef DEBUG
	printf("\nNumber of errors and erasures found in RS_decode(): %d\n", count);
	printf("Final error and erasure positions:\t");
	for (i = 0; i < count; i++)
		{
		printf("%d ", loc[i]);
		}
	printf("\n");
#endif

	if (deg_lambda != count)
		{
/*
** deg(lambda) unequal to number of roots => uncorrectable
** error detected
*/
		count = -3;
		goto Ret;
		}
/*
** Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
** x**(NN-KK)). in index form. Also find deg(omega).
*/
	deg_omega = 0;
	for (i = 0; i < RS_Co->NN-RS_Co->KK;i++)
		{
		tmp = 0;
		j = (deg_lambda < i) ? deg_lambda : i;
		for(;j >= 0; j--)
			{
			if ((s[i + 1 - j] != RS_Co->A0) && (lambda[j] != RS_Co->A0))
				tmp ^= RS_Co->Alpha_to[modnn(s[i + 1 - j] + lambda[j], RS_Co->NN, RS_Co->MM)];
			}
		if(tmp != 0)
			deg_omega = i;
		omega[i] = RS_Co->Index_of[tmp];
		}
	omega[RS_Co->NN-RS_Co->KK] = RS_Co->A0;

/*
** Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
** inv(X(l))**(B0-1) and den = lambda_pr(inv(X(l))) all in poly-form
*/
	for (j = count-1; j >=0; j--)
		{
		num1 = 0;
		for (i = deg_omega; i >= 0; i--)
			{
			if (omega[i] != RS_Co->A0)
				num1  ^= RS_Co->Alpha_to[modnn(omega[i] + i * root[j], RS_Co->NN, RS_Co->MM)];
			}
		num2 = RS_Co->Alpha_to[modnn(root[j] * (RS_Co->B0 - 1) + RS_Co->NN, RS_Co->NN, RS_Co->MM)];
		den = 0;

/*
** lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i]
*/
		for (i = min(deg_lambda,RS_Co->NN-RS_Co->KK-1) & ~1; i >= 0; i -=2)
			{
			if(lambda[i+1] != RS_Co->A0)
				den ^= RS_Co->Alpha_to[modnn(lambda[i+1] + i * root[j], RS_Co->NN, RS_Co->MM)];
			}
		if (den == 0)
			{
#ifdef DEBUG
			printf("\n ERROR: denominator = 0\n");
#endif
			count = -4;
			goto Ret;
			}
/*
** Apply error to data
*/
		if (num1 != 0)
			{
			data[loc[j]] ^= RS_Co->Alpha_to[modnn(RS_Co->Index_of[num1] + RS_Co->Index_of[num2] + RS_Co->NN - RS_Co->Index_of[den], RS_Co->NN, RS_Co->MM)];
			}
		}
Ret:
	free (recd);
	free (lambda);
	free (s);
	free (b);
	free (t);
	free (omega);
	free (root);
	free (reg);
	free (loc);

	return count;
	}
/*******************************************************************************
	Private Function:	SetPP_RS

	This function will create the Premitive Polynomial (PP) for the
	specified (MM) degree of the polynomial.

	Th value of MM shall be between 2 and 8, otherwise the function will return an error.
	The defined PP_const values are specified by see Lin & Costello, Appendix A,
	and Lee & Messerschmitt, p. 453.
	
	It shall be noted, that the PP of degree MM=8 is compatible with the CCSDS
	standard, when selecting KK=239 (see function Init_RS). I.e. the RS coding
	scheme is RS(255,239), allowing 16 FEC words per 255 bytes to be transmitted.
	This is the CCSDS field generator polynomial F(x) for GF(2**8).
	
	This RS(255,239) code can correct 16 (NN-KK) erasures or 8 errors (NN-KK)/2 or
	combinations thereof with each error counting as two erasures.

	Below the valid PP as function of degree MM are listed for completeness:

	MM == 2	 -->	1 + x + x^2
					RS_Co->Pp[MM+1] = { 1, 1, 1 };
	MM == 3	 -->	1 + x + x^3
					RS_Co->Pp[MM+1] = { 1, 1, 0, 1 };
	MM == 4 -->		1 + x + x^4
					RS_Co->Pp[MM+1] = { 1, 1, 0, 0, 1 };
	MM == 5	-->		1 + x^2 + x^5
					RS_Co->Pp[MM+1] = { 1, 0, 1, 0, 0, 1 };
	MM == 6 -->		1 + x + x^6
					RS_Co->Pp[MM+1] = { 1, 1, 0, 0, 0, 0, 1 };
	MM == 7 -->		1 + x^3 + x^7
					RS_Co->Pp[MM+1] = { 1, 0, 0, 1, 0, 0, 0, 1 };
	MM == 8 -->		1 + x + x^2 + x^7 + x^8; // Which is the CCSDS field generator polynomial F(x) for GF(2**8)
					RS_Co->Pp[MM+1] = { 1, 1, 1, 0, 0, 0, 0, 1, 1 };

	Th reason for the obiously stange implementation (could be done much simpler with #define)
	is, that this software shall be implemented in a mico controller, so saving every byte of memory is
	a must. This implementation only needs 8 int constants (PP_const), instead of having 42 int values
	in the micro flash memory area, where maximum 8 (for MM=8) will finally be used.
	Note: 142 because of: see RS_Co->Pp[MM+1] above. The additional code for this is negligible.

	The RS_Co->Pp[MM+1] array itselve is defined as 'unsigned char', because they take only the values 0 or 1.

	Input parameter:

	- int		MM			Degree of Premitive Polynomial to be created
	- RS_Co_t*	RS_Co		Address of Reed-Solomon parameter structure

	Output parameter:

	- RS_Co_t*	RS_Co->Pp[]	Here the valid Premitive Polynomial parameters will be written

	Return parameter:
	- int		 = 0	Function performed correctly
				 > 0	Error in function
*******************************************************************************/
int SetPP_RS (int MM, RS_Co_t* RS_Co)
	{
	int		i;
	int		*PP_const;
/*
** Input data error check, also check valid memory allocation in structure RS_Co
*/
	if (MM < 2 || MM > 8) return 1;
	if (RS_Co == NULL) return 2;
	if (RS_Co->Pp == NULL) return 3;
/*
** Allocate temporary memory for the PP constants
*/
	PP_const = (int*) malloc (17 * sizeof(int));
/*
** Define the PP_const values.
** Constants defined as hex values, because C does not allow
** binary value definitions in the form of e.g. 0b0011
**
**     int           hex             binary         decimal
*/
	PP_const[ 2] = 0x00007;	// 0b00000000000000111;	7
	PP_const[ 3] = 0x0000D;	// 0b00000000000001101;	13
	PP_const[ 4] = 0x00019;	// 0b00000000000011001;	25
	PP_const[ 5] = 0x00029;	// 0b00000000000101001;	41
	PP_const[ 6] = 0x00061;	// 0b00000000001100001;	97
	PP_const[ 7] = 0x00091;	// 0b00000000010010001;	145
	PP_const[ 8] = 0x001C3;	// 0b00000000111000011;	451
/*
** Depending on the specified degree MM of the PP,
** store the valid PP_const values to the RS_Co structure for later use.
** This is implemented by shifting the 0s and 1s to the right and
** extracting the most right bit as the valid value for the Pp.
*/
	for (i=MM; i>=0; i--)
		{
		RS_Co->Pp[i] = PP_const[MM] & 1;
		PP_const[MM] = PP_const[MM] >> 1;
		}
/*
* Free the temporary array
*/
	free (PP_const);

	return 0;
	}
/*******************************************************************************
	Private Function:   modnn

	This function performs modulo NN function
 
 ------------------------------------------------------------------------------- 
	Input parameter:
	- int		x		The value to be modulo'ed
	- int		NN		(1 << MM) - 1
	- int		MM		Degree of Premitive Polynomial to be created

	Output parameter:	None

	Return parameter:
	- int		The computed modulo value

*******************************************************************************/
int modnn (int x, int NN, int MM)
	{
	while (x >= NN)
		{
		x -= NN;
		x = (x >> MM) + (x & NN);
		}
	return x;
	}