#ifndef BASE64LIB_BASE64
#define BASE64LIB_BASE64

#include "stdlib.h"
#include "result_codes.h"

//
// defines a 32-bit integer type
//
#if !defined(uint32_t)
#define uint32_t		unsigned long
#endif

//
// uncomment this line to skip base64 validation while
// decoding if you're having performance problems. A basic
// verification that the input length is divisible by 4 will still
// be performed but if this verification passes and the file is
// otherwise invalid you'll get garbage for output.
//
//#define SKIP_BASE64_VALIDATION

//
// by default if you pass a pointer to a null pointer to the output
// parameter of the encode/decode functions memory will be allocated
// from the heap and the caller is responsible for freeing it. If you
// uncomment this line and pass a null pointer to these the function will
// return a BASE64_HEAP_ALLOCATION_DISABLED error code.
//
//#define DISABLE_HEAP_ALLOCATION

#if defined(__cplusplus)
extern "C"
{
#endif

//
// Encodes a byte array to a null terminated base64 string.
//
// note: this function uses malloc() to allocate the output buffer so
// it must be released with free().
//
// in: data - pointer to the byte array containing the data to be encoded.
// in: len - the length of the data to be encoded
// in: output - a pointer-to-pointer of output buffer. if a pointer to
//		a null pointer is given it will be allocated using malloc.
//
int base64_encode(const unsigned char* data, size_t* const len, char** const output);

//
// Encodes a base64 null terminated string to a byte array.
//
// note: this function uses malloc() to allocate the output buffer so
// it must be released with free().
//
// in: data - pointer to the base64 string to be decoded.
// in: len - length of the base64 string to be decoded (not including null terminator).
// out: len - length of the decoded byte array.
//
int base64_decode(const char* data, size_t* const len, unsigned char** const output);

//
// Gets the size of the base64 encoder output for a given
// input length including the null terminator
//
static __forceinline size_t base64_get_encoder_output_length(const size_t input_len)
{
	return (size_t) (((input_len + 2) / 3) * 4);
}

//
// Gets the max size of the base64 decoder output for a given
// input length.
//
// Note: this is the MAX length, the actual output may be up to 2 bytes
// shorter so use this only to allocate output buffers when allocating
// your own buffers.
//
static __forceinline size_t base64_get_decoder_output_length(const size_t input_len)
{
	return (input_len / 4) * 3;
}

#if defined(__cplusplus)
}
#endif

#endif