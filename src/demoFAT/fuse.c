
/*#define FUSE_USE_VERSION  26
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "fatTypes.h"
#include "fat.h"
#include "ctype.h"
#include <fuse.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



struct myFile{
	char name[255];
	struct stat fileStat;
	char data[4096];
	struct myFile * next;
} *rootDir;

static int hello_getattr(const char *path, struct stat *stbuf)
{
	//funcion que se ejecuta cuando me piden los atributos de
	//un directorio/archivo
	//devuelve 0 si fue exitoso o El numero de Error (Creo)
	//path es el dir/archivo que quiere los stats ('/','/unArchivo','/Dir1/Archivo.txt',etc)
	//stbuf es la estructura en la que tengo que devolver los stats
	int res = 0;
	struct myFile * p;

	memset(stbuf, 0, sizeof(struct stat));//Limpio stbuf
	if(strcmp(path, "/") == 0) {
		// si es el root del FS (no lo tengo en la lista)
		// por ende, hardcodeado
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else {
		//si no, recorre la lista hasta que lo encuentres
		p=rootDir;
		while(p!=NULL){
			if(strcmp(path+1,p->name)==0){//cuando lo encuentra
				//copia los stats del archivo al parametro de salida
				stbuf->st_mode=p->fileStat.st_mode;
				stbuf->st_nlink=p->fileStat.st_nlink;
				stbuf->st_size=p->fileStat.st_size;
				return 0; //y salgo
			}
			p=p->next;
		}
		return -ENOENT;

	}
	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					   off_t offset, struct fuse_file_info *fi)
{
	//funcion que se ejecuta cuando necesitas listar un directorio
	//path es el path :P
	//buf es donde voy a guardar el contenido del directorio
	//filler es la funcion que tengo que usar para cargar el contenido en buf
	//El resto por ahora no se usa
	(void) offset;
	(void) fi;
	struct myFile * p;

	if(strcmp(path, "/") != 0)//si me pide que liste algo que no sea el root
		return -ENOENT;//devuelvo error

	filler(buf, ".", NULL, 0);//cargo en el buffer la entrada del dir actual
	filler(buf, "..", NULL, 0);//cargo en el buffer la entrada del dir padre
	//ambos 2 los resuelve solo FUSE
	p=rootDir;
	while(p!=NULL){//recorro la lista y voy cargando los dirs/archivos
		filler(buf,p->name,NULL,0);
		p=p->next;
	}

	return 0;//salgo
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	//funcion que checkea si podes abrir o no el archivo
	//path es el archivo
	//fi tiene la data del archivo,

	//Le creo al tutorial que esto funciona
	if((fi->flags & 3) != O_RDONLY)//si no es una operacion de solo lectura
	  return -EACCES;//devolve error

	return 0;//devolve que lo podes abrir
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
					struct fuse_file_info *fi)
{
	//funcion que se llama cuando se quiere leer un archivo
	//path es el archivo
	//buf es donde guardar el contenido
	//size es la cantidad a leer
	//offset es a partir de donde leer
	//fi no lo uso por ahora
	//devuelve la cantidad leida
	size_t len;
	(void) fi;
	struct myFile * p;

	p=rootDir;
	while(p!=NULL){//recorro el dir hasta que lo encuentro
		if(strcmp(path+1,p->name)==0){
			len=strlen(p->data);
			if (offset > len)//si quiere arrancar a leer mas alla del final
				return 0;//decile que no pudiste leer nada
			strncpy(buf,(p->data)+offset,size);//sino lee lo que te pide
											   //Creo que corta solo al final
			return size;//deberia devolver lo que leyo en realidad
						//Si se corto antes de esa cantidad, deberia devolver eso
		}
		p=p->next;
	}
	return -ENOENT;
}

static struct fuse_operations fsp_oper = {
	.getattr   = hello_getattr,
	.readdir = hello_readdir,
	.open   = hello_open,
	.read   = hello_read,
};

int main2(int argc, char *argv[])
{
	return fuse_main(argc, argv, &fsp_oper, NULL);
}*/
