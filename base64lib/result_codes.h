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

#ifndef BASE64LIB_RESULT_CODES
#define BASE64LIB_RESULT_CODES

//
// define result codes
//
#define BASE64_SUCCESS						0x01
#define BASE64_INVALID_DATA					0x02
#define BASE64_INSUFFICIENT_MEMORY			0x03
#define BASE64_HEAP_ALLOCATION_DISABLED		0x04
#define BASE64IO_IO_ERROR					0x05
#define BASE64IO_UNKNOWN_ERROR				0x06
#define BASE64IO_BUFFER_TOO_SMALL			0x07
#define BASE64IO_NULL_FILE					0x08
#define BASE64IO_OPERATION_ABORTED			0x09

#if defined(__cplusplus)
extern "C" {
#endif

//
// translates result code into human readable string
//
char* base64_translate_result(int result);

#if defined(__cplusplus)
}
#endif

#endif
