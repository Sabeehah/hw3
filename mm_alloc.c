/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
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
