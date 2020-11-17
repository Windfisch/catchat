from ctypes import *

lib = cdll.LoadLibrary("./libdct.so")
manipulate = lib.manipulate_jpeg
manipulate.argtypes = [c_char_p, c_ulong, POINTER(c_void_p), POINTER(c_ulong)]

inp = open('x.jpg', 'rb').read()

outbuf = c_void_p()
outsize = c_ulong()
result = manipulate(inp, c_ulong(len(inp)), byref(outbuf), byref(outsize))

out = bytes(outsize.value)
memmove(out, outbuf, outsize)
