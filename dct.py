from ctypes import *

lib = cdll.LoadLibrary("./libdct.so")
manipulate = lib.manipulate_jpeg
manipulate.argtypes = [c_char_p, c_ulong, POINTER(c_void_p), POINTER(c_ulong), c_char_p, c_ulong]


def encode_jpeg(carrier_jpeg_bytes, secrettext):
	outbuf = c_void_p()
	outsize = c_ulong()
	manipulate(carrier_jpeg_bytes, c_ulong(len(carrier_jpeg_bytes)), byref(outbuf), byref(outsize), secrettext, len(secrettext))
	result = bytes(outsize.value)
	memmove(result, outbuf, outsize)

	return result

def decode_jpeg(carrier_jpeg_bytes):
	outbuf = c_void_p()
	outsize = c_ulong()
	manipulate(carrier_jpeg_bytes, c_ulong(len(carrier_jpeg_bytes)), byref(outbuf), byref(outsize), c_char_p(0), c_ulong(0))
	recovered = bytes(outsize.value)
	memmove(recovered, outbuf, outsize)
	return recovered

def encode_str(secrettext):
	if type(secrettext == str):
		secrettext = bytes(secrettext, 'utf8')
	
	l = len(secrettext)
	return l.to_bytes(4,'little') + secrettext

def decode_str(data):
	l = int.from_bytes(data[0:4], 'little')
	return data[4:4+l].decode('utf8')

#inp = open('x.jpg', 'rb').read()
#encoded_image = encode_jpeg(inp, encode_str("Hallo Welt! Dies ist ein Test"))
#recovered = decode_str(decode_jpeg(encoded_image))
