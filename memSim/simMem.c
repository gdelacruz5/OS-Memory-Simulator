#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simMem.h"
#include "linkedList.h"

int PageFaults = 0;
int TLBHits = 0;
int Frame = 0;
int RFRAMES = 256;
int PRA = 1;
Ref *refs;
int numRefs = 0;

//prints the information from the reference
void printInfo(int address, char val, int frame, unsigned char fContents[]){

   int i;
   printf("%d, %d, %d, ", address, val, frame);
   for(i = 0; i < 256; i++)
   {
      printf("%02X", fContents[i]);
   }
   printf("\n");
}

//gets the byte referenced and the contents of the frame
void getContents(int page, int offset, FILE *bS, char *val, 
   unsigned char fContents[]){

   if (fseek(bS, (offset + page * 256), SEEK_SET) != 0)
   {
      fprintf(stderr, "fseek error when getting data reference\n");
      exit(EXIT_FAILURE);
   }

   *val = getc(bS);

   if (fseek(bS, (page * 256), SEEK_SET) != 0)
   {
      fprintf(stderr, "fseek error when getting frame data\n");
      exit(EXIT_FAILURE);
   }

   if (fread(fContents, sizeof(char), 256, bS) != 256)
   {
      fprintf(stderr, "fread error when getting block data\n");
      exit(EXIT_FAILURE);
   }

   rewind(bS);
}

//deletes a node from the TLB if the TLB has 16 elements and needs to add one
ListNode* deletePage(ListNode *TLBHead, int page){

   ListNode *ptr;
   int index;

   ptr = TLBHead;

   if ((index = getIndex(ptr, page)) != -1) //if its in the TLB
   {
      ptr = deleteNode(ptr, index);
   }
   
   return ptr;
}

//returns the page that is referenced last in the page Q
int getLastUsed(ListNode *PageQ, int currentRef){

   ListNode *ptr = PageQ;
   int pageToDelete = ptr->value;
   int pageToDeleteNextUse = 0;
   int whenUsed = 0;
   int i;

   while(ptr != NULL)
   {
      for(i = currentRef + 1; (i < numRefs) && (ptr->value != refs[i].pageNum);
         i++)
      {
         whenUsed++;
      }
      if (whenUsed > pageToDeleteNextUse)
      {
         pageToDelete = ptr->value;
         pageToDeleteNextUse = whenUsed;
      }
      whenUsed = 0;
      ptr = ptr->next;
   }

   return pageToDelete;

}

//returns the page that needs to be removed from ram
int getPageToDelete(ListNode *PageQ, int currentRef){

   int pageToDelete;

   if (PRA == OPT)
   {
      pageToDelete = getLastUsed(PageQ, currentRef);
   }
   else
   {
      pageToDelete = PageQ->value;
   }

   return pageToDelete;
}

//checks to see if the page is in the page table, gives a new frame and increments page faults if needed
int checkPageT(int page, Page pageT[], ListNode **PageQ, int *deletedPage,
   int currentRef){

   int pageToDelete = -1;
   int newFrame;
   int index;

   if (pageT[page].pres == 0)
   {
      if (Frame >= RFRAMES) //if we need to remove something from RAM
      {
         pageToDelete = getPageToDelete(*PageQ, currentRef);
         newFrame = pageT[pageToDelete].frame;
         index = getIndex(*PageQ, pageToDelete);
         *PageQ = deleteNode(*PageQ, index);
         pageT[pageToDelete].pres = 0;

      }
      else //we can just add something to RAM
      {
         newFrame = Frame;
         Frame++;
      }
      *PageQ = addTail(*PageQ, page, newFrame);
      pageT[page].frame = newFrame;
      pageT[page].pres = 1;
      PageFaults++;
   }
   else if (PRA == LRU) //else if the page is in there and it's LRU do the following
   {
      if ((index = getIndex(*PageQ, page)) == -1)
      {
         fprintf(stderr, "Page is in pageTable but not in page queue\n");
         exit(EXIT_FAILURE);
      }
      *PageQ = deleteNode(*PageQ, index);
      *PageQ = addTail(*PageQ, page, pageT[page].frame);
   }

   *deletedPage = pageToDelete;

   return pageT[page].frame;
}

//checks the TLB for the frame number, increments TLB faults if it is not
int checkTLB(ListNode *TLB, int page){

   ListNode *search = TLB;

   while (search != NULL)
   {
      if (search->value == page)
      {
         TLBHits++;
         return search->frame;
      }
      search = search->next;
   }

   return -1;
}

//Goes through the references and populates the page table and counts page faults
void readRefs(){

   Page pageT[256];
   int i; //for incrementing
   int page; //the page being referenced
   int offset; //the offset from the frame
   FILE *bS; //the backing store
   char val; //the value of the byte referenced
   unsigned char fContents[256]; //the contents of the frame
   ListNode *TLBHead = NULL; //change this name to TLB head so you don't mix it with PRA head
   int frame; //the frame of the given reference
   ListNode *PageQ = NULL; //the queue for the pages
   int deletedPage = -1; //the page deleted from the page Q

   //open the backing store
   if ((bS = fopen("BACKING_STORE.bin", "r")) == NULL)
   {
      perror("BACKING_STORE.bin");
      exit(EXIT_FAILURE);
   }

   //set all pages to not present
   for(i = 0; i < 256; i++)
   {
      pageT[i].pres = 0;
   }

   for(i = 0; i < numRefs; i++)
   {
      page = refs[i].pageNum;
      offset = refs[i].offset;

      if ((frame = checkTLB(TLBHead, page)) == -1)
      {
         frame = checkPageT(page, pageT, &PageQ, &deletedPage, i);
         if (deletedPage != -1)
         {
            TLBHead = deletePage(TLBHead, deletedPage);
         }
         else if ((Frame > RFRAMES) || (Frame > 16))
         {
            TLBHead = deleteNode(TLBHead, 0);
         }
         TLBHead = addTail(TLBHead, page, frame);
      }
      getContents(page, offset, bS, &val, fContents);
      printInfo(refs[i].add, val, frame, fContents);
   }

   while (TLBHead != NULL) //free the TLB
   {
      TLBHead = deleteNode(TLBHead, 0);
   }

   while (PageQ != NULL) //free the PageQ
   {
      PageQ = deleteNode(PageQ, 0);
   }

   fclose(bS);
}

//simulates the memory accesses given in the input file using the frames and PRA given
void simMem(FILE *fp, int frames, int pra){

   char line[100]; //string for scanning the lines of the input file
   int ref; //for the input to be scanned into
   int i; //incrementer
   int check; //for error checking

   //set the RAM frames and the PRA to those given
   RFRAMES = frames;
   PRA = pra;

   while(fgets(line, 100, fp) !=NULL)
   {
      numRefs++;
   }

   rewind(fp);

   refs = malloc(numRefs * sizeof(Ref));
   if (refs == NULL) //error checking for malloc failure
   {
      fprintf(stderr, "malloc failure\n");
      exit(EXIT_FAILURE);
   }

   //scans the references into the reference array
   for(i = 0; i < numRefs; i++)
   {
      check = fscanf(fp, "%d\n", &ref);
      if ((check !=1) || (ref < 0) || (ref > 65535)) //error checking for input file format
      {
         fprintf(stderr, "Input text is not formatted properly\n");
         fprintf(stderr, "Usage: <reference ");
         fprintf(stderr, "(must be a positive number 0 <= n < 65536)>\n");
         exit(EXIT_FAILURE);
      }

      (refs[i]).add = ref;
      (refs[i]).pageNum = (ref >> 8);
      (refs[i]).offset = (ref & 255);
   }

   //read the references and count page faults/TLB Hits
   readRefs();

   printf("Number of Translated Addresses = %d\n", numRefs);
   printf("Page Faults = %d\n", PageFaults);
   printf("Page Fault Rate = %.3f\n", (PageFaults * 1.0 / numRefs));
   printf("TLB Hits = %d\n", TLBHits);
   printf("TLB Misses = %d\n", (numRefs - TLBHits));
   printf("TLB Hit Rate = %.3f\n", (TLBHits * 1.0 / numRefs));

   free(refs);
}
