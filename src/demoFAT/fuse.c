
#define FUSE_USE_VERSION  26
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
#include <assert.h>


static int hello_getattr(const char *path, struct stat *stbuf)
{
	//funcion que se ejecuta cuando me piden los atributos de
	//un directorio/archivo
	//devuelve 0 si fue exitoso o El numero de Error (Creo)
	//path es el dir/archivo que quiere los stats ('/','/unArchivo','/Dir1/Archivo.txt',etc)
	//stbuf es la estructura en la que tengo que devolver los stats
	t_fat_file_entry file;
	t_stat temp;
	//assert(0==1);
    memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}
	if (!fat_getFileFromPath(path,&file)){
		return -ENOENT;
	}
	temp=fat_statFile(&file);
	memcpy(stbuf,&temp,sizeof(t_stat));
	return 0;
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
	char fileName[12];
	t_fat_file_entry file;
	t_fat_file_list * dir;
	t_fat_file_list * p;
	if (fat_getFileFromPath(path,&file)){
		dir = fat_getDirectoryListing(&file);
	}else {
		dir = fat_getRootDirectory();
	}
	filler(buf, ".", NULL, 0);//cargo en el buffer la entrada del dir actual
	filler(buf, "..", NULL, 0);//cargo en el buffer la entrada del dir padre
	//filler(buf, "pepe", NULL, 0);
	//ambos 2 los resuelve solo FUSE
	p=dir;
	while(p!=NULL){//recorro la lista y voy cargando los dirs/archivos
		fat_getName(&p->fileEntry,fileName);
		filler(buf,fileName,NULL,0);
		p=p->next;
	}
	fat_destroyFileList(dir);
	return 0;//salgo
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
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
	(void) fi;
	t_fat_file_entry file;
	fat_getFileFromPath(path,&file);
	return fat_readFileContents(&file,size,offset,buf);
}

static struct fuse_operations fsp_oper = {
	.getattr   = hello_getattr,
	.readdir = hello_readdir,
	.open   = hello_open,
	.read   = hello_read,
};

int main(int argc, char *argv[])
{
	fat_initialize();
	return fuse_main(argc, argv, &fsp_oper, NULL);
}
