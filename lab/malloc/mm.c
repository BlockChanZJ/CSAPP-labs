/*
 * mm1.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * basic version
 * use LINKLIST to implement it
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
        /* Team name */
        "QWQ",
        /* First member's full name */
        "BlockChanZJ",
        /* First member's email address */
        "blockchanzj@163.com",
        /* Second member's full name (leave blank if none) */
        "",
        /* Second member's email address (leave blank if none) */
        ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write and allocated fields from address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))
#define IS_LAST_BLK(bp) (GET_SIZE(HDRP(((char*)bp))) == 1 && GET_ALLOC(HDRP(((char*)bp))))


/* global variables */
void *heap_listp = NULL;


/* debug functions */
//#define DEBUG
#ifdef DEBUG
#define dbg(...) fprintf(stderr, __VA_ARGS__)
#else
#define dbg(...) 42
#endif


/* functions */


static void list() {
    void *bp = heap_listp;
    int block = 0;
    while (!(GET_SIZE(HDRP(bp)) == 0 && GET_ALLOC(HDRP(bp)) == 1)) {
        dbg("block [%d], size: %d, alloc: %d\n", block++, (int) (GET_SIZE(HDRP(bp))), (int) (GET_ALLOC(HDRP(bp))));
        bp = NEXT_BLKP(bp);
        if (block > 10) exit(0);
    }
    dbg("block [%d], size: %d, alloc: %d\n", block++, (int) (GET_SIZE(HDRP(bp))), (int) (GET_ALLOC(HDRP(bp))));
}

static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    } else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT((FTRP(bp)), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        size += GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* alignment for 2 words*/
    size = (words % 2) ? ((words + 1) * WSIZE) : (words * WSIZE);
    if ((long) (bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0)); /* free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

//    dbg("words: %d, size: %d\n", (int) (words), (int) (size));
//    dbg("bp: %p, size: %d, alloc: %d\n", bp, (int) GET_SIZE(HDRP(bp)), (int) GET_ALLOC(HDRP(bp)));

    return coalesce(bp);
}


static void *find_fit(size_t size) {
    void *bp = heap_listp;
    while (!(GET_SIZE(HDRP(bp)) == 0 && GET_ALLOC(HDRP(bp)) == 1)) {
        if (GET_ALLOC(HDRP(bp)) == 1) {
            bp = NEXT_BLKP(bp);
        } else {
            if (GET_SIZE(HDRP(bp)) >= size) {
                return bp;
            } else {
                bp = NEXT_BLKP(bp);
            }
        }
    }
    return NULL;
}

static void place(void *bp, size_t size) {
    size_t ori_size = GET_SIZE(HDRP(bp));
    if (size < ori_size) {
        int is_last_blk = IS_LAST_BLK(NEXT_BLKP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        if (is_last_blk) {
            PUT(HDRP(bp), PACK(ori_size - size, 0));
            PUT(FTRP(bp), PACK(ori_size - size, 0));
            PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
        } else {
            PUT(HDRP(bp), PACK(ori_size - size, 0));
            PUT(FTRP(bp), PACK(ori_size - size, 0));
        }
    } else {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}


/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    /* initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1)
        return -1;
    PUT(heap_listp, 0); /* alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); /* Epilogue footer */
    heap_listp += (2 * WSIZE);
    dbg("heap_listp: %p\n", heap_listp);

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    dbg("extend heap success!\n");
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    size_t adjsize; /* adjusted block size */
    size_t extsize; /* extend heap size if no fit */
    char *bp;

    dbg("malloc: %d\n", (int) (size));

    if (size == 0) return NULL;

    if (size <= DSIZE) /* alignment */
        adjsize = 2 * DSIZE;
    else /* alignment + header + footer */
        adjsize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    if ((bp = find_fit(adjsize)) != NULL) {
        place(bp, adjsize);
        return bp;
    }
    extsize = MAX(adjsize, CHUNKSIZE);
    if ((bp = extend_heap(extsize / WSIZE)) == NULL)
        return NULL;
    place(bp, adjsize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
    dbg("mm_free %zu success!\n", size);
}


void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}















