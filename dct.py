from ctypes import *

lib = cdll.LoadLibrary("./libdct.so")
manipulate = lib.manipulate_jpeg
manipulate.argtypes = [c_char_p, c_ulong, POINTER(c_void_p), POINTER(c_ulong), c_char_p, c_ulong]

inp = open('x.jpg', 'rb').read()

secrettext = bytes("Hallo Welt! das ist geheim!", 'ascii')
#secrettext = bytes([0xff, 0] * 100)

outbuf = c_void_p()
outsize = c_ulong()
result = manipulate(inp, c_ulong(len(inp)), byref(outbuf), byref(outsize), secrettext, len(secrettext))

out = bytes(outsize.value)
memmove(out, outbuf, outsize)



manipulate(out, c_ulong(len(out)), byref(outbuf), byref(outsize), c_char_p(0), c_ulong(0))

recovered = bytes(outsize.value)
memmove(recovered, outbuf, outsize)
