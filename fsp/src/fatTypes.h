/*
 * fatTypes.h
 *
 *  Created on: Sep 18, 2011
 *      Author: nico
 */

#ifndef FATTYPES_H_
#define FATTYPES_H_
#include <stdint.h>

#define FAT_SECTOR_SIZE 512
#define FAT_CLUSTER_SIZE 4096
#define FAT_LAST_CLUSTER 0x0FFFFFFF
#define FAT_FREE_CLUSTER 0x00000000
#define FAT_FAT_ENTRY_SIZE 4

typedef uint8_t t_sector[FAT_SECTOR_SIZE];
typedef uint8_t t_cluster[FAT_CLUSTER_SIZE];

typedef struct {
	uint8_t  jmpBoot[3];                    /* 0  Jump to boot code.                                */
	uint8_t  OEMName[8];                    /* 3  OEM name & version.                       */
	uint16_t bytesPerSector;                 /* 11 uint8_ts per sector hopefully 512.        */
	uint8_t  sectorPerCluster;              /* 13 Cluster size in sectors.                  */
	uint16_t reservedSectorCount;   /* 14 Number of reserved (boot) sectors.        */
	uint8_t  numberFATs;                            /* 16 Number of FAT tables hopefully 2.         */
	uint16_t maxRootDirectoryEntries;       /* 17 Number of directory slots.        */
	uint16_t totalSectos16;                 /* 19 Total sectors on disk.                    */

	uint8_t  media;                                 /* 21 Media descriptor=first uint8_t of FAT. */
																	/*    0xF8 is the standard value for fixed media */

	uint16_t sectorPerFAT16;                /* 22 Sectors in FAT.                           */
	uint16_t sectorPerTrack;                /* 24 Sectors/track.                            */
	uint16_t numberOfHeads;                 /* 26 Heads.                                                            */

	uint32_t countOfHiddenSectors;  /* 28 number of hidden sectors.                 */
	uint32_t totalSectors32;                 /* 32 big total sectors.                        */

	/* Here start the FAT32 specific fields (offset 36) */

	uint32_t sectorPerFAT32;                /* 36 32bit count of sectors occupied by each FAT. totalSectos16 must be 0    */
	uint16_t FATFlags;                              /* 40 extension flags. Usually 0.
																	 * Bits 0-3: Zero-based number of active Fat. Valid if mirroring is disabled
																	 * Bits 4-6: Reserved
																	 * Bit 7: 0 = FAT mirrored into all FATs at runtime. 1 = only 1 FAT active
																	 * Bits 8-15: Reserved */

	uint16_t version;                               /* 42 Version number of the FAT32 volume. For future extension. Must be 0:0 */
	uint32_t rootCluster;                   /* 44 start cluster of root dir. Usually 2 */
	uint16_t rootFSInfo;                    /* 48 Sector number of FS Information Sector */
	uint16_t backupBootSector;              /* 50 back up boot sector. Recomended 6 */
																	/* Sector number of a copy of this boot sector (0 if no backup copy exists) */

	uint8_t reserved[12];                   /* 52 reserved for future expansion */

	/* The following fields are present also in a FAT12/FAT16 but at offset 36. In a FAT32 they are at offset 64 instead. */

	uint8_t driverNumber;                   /* 64 physical drive ?          */
	uint8_t reserved1;                              /* 65 reserved                  */
	uint8_t extBootSignature;               /* 66 dos > 4.0 diskette. signature.    */
	uint32_t volumeID;                              /* 67 serial number             */
	uint8_t volumeLabel[11];                /* 71 disk label                */
	uint8_t FSType[8];                              /* 82 FAT type                  */
} __attribute__ ((packed)) t_fat_bootsector;

typedef struct {
	uint8_t name[8]; //Hay que ver el primer byte
	uint8_t extension[3];
	uint8_t attributes; //Si es 0x0F es una entrada de nombre largo
	uint8_t NTcase;
	uint8_t createTimeFineResolution;
	uint16_t createTimeHour;
	uint16_t createTimeDay;
	uint16_t lastAccessDay;
	uint16_t firstClusterHigh; // <--------- IMPORTANTE
	uint16_t lastModifiedHour;
	uint16_t lastModifiedDay;
	uint16_t firstClusterLow; // <--------- IMPORTANTE
	uint32_t fileSize;
} __attribute__ ((packed)) t_fat_file_data_entry;


typedef struct {
	uint8_t sequenceN;
	uint16_t nameStart[5];
	uint8_t attributes;//always 0x0F
	uint8_t reserved;//always 0x00
	uint8_t checksum;
	uint16_t nameMiddle[6];
	uint16_t firstCluster;//always 0x0000
	uint16_t nameEnd[2];
} __attribute__ ((packed)) t_fat_long_name_entry;


typedef struct{
	int hasLongNameEntry;
	t_fat_long_name_entry longNameEntry;
	t_fat_file_data_entry dataEntry;
} __attribute__ ((packed)) t_fat_file_entry;

struct fat_directory_contents {
	t_fat_file_entry fileEntry;
	struct fat_directory_contents * next;
};

typedef struct fat_directory_contents t_fat_file_list;


#endif /* FATTYPES_H_ */
