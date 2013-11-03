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
}
BASE64IO;


int base64io_encode(BASE64IO* io, FILE* input, FILE* output);
int base64io_decode(BASE64IO* io, FILE* input, FILE* output);


#if defined(__cplusplus)
}
#endif
#endif
