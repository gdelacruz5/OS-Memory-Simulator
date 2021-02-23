#ifndef SIMMEM_H
#define SIMMEM_H

#define FIFO 1
#define LRU 2
#define OPT 3

typedef struct{
   int add; //address given in reference file
   int pageNum; //the page number
   int offset; //the page offset
} Ref;

typedef struct{
   int frame;
   int pres;
} Page;

void simMem(FILE *fp, int frames, int pra);

#endif
