/* Gian Delacruz
 * CPE 453
 * Professor Peterson
 * Program 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simMem.h"

//checks the PRA given 
void checkPRA(char *arg, int *pRA){

   if (strcmp("OPT", arg) == 0)
   {
      *pRA = OPT;
   }
   else if (strcmp("LRU", arg) == 0)
   {
      *pRA = LRU;
   }
   else if (strcmp("FIFO", arg) == 0)
   {
   }
   else
   {
      fprintf(stderr, "PRA must be either FIFO, LRU, or OPT\n");
      exit(EXIT_FAILURE);
   }
}
//checks the argument for the number of frames
void checkFrames(char *arg, int *frames){
   
   int check;

   check = sscanf(arg, "%d", frames);
   if (check != 1)
   {
      fprintf(stderr, "Frame value given must be an integer\n");
      exit(EXIT_FAILURE);
   }

   if ((*frames <= 0) || (*frames > 256))
   {
      fprintf(stderr, "Frame value must be greater than zero and less than 256\n");
      exit(EXIT_FAILURE);
   }
}

FILE* openFile(char *fileName){

   FILE *fp;

   if ((fp = fopen(fileName, "r")) == NULL)
   {
      perror(fileName);
      exit(EXIT_FAILURE);
   }

   return fp;
}

FILE* checkArgs(int argc, char *argv[], int *frames, int *pRA){

   if (argc == 4)
   {
      checkFrames(argv[2], frames);
      checkPRA(argv[3], pRA);
   }
   else if (argc == 3)
   {
      checkFrames(argv[2], frames);
   }
   else if (argc == 2)
   {
   }
   else
   {
      fprintf(stderr, "Usage: memSim <reference-sequence-file.txt> <FRAMES> <PRA>\n");
      exit(EXIT_FAILURE);
   }

   return openFile(argv[1]);
}


int main (int argc, char *argv[]){

   FILE *fp;
   int frames = 256;
   int pRA = FIFO;

   fp = checkArgs(argc, argv, &frames, &pRA);

   simMem(fp, frames, pRA);

   fclose(fp);

   return EXIT_SUCCESS;
}
