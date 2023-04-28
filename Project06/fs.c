/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern struct disk *thedisk;

//Noah - bitmap array - malloc in mount - value is 1 if used - 0 if not
int* bitmap;

//Noah - Can only mount once - value changes to 1 after successful mount
int isMounted = 0;

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
	union fs_block block;

	disk_read(thedisk,0,block.data);

	//Noah - check if a disk already mounted
	if(isMounted)
	{
		printf("Mount: already mounted disk\n");
		return 0;
	}

	if (block.super.magic != FS_MAGIC)
	{
		printf("Mount: invalid magic number\n");
		return 0;
	}

	int size = disk_size(thedisk);
	//Noah- Check if disk size and number of blocks match
	if(size != block.super.nblocks)
	{
		printf("Mount: Number of blocks and disk size do not match\n");
		return 0;
	}

	//Noah - Allocate space based on # blocks
	bitmap = (int*)malloc(block.super.nblocks * sizeof(int));
	memset(bitmap,0,block.super.nblocks);

	for (int i=1; i< block.super.ninodeblocks+1; i++)//Loop through each inode block
	{
		disk_read(thedisk, i, block.data);
		for (int j=0; j<INODES_PER_BLOCK; j++){//Loop through inodes in each block
			if (block.inode[j].isvalid)//Found valid inode - find which blocks (direct or indirect) are used by inode
			{
				for (int k=0; k< POINTERS_PER_INODE; k++)//Loop through direct pointers
				{
					if (block.inode[j].direct[k]){
						bitmap[(int)block.inode[j].direct[k]] = 1;//block is in use by direct pointer
					}
				}

				if (block.inode[j].indirect)//There is indirection
				{
					bitmap[(int)block.inode[j].indirect] = 1;//Indirect block in use
					disk_read(thedisk, block.inode[j].indirect, block.data);
					for (int k=0; k<POINTERS_PER_BLOCK; k++){// find indirect blocks
						if (block.pointers[k]){
							bitmap[block.pointers[k]] = 1;//Found indirect blocks
						}
					}
					disk_read(thedisk, i, block.data);
				}
			}
		}
	}
	for(int i = 0; i<block.super.nblocks; i++)
	{
		if(bitmap[i])
		{
			printf("%d ",i);
		}
	}
	printf("\n");
	isMounted = 1;//Noah - Successful mount - can't do another mount
	return 1;
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
