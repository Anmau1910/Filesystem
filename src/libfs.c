#include "libfs.h"
#include "libdisk.h"
#include <string.h>

typedef struct inode
{
	char name[16];
	int size, type;
	int bloque[30];
}Inode;

struct filep
{
	unsigned int ibloq, offset;
	unsigned int fpr, fpw;
};

struct filep tabla[20];

/* global errno value here */
int os_errno;
char *pat, *ibitmap, *bbitmap;
Inode root;

void *makebitmap(size_t tam){
	return calloc(tam/8, 1);
}

char getbit(int indice, char* array){
	int i = indice / 8;
	indice %= 8;
	return (array[i] >> (7 - indice)) & 1;
}

void setbit(int indice, char* array, int op){
	int i = indice / 8;
	indice %= 8;
	if(!op){
		array[i] = (((array[i] >> (7 - indice)) & 0xfffffffe) << (7 - indice))| array[i];
	}else{
		array[i] = (((array[i] >> (7 - indice)) | 1) << (7 - indice))| array[i];
	}
}

int nextbavail(){
	int i = 286;
	while(getbit(i, bbitmap))
		i++;
		
	if (getbit(i, bbitmap))	
		return -1;

	setbit(i, bbitmap, 1);
	return i;
}
int nextiavail(){
	int i = 4;
	while(getbit(i, ibitmap))
		i++;
		
	if (getbit(i, ibitmap))	
		return -1;

	setbit(i, ibitmap, 1);
	return i;
}
/*----------------------------------------------------------------------
	sector[0] = boot {3 bloques: 1 para superbloque, 2 para bitmaps y 3 root}
	sector[3] = Inodos{282 bloques}
	sector[286] = datos{9999}
-----------------------------------------------------------------------*/
int fs_boot(char *path)
{

	char try[512];
	pat = malloc(strlen(path));
	memcpy(pat, path, strlen(path));

	if(disk_init() == -1) {
		os_errno = E_GENERAL;
		return -1;
	}

	if(disk_load(pat) < 0 && disk_errno != E_OPENING_FILE) {
		os_errno = E_GENERAL;
		return -1;
	}

	disk_read(0, try);

	if (try[0] != 127 && try[0] != 0)
	{
		os_errno = E_GENERAL;
		return -1;
	}

	if (try[0] == 0)
	{
		try[0] = 127;
		disk_write(0, try);
		memset(try, 0, 512);
		ibitmap = makebitmap(1001); //bitmap de inodos 
		setbit(0, ibitmap, 1);
		bbitmap = makebitmap(900); //bitmap de bloques (30 bloques * 30 archivos max)
		setbit(0, bbitmap, 1);
		setbit(1, bbitmap, 1);
		setbit(2, bbitmap, 1);
		memcpy(try, ibitmap, 126);
		memcpy(try + 127, bbitmap, 113);
		disk_write(1, try);
		root.name[0] = '/';
		root.size = 0;
		root.type = 1;
		memset(root.bloque, 0, 120);
		memset(try, 0, 512);
		memcpy(try, &root,sizeof(root));
		disk_write(2, try);
	} else {
		disk_read(2, try);
		memcpy(&root, try, sizeof(root));
		ibitmap = makebitmap(1001);
		bbitmap = makebitmap(900);
		disk_read(1, try);
		memcpy(ibitmap, try, 126);
		memcpy(bbitmap, try+127, 113);
	}

	return 0;
}

int fs_sync(void)
{
	if(!disk_save(pat)){
		os_errno = E_GENERAL;
		return -1;
	}

	return 0;
}

int file_create(char *file)
{
	int i;
	char buffer[512];
	Inode aux;
	memset(&aux, 0, sizeof(aux));

	for (i = 0; i < 30; ++i)
	{
		if(root.bloque[i]){
			disk_read(root.bloque[i], buffer);
			for (int j = 0; j < 512; j+=144)
			{
				if(!strcmp(file, &buffer[j])){
					os_errno = E_CREATE;
					return -1;
				}
			}
		}
	}

	for (i = 0; i < 30; ++i)
	{
		if(!root.bloque[i])
			break;
	}

	if(i == 30)
	{
		os_errno = E_NO_SPACE;
		return -1;
	}

	root.bloque[i] = nextbavail();
	if(root.bloque[i] == -1){
		os_errno = E_NO_SPACE;
		return -1;
	}
	memset(buffer, 0, 512);
	memcpy(aux.name, file, strlen(file));
	aux.size = 0;
	aux.type = 0;
	memset(aux.bloque, 0, 120);
	memcpy(buffer, &aux, sizeof(aux));
	disk_write(root.bloque[i], buffer);

	return 0;
}

int file_open(char *file)
{
	int set = 1;
	char buffer[512];

	for (int i = 0; i < 20; ++i)
	{
		if (tabla[i].ibloq)
		{
			disk_read(tabla[i].ibloq, buffer);

			if(!strcmp(file, &buffer[tabla[i].offset])){
				os_errno = E_FILE_IN_USE;
				return -1;
			}
		}
	}
	
	for (int i = 0; i < 20; ++i)
	{
		if (!tabla[i].ibloq)
		{
			for (int k = 0; k < 30; k++)
			{
				if(root.bloque[k]){
					disk_read(root.bloque[k], buffer);
					for (int j = 0; j < 512; j+=144)
					{
						if(!strcmp(file, &buffer[j])){
							set = 0;
							tabla[i].ibloq = root.bloque[k]; // tabla[i].ibloq tiene el sector donde se encuentra el inodo;
							tabla[i].offset = j; //offset en el bloque;
							break;
						}
					}
				}
				if(!set)
					break;			
			}
			if(set)
			{
				os_errno = E_NO_SUCH_FILE;
				return -1;
			}else
				return i;
		}
	}
	
	os_errno = E_TOO_MANY_OPEN_FILES;
	return -1;
}

int file_read(int fd, void *buffer, int size)
{
	Inode *aux;
	int c, d, f, e = 0;
	char bufferr[512];
	char blockin[512];

	if(!tabla[fd].ibloq){
		os_errno = E_BAD_FD;
		return -1;
	}

	disk_read(tabla[fd].ibloq, blockin);
	aux =  (Inode *)&blockin[tabla[fd].offset];

	if (tabla[fd].fpr == aux->size)
		return 0;

	if (size % 512)
		c = (size/512) + 1;
	else
		c = size/512;

	if (tabla[fd].fpr % 512)
		c++;

	d = tabla[fd].fpr/512;

	for (int i = d; i < (c + d); ++i)
	{
		if (i > 30 || !aux->bloque[i])
			return e;

		disk_read(aux->bloque[i], bufferr);

		if ((512 - (tabla[fd].fpr % 512)) >= size)
		{
			memcpy(buffer, &bufferr[tabla[fd].fpr % 512], size);
			f = (aux->size - tabla[fd].fpr) % 512;

			if(f < size){
				tabla[fd].fpr += (aux->size - tabla[fd].fpr) % 512;
				return f;
			}
			tabla[fd].fpr += size;
			return e + size;
		}

		memcpy(buffer, &bufferr[tabla[fd].fpr % 512], 512 - (tabla[fd].fpr % 512));
		e += 512 -(tabla[fd].fpr % 512);
		size -= 512 -(tabla[fd].fpr % 512);
		buffer += 512 - (tabla[fd].fpr % 512);
		tabla[fd].fpr += 512 - (tabla[fd].fpr % 512);
	}
	return e;
}

int file_write(int fd, void *buffer, int size)
{
	Inode aux;
	int c, d, e = 0;
	char bufferr[512], buf[512];

	if(!tabla[fd].ibloq){
		os_errno = E_BAD_FD;
		return -1;
	}

	disk_read(tabla[fd].ibloq, buf);
	memcpy(aux.name, &buf[tabla[fd].offset], 16);
	aux.size = (int)buf[tabla[fd].offset + 16];
	aux.type = (int)buf[tabla[fd].offset + 20];
	memcpy(aux.bloque, &buf[tabla[fd].offset + 24], 120);

	if (size % 512)
		c = (size/512) + 1;
	else
		c = size/512;

	if (tabla[fd].fpw % 512)
		c++;

	d = tabla[fd].fpw/512;

	for (int i = d; i < (c + d); ++i)
	{
		if (i > 29)
		{
			os_errno = E_FILE_TOO_BIG;
			memcpy(buf + tabla[fd].offset, &aux, sizeof(aux));
			disk_write(tabla[fd].ibloq, buf);
			return -1;
		}
		if (!aux.bloque[i])
		{
			if ((aux.bloque[i] = nextbavail()) == -1){
				os_errno = E_NO_SPACE;
				memcpy(buf + tabla[fd].offset, &aux, sizeof(aux));
				disk_write(tabla[fd].ibloq, buf);
				return -1;
			}
		}

		disk_read(aux.bloque[i], bufferr);

		setbit(aux.bloque[i], bbitmap, 1);

		if ((512 - (tabla[fd].fpw % 512)) >= size)
		{
			memcpy(&bufferr[tabla[fd].fpw % 512], buffer, size);
			disk_write(aux.bloque[i], bufferr);
			aux.size += size;
			memcpy(buf + tabla[fd].offset, &aux, sizeof(aux));
			disk_write(tabla[fd].ibloq, buf);
			tabla[fd].fpw += size;
			e += size;
			return e;
		}

		memcpy(&bufferr[tabla[fd].fpw % 512], buffer, 512 - (tabla[fd].fpw % 512));
		disk_write(aux.bloque[i], bufferr);
		e += 512 - (tabla[fd].fpw % 512);
		size -= 512 - (tabla[fd].fpw % 512);
		buffer += 512 - (tabla[fd].fpw % 512);
		aux.size += 512 - (tabla[fd].fpw % 512);
		tabla[fd].fpw += 512 - (tabla[fd].fpw % 512);
	}

	memcpy(buf + tabla[fd].offset, &aux, sizeof(aux));
	disk_write(tabla[fd].ibloq, buf);
	return e;
}

int file_seek(int fd, int offset)
{
	char buffer[512];
	Inode *aux;

	if (!tabla[fd].ibloq)
	{
		os_errno = E_BAD_FD;
		return -1;
	}

	disk_read(tabla[fd].ibloq, buffer);
	aux = (Inode *)&buffer[tabla[fd].offset];

	if ((aux->size < offset) || offset < 0)
	{
		os_errno = E_SEEK_OUT_OF_BOUNDS;
		return -1;
	}
	tabla[fd].fpr = tabla[fd].fpw = offset;
	return 0;
}

int file_close(int fd)
{

	if (!tabla[fd].ibloq)
	{
		os_errno = E_BAD_FD;
		return -1;
	}
	memset(&tabla[fd], 0, sizeof(tabla[fd]));
	return 0;
}

int file_unlink(char *file)
{
	Inode *aux;
	char buffer[512], del[512];
	int k = 0;

	for (int i = 0; i < 20; ++i)
	{
		if (tabla[i].ibloq)
		{
			disk_read(tabla[i].ibloq, buffer);

			if(!strcmp(file, &buffer[tabla[i].offset])){
				os_errno = E_FILE_IN_USE;
				return -1;
			}
		}
	}

	for (int i = 0; i < 30; ++i)
	{
		if(root.bloque[i]){

			disk_read(root.bloque[i], buffer);

			for (int j = 0; j < 512; j += 144)
			{
				if(!strcmp(file, &buffer[j])){

					setbit(root.bloque[i], bbitmap, 0);
					setbit(i, ibitmap, 0);
					aux = (Inode *)&buffer[j];

					while(aux->bloque[k]){

						setbit(aux->bloque[k], bbitmap, 0);
						memset(del, 0, 512);
						disk_write(aux->bloque[k], del);
						k++;
					}

					memset(&buffer[j], 0, 144);
					disk_write(root.bloque[i], buffer);
					root.bloque[i] = 0;
					return 0;
				}
			}
		}
	}
	os_errno = E_NO_SUCH_FILE;
	return -1;
}
