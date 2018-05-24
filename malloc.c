/************************************************************************
On my honor, I have neither given nor received any academic 
aid or information that would violate the Honor Code of Mars 
Hill University

malloc.c  by Derek T. Klapchar

The malloc library takes care of allocating and releasing memory to the system.

It keeps track of a list of free memory blocks that can be used to fullfil
requests for dynamic memory.

The individual functions are described in the comments below

If malloc can't fill the request, it returns NULL.

calloc() calls malloc and initializes the integer array to all zeroes

realloc() checks to see if there is enough extra in the current memory
block for the new size requested and, if so, returns the pointer it was sent.  
Otherwise, it gets a new block of the right size and copies
the old info into this new block, making no guarantees with regard to
what is in the extra part of the new memory.

free() takes a block of memory supplied by the user. If valid, and if size is
sufficient, it puts it on the free list.

***********************************************************************/

#include "myMalloc.h" 

Flist flist=NULL;//pointer to start of free list
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**********************************************************************
malloc() 

malloc makes sure there are at least BOOKKEEPING (24) bytes carved off
plus enough more to keep it aligned on 16 byte boundaries.

malloc pads the total size to a multiple of ALIGNMENT (16) to avoid 
bus errors (rare anymore) and to make sure performance doesn't suffer.

It then searches the free list looking for a chunk of memory of sufficient size
to use to satisfy the request. It chooses the first fit.

If the first fit has enough left over to satisfy at least a minimal request, it 
carves off only what is needed. Otherwise, it returns the whole chunk. 

If the free list does not contain enough memory, malloc calls sbrk() to get exactly
what is needed for this request.

If the request cannot be filled, malloc returns NULL

**********************************************************************/

void *myMalloc(size_t requestedSize)
{ 
  size_t sizeNeeded; //the size we need (user request plus bookkeeping) 
  Flist runner;//for searching the free list
  Flist carver;//for carving chunks off larger chunks
  size_t sbrkSize=SBRK_SIZE;//the size needed if we must call sbrk()
  //bool reFlag=false;//flag to help us re-enter the loop in the right place
  
  //printf("****************** Calling Malloc **********************\n");
  
  pthread_mutex_lock(&lock);//keep this thread safe
  
  if(requestedSize<=0)//if user asked for 0 or fewer bytes
    { 
      errno=EINVAL;//set an error code and return NULL
      pthread_mutex_unlock(&lock);
      return(NULL);
    } 
  
  sizeNeeded=requestedSize+SIZE_OFFSET; //add extra bytes plus alignment requirements
  // needed for storing size of chunk behind the pointer we send to the user
  
  if(sizeNeeded<BOOKKEEPING)//make sure there is enough space for entire bookkeeping structure
    sizeNeeded=BOOKKEEPING;//plus alignment
  
  while(sizeNeeded%ALIGNMENT!=0) //pad to a multiple of ALIGNMENT to avoid bus errors
    sizeNeeded++;
  
  while(1)//do this until we return, one way or another
    {
      if(flist!=NULL){//if the free list is not empty
		runner = flist;
	do{ 
		if(runner->size >= sizeNeeded){
			if(runner->size - sizeNeeded < 32){
				if(runner->blink == runner){
					flist = NULL;
					pthread_mutex_unlock(&lock);
					return (&(runner-> blink));
				}
					runner->flink->blink = runner->blink;
					runner->blink->flink = runner->flink;
					pthread_mutex_unlock(&lock);
					return (&(runner-> blink));
			}else{
				carver = (Flist)((void *) runner + (runner-> size - sizeNeeded));
				runner->size = runner->size - sizeNeeded;
				carver->size = sizeNeeded;
				pthread_mutex_unlock(&lock);
				return(&(carver-> blink));//maybe carver instead of runner
			}
		}
	  
	    runner = runner->flink;
		}while(runner!=flist);//as long as we haven't gotten back to the start, continue 
      
	  }
      
      //if we get here, the free list was either empty, or there was not a block of 
      //sufficient size to fulfill request
      
      //determine if we need to get min amount
      //or if we need to get a larger size to match the user's request
      
      //get new, large chunk
      if(sizeNeeded < sbrkSize){
		carver = sbrk(sbrkSize);
		carver->size = sbrkSize;
	  }else{
		carver = sbrk(sizeNeeded);
		carver-> size = sizeNeeded;
	  }
      if(carver==(Flist)-1){//if sbrk returned -1, error
	  errno=ENOMEM;
	  pthread_mutex_unlock(&lock);
	  return (NULL);
	}
      
      //set size of this chunk	 
     	//carver->size = carver-> size - sizeNeeded; 
      //did we carve off exactly what we needed?
      //if so, go straight to a return and don't bother putting it on free list...
      //unlock the mutex and return the address of blink
     if(carver->size - sizeNeeded == 0){
	  pthread_mutex_unlock(&lock);
	  return (&(carver->blink));
	 }//else{
		//carver->blink = flist->blink;
		//flist->blink = carver;
		//carver-flink = flist;
	 //}
      //...otherwise, add it to the free list     
      
      if(flist==NULL)//if the free list is empty
	{
	  
	  flist = carver;	
	  flist->flink = flist;
	  flist->blink = flist;
	}
      else//otherwise, add it to the end of the list
	{	//(because it will have the highest address,it will go at the end)
	  //add to end of list. After we do this, the while(1) will start over and we will search the free list again
	  //this time, we will find what we need
		carver->flink = flist;
		carver->blink = flist->blink;
		flist->blink = carver;
		carver->blink->flink = carver;
	}
    }
}


/*********************************************************************

free() puts a block of memory on the free list.  If the free list
is empty, it creates one.

*********************************************************************/

void myFree(void *ptr)
{
  Flist memToFree; 
  Flist runner=NULL;
  
  pthread_mutex_lock(&lock);//make it thread safe
  
  if(((long int)ptr%ALIGNMENT)!=0)//if the pointer given is not a multiple of ALIGNMENT
    { 
      errno=EINVAL;//set error and leave
      pthread_mutex_unlock(&lock);
      return;
    }
  
  memToFree=(Flist)(ptr-SIZE_OFFSET);//point the bookkeeping structure
  
  if(memToFree->size>=BOOKKEEPING)//only add to the free list if there is
    { //enough space in the block to accomodate the bookkeeping structure plus alignment
      
      if(flist==NULL)//if free list is empty, start one
	{
	  /* To Do */
	  //3 lines of code
	 //flist = memToFree;
	 //memToFree = runner-> flink;
	 //memToFree = runner-> blink;
	  flist = memToFree;	
	  flist->flink = flist;
	  flist->blink = flist;
	}
      
      //otherwise, just add it at the end of the list and don't try to coalesce the chunks
      
      else
	{
	  /* To do */
	  //4 lines of code
	  //flist = runner-> flink;
	  //flist = runner-> blink;
	  //memToFree = runner-> flink;
	  //memToFree = runner-> flink;
		memToFree->flink = flist;
		memToFree->blink = flist->blink;
		flist->blink = memToFree;
		memToFree->blink->flink = memToFree;
	}
      
      pthread_mutex_unlock(&lock);
      return;
    }
}


/**********************************************************************
calloc() allocates enough space for an array of "n" elements each with
a size "size".  It then initializes all the memory to zero.
**********************************************************************/

void *calloc(size_t n, size_t requestedSize){
  void *temp;
  
  printf("Allocate some memory and clear it\n");
  
  return(NULL);
}

/*******************************************************************
realloc() takes a pointer to memory previously allocated by malloc and
an integer with the number of bytes now requested for a replacement.
  
It checks to see if, by any chance, there is enough in the space it 
already has, otherwise it gets more space and copies all the contents 
of the original storage into the new storage space and returns a 
pointer to this new space. It then frees up the old space.

If it already has enough in the original space, it attempts to return 
any leftover to the free list.
*******************************************************************/

void *realloc(void *ptr, size_t newsize){
  size_t size, sizeNeeded;
  Flist temp;
  void *newMem;

  printf("Re-allocate some memory\n");
  
  if((long int)ptr%ALIGNMENT!=0){//make sure the pointer given is aligned
      errno=EINVAL;
      return(NULL);
    }
  return(NULL);
 
}

/*********************************************************************
This function simply traverses the free list and prints the tag structures
so we can verify the operations on the free list. This function is used
only for testing and debugging purposes.
*********************************************************************/
void printFreeList(void)
{
  Flist runner;
  
  pthread_mutex_lock(&lock);
  printf("*******Start Free List*******\n\n");
  runner=flist;
  
  if(!runner)
    printf("Empty\n\n");
  else 
    do{
      printBookStruct(runner);
      runner=runner->flink;
    }while(runner!=flist);
  
  printf("********End Free List********\n\n");
  pthread_mutex_unlock(&lock);
}

/*********************************************************************
This function takes a pointer to a bookkeeping structure and prints the
address and all fields. This function is used only for testing and 
debugging purposes.
*********************************************************************/
void printBookStruct(Flist temp)
{
  printf("Address: %p", temp);
  printf("\nSize:    %lu (0x%lx)", temp->size, temp->size);
  printf("\nFlink:   %p", temp->flink);
  printf("\nBlink:   %p\n\n", temp->blink);
}

/*********************************************************************
This function takes a pointer to a bookkeeping structure and prints the
Address and the size fields. This function is used only for testing and 
debugging purposes.
*********************************************************************/
void printVitals(Flist temp)
{
  //printf("\n************ Book Struct ***********\n");
  printf("Address: %p\n", temp);
  printf("Size:    %lu (0x%lx)\n", temp->size, temp->size);
  //printf("************************************\n\n");
}

/*********************************************************************
This function simply subtracts from the malloc-supplied point the number 
of bytes needed to find the size of the chunk.

Don't use this in malloc, it is only for using in the test programs.
*********************************************************************/
Flist getPtrToBookStruct(void *ptr)
{
  return ptr-SIZE_OFFSET;
}
