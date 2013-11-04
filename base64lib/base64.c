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


#include "base64.h"

//
// masks for obtaining each six bit segment
// from a 24-bit word
//
#define SEXTET1		0xFFFFFFFF << 26 >> 0
#define SEXTET2		0xFFFFFFFF << 26 >> 6
#define SEXTET3		0xFFFFFFFF << 26 >> 12
#define SEXTET4		0xFFFFFFFF << 26 >> 18

//
// base64 lookup table
//
static const char base64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

//
// base 64 reverse lookup table
//
static const unsigned char base64_reverse[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

int base64_encode(const unsigned char* data, size_t* const len, char** const output)
{
    uint32_t segment ;									// memory to store the 3 working bytes
	register size_t i;									// index for iterating through data
    register char* output_cursor;								// pointer for writing to output buffer
	register unsigned char* segment_bytes = (unsigned char*) &segment;
	//
	// if the caller didn't supply a valid pointer then
	// allocate memory
	//
	if (!*output)
	{
		#if !defined(DISABLE_HEAP_ALLOCATION)
		//
		// attempt to allocate the buffer
		//
		*output = (char*) malloc(base64_get_encoder_output_length(*len));
		//
		// if allocation fails return error code
		//
		if (!*output)
			return BASE64_INSUFFICIENT_MEMORY;
		#else
		return BASE64_HEAP_ALLOCATION_DISABLED;
		#endif
	}
	//
	// if the input length is 0 there's nothing to do
	//
	if (!*len)
		return BASE64_SUCCESS;
	//
	// create pointer for writting to output buffer
	//
	output_cursor = *output;
	//
    // loop through every 3 byte segment (we'll output 4 base64 chars
    // for every 3 bytes)
    //
    for (i = 0; i < *len - 3; i += 3)
    {
        // 
        // reset the 32 bit buffer to 0
        //
        segment = 0;
        //
        // load the next 3 bytes into the segment
        //
        segment_bytes[3] = *data++;
        segment_bytes[2] = *data++;
        segment_bytes[1] = *data++;
        //
        // lookup the values on the base64 table and add
        // them to the output buffer
        //
        *output_cursor++ = base64[(segment & SEXTET1) >> 26];
        *output_cursor++ = base64[(segment & SEXTET2) >> 20];
        *output_cursor++ = base64[(segment & SEXTET3) >> 14];
        *output_cursor++ = base64[(segment & SEXTET4) >> 8];
    }
    // 
    // reset the 32 bit buffer to 0
    //
    segment = 0;
    //
    // load the next 3 bytes into the segment
    //
    segment_bytes[3] = *data++;
    if (*len - i > 1) segment_bytes[2] = *data++;
    if (*len - i > 2) segment_bytes[1] = *data++;
    //
    // lookup the values on the base64 table and add
    // them to the output buffer
    //
    // note: if we only got one byte in the segment we only output 2 base64 chars,
    // and if we only got 2 bytes in the segment we output 3 chars. The remaining chars
    // are filled with equal signs.
    //
    *output_cursor++ = base64[(segment & SEXTET1) >> 26];
    *output_cursor++ = base64[(segment & SEXTET2) >> 20];
    *output_cursor++ = (*len - i > 1) ? base64[(segment & SEXTET3) >> 14] : '=';
    *output_cursor++ = (*len - i > 2) ? base64[(segment & SEXTET4) >> 8] : '=';
	//
	// update the len pointer with the output length
	//
	*len = base64_get_encoder_output_length(*len);
	//
	// return it
	//
	return BASE64_SUCCESS;
}

int base64_decode(const char* data, size_t* const len, unsigned char** const output)
{
	size_t i;
    size_t output_length;
    uint32_t segment;
	register unsigned char* segment_bytes = (unsigned char*) &segment;
    register size_t output_cursor = 0;

	#if !defined(DISABLE_HEAP_ALLOCATION)
	char output_allocated = 0;
	#endif
	//
	// if the length is not divisible by 4 then this
	// is not valid base64 data.
	//
	if ((*len % 4) != 0)
		return BASE64_INVALID_DATA;
    //
    // calculate the length of the decoded data buffer
    //
    output_length = base64_get_decoder_output_length(*len);
    //
    // allocate output buffer if necessary
    //
	if (!*output)
	{
		#if !defined(DISABLE_HEAP_ALLOCATION)
		*output = (unsigned char*) malloc(output_length);
		if (!output)
			return BASE64_INSUFFICIENT_MEMORY;
		output_allocated = 1;
		#else
		return BASE64_HEAP_ALLOCATION_DISABLED;
		#endif
	}
	//
	// if the input length is zero then there's 
	// nothing to do
	//
	if (*len == 0)
		return BASE64_SUCCESS;
	//
	// iterate through every base64 segment
	//
    for (i = 0; i < *len; i+= 4)
    {
		#if !defined(SKIP_BASE64_VALIDATION)
		//
		// verify that we're working with valid base64 data
		//		
		if ((base64_reverse[(int)data[i + 0]] | base64_reverse[(int)data[i + 1]] |
			base64_reverse[(int)data[i + 2]] | base64_reverse[(int)data[i + 3]]) == 0xFF)
		{
			if (i == *len - 4) 
			{
				if (data[i + 2] == '=' && data[i + 3] == '=')
				{
					if ((base64_reverse[(int)data[i + 0]] | base64_reverse[(int)data[i + 1]]) != 0xFF)
						goto validation_passed;
				}
				else if (data[i + 3] == '=')
				{
					if ((base64_reverse[(int)data[i + 0]] | base64_reverse[(int)data[i + 1]] |
						base64_reverse[(int)data[i + 2]]) != 0xFF) goto validation_passed;
				}
			}
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (output_allocated && *output)
			{
				free(*output);
				*output = 0;
			}
			#endif
			return BASE64_INVALID_DATA;
			//
			// this is just a label, the (void)sizeof(int) is just
			// a meaningless statement to keep the code from breaking because
			// the label is followed by a closing bracket
			//
			validation_passed: (void)sizeof(int);
		}
		#endif
		//
		// clear the segment variable
		//
        segment = 0;
		//
		// lookup the base64 bits (indexes) on the lookup table
		// and copy them to the appropiate bits on the segment
		// variable
		//
        segment |= ((uint32_t)base64_reverse[(int)data[i + 0]]) << 26;
        segment |= ((uint32_t)base64_reverse[(int)data[i + 1]]) << 20;
        segment |= ((uint32_t)base64_reverse[(int)data[i + 2]]) << 14;
        segment |= ((uint32_t)base64_reverse[(int)data[i + 3]]) << 8;
		//
		// copy the relevant bits to the output buffer
		//
		(*output)[output_cursor++] = segment_bytes[3];
		(*output)[output_cursor++] = segment_bytes[2];
		(*output)[output_cursor++] = segment_bytes[1];
    }
	//
	// if the last 2 bytes are not significant discard them
	//
    if (data[*len - 2] == '=' && data[*len - 1] == '=') output_length -= 2;
    else if (data[*len - 1] == '=') output_length--;
	//
	// save the output lenght
	//
    *len = output_length;
	// 
	// return the success code
	//
    return BASE64_SUCCESS;
}
