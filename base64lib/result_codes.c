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

#include "result_codes.h"

static char* base64_result_strings[] = {
	"NO_RESULT",
	"BASE64_SUCCESS",
	"BASE64_INVALID_DATA",
	"BASE64_INSUFFICIENT_MEMORY",
	"BASE64_HEAP_ALLOCATION_DISABLED",
	"BASE64IO_IO_ERROR",
	"BASE64IO_UNKNOWN_ERROR",
	"BASE64IO_BUFFER_TOO_SMALL",
	"BASE64IO_NULL_FILE",
	"BASE64IO_OPERATION_ABORTED"
};

char* base64_translate_result(int result)
{
	return base64_result_strings[result];
}
