libdct.so: dct.c
	gcc -fPIC -shared -rdynamic  -ljpeg dct.c -o libdct.so
