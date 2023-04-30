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
	if(isMounted){//Noah - Cannot format if disk mounted
		printf("Format: Disk already mounted\n");
		return 0;
	}

	union fs_block superblock;//Noah - Write new superblock
	superblock.super.magic = FS_MAGIC;//Assign Magic

	int size = disk_size();//Get number of blocks on disk
	superblock.super.nblocks = size;

	int inodeblocks = size/10;
	if(size%10 != 0)//Round up - there is a remainder
	{
		inodeblocks++;
	}
	superblock.super.ninodeblocks = inodeblocks;

	int inodes = inodeblocks * INODES_PER_BLOCK;
	superblock.super.ninodes = inodes;

	disk_write(thedisk,0,superblock.data);//Write new superblock

	//Noah- Write all inodes - invalid all inodes
	union fs_block inodeblock;

		for (int i=1; i< inodeblocks+1; i++)
	{
		for (int j=0; j<INODES_PER_BLOCK; j++){
			if (inodeblock.inode[j].isvalid)
			{
				inodeblock.inode[j].isvalid = 0;//Set to invalid

				for (int k=0; k< POINTERS_PER_INODE; k++)
				{
					if (inodeblock.inode[j].direct[k]){//Point to no blocks
						inodeblock.inode[j].direct[k] = 0;
					}
				}
				inodeblock.inode[j].indirect = 0;//No indirection
			}
		}
		disk_write(thedisk,i,inodeblock.data);//Noah - Write each inode block
	}
	return 1;
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
						bitmap[block.inode[j].direct[k]] = 1;//block is in use by direct pointer
					}
				}

				if (block.inode[j].indirect)//There is indirection
				{
					bitmap[block.inode[j].indirect] = 1;//Indirect block in use
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
	isMounted = 1;//Noah - Successful mount - can't do another mount
	return 1;
}

int fs_create()
{
	if(!isMounted)//Noah - Can't create with unmounted disk
	{
		printf("Create: No mounted disk\n");
		return 0;
	}
	union fs_block block;//Need superblock to know how many inode blocks
	disk_read(thedisk,0,block.data);

	int inumber;//Noah - Need to find empty inode
	union fs_block inodeblock;
	for(int i=1; i< block.super.ninodeblocks+1; i++)
	{
		disk_read(thedisk, i, inodeblock.data);
		for (int j=1; j<INODES_PER_BLOCK; j++){
			if (!inodeblock.inode[j].isvalid)//Found an invalid inode - make it valid
			{
				inumber = (INODES_PER_BLOCK * (i-1)) + j;//Get unique inumber
				inodeblock.inode[j].isvalid = 1;//Valid inode
				inodeblock.inode[j].size = 0;//Make size 0
				for(int k = 0; k<POINTERS_PER_INODE; k++)//Don't point to any existing direct/indirect blocks
				{
					inodeblock.inode[j].direct[k] = 0;
				}
				inodeblock.inode[j].indirect = 0;
				disk_write(thedisk,i,inodeblock.data);
				return inumber;
			}
		}
	}
	printf("Create: Could not find empty inode\n");
	return 0;
}

int fs_delete( int inumber )
{
	if(!isMounted)//Noah - can't delete on an unmounted disk
	{
		printf("Delete: No mounted disk\n");
		return 0;
	}

	union fs_block superblock;//Noah - Need superblock information to get where inode is in blocks
	disk_read(thedisk,0,superblock.data);

	if(inumber > superblock.super.ninodes || inumber <= 0)//Invalid inumber - either negative, 0, or larger than total inodes
	{
		printf("Delete: Invalid inumber\n");
		return 0;
	}

	int blockNum = inumber / INODES_PER_BLOCK;//Noah - Which inode block is inumber in?
	int blockIndex = inumber - (INODES_PER_BLOCK*blockNum);
	blockNum++;//inode blocks start at block 1 due to superblock being block 0;

	union fs_block inodeblock;
	disk_read(thedisk,blockNum,inodeblock.data);//Read correct inode block where inode is

	if(!inodeblock.inode[blockIndex].isvalid)//Noah - can't delete invalid inode
	{
		printf("Delete: inumber is not valid\n");
		return 0;
	}

	inodeblock.inode[blockIndex].isvalid = 0;//Mark as invalid

	for(int j = 0; j<POINTERS_PER_INODE; j++)//Noah - Free direct pointers in inode
	{
		if(inodeblock.inode[blockIndex].direct[j])
		{
			bitmap[inodeblock.inode[blockIndex].direct[j]] = 0;//Mark as free in bitmap
		}
	}

	if(inodeblock.inode[blockIndex].indirect)//Noah - There are indirect pointers to free
	{
		for (int k=0; k<POINTERS_PER_BLOCK; k++){
			if (inodeblock.pointers[k]){
				bitmap[inodeblock.pointers[k]] = 0;//Free indirect blocks in bitmap
			}
		}
	}

	disk_write(thedisk,blockNum,inodeblock.data);//Noah - Write back to disk
	return 1;
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
