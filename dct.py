from ctypes import *

lib = cdll.LoadLibrary("./libdct.so")
manipulate = lib.manipulate_jpeg
manipulate.argtypes = [c_char_p, c_ulong, POINTER(c_void_p), POINTER(c_ulong), c_char_p, c_ulong]

inp = open('x.jpg', 'rb').read()

secrettext = bytes("Hallo Welt! das ist geheim!", 'ascii')
#secrettext = bytes([0xff, 0] * 100)

def encode(carrier_jpeg_bytes, secrettext):
	outbuf = c_void_p()
	outsize = c_ulong()
	manipulate(inp, c_ulong(len(inp)), byref(outbuf), byref(outsize), secrettext, len(secrettext))
	result = bytes(outsize.value)
	memmove(result, outbuf, outsize)

	return result

def decode(carrier_jpeg_bytes):
	outbuf = c_void_p()
	outsize = c_ulong()
	manipulate(carrier_jpeg_bytes, c_ulong(len(carrier_jpeg_bytes)), byref(outbuf), byref(outsize), c_char_p(0), c_ulong(0))
	recovered = bytes(outsize.value)
	memmove(recovered, outbuf, outsize)
	return recovered

encoded_image = encode(inp, secrettext)
recovered = decode(encoded_image)
