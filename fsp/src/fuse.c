
#define FUSE_USE_VERSION  26
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "fatTypes.h"
#include "fat.h"
#include "fatHelpers.h"
#include "ctype.h"
#include <fuse.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "common/utils/log.h"
#include "console.h"

t_log * logFile;
config_fsp * config;

static int fuselage_getattr(const char *path, struct stat *stbuf)
{
	//funcion que se ejecuta cuando me piden los atributos de
	//un directorio/archivo
	//devuelve 0 si fue exitoso o El numero de Error (Creo)
	//path es el dir/archivo que quiere los stats ('/','/unArchivo','/Dir1/Archivo.txt',etc)
	//stbuf es la estructura en la que tengo que devolver los stats
	t_fat_file_entry file;
	t_stat temp;
	log_debug(logFile,"FSP","get attr %s\n",path);
    memset(stbuf, 0, sizeof(struct stat));
	if (!fat_getFileFromPath(path,&file)){
		return -ENOENT;
	}
	temp=fat_statFile(&file);
	memcpy(stbuf,&temp,sizeof(t_stat));
	return 0;
}

static int fuselage_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					   off_t offset, struct fuse_file_info *fi)
{
	//funcion que se ejecuta cuando necesitas listar un directorio
	//path es el path :P
	//buf es donde voy a guardar el contenido del directorio
	//filler es la funcion que tengo que usar para cargar el contenido en buf
	//El resto por ahora no se usa
	(void) offset;
	(void) fi;
	char fileName[14];
	int res;
	t_fat_file_entry file;
	t_fat_file_list * dir;
	t_fat_file_list * p;
	log_debug(logFile,"FSP","ls dir %s\n",path);
	res=fat_getFileFromPath(path,&file);

	if (res==1){
		dir = fat_getDirectoryListing(&file);
	}else{
		return -ENOENT;
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

static int fuselage_open(const char *path, struct fuse_file_info *fi)
{
	uint32_t clusterN;
	t_fat_file_entry fileEntry;
	log_debug(logFile,"FSP","open archivo %s\n",path);
    /*if((fi->flags & 3) != O_RDONLY)
        return -EACCES;*/
	fat_getFileFromPath(path,&fileEntry);
	clusterN=fat_getEntryFirstCluster(&fileEntry.dataEntry);
	while (clusterN!=FAT_LAST_CLUSTER){
		log_debug(logFile,"FSP","El archivo %s, usa el cluster %u,\n",path,clusterN);
		clusterN = fat_getNextCluster(clusterN);
	}
    return 0;
}

static int fuselage_read(const char *path, char *buf, size_t size, off_t offset,
					struct fuse_file_info *fi)
{
	//funcion que se llama cuando se quiere leer un archivo
	//path es el archivo
	//buf es donde guardar el contenido
	//size es la cantidad a leer
	//offset es a partir de donde leer
	//fi no lo uso por ahora
	//devuelve la cantidad leida
	log_debug(logFile,"FSP","START READ archivo %s\nsize:%d\noffset%d\n",path,size,offset);
	(void) fi;
	int res;
	t_fat_file_entry file;
	fat_getFileFromPath(path,&file);
	res = fat_readFileContents(&file,size,offset,buf);
	log_debug(logFile,"FSP","END READ archivo %s\nsize:%d\noffset%d\n",path,size,offset);
	return res;
}

static int fuselage_truncate (const char * path, off_t size){
	return fat_truncate(path,size);
}
static int fuselage_rename(const char * from,const char * to){
	return fat_move(from,to);
}
static struct fuse_operations fsp_oper = {
	.getattr   = fuselage_getattr,
	.readdir = fuselage_readdir,
	.open   = fuselage_open,
	.read   = fuselage_read,
	.truncate = fuselage_truncate,
	.rename = fuselage_rename
};

int main(int argc, char *argv[])
{
	t_xmlFile * configFile = loadConfig("config.xml");
	config = xmlGetConfigStructFsp(configFile);
	logFile= log_create("FSP",config->logFilePath,8,1);
	fat_initialize();
	pthread_t consoleThread;
	pthread_attr_t consoleAttr;
	pthread_attr_init(&consoleAttr);
	pthread_attr_setdetachstate(&consoleAttr, PTHREAD_CREATE_DETACHED);
	pthread_create(&consoleThread, &consoleAttr, &console, NULL);
	fuse_main(argc, argv, &fsp_oper, NULL);
	free(config);
	freeXmlFile(configFile);
	pthread_cancel(consoleThread);
	return 0;
}
