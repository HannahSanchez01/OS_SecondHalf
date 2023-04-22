
#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int direct[POINTERS_PER_INODE];
	int indirect;
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

int fs_format()
{
	return 0;
}

/*
 * Scan a mounted filesystem and report on how the inodes
 * and blocks are organized. If you can write this function,
 * half the battle is over. After scanning and reporting
 * upon the file system structures, the rest is easy.
 * 
 * Output example:
 * superblock:
		magic number is valid
		1010 blocks on disk
		101 blocks for inodes
		12928 inodes total
	inode 3:
		size: 45 bytes
		direct blocks: 103
	inode 5:
		size 81929 bytes
		direct blocks: 105 109 .. ..
		indirect block: 210
		indirect data blocks: 211 212 213 214 ...



		60% Operate correctly given a proper file block of varying sizes.  
		You do not need to assess whether or not the ratio of nodes to data 
		is correct, only that the file system itself checks out.  Your 
		output should match the expected output as described in the writeup 
		for Project 6.  

		30% Operate correctly without crashing on file blocks that are 
		not set up correctly applying appropriate sanity checks to ensure 
		robust operation.  For example, what if the various counts do not 
		match up correctly (the disk block is 20 blocks but the super block 
		says there are 1000 blocks)?  What if the magic number is incorrect?
		What if a direct block or indirect block is out of range? Note that 
		if the magic number fails, you can stop right there.  
 */

void fs_debug()
{
	union fs_block block;

	disk_read(0,block.data);

	/* SUPERBLOCK */
	printf("superblock:\n");

   // TODO: magic num always 0?
	if (block.super.magic == FS_MAGIC )  // check magic number validity
	{
		printf("magic number is valid\n");
	}
	else
	{
		printf("magic number is invalid\n");
		printf("magic number=%d \n", block.super.magic);
		return; // disk fails
	}

	printf("    %d blocks\n",block.super.nblocks);
	// ninodeblocks should be 10% of nblocks, rounding up
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);


	/* INODE */
   // Might need a for loop encasing //////////////// for inode
	for (int i=0; i< INODES_PER_BLOCK; i++)
	{
		if (block.inode[i].isvalid)
		{
			printf("inode %d:\n",i); // print which inode?
			printf("    size: %d bytes\n",block.inode[i].size); // print size?
			printf("    direct blocks:");
			//for loop to print all the direct blocks
			for (int j=0; j< POINTERS_PER_INODE; j++)
			{
				printf(" %d",block.inode[i].direct[j]);
			}
			printf("\n"); // newline

			if (block.inode[i].indirect)
			{
				printf("    indirect blocks: %d",block.inode[i].indirect); // find indirect block
				printf("    indirect data blocks:"); // TODO
				// for loop to print all the indirect data blocks
				// for (int i=0; i< ; i++)
				// {
				//		printf(" %d",);
				// }
				// printf("\n");
				// /////////////////////////////////////////////// end for inode
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
	return -1;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
