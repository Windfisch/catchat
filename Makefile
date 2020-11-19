libdct.so: dct.o
	gcc -shared dct.o  -ljpeg -o libdct.so

dct.o: dct.c
	gcc -fPIC -c dct.c


