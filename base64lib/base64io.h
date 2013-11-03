/*
 * base64lib - Base64 Encoding/Decoding Library
 * Copyright (C) 2013 Fernando Rodriguez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef BASE64LIB_BASE64IO
#define BASE64LIB_BASE64IO

#include "stdlib.h"
#include "stdio.h"
#include "base64.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct BASE64IO {
	size_t bytes_total;
	size_t bytes_processed;
	size_t chunk_buffer_size;
	char* chunk_buffer;
	char abort_requested;
	char started;
	char running;
	int result;
	void (*callback_function)(void*);
}
BASE64IO;


int base64io_encode(BASE64IO* io, FILE* input, FILE* output);
int base64io_decode(BASE64IO* io, FILE* input, FILE* output);


#if defined(__cplusplus)
}
#endif
#endif
