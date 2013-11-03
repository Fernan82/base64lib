base64lib_console_test
======================

A portable console application for testing base64lib library.

Usage: base64 [COMMANND] [INPUT] [OUTPUT] [optional:BUFFER_SIZE]

COMMAND         One of the following:

                        encode - base64 encodes input file.
                        decode - decodes base64 input file.

INPUT           The file to be encoded/decoded.
OUTPUT          The output file.
BUFFER_SIZE     The size in bytes of the working buffer.
                Minimum size is 7, default is 1024.
