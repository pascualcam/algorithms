/*  File: Implicit Allocator Implementation
 *  Author: Pascual Eduardo Camacho
 *  Stanford University - CS 107 - Winter 2022
 *  Assignment 6: Heap Allocator
 */

/* IMPLICIT LIST ALLOCATOR
 * -----------------------
 *
 */

#include <stdio.h>
#include <string.h>
#include "./allocator.h"
#include "./debug_break.h"


// Define constants
#define MIN_HEAP_SIZE 24
#define BYTES_PER_LINE 32
#define HEADER_SIZE 8
#define FREE 0
#define USED 1

// Initialize global variables
static void *g_heap_start;
static void *g_heap_end;
static size_t g_heap_size;
size_t bytes_used;


/*  Function: is_used
 *  -----------------
 *  Helper function that checks if a given block is used by checking the lsb.
 *  From class, a free block will have its lsb set to 0, while a used block will be 1.
 */
bool is_used(void *block) {
    return *((size_t *)block) & 1;  // false if 0, true otherwise
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
 *  Helper function takes a pointer and gets the header number. Function uses is_used
 *  helper to check if the header has the bit on or off. If on, the bit
 *  is turned off for the return function.
 */
size_t header_num(void *ptr) {
    return is_used(ptr) ? (*(size_t *)ptr - 1) : *((size_t *)ptr);
}

/*  Function: roundup
 *  -----------------
 *  Helper function adapted from bump.c to roundup numbers.
 */
size_t roundup(size_t sz, size_t mult) {
    return (sz + mult - 1) & ~(mult -1);
}

/*  Function: toggle_free
 *  ---------------------
 *  Helper function to toggle bit off (off = 0 = FREE)
 */
void toggle_free(void *block) {
    *((size_t *)block) >>= 1; 
    *((size_t *)block) <<= 1;
}

/*  Function: myinit
 *  ----------------
 *  Function initializes the heap and returns true if this initialization was
 *  successful, or false otherwise. Function sets global variables that keep 
 *  the start, end and size of the heap and bytes_used available to other functions.
 */
bool myinit(void *heap_start, size_t heap_size) {
    // Cheack size of heap
    if (heap_size <= MIN_HEAP_SIZE) {
        breakpoint();
        return false;
    }
    // Set up heap
    g_heap_start = heap_start;
    g_heap_size = heap_size;
    g_heap_end = (size_t *)((char *)g_heap_start + g_heap_size - ALIGNMENT);
    bytes_used = 0;
    
    // Create start header pointer and initialize value
    void *start_header = heap_start;
    create_header(heap_size - 2 * HEADER_SIZE, start_header, FREE);

    // Create end header pointer
    void *end_header = (void *)((char *)heap_start + heap_size - HEADER_SIZE);
    create_header(0, end_header, USED);

    return true;
}

/*  Function: mymalloc
 *  ------------------
 *  Calls helper function roundup to ensure alignment is respected. Function 
 *  traverses heap until an adequate block (right size and free) is found and is returned.
 *  If no such block is found, function returns NULL.
 */
void *mymalloc(size_t requested_size) {
    if (((requested_size > (g_heap_size - ALIGNMENT)) | (requested_size == 0))) {
        return NULL;
    }
    // Compute total required size and round up
   size_t size_reqd = roundup(requested_size, ALIGNMENT);
    
    // Get first block's header, size, and status
    void *current = g_heap_start;
    size_t cur_size = header_num(current);
    bool cur_used = is_used(current);

    // Traverse heap block by block until required size is found
    // Loop stops when block size is 0 and block is used (ie end of heap)
    while (!(cur_size == 0 && cur_used)) {
        // If block is adequate (enough space and not used)
        if (!cur_used && (cur_size >= size_reqd)) {
            // Update current block header
            create_header(size_reqd, current, USED);
            
            // Create block header if more space available than requested
            if (cur_size > size_reqd) {
                void *next_header = (void *)((char *)current + size_reqd + ALIGNMENT);
                create_header(cur_size - size_reqd - ALIGNMENT, next_header, FREE);
            }
            // Return pointer to allocated memory
            void *mem_ptr = (void *)((char *)current + ALIGNMENT);
            return mem_ptr;
        }
        // Move to next block
        current = (void *)(((char *)current) + cur_size + ALIGNMENT);
        cur_used = is_used(current);
        cur_size = header_num(current);
    }
    // Adequate block not found
    return NULL;
}

/*  Function: myfree 
 *  ----------------
 *  Calls the helper function toggle_free which toggles the lsb from (1=USED) to (0=FREE).
 *  Checks if the passed pointer is NULL.
 */
void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    size_t *new_ptr = (size_t *)((char *)ptr - ALIGNMENT);
    toggle_free(new_ptr);
}

/*  Function: myrealloc
 *  -------------------
 *  Arguments are a pointer to the memory to move and the size to reallocate.
 *  Function handles two edge cases, first, if the passed pointer is NULL,
 *  mymalloc(new_size) is called and if the allocation is successful, the pointer 
 *  returned, otherwise NULL. Null is also returned for a new_size of 0.
 *  Function uses memmove to move the contents at the previous pointer to the
 *  newly allocated pointer.
 *  Finally, the old pointer is freed.
 */
void *myrealloc(void *old_ptr, size_t new_size) {
    if (old_ptr == NULL) {
        void *ptr = mymalloc(new_size);
        return (ptr) ? ptr : NULL;
    } else if (new_size == 0) {
        return NULL;
    }

    void *new_ptr = mymalloc(new_size);
    memmove(new_ptr, old_ptr, new_size);
    myfree(old_ptr);
    return new_ptr;
}

/*  Function: validate_heap
 *  -----------------------
 *  This function is called periodically by test harness to check the
 *  state of the heap. 
 */
bool validate_heap() {
    void *current = g_heap_start;
    bool cur_used = is_used(current);
    size_t cur_size = header_num(current);

    // Header byte counter
    size_t byte_count = ALIGNMENT;

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
        current = (void *)((char *)current + cur_size + ALIGNMENT);
        cur_used = is_used(current);
        cur_size = header_num(current);
    }
    // Validate size
    if (byte_count != g_heap_size) {
        breakpoint();
        return false;
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
    
    printf("\nHeap segment starts at address %p and ends at %p. Size of heap is %lu.\n", g_heap_start, g_heap_end, g_heap_size);
    printf("------------------------------------------------------------\n");
    
    // Traverse block and print each
    while (!(cur_size == 0 && cur_used)) {
        if (cur_used) {
            status = "USED";
        } 
        printf("Block #%d at %p of size %lu with status %s\n", count, current, cur_size, status);
        
        // Move to next block
        current = (void *)((char *)current + cur_size + ALIGNMENT);
        cur_used = is_used(current);
        cur_size = header_num(current);
        status = "FREE";
        count++;
    }
    // Print last block (heap end - always "USED")
    status = "USED";
    printf("Block #%d at %p of size %lu with status %s\n\n", count, current, cur_size, status);
    printf("Heap end.\n\n");
}
