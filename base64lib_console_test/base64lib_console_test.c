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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\base64lib\base64.h"
#include "..\base64lib\base64io.h"

void print_usage();
void print_progress(size_t processed, size_t total);
void clear_progress();
void base64io_callback(BASE64IO* io);

int main(int argc, char* argv[])
{
	int result = 0;
	if (argc == 4 || argc == 5)
	{
		if (!strcmp("encode", argv[1]) || !strcmp("decode", argv[1]))
		{
			FILE* in;
			FILE* out;

			if (in = fopen(argv[2], "rb"))
			{
				if (out = fopen(argv[3], "wb"))
				{
					int ret;
					BASE64IO io = { 0 };
					io.callback_function = &base64io_callback;
					io.chunk_buffer_size = (argc == 5) ? (size_t) atol(argv[4]) : 1024;

					if (!strcmp("encode", argv[1]))
					{
						printf("Encoding %s...\n\n", argv[2]);
						print_progress(0, 100);
						ret = base64io_encode(&io, in, out);
						clear_progress();
					}
					else
					{
						printf("Decoding %s...\n\n", argv[2]);
						print_progress(0, 100);
						ret = base64io_decode(&io, in, out);
						clear_progress();
					}

					if (ret == BASE64_SUCCESS)
					{
						printf("Done.\n\n");
					}
					else
					{
						printf("base64lib error: %s (0x%x).\n\n", 
							base64_translate_result(ret), ret);
						result = -1;
					}

					fclose(out);
				}
				else
				{
					printf("Cannot open: %s\n\n", argv[3]);
					result = -1;
				}
				fclose(in);
			}
			else 
			{
				printf("Cannot open: %s\n\n", argv[2]);
				result = -1;
			}
		}
		else
		{
			print_usage();
			result = -1;
		}
	}
	else
	{
		print_usage();
		result = -1;
	}

	#if !defined(NDEBUG)
	printf("Press ENTER to exit...");
	getc(stdin);
	#endif
	return result;;
}

void base64io_callback(BASE64IO* io)
{
	print_progress(io->bytes_processed, io->bytes_total);
}

void print_progress(size_t processed, size_t total)
{
	int i;
	static int last_percent = -1;
	int percent = (int)((((float) processed / (float) total) * 100) + 0.25);
	int p = percent * 30 / 100;

	if (percent == last_percent)
		return;

	last_percent = percent;

	printf("[");
	for (i = 0; i < p; i++) printf("*");
	for (i = p; i < 30; i++) printf("-");
	printf("] %i%%\r", percent);
}

void clear_progress()
{
	printf("                                     \r");
}

void print_usage()
{
	printf("Usage: base64 [COMMANND] [INPUT] [OUTPUT] [optional:BUFFER_SIZE]\n\n");
	printf("COMMAND\t\tOne of the following:\n\n\t\t\t");
	printf("encode - base64 encodes input file.\n\t\t\t");
	printf("decode - decodes base64 input file.\n\n");
	printf("INPUT\t\tThe file to be encoded/decoded.\n");
	printf("OUTPUT\t\tThe output file.\n");
	printf("BUFFER_SIZE\tThe size in bytes of the working buffer.\n\t\tMinimum size is 7 bytes, default is 1024.\n");
	printf("\n");
}
