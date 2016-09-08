#include "ext2.h"
#define BLK 1024

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct ext2_group_desc 	GD;
typedef struct ext2_inode				INODE;
typedef struct ext2_dir_entry_2	DIR;

u16 NSEC = 2;
char buf1[BLK], buf2[BLK]; //2 I/O buffers of 1KB each

int prints (char *s)
{
  char i = *s;
  while(i != 0)
  {
    i = *(s++);
    putc(i);
  }
}

int gets(char *s)
{
	//to keep things simple, no length checking
	while ((*s = getc()) != '\r')
		putc(*s++);
	*s = 0;
}

int getblk(u16 blk, char *buf)
{
	readfd(blk/18, ((2*blk)%36)/18, ((2*blk)%36)%18, buf);
}

u16 search (INODE *ip, char *name)
{
	int i;
	char c;
	DIR *dp;

	for(i = 0; i < 12; i++) //assume DIR has at most 12 direct blocks
	{
		if ((u16)ip->i_block[i])
		{
			getblk((u16)ip->i_block[i], buf2);
			dp = (DIR *)buf2;

			while((char*)dp < &buf2[BLK])
			{
				c = dp->name[dp->name_len]; 	//save the last byte
				dp->name[dp->name_len] = 0; 	//make name into a string
				prints(dp->name); 
				putc(' ');										//show dp->name string

				if(strcmp(dp->name, name) == 0)
				{
					prints("\n\r");
					return((u16)dp->inode);
				}

				dp->name[dp->name_len] = c;		//restore the last byte
				dp = (char *)dp + dp->rec_len;
			}
		}
	}

	error(); //if we can't find the file name
}

main()
{
	char 	*cp, *name[2], filename[64];
	u16 	i, ino, blk, iblk;
	u32 	*up;
	
	GD 		*gd;
	INODE *ip;
	DIR		*dp;

	name[0] = "boot";
	name[1] = filename;

	prints("bootname: ");
	gets(filename);

	if(filename[0] == 0)
		name[1] = "mtx";

	getblk(2, buf1);										//read blk#2 to get GD 0
	gp = (GD *)buf1;
	
	iblk = (u16)gp->bg_inode_table;			//inodes begin block
	getblk(iblk, buf1);									//read first inode block

	ip = (INODE *)buf1 + 1;							//ip->root inode #2
	
	for(i = 0; i < 2; i++)							//search for system name
	{
		ino = search(ip, name[i]) -1;
		if(ino < 0)
			error();

		getblk(iblk + (ino/8), buf1);			//read inode block of ino
		ip = (INODE *)buf1 + (ino % 8);
	}

	if((u16)ip->i_block[12]) 		//read indirect block into buf2, if any
		getblk((u16)ip->i_block[12], buf2);

	setes(0x1000);							//set ES to loading segment
	
	for(i = 0; i < 12; i++)
	{
		getblk((u16)ip->i_block[i], 0);
		inces();
		putc('*');								// show a * for each direct block loaded
	}

	if((u16)ip->i_block[12]) 		//load indirect blocks, if any
	{
		up = (u32 *)buf2;

		while(*up++)
		{
			getblk((u16)*up, 0);
			inces();
			putc('.');							//show a . for each indirect block loaded
		}
 	}

	prints("ready to go?");
	getc();
}
