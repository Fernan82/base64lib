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

#include "base64io.h"

int base64io_encode(BASE64IO* io, FILE* input, FILE* output)
{
	size_t len;
	size_t bytes_read;
	size_t output_buffer_len;
	size_t input_buffer_len;
	int result;
	unsigned char* input_buffer;
	char* output_buffer;
	#if !defined(DISABLE_HEAP_ALLOCATION)
	char chunk_buffer_allocated = 0;
	#endif
	//
	// update state structure
	//
	io->result = 0;
	io->bytes_total = 0xFF;
	io->bytes_processed = 0;
	io->started = 1;
	io->running = 1;
	//
	// do callback
	//
	if (io->callback_function)
		io->callback_function((void*)io);
	//
	// if an abort has been requested abort
	//
	if (io->abort_requested)
	{
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64IO_OPERATION_ABORTED;
	}
	//
	// checkk that the buffer is big enough
	//
	if (io->chunk_buffer_size < 7)
	{
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64IO_BUFFER_TOO_SMALL;
	}
	//
	// check that the FILE pointers are valid
	//
	if (!input || !output)
	{
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64IO_NULL_FILE;
	}
	//
	// if the user didn;t supply chunk buffer then
	// allocate it
	//
	if (!io->chunk_buffer)
	{
		#if !defined(DISABLE_HEAP_ALLOCATION)
		io->chunk_buffer = (char*) malloc(io->chunk_buffer_size);
		if (!io->chunk_buffer)
		{
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64_INSUFFICIENT_MEMORY;
		}
		chunk_buffer_allocated = 1;
		#else
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64_HEAP_ALLOCATION_DISABLED;
		#endif
	}
	//
	// split the chunk buffer
	//
	input_buffer_len = ((io->chunk_buffer_size * 3) / 7);
	output_buffer_len = ((io->chunk_buffer_size * 4) / 7);
	input_buffer_len = input_buffer_len - (input_buffer_len % 3);
	output_buffer_len = output_buffer_len - (output_buffer_len % 4);
	input_buffer = (unsigned char*) io->chunk_buffer;
	output_buffer = io->chunk_buffer + input_buffer_len;
	//
	// read input file size
	//
	fseek(input, 0, SEEK_END);
	io->bytes_total = ftell(input);
	rewind(input);
	//
	// do callback
	//
	if (io->callback_function)
		io->callback_function((void*)io);
	//
	// read the file in chunks
	//
	while ((len = bytes_read = fread(input_buffer, 1, input_buffer_len, input)))
	{
		//
		// encode the chunk
		//
		result = base64_encode(input_buffer, &len, &output_buffer);
		//
		// if encoding failed return error
		//
		if (result != BASE64_SUCCESS)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = result;
		}
		//
		// write encoded bytes to output file
		//
		if (fwrite(output_buffer, 1, len, output) < len)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64IO_IO_ERROR;
		}
		//
		// increase the processed bytes counter
		//
		io->bytes_processed += bytes_read;
		//
		// if an abort has been requested abort
		//
		if (io->abort_requested)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64IO_OPERATION_ABORTED;
		}
		//
		// do callback
		//
		if (io->callback_function)
			io->callback_function((void*)io);
	}
	//
	// if the chunk buffer was allocated by this function
	// then free it
	//
	#if !defined(DISABLE_HEAP_ALLOCATION)
	if (chunk_buffer_allocated)
	{
		free(io->chunk_buffer);
		io->chunk_buffer = 0;
	}
	#endif
	//
	// if the whole file wasn't processed
	// return unknownn error (probably another
	// thread messed with the io structure).
	//
	if (io->bytes_processed < io->bytes_total)
	{
		io->running = 0;
		io->abort_requested = 0;
		if (ferror(input))
			return io->result = BASE64IO_IO_ERROR;
		return io->result = BASE64IO_UNKNOWN_ERROR;
	}
	//
	// returnn success code
	//
	io->running = 0;
	io->abort_requested = 0;
	return io->result = BASE64_SUCCESS;
}

int base64io_decode(BASE64IO* io, FILE* input, FILE* output)
{
	size_t len;
	size_t bytes_read;
	size_t output_buffer_len;
	size_t input_buffer_len;
	int result;
	char* input_buffer;
	unsigned char* output_buffer;
	#if !defined(DISABLE_HEAP_ALLOCATION)
	char chunk_buffer_allocated = 0;
	#endif
	//
	// update state structure
	//
	io->result = 0;
	io->bytes_total = 0xFF;
	io->bytes_processed = 0;
	io->started = 1;
	io->running = 1;
	//
	// do callback
	//
	if (io->callback_function)
		io->callback_function((void*)io);
	//
	// checkk that the buffer is big enough
	//
	if (io->chunk_buffer_size < 7)
	{
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64IO_BUFFER_TOO_SMALL;
	}
	//
	// check that the FILE pointers are valid
	//
	if (!input || !output)
	{
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64IO_NULL_FILE;
	}
	//
	// if the user didn;t supply chunk buffer then
	// allocate it
	//
	if (!io->chunk_buffer)
	{
		#if !defined(DISABLE_HEAP_ALLOCATION)
		io->chunk_buffer = (char*) malloc(io->chunk_buffer_size);
		if (!io->chunk_buffer)
		{
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64_INSUFFICIENT_MEMORY;
		}
		chunk_buffer_allocated = 1;
		#else
		io->running = 0;
		io->abort_requested = 0;
		return io->result = BASE64_HEAP_ALLOCATION_DISABLED;
		#endif
	}
	//
	// split the chunk buffer
	//
	input_buffer_len = ((io->chunk_buffer_size * 4) / 7);
	output_buffer_len = ((io->chunk_buffer_size * 3) / 7);
	input_buffer_len = input_buffer_len - (input_buffer_len % 4);
	output_buffer_len = output_buffer_len - (output_buffer_len % 3);
	input_buffer = io->chunk_buffer;
	output_buffer = (unsigned char*) io->chunk_buffer + input_buffer_len;
	//
	// read input file size
	//
	fseek(input, 0, SEEK_END);
	io->bytes_total = ftell(input);
	rewind(input);
	//
	// do callback
	//
	if (io->callback_function)
		io->callback_function((void*)io);
	//
	// read the file in chunks
	//
	while ((len = bytes_read = fread(input_buffer, 1, input_buffer_len, input)))
	{
		//
		// encode the chunk
		//
		result = base64_decode(input_buffer, &len, &output_buffer);
		//
		// if encoding failed return error
		//
		if (result != BASE64_SUCCESS)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = result;
		}
		//
		// write encoded bytes to output file
		//
		if (fwrite(output_buffer, 1, len, output) < len)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64IO_IO_ERROR;
		}
		//
		// increase the processed bytes counter
		//
		io->bytes_processed += bytes_read;
		//
		// if an abort has been requested abort
		//
		if (io->abort_requested)
		{
			#if !defined(DISABLE_HEAP_ALLOCATION)
			if (chunk_buffer_allocated)
			{
				free(io->chunk_buffer);
				io->chunk_buffer = 0;
			}
			#endif
			io->running = 0;
			io->abort_requested = 0;
			return io->result = BASE64IO_OPERATION_ABORTED;
		}
		//
		// do callback
		//
		if (io->callback_function)
			io->callback_function((void*)io);
	}
	//
	// if the chunk buffer was allocated by this function
	// then free it
	//
	#if !defined(DISABLE_HEAP_ALLOCATION)
	if (chunk_buffer_allocated)
	{
		free(io->chunk_buffer);
		io->chunk_buffer = 0;
	}
	#endif
	//
	// if the whole file wasn't processed
	// return unknownn error (probably another
	// thread messed with the io structure).
	//
	if (io->bytes_processed < io->bytes_total)
	{
		io->running = 0;
		io->abort_requested = 0;
		if (ferror(input))
			return io->result = BASE64IO_IO_ERROR;
		return io->result = BASE64IO_UNKNOWN_ERROR;
	}
	//
	// returnn success code
	//
	io->running = 0;
	io->abort_requested = 0;
	return io->result = BASE64_SUCCESS;
}
