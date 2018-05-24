#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include "myMalloc.h"

#define SIZE 1024
#define PROMPTS

void printMallocList(char *test[], int size);
int comparePtrs(const void *a, const void *b);

int main(void){
  char *test[SIZE];
  Flist temp;
  char choice[1024]="m";
  int i=0;
  int size;
  char junk;
  int toFree;
  int j;
  
  for(i=0;i<SIZE;i++)
    test[i]=NULL;
  
  i=0;
  
  while(choice[0]!='q'){
     
    #ifdef PROMPTS
    printf("MENU:\n(m)alloc\n(f)ree\nprint f(r)ee list\nprint m(a)lloc list\n(q)uit? > ");
    #endif

    choice[0]=getchar();
    junk=getchar();
    
    switch(choice[0]){
    case 'q':
      exit(0);
    case 'm':

      for(i=0;i<SIZE;i++)
	if(test[i]==NULL)
	  break;
	
      if(i==SIZE){
	printf("\nSorry, no more mallocing allowed in this test program\n\n");
	break;
      }
      #ifdef PROMPTS
      printf("\nHow much would you like to request? >");
      #endif
      scanf("%d", &size);
      junk=getchar();
      printf("\n-----------------------------\n");
      printf("Requesting %d byte(s)\n\n", size);
      test[i]=(char *)myMalloc(size);

      if(test[i]==NULL){
	printf("ERROR: Out of memory\n");
	exit(1);
      }

      temp=getPtrToBookStruct(test[i]);
      printf("What we got:%p\n\nBookkeeping Struct:\n", test[i]);
      printVitals(temp);
      printf("-----------------------------\n\n");
      i++;   
      break;
    case 'f':
      #ifdef PROMPTS 
      printMallocList(test, SIZE);
      printf("\nWhich one would you like to free? > ");
      #endif

      scanf("%d", &toFree);
      junk=getchar();
      
      if(toFree>31 || toFree<0 || !test[toFree]){
	printf("That is an invalid choice\n");
	break;
      }
      
      printf("Releasing memory at address %p (item %d)\n", test[toFree], toFree);
      myFree(test[toFree]);
      test[toFree]=NULL;
      break;
      
    case 'r':
      printf("\n");
      printFreeList();
      printf("\n");
      break;

    case 'a':
      printMallocList(test, SIZE);
      break;

    default:
      printf("\nInvalid Choice, try again\n");
      break;
    }
  }
}

void printMallocList(char *test[], int size){
  int i;
  Flist temp;
  int j;

  printf("\n######### Start Malloced list #########\n");

  for(i=0;i<size;i++){
    
    if(test[i]!=NULL){
      printf("\n");
      printf("-------- item # %d ----------\n", i);
      temp=getPtrToBookStruct(test[i]);
      printVitals(temp);
      printf("-----------------------------\n");
    } 
  }
  printf("\n######### End Malloced list ##########\n\n");  
}
int comparePtrs(const void *a, const void *b){
  return(*(char *)a - *(char *)b); 
}
