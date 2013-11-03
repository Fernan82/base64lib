#ifndef BASE64_RESULT_CODES
#define BASE64_RESULT_COODES

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
}
#endif


#endif