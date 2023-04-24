/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>

extern struct disk *thedisk;

int fs_format()
{
	return 0;
}

void fs_debug()
{
	union fs_block block;

	disk_read(thedisk,0,block.data);

	printf("superblock:\n");
	if (block.super.magic == FS_MAGIC){
		printf("magic number is valid\n");
	}
	else {
		printf("magic number is invalid\n");
		return;
	}

	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);

	for (int i=1; i< block.super.ninodeblocks+1; i++) // not sure yet 
	{
		disk_read(thedisk, i, block.data);
		for (int j=0; j<INODES_PER_BLOCK; j++){
			if (block.inode[j].isvalid)
			{
				printf("inode %d:\n",j); // print which inode?
				printf("    size: %d bytes\n",block.inode[j].size); // print size?
				printf("    direct blocks:");

				//for loop to print all the direct blocks
				for (int k=0; k< POINTERS_PER_INODE; k++)
				{
					if (block.inode[j].direct[k]){
						printf(" %d",block.inode[j].direct[k]);
					}
				}
				printf("\n"); 

				if (block.inode[j].indirect)
				{
					printf("    indirect blocks: %d\n",block.inode[j].indirect); // find indirect block
					printf("    indirect data blocks: "); 
					disk_read(thedisk, block.inode[j].indirect, block.data);
					for (int k=0; k<POINTERS_PER_BLOCK; k++){
						if (block.pointers[k]){
							printf("%d ", block.pointers[k]);
						}
					}
					printf("\n");
					disk_read(thedisk, i, block.data);
				}
			}
		}
	}
}

int fs_mount()
{
	return 0;
}

int fs_create()
{
	return 0;
}

int fs_delete( int inumber )
{
	return 0;
}

int fs_getsize( int inumber )
{
	return 0;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
