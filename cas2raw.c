/*--------------------------------------------------------------------*/
/* CAS2RAW                                                            */
/* by GienekP                                                         */
/* (c) 2024                                                           */
/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
/*--------------------------------------------------------------------*/
#define CASMAX (1024*1024)
/*--------------------------------------------------------------------*/
unsigned int loadCAS(const char *filename, U8 *data, unsigned int size)
{
	FILE *pf;
	unsigned int i,ret=0;
	for (i=0; i<size; i++) {data[i]=0x00;};
	pf=fopen(filename,"rb");
	if (pf)
	{
		ret=fread(data,sizeof(U8),size,pf);
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int saveRAW(const char *filename, U8 *data, unsigned int size)
{
	FILE *pf;
	unsigned int ret=0;
	pf=fopen(filename,"wb");
	if (pf)
	{
		ret=fwrite(data,sizeof(U8),size,pf);
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int b2i(U8 *data)
{
	return ((data[3]<<24) | (data[2]<<16) | (data[1]<<8) | data[0]);
}
/*--------------------------------------------------------------------*/
unsigned int b2w(U8 *data)
{
	return ((data[1]<<8) | data[0]);
}
/*--------------------------------------------------------------------*/
void assignName(char *fn, unsigned int pos, U8 value)
{
	fn[pos+1]=((value % 10)+'0');
	fn[pos]=(((value/10) % 10)+'0');
}
/*--------------------------------------------------------------------*/
void scanCAS(char *fn, unsigned int pos, U8 *data, unsigned int size)
{
	unsigned int f=0,p=0,i,r=0;
	U8 buf[CASMAX];
	while (p<size)
	{
		unsigned int t=b2i(&data[p]);
		unsigned int l=b2w(&data[p+4]);
		unsigned int a=b2w(&data[p+6]);
		U8 *d=&data[p+8];
		p+=(8+l);
		if (t==0x494A5546)
		{
			printf("FUJI: ");
			for (i=0; i<l; i++) {printf("%c",d[i]);};
			printf("\n");
		}
		else if (t==0x64756162)
		{
			printf("BOUD: %i\n",a);
			if (a>1300) {p=size;};
		}
		else if (t==0x61746164)
		{
			printf("DATA: %i bytes %i ms\n",l,a);
			if (a>10000)
			{
				if (r)
				{
					saveRAW(fn,buf,r);
					r=0;						
				};
				if (f)
				{
					assignName(fn,pos,f);
				};
				f++;
			}
			for (i=3; i<(l-1); i++)
			{
				if (r<CASMAX)
				{
					buf[r]=d[i];
					r++;
				}
				else {p=size;};
			};
		}
		else if (t==0x206B7366)
		{
			printf("FSK: %i bytes %i ms\n",l,a);
			if (r)
			{
				saveRAW(fn,buf,r);
				r=0;						
			};
			if (f)
			{
				assignName(fn,pos,f);
			};
			f++;			
		}
		else
		{
			printf("Unknown: %i\n",t);
			p=size;
		};
	};
	if (r)
	{
		saveRAW(fn,buf,r);
		r=0;
	};
}
/*--------------------------------------------------------------------*/
void cas2raw(const char *casfn, char *blckfn)
{
	U8 casdata[CASMAX];
	unsigned int cassize,i=0;
	cassize=loadCAS(casfn,casdata,sizeof(casdata));
	if (cassize)
	{
		printf("Load \"%s\" [%i bytes]\n",casfn,cassize);
		while (blckfn[i]) {i++;};
		if (i>6) {scanCAS(blckfn,i-6,casdata,cassize);}
		else {printf("File name \"%s\" to short\n",blckfn);};
	}
	else
	{
		printf("Load \"%s\" ERROR!\n",casfn);
	};
}
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{	
	printf("CAS2RAW - ver: %s\n",__DATE__);
	if (argc==3)
	{
		cas2raw(argv[1],argv[2]);
	}
	else
	{
		printf("(c) GienekP\n");
		printf("use:\ncas2raw file.CAS file00.raw\n");
	};
	printf("\n");
	return 0;
}
/*--------------------------------------------------------------------*/
