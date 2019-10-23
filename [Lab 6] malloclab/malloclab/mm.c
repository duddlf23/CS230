/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


// macro
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y)? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *) (bp) - WSIZE)
#define FTRP(bp) ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) -WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) -DSIZE))

#define prev(bp) ((char *)(bp))
#define next(bp) ((char *)(bp) + WSIZE)

#define get_prev(bp) (*(void **)(bp))
#define get_next(bp) (*(void **)(next(bp)))

#define set(x, bp) (*(unsigned int *)(x) = (unsigned int)(bp))

static void *heap_listp;
static void *head;

static void delete(void *bp);
static void *extend_heap(size_t x);
static void *coalesce(void *bp);
static void insert(void *bp);
static void place(void *bp, size_t asize);

static void insert(void *bp){
    
    
    if (head == NULL){
	head = bp;
    	set(next(bp), NULL);
	set(prev(bp), NULL);
	return;
    }
    void *imsi = head;
    set(next(bp), imsi);
    set(prev(imsi), bp);
    set(prev(bp), NULL);
    head = bp;
    
}
static void delete(void *bp){
    
    if(get_prev(bp) != NULL){
	set(next(get_prev(bp)), get_next(bp));
    }else{
	head = get_next(bp);
    }
    if(get_next(bp) !=NULL)
	set(prev(get_next(bp)), get_prev(bp));
    
}
static void *extend_heap(size_t x){
    void *bp;
    size_t size;

    size = ALIGN(x);

    if ((bp = mem_sbrk(size)) == (void *)-1)
	return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *coalesce(void *bp){
    size_t prev_a = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_a = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_a && next_a){
	insert(bp);
	return bp;
    }

    if (prev_a && !next_a){
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	delete(NEXT_BLKP(bp));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    }else if (!prev_a && next_a){
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	delete(PREV_BLKP(bp));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }else{
	size += GET_SIZE(HDRP(NEXT_BLKP(bp))) +  GET_SIZE(HDRP(PREV_BLKP(bp)));
	delete(PREV_BLKP(bp));
	delete(NEXT_BLKP(bp));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }

    insert(bp);

    return bp;
}

static void place(void *bp, size_t asize){

    size_t csize = GET_SIZE(HDRP(bp));
    
    delete(bp);

    if ((csize - asize) >= 2*DSIZE){
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
	coalesce(NEXT_BLKP(bp));
    }else{
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
   
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    if ( (heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	return -1;

    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (2* WSIZE) , PACK(DSIZE, 1));
    PUT(heap_listp + (3* WSIZE) , PACK(0, 1));
    head = NULL;
    
    if (extend_heap(CHUNKSIZE) == NULL)
	return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    void *imsi = NULL;
    int cnt = 0;

    if(size==0) return NULL;

    if(size <= DSIZE) asize = 2*DSIZE;
    else asize = ALIGN(size + DSIZE);

    
    imsi = head;

    while(1){
	if (imsi == NULL) break;
	if ( GET_SIZE(HDRP(imsi)) >= asize && !GET_ALLOC(HDRP(imsi))  ){
	    cnt =1;
	    break;
	}

	imsi = get_next(imsi);
    }

    if (cnt==1){
	place(imsi, asize);
	return imsi;
    }

    extendsize = MAX(asize, CHUNKSIZE);

    if ((imsi = extend_heap(extendsize)) == NULL)
	return NULL;

    place(imsi, asize);
    return imsi;

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));


    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t csize = GET_SIZE(HDRP(ptr));
    size_t nsize ;
    if(size <= DSIZE) nsize = 2*DSIZE;
    else nsize = ALIGN(size + DSIZE);

    if (ptr==NULL) return mm_malloc(size);
    if (size==0){
	mm_free(ptr);
	return NULL;
    }
    
    if (nsize <= csize){
	return ptr;
    }else{
	void *imsi = NEXT_BLKP(ptr);

	if(!GET_ALLOC(HDRP(imsi))){
	    csize += GET_SIZE(HDRP(imsi));
	    if (nsize <= csize){
		delete(imsi);
		PUT(HDRP(ptr), PACK(csize, 1));
		PUT(FTRP(ptr), PACK(csize, 1));
		return ptr;
	    }else if( GET_SIZE(HDRP(NEXT_BLKP(imsi))) == 0){
		if(extend_heap(nsize - csize + DSIZE) == NULL)
		    return NULL;
		delete(imsi);
		PUT(HDRP(ptr), PACK(nsize + DSIZE, 1));
		PUT(FTRP(ptr), PACK(nsize + DSIZE, 1));
		return ptr;
	    }
	}else if(GET_SIZE(HDRP(imsi)) == 0){
	    if(extend_heap(nsize - csize + DSIZE) == NULL)
		return NULL;
	    delete(imsi);
	    PUT(HDRP(ptr), PACK(nsize + DSIZE, 1));
	    PUT(FTRP(ptr), PACK(nsize + DSIZE, 1));
	    return ptr;
	}
	
    }

    void *nptr = mm_malloc(nsize);
    memcpy(nptr, ptr, csize);
    mm_free(ptr);
    return nptr;

}














