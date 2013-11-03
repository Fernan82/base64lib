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