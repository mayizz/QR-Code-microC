/********************************************************
 Author:					Manfred Langemann
 mailto:					Manfred.Langemann �t t-online.de
 Begin of project:			17.08.2012
 Latest version generated:	21.08.2012
 Filename:					Reed-Solomon.h
 Description:    			Include file for Main.c and Reed-Solomon.c
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
 typedef struct tag_RS_Co
	{
	unsigned char* Alpha_to;
	unsigned char* Index_of;
	unsigned char* Gg;
	unsigned char* Pp;
	int MM;
	int NN;
	int KK;
	int A0;
	int B0;
	} RS_Co_t;

int Init_RS (int MM, int KK, RS_Co_t* RS_Co, int NN);
void Clear_RS (RS_Co_t* RS_Co);
int Encode_RS (unsigned char* data, unsigned char* bb,  RS_Co_t* RS_Co);
int Decode_RS (unsigned char* data, unsigned char* eras_pos, int num_eras,  RS_Co_t* RS_Co);
/*
** Set this to allow printf of intermediate and error results in function Decode_RS
*/
#define DEBUG
