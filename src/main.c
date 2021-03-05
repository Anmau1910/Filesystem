#include <stdio.h>
#include <string.h>

#include "libfs.h"
#include "libfs.c"
#define CHECKSUCC(x) do { \
  retval = (x); \
  fprintf(stderr, "Probando\t%s, debe funcionar\n", #x); \
  if (retval < 0) { \
    fprintf(stderr, "ERROR\t\tfunción %s falló, retornando %d con os_errno %d\n", #x, retval, os_errno); \
  } else { \
  	fprintf(stderr, "EXITO\t\tfunción %s exitosa, retornando %d\n", #x, retval); \
  } \
} while (0); \
puts("")

#define CHECKFAIL(x) do { \
  retval = (x); \
  fprintf(stderr, "Probando\t%s, debe fallar\n", #x); \
  if (retval < 0) { \
    fprintf(stderr, "EXITO\t\tfunción %s falló, retornando %d with os_errno %d\n", #x, retval, os_errno); \
  } else { \
  	fprintf(stderr, "ERROR\t\tfunción %s exitosa, retornando %d\n", #x, retval); \
  } \
} while (0); \
puts("")



void usage(char *prog)
{
	fprintf(stderr, "usage: %s <disk image file>\n", prog);
	exit(1);
}

int main(int argc, char *argv[])
{
	int retval, fd1, fd2;
	char buff5[5], buff512[512], buff1025[1025];
	char buff5ret[5], buff512ret[512], buff1025ret[1025];

	if (argc != 2) {
		usage(argv[0]);
	}
	char *path = argv[1];

	memset(buff5, 'a', 5);
	memset(buff512, 'b', 512);
	memset(buff1025, 'c', 1025);

	fprintf(stderr, "---------------------------------\n      Probando uso normal de archivos      \n---------------------------------");
	CHECKSUCC(fs_boot(path));

	CHECKSUCC(file_create("hola"));

	CHECKFAIL(file_create("hola"));
	

	CHECKSUCC(file_create("chao"));
	

	CHECKSUCC(file_open("hola"));
	fd1 = retval;
	

	CHECKSUCC(file_open("chao"));
	fd2 = retval;
	

	CHECKFAIL(file_open("EPALE"));
	

	fprintf(stderr,"Probando la distinción entre ambos file descriptors:\n");
	if(fd1 == fd2) {
		fprintf(stderr, "ERROR, fds iguales\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_write(fd1, buff5, 5));
	
	fprintf(stderr,"Probando número de bytes escritos\n");
	if(5 != retval) {
		fprintf(stderr, "ERROR, no se escribieron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_write(fd1, buff512, 512));
	
	fprintf(stderr,"Probando número de bytes escritos\n");
	if(512 != retval) {
		fprintf(stderr, "ERROR, no se escribieron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_write(fd2, buff1025, 1025));
	
	fprintf(stderr,"Probando número de bytes escritos\n");
	if(1025 != retval) {
		fprintf(stderr, "ERROR, no se escribieron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_write(fd2, buff512, 512));
	
	fprintf(stderr,"Probando número de bytes escritos\n");
	if(512 != retval) {
		fprintf(stderr, "ERROR, no se escribieron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_close(fd1));
	

	CHECKSUCC(file_close(fd2));
	

	CHECKFAIL(file_close(fd2));
	

	fd1 = file_open("hola");
	fd2 = file_open("chao");

	CHECKSUCC(file_read(fd1, buff5ret, 5));
	
	fprintf(stderr,"Probando número de bytes leídos\n");
	if(5 != retval) {
		fprintf(stderr, "ERROR, no se retornó la cantidad correcta\n");
	} else if (memcmp(buff5, buff5ret, 5)) {
		fprintf(stderr, "ERROR, no se retornaron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	
	
	CHECKSUCC(file_read(fd1, buff512ret, 512));
	
	fprintf(stderr,"Probando número de bytes leídos\n");
	if(512 != retval) {
		fprintf(stderr, "ERROR, no se retornó la cantidad correcta\n");
	} else if (memcmp(buff512, buff512ret, 512)) {
		fprintf(stderr, "ERROR, no se retornaron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_read(fd2, buff1025ret, 1025));
	
	fprintf(stderr,"Probando número de bytes leídos\n");
	if(1025 != retval) {
		fprintf(stderr, "ERROR, no se retornó la cantidad correcta\n");
	} else if (memcmp(buff1025, buff1025ret, 1025)) {
		fprintf(stderr, "ERROR, no se retornaron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_read(fd2, buff512ret, 512));
	
	fprintf(stderr,"Probando número de bytes leídos\n");
	if(512 != retval) {
		fprintf(stderr, "ERROR, no se retornó la cantidad correcta\n");
	} else if (memcmp(buff512, buff512ret, 512)) {
		fprintf(stderr, "ERROR, no se retornaron los bytes correctos\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKSUCC(file_read(fd1, buff5ret, 5));
	
	fprintf(stderr,"Probando número de bytes leídos\n");
	if(0 != retval) {
		fprintf(stderr, "ERROR, no se retornó la cantidad correcta\n");
	} else {
		fprintf(stderr, "EXITO\n");
	}
	

	CHECKFAIL(file_unlink("hola"));

	file_close(fd1);
	file_close(fd2);

	CHECKSUCC(file_unlink("hola"));
	CHECKSUCC(file_unlink("chao"));

	CHECKSUCC(file_create("1.txt"));
	CHECKSUCC(file_create("2.txt"));
	CHECKSUCC(file_create("3.txt"));
	CHECKSUCC(file_create("4.txt"));
	CHECKSUCC(file_create("5.txt"));
	CHECKSUCC(file_create("6.txt"));
	CHECKSUCC(file_create("7.txt"));
	CHECKSUCC(file_create("8.txt"));
	CHECKSUCC(file_create("9.txt"));
	CHECKSUCC(file_create("10.txt"));
	CHECKSUCC(file_create("11.txt"));
	CHECKSUCC(file_create("12.txt"));
	CHECKSUCC(file_create("13.txt"));
	CHECKSUCC(file_create("14.txt"));
	CHECKSUCC(file_create("15.txt"));
	CHECKSUCC(file_create("16.txt"));
	CHECKSUCC(file_create("17.txt"));
	CHECKSUCC(file_create("18.txt"));
	CHECKSUCC(file_create("19.txt"));
	CHECKSUCC(file_create("20.txt"));
	CHECKSUCC(file_create("21.txt"));
	CHECKSUCC(file_create("22.txt"));
	CHECKSUCC(file_create("23.txt"));
	CHECKSUCC(file_create("24.txt"));
	CHECKSUCC(file_create("25.txt"));
	CHECKSUCC(file_create("26.txt"));
	CHECKSUCC(file_create("27.txt"));
	CHECKSUCC(file_create("28.txt"));
	CHECKSUCC(file_create("29.txt"));
	CHECKSUCC(file_create("30.txt"));

	CHECKFAIL(file_create("31.txt"));
	CHECKFAIL(file_create("32.txt"));

	fd1 = file_open("1.txt");
	file_write(fd1, buff512, 512);
	file_close(fd1);

	fd1 = file_open("1.txt");
	CHECKSUCC(file_seek(fd1, 0));
	CHECKSUCC(file_seek(fd1, 128));
	CHECKSUCC(file_seek(fd1, 512));
	CHECKFAIL(file_seek(fd1, 1024));
	file_close(fd1);

	fd1 = file_open("1.txt");
	for(int i = 0; i < 30; i++)
		CHECKSUCC(file_write(fd1, buff512, 512));
	CHECKFAIL(file_write(fd1, buff512, 512));


	fs_sync();

	return 0;
}
