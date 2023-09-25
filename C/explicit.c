/*  File: Explicit Allocator Implementation
 *  Author: Pascual Eduardo Camacho
 *  Stanford University - CS 107 - Winter 2022
 *  Assignment 6: Heap Allocator
 */

#include <stdio.h>
#include <string.h>
#include "./allocator.h"
#include "./debug_break.h"
#define FREE 0
#define USED 1

/*  Struct definition for block headers.
 *  Includes pointers to previous free block and next free block
 */
typedef struct block {
    struct block *prev;
    struct block *next;
} block;

// First free block pointer
static block *first_free_block;

// Initialize constants
const size_t FREE_SIZE = sizeof(block);
const size_t HEADER_SIZE = ALIGNMENT;
const size_t MIN_HEADER_SIZE = 2 * HEADER_SIZE;
const size_t MIN_HEAP_SIZE = HEADER_SIZE + FREE_SIZE + HEADER_SIZE; // size = 8 + 16 + 8;
const size_t MIN_ALLOC_SIZE = HEADER_SIZE + FREE_SIZE;

// Initialize global variables (g = global)
static void *g_heap_start;
static void *g_heap_end;
static size_t g_heap_size;


/*  Function: is_used
 *  -----------------
 *  Helper function that checks if a given block is used by checking the lsb.
 *  Free block has lsb = 0, used block has lsb = 1.
 */
bool is_used(void *ptr) {
    return *((size_t *)ptr) & 1;  // false if 0, true otherwise
}

/*  Function: create_header
 *  -----------------------
 *  Helper function to create a header given a number, a pointer and a USED/FREE status.
 */
void create_header(size_t num, void *ptr, int status) {
    *((size_t *)ptr) = num + status;
}

/*  Function: header_num
 *  --------------------
 *  Helper function takes a pointer and returns the header number. 
 *  Function is_used to check if bit is on, if so, the bit is turned off to return the correct number.
 */
size_t header_num(void *ptr) {
    return is_used(ptr) ? (*(size_t *)ptr - 1) : *((size_t *)ptr);
}

/*  Function: roundup
 *  -----------------
 *  Helper function adapted from bump.c to round up numbers.
 */
size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult - 1);
}

/* Function: toggle_free
 * ---------------------
 *  Helper function to toggle bit off (off = 0 = FREE).
 */
void toggle_free(void *ptr) {
    *((size_t *)ptr) >>= 1;
    *((size_t *)ptr) <<= 1;
}

/*  Function: myinit
 *  ----------------
 *  Initialize heap and set global variables. Check for basic erroneous initializations.
 */
bool myinit(void *heap_start, size_t heap_size) {
    // Check if heap start is null
    if (heap_start == NULL) {
        return NULL;
    }
    // Check adequate heap size
    if (heap_size < MIN_HEAP_SIZE) {
        breakpoint();
        return false;
    }

    // Initialize global variables
    g_heap_start = heap_start;
    g_heap_size = heap_size;
    g_heap_end = (size_t *)((char *)heap_start + heap_size);

    // Create starting header
    void *start_header = heap_start;
    create_header(heap_size - 2 * HEADER_SIZE, start_header, FREE);

    // Create ending header
    void *end_header = (void *)((char *)g_heap_end - HEADER_SIZE);
    create_header(0, end_header, USED);
    
    // Create first free block
    first_free_block = (block *)((char *)heap_start + HEADER_SIZE);
    first_free_block->prev = NULL;
    first_free_block->next = NULL;
    
    // Successful initialization
    return true;
}
/*  Function: updatedHeader
 *  -----------------------
 *  Function called by mymalloc to update headers
 *
 *  Helper function to update the header of a given block depending on the space available
 *  and the memory to store.
 *  Returns a block struct with updated header value.
 */
block *updatedHeader(size_t min_size_reqd, size_t cur_header_size, block *cur_block) {
    size_t size_new = min_size_reqd + HEADER_SIZE + FREE_SIZE;
    // Check min_size_reqd and update if needed
    size_t validate_size = (FREE_SIZE > min_size_reqd) ? FREE_SIZE : min_size_reqd;
    // Compute space to store in header
    size_t space = (cur_header_size >= size_new) ? validate_size : cur_header_size;
    void *cur_header = (void *)((char *)cur_block - HEADER_SIZE);
    create_header(space, cur_header , USED);
    block *new_block = NULL;

    // If enough space for a new header after allocated space
    if (cur_header_size >= size_new) {
        void *next_header = (void *)((char *)cur_block + space);
        size_t next_size = cur_header_size - space - HEADER_SIZE;
        create_header(next_size, next_header, FREE);
        new_block = (block *)((char *)next_header + HEADER_SIZE);
    }
    return new_block;
}

/*  Funtion: rewireAdd
 *  -------------------
 *  Helper function that inserts new block in the free block list.
 *  New block is inserted after current block.
 *  It handles edge cases based on where the current block is on the list.
 */
void rewireAdd(block *cur_block, block *new_block) {
    // CASE 1 - Block at beginning of list
    if (cur_block->prev == NULL) {
        first_free_block = new_block;  // move start to the new block
                    
        // First check if not an empty list
        if (first_free_block != NULL) {
            // Rewire first free
            first_free_block->prev = NULL;
            first_free_block->next = cur_block->next;
            
            // If we are not in the end
            if (first_free_block->next != NULL) {
                first_free_block->next->prev = first_free_block;
            }
        }
    // CASE 2 - Header at middle of list
    } else {
        cur_block->prev->next = new_block;
        
        // Check if end of list
        if (cur_block->next != NULL) {
            cur_block->next->prev = new_block;
        }
        new_block->prev = cur_block->prev;
        new_block->next = cur_block->next;
    }
}


/*  Function: rewireNoAdd
 *  ----------------------
 *  Helper function that essentially cuts off the current block from the free block list.
 *  To do so, it rewires previous and next pointers, connecting the previous to the next block.
 *  It handles edge cases based on where the current block is on the list.
 */
void rewireNoAdd(block *cur_block) {
     // CASE 1 - Header at beginning of list
     if (cur_block->prev == NULL) {
         first_free_block = cur_block->next;
         // If list is not empty
         if (first_free_block != NULL) {
             first_free_block->prev = NULL;
             // If not at the end
             if (first_free_block->next != NULL) {
                 first_free_block->next->prev = first_free_block;
             }
         }  
     // CASE 2 - Header at middle of list
     } else {
         cur_block->prev->next = cur_block->next;
         if (cur_block->next != NULL) {
             cur_block->next->prev = cur_block->prev;
         }
     }
}


/*  Function: mymalloc
 *  ------------------
 *  Calls helper functions to rewire pointers depending on state of heap.
 *  rewireAdd inserts a block into the linked-list while rewireNoAdd only rewires.
 *  Function returns a heap-allocated pointer to memory. If no adequate size to conform
 *  to user's request, function returns NULL.
 */
void *mymalloc(size_t requested_size) {
    if (((requested_size > (g_heap_size - HEADER_SIZE))) | (requested_size == 0)) {
        return NULL;
    }
    // Compute minimum required size, required size for payload, initialize block struct 
    size_t min_size_reqd = roundup(requested_size, ALIGNMENT); 
    size_t payload_size_reqd = roundup(requested_size, ALIGNMENT) + 3 * HEADER_SIZE;
    block *cur_block = (block *)first_free_block;

    // Traverse list
    while (cur_block != NULL) {
        void *cur_header = (void *)((char *)cur_block - HEADER_SIZE);
        size_t cur_header_size = header_num(cur_header); 
    
        // Enough space found
        if (cur_header_size >= min_size_reqd) {
            block *new_block = updatedHeader(min_size_reqd, cur_header_size, cur_block);
                
            // CASE 1 - Enough space for request and a new header
            if (cur_header_size >= payload_size_reqd) {
                rewireAdd(cur_block, new_block);
            // CASE 2 - Enough space for request, not for header
            } else {
                rewireNoAdd(cur_block);
            }
            // Return pointer to allocated memory
            void *mem_ptr = (void *)cur_block;
            return mem_ptr;
        }
        // Not enough space in current block
        cur_block = cur_block->next;
    }
    // Adequate block not found
    return NULL;
}

/*  Function: coalesce
 *  ------------------
 *  Helper function that performs coalescing. 
 *  b1 - current block
 *  b2 - next block
 *
 */
void coalesce(block *b1, block *b2) {
    // If b1 is before first_free_block, b2=first_free_block
    b1->prev = b2->prev;
    b1->next = b2->next;
    
    if (b1->prev == NULL) {
        first_free_block = b1;
    } else {
        b1->prev->next = b1;
    }
    if (b1->next != NULL) {
        b1->next->prev = b1;
    }
}

/*  Function: coalesceFree
 *  ----------------------
 *  Helper function that performs actual freeing of block. 
 *  Called by myfree, this function is responsible of coalescing the next free block with
 *  the current block. Parameters are the block to free, the header to free, the current size
 *  that will be freed, the next header, and the next header free size.
 *
 */
void coalesceFree(block *tofree_block, void *tofree_header, size_t tofree_size, void *next_header, size_t next_size) {
    // Create next block
    block *next_block = (block *)((char *)next_header + HEADER_SIZE);   
    
    // Update header to include next free block then coalesce
    create_header(HEADER_SIZE + tofree_size + next_size, tofree_header, FREE);
    coalesce(tofree_block, next_block); 
}

/*  Function: regularFree
 *  ---------------------
 *  Helper function that performs regular freeing of a block -- no coalescing.
 *  Called by myfree, this function inserts a free block at the pointer provided
 *  by the user and rewires the pointers.
 *
 */
void regularFree(block *tofree_block, void *tofree_header) {
    block *cur_ptr = first_free_block;
    toggle_free(tofree_header);
    
    // Check
    if (cur_ptr == NULL) {
        breakpoint();
        return;
    }
    
    while (cur_ptr != NULL) {
        if (cur_ptr > tofree_block) {
            break;
        }
        cur_ptr = cur_ptr->next;
    }
    
    if (cur_ptr->prev == NULL) {
        first_free_block = tofree_block;
    } else {
        cur_ptr->prev->next = tofree_block;
    }

    tofree_block->prev = cur_ptr->prev;
    tofree_block->next = cur_ptr;
    cur_ptr->prev = tofree_block;
}

/*  Function: myfree
 *  ----------------
 *  Function calls helper functions regularFree and coalesceFree to free the provided block pointer.
 *
 */
void myfree(void *ptr) {
    // Check pointer
    if (ptr == NULL) {
        return;
    }
    
    // Get header, header size, and block of memory location to free
    void *tofree_header = (void *)((char *)ptr - HEADER_SIZE);
    if (!is_used(tofree_header)) {
        return;
    }

    // Get header and size to be freed
    size_t tofree_size = header_num(tofree_header);
    block *tofree_block = (block *)ptr; 
    // Get next header information and size
    void *next_header = (void *)((char *)tofree_block + tofree_size);
    size_t next_size = header_num(next_header);
    
    // Check if coalescing is needed, then free
    if (!is_used(next_header)) {
        coalesceFree(tofree_block, tofree_header, tofree_size, next_header, next_size);
    } else {
        regularFree(tofree_block, tofree_header);
    }
}

/*  Function: coalescePossible
 *  --------------------------
 *  Helper function that checks if it is possible to coalesce to the right of a given pointer.
 *  Called by myrealloc.
 *  Returns true if coalesce to the right is possible.
 */
bool coalescePossible(void *ptr) {
    void *cur_header = (void *)((char *)ptr - HEADER_SIZE);
    size_t cur_size = header_num(cur_header);
    void *next_header = (void *)((char *)ptr + cur_size);
    bool next_free = !is_used(next_header);
    return (next_free) ? 1 : 0;
}

/*  Function: coalesceReal
 *  ----------------------
 *  Helper function to coalesce to the right when reallocating memory.
 *  Called by myrealloc.
 */
void coalesceReal(void *ptr) {
    while (coalescePossible(ptr)) { 
        // Current header information
        void *cur_header = (void *)((char *)ptr - HEADER_SIZE);
        size_t cur_size = header_num(cur_header);
       
        // Next header information
        void *next_header = (void *)((char *)ptr + cur_size);
        size_t next_size = header_num(next_header);
        block *next_block = (block *)((char *)next_header + HEADER_SIZE);

        rewireNoAdd(next_block);  // rewire
        
        size_t updated_size = cur_size + next_size + HEADER_SIZE;
        create_header(updated_size, cur_header, FREE);  // update header
        ptr = (void *)((char *)ptr);
    }
}

/*  Function: myrealloc
 *  -------------------
 *  Function that dynamically reallocates memory.
 */
void *myrealloc(void *old_ptr, size_t new_size) {
    if (old_ptr == NULL) {
        void *ptr = mymalloc(new_size);
        return (ptr) ? ptr : NULL;
    }
    if (new_size == 0) {
        return NULL;
    }

    // Get current header information
    void *cur_header = (void *)((char *)old_ptr - HEADER_SIZE);
    size_t cur_size = header_num(cur_header);
    block *cur_block = (void *)((char *)cur_header + HEADER_SIZE);
    
    size_t old_size = header_num(cur_header);
    new_size = roundup(new_size, ALIGNMENT);
    
    void *next_header = (void *)((char *)old_ptr + cur_size);
    block *next_block = (void *)((char *)next_header + HEADER_SIZE);
    void *mem_ptr = cur_block;

    // Realloc is NOT possible at current location
    if (new_size > cur_size) {
        // If coalescing is possible, coalesce all possible blocks
        if (coalescePossible(next_block)) {
            coalesceReal(next_block);
        }
        void *temp_header = (void *)((char *)next_block - HEADER_SIZE);
        size_t temp_size = header_num(temp_header);
        
        // Copy memory
        if (temp_size > new_size) {
            mem_ptr = mymalloc(new_size);
            memmove(mem_ptr, old_ptr, new_size);
            myfree(old_ptr);
        }

        // If after coalescing it fits
        if (new_size < temp_size) {
            // Free current
            regularFree(cur_block, cur_header);
            //rewireNoAdd(cur_block);
            block *new_next_block = (block *)((char *)cur_block + new_size + HEADER_SIZE);
            if (cur_block->prev == NULL) {
                first_free_block = new_next_block;
            }
            new_next_block->prev = cur_block->prev;
            if (cur_block->next != NULL) {
                new_next_block->next = cur_block->next->next;
                cur_block->next->prev = new_next_block;
            }

            // Update headers
            void *new_next_header = (void *)((char *)cur_header + new_size + HEADER_SIZE);
            create_header(new_size, cur_header, USED);
            create_header(temp_size + cur_size - new_size, new_next_header, FREE);
            return mem_ptr;
        // If it doesn't fit, call mymalloc
        } else {
            mem_ptr = mymalloc(new_size);
            if (mem_ptr) {
                memmove(mem_ptr, old_ptr, new_size);
                myfree(old_ptr);
                return mem_ptr;
            }
        }
    } else {  // Realloc IS possible at current location
        // Check if there's enough space to create a header
        if ((old_size - new_size) > MIN_ALLOC_SIZE) {
            // Update header
            create_header(new_size, cur_header, USED);
            // Add new header
            void *for_header = (void *)((char *)cur_header + HEADER_SIZE + new_size);
            create_header(cur_size - new_size - HEADER_SIZE, for_header, FREE);
            return cur_block;
        // Do not create a header
        } else {
            return cur_block;
        }
    }
    return NULL;
}

/*  Function: validate_heap
 *  -----------------------
 */
bool validate_heap() {
    void *current = g_heap_start;
    bool cur_used = is_used(current);
    size_t cur_size = header_num(current);
    
    // Header byte counter
    size_t byte_count = ALIGNMENT;

    // Check  pointer
    if (current == NULL) {
        return false;
    }

    // Traverse heap while size!=0 and not free
    while (!(cur_size == 0 && cur_used)) {
        byte_count += cur_size + ALIGNMENT;
        if (cur_used) {
            if (*(size_t *)current % ALIGNMENT != 1) {
                breakpoint();
                return false;
            } else {
                if (*(size_t *)current % ALIGNMENT == 0) {
                    breakpoint();
                    return false;
                }
            }
        }
        // Move to next block
        current = (void *)((char *)current + cur_size + HEADER_SIZE);
        cur_used = is_used(current);
        cur_size = header_num(current);
    }

    // Check linked list
    block *cur_block = first_free_block;
    unsigned int block_count = 0;
    //block *visited = NULL;

    // Traverse linked list
    while (cur_block != NULL) {
        block_count++;
        cur_block = cur_block->next;
    }
    return true;
}

/* Function: dump_heap
 * -------------------
 * This function prints out the the block contents of the heap.  It is not
 * called anywhere, but is a useful helper function to call from gdb when
 * tracing through programs.  It prints out the total range of the heap, and
 * information about each block within it.
 */
void dump_heap() {
    void *current = g_heap_start;
    size_t cur_size = header_num(current);
    bool cur_used = is_used(current);
    char *status = "FREE";
    unsigned int count = 0;
    
    printf("\nSTART HEAP\n");
    printf("------------------------------------------------------------\n");
    printf("Heap starts at address: %p\nHeap ends at address: %p\nHeap size is: %lu\n", g_heap_start, g_heap_end, g_heap_size);
    printf("------------------------------------------------------------\n");
    // Traverse and print each block
    while (!(cur_size == 0 && cur_used)) {
        //(cur_used) ? status == "USED" : status == "FREE";
        if (cur_used) {
            status = "u";
        }
        
        printf("Block header #%d at %p of size %lu is %s\n", count, current, cur_size, status);
        //breakpoint();
        // Move to next block
        current = (void *)((char *)current + cur_size + HEADER_SIZE);
        cur_used = is_used(current);
        cur_size = header_num(current);
        status = "FREE";
        count++;
    }
    // Print last block (heap end always "USED")
    status = "USED";
    printf("Block header #%d at %p of size %lu is %s\n", count, current, cur_size, status);
    printf("------------------------------------------------------------\n");
    printf("END HEAP\n\n");

    // Linked list
    block *cur_block = first_free_block;
    block *prev_block = cur_block->prev;
    block *next_block = cur_block->next;
    unsigned int block_count = 0;

    printf("\nSTART LIST OF FREE BLOCKS\n");
    printf("------------------------------------------------------------\n");
   // Traverse linked list
    while (cur_block != NULL) {
        //breakpoint();
        printf("Free block #%d at %p with previous block at %p and next block at %p\n", block_count, cur_block, prev_block, next_block);
        block_count++;
        
        prev_block = cur_block;
        cur_block = cur_block->next;
        if (cur_block != NULL){
            next_block = cur_block->next;
        }
    }
    printf("------------------------------------------------------------\n");
    printf("Total num free blocks: %d\n", block_count);
    printf("------------------------------------------------------------\n");
    printf("END LIST OF FREE BLOCKS\n");
}
