Husky Malloc

In this assignment you will build a free-list based memory allocator, Husky Malloc.

Your allocator must provide three functions:

void* hmalloc(size_t size); // Allocate "size" bytes of memory.
void hfree(void* item); // Free the memory pointed to by "item".
void hprintstats(); // Print allocator stats to stderr in a specific format.
Your allocator should maintain a free list of available blocks of memory. This should be a singly linked list sorted by block address.

hmalloc
To allocate memory B bytes of memory, you should first add sizeof(size_t) to B to make space to track the size of the block, and then:

For requests with (B < 1 page = 4096 bytes):

See if there's a big enough block on the free list. If so, select the first one and remove it from the list.
If you don't have a block, mmap a new block (1 page = 4096).
If the block is bigger than the request, and the leftover is big enough to store a free list cell, return the extra to the free list.
Use the start of the block to store its size.
Return a pointer to the block *after* the size field.
For requests with (B >= 1 page = 4096 bytes):

Calculate the number of pages needed for this block.
Allocate that many pages with mmap.
Fill in the size of the block as (# of pages * 4096).
Return a pointer to the block *after* the size field.
hfree
To free a block of memory, first find the beginning of the block by subtracting sizeof(size_t) from the provided pointer.

If the block is < 1 page then stick it on the free list.

If the block is >= 1 page, then munmap it.

Tracking stats
Your memory allocator should track 5 stats:

pages_mapped: How many pages total has your allocator requested with mmap?
pages_unmapped: How many pages total has your allocator given back with munmap?
chunks_allocated: How many hmalloc calls have happened?
chunks_freed: How many hfree calls have happend?
free_length: How long is your free list currently? (this can be calculated when stats are requested)
Coalescing your free list
When inserting items into your free list, you should maintain two invariants:

The free list is sorted by memory address of the blocks.
Any two adjacent blocks on the free list get coalesced (joined together) into one bigger block.
Since you insert into the free list and need to handle this in two different places, a helper function is a good idea.
