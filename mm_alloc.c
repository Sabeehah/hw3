/*
 * mm_alloc.c
 
 Sabeehah Ismail (797 621)
 
 Summary:
 
 	A) My mm_malloc function will first check if the base is initialised or not (NULL).
 	
	   If the base is initialised then:
	   
	   *It will look for a chunk of memory that is free and also where the size is enough as needed; the find_block function.
	   *If the right chunk of memory is found:
	   
	               i.  The split_block function will try to split the memory chunk.
	               ii. It will then mark the memory chunk as used, with the free function.
	               
	   *If we can't find an appropriate chunk of free memory, then we extend the heap.
	  
	  If the base is not initialised (NULL) then:
	  
	  *The heap will be extended with the extend_heap function.
	  *The *l pointer in the find_block function points to the memory chunk that we visited last; this allows us to access it during the extension so that we do not have to traverse through the entire list again.

        B) My mm_realloc function will:
        
        *Use my mm_malloc function to find a new memory chunk of a required size, and copy the data from the previous block to this new block.
        *The previous block will then be free.
        *The function will return the new pointer.
        
        *However, if there is a sufficient amount of memory available then we don't have to re-allocate a new chunk of memory. The same goes for the case where the size doesn't change.
        *If the next memory block is free and has sufficent space available, it fuses (fusion function) and will try to split if the need occurs.
        *If the chunk gets smaller, it will try to split with the split_block function.
	
	C) My mm_free function will:
	
	*Mark the next block free and fuse with it (if it can) when veifying the pointer. If the pointer is invalid, nothing will be done.
	
	*If it reaches the end of the heap, it will attempt to release memory by putting a break "brk(bptr)" at the memory chunk's position.
	
 */
 
#include "mm_alloc.h"
#include <stdlib.h>

/* Your final implementation should comment out this macro. */
//#define MM_USE_STUBS

void *base;

s_block_ptr extend_heap(s_block_ptr l, size_t s){

	s_block_ptr bptr;

	bptr = sbrk(0);

	if(sbrk(BLOCK_SIZE + s) == (void*)-1){

		return (NULL);
	}

	bptr->size = s;

	bptr->next = NULL;

	if (l){


		l->next = bptr;
	}

	bptr->free = 0;

	return (bptr);

}

s_block_ptr find_block(s_block_ptr *l, size_t size){

	base = NULL;
	s_block_ptr bptr;
	bptr = base;

	while(bptr && !(bptr->free && bptr->size >= size)) {

		*l = bptr;
	
		bptr = bptr->next;

	}

	return (bptr);

}

void split_block(s_block_ptr bptr, size_t s){

	s_block_ptr n;

	n = bptr->data + s;

	n->size = bptr->size - s - BLOCK_SIZE;

	n->next = bptr->next;

	n->free = 1;

	bptr->size = s;

	bptr->next = n;

}

void copy_block(s_block_ptr src, s_block_ptr dst){

	int *sdata;
	int *ddata;

	size_t i;


	ddata = dst->ptr;
	sdata = src->ptr;

	for(i=0; i*4<src->size && i*4<dst->size; i++){

		ddata[i] = sdata[i];
	}

}

s_block_ptr fusion(s_block_ptr bptr){

	if(bptr->next && bptr->next->free){

		bptr->size += BLOCK_SIZE + bptr->next->size;

		bptr->next = bptr->next->next;

		if (bptr->next){

			bptr->next->prev = bptr;
		}

	}

	return (bptr);

}

int valid_addr(void *p){

	base = NULL;

	if(base){

		if( p>base && p<sbrk(0)){

			return(p == (get_block(p))->ptr);

		}

	}

	return (0);

}

s_block_ptr get_block(void *p){

	char *tmp;

	tmp = p;

	return (p = (tmp -= BLOCK_SIZE));

}

void* mm_malloc(size_t size){

	#ifdef MM_USE_STUBS
		return calloc(1, size);
	#else

	base = NULL;

	#define align4(size)

	s_block_ptr l;
	s_block_ptr bptr;
	
	size_t s;

	s = size;

	if (base){

		l = base;

		bptr = find_block(&l,s);

		if (bptr){

			if ((bptr->size - s) >= (BLOCK_SIZE + 4)){

				split_block(bptr,s);
			}

			bptr->free=0;

		}
		else{

			bptr = extend_heap(l,s);

			if (!bptr){

				return (NULL);
			}

		}

	}

	else{

		bptr = extend_heap(NULL,s);

		if (!bptr){

			return (NULL);
		}

		base = bptr;
	}

	return (bptr->data);

	#endif

}

void* mm_realloc(void* ptr, size_t size){

	#ifdef MM_USE_STUBS
		return realloc(ptr, size);
	#else

	size_t s;

	s_block_ptr n;
	s_block_ptr bptr;


	void *np;

	if (!ptr){

		return (mm_malloc(size));
	}

	if (valid_addr(ptr)){

		#define align4(size)
		s = align4(size) ( ( ( ( (size)-1) >> 2) << 2) + 4);

		bptr = get_block(ptr);

		if (bptr->size >= s){

			if (bptr->size - s >= (BLOCK_SIZE + 4)){

				split_block(bptr,s);
			}
		}

		else{

			if (bptr->next && bptr->next->free && (bptr->size + BLOCK_SIZE + bptr->next->size) >= s){

				fusion(bptr);

				if (bptr->size - s >= (BLOCK_SIZE + 4)){

					split_block(bptr,s);
				}

			}

			else{

				np = mm_malloc(s);

				if (!np){

					return (NULL);
				}

				n = get_block(np);

				copy_block(bptr,n);

				free(ptr);

				return (np);

			}

		}

		return (ptr);

	}

	return (NULL);
	#endif
}

void mm_free(void* ptr){

	#ifdef MM_USE_STUBS
		free(ptr);
	#else

	base = NULL;

	s_block_ptr bptr;

	if (valid_addr(ptr)){

		bptr = get_block(ptr);

		bptr->free = 1;

		if (bptr->prev && bptr->prev->free){

			bptr = fusion(bptr->prev);
		}

		if (bptr->next){

			fusion(bptr);
		}

		else{

			if (bptr->prev){

				bptr->prev->next = NULL;
			}
			else{

				base = NULL;
			}

			brk(bptr);

		}

	}

#endif

}
