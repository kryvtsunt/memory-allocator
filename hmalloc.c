// Starter code is provided by Prof. Tuck 
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include "hmalloc.h"

//size of one page of memory
const size_t PAGE_SIZE = 4096;
static hm_stats stats;
// first node of the freeList
node* listHead = NULL;

// function to calculate the length of the freeList
	long
free_list_length(node* n)
{
	if (n->next == NULL) return 0;
	else return 1 + free_list_length(n->next);
}

// function to get stats
	hm_stats*
hgetstats()
{
	stats.free_length = free_list_length(listHead);
	return &stats;
}

// function to print stats
	void
hprintstats()
{
	stats.free_length = free_list_length(listHead);
	fprintf(stderr, "\n== husky malloc stats ==\n");
	fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
	fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
	fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
	fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
	fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}

// function to find number of pages to be allocated 
static
	size_t
div_up(size_t xx, size_t yy)
{
	size_t zz = xx / yy;

	if (zz * yy == xx) {
		return zz;
	}
	else {
		return zz + 1;
	}
}

// function to find node of appropriate size in the freeList
node*
findMemory(node* n, size_t size){
	node* nextNode = n->next;
	if (n->next== NULL) return NULL; 
	else if (nextNode->size >= size) return n;
	else return findMemory(n->next,size);
}

// function to insert node into the sorted freeList by adress
void
insertNode(node* n, node* n2){
	if (n->next == NULL) {  n->next = n2; n2->next = NULL;}
	else if (n->next > n2){  n2->next = n->next; n->next = n2; }
	else insertNode(n->next, n2); 
}

// function to merge coalesced nodes together
void 
merge(node* n){
	node* nextNode = n->next;
	if (nextNode == NULL) return;
	else if (n->size == (n->next - n)*sizeof(node)) {
		n->size = n->size + nextNode->size;
		n->next = nextNode->next;
		merge(n);
	}
	else merge(n->next);
}

// function to itterate through the list and unmap large nodes
void 
unmapNode(node* n){
	node* nextNode = n->next;
	if (nextNode == NULL) return ;
	if (nextNode->size > PAGE_SIZE ) {
		n->next = nextNode->next;
		int nn = div_up(nextNode->size, PAGE_SIZE);
		munmap(nextNode, nextNode->size); 
		stats.pages_unmapped += nn; 
		unmapNode(n);
	}
	else unmapNode(n->next);
}

// function to delete Node from the freeList
void 
deleteNode(node* n, node* n2) {
	if (n->next == n2) n->next = n2->next;
	else deleteNode(n->next, n2);
}

// function to initialize the freeList
void 
init() {
	listHead = mmap(NULL, sizeof(node), 
			PROT_READ|PROT_WRITE,MAP_ANON| MAP_PRIVATE, -1,0);
	listHead->size = 0; 
	listHead->next = NULL;
}

// function that allocates chunk of memory of given size and returns a pointer
	void*
hmalloc(size_t size)
{  
	stats.chunks_allocated += 1;
	size += sizeof(size_t);
	// never allocate less than 16 bytes
	if (size < sizeof(node)) size = sizeof(node);
	// inititalize freeList
	if (listHead == NULL) init();
	// allocate new memory if no suitable node in the freeList
	if (findMemory(listHead, size) == NULL){
		int n = div_up(size, PAGE_SIZE);
		node* newNode2 = mmap(NULL, n*PAGE_SIZE, 
				PROT_READ|PROT_WRITE,MAP_ANON| MAP_PRIVATE, -1,0);
		newNode2->size = n*4096;
		newNode2->next = NULL;
		insertNode(listHead, newNode2);
		stats.pages_mapped += n*1;
	}
	// find the node of appropriate size
	node* firstNode = findMemory(listHead, size);
	node* secondNode = firstNode->next;
	// delete the node from the freeList
	deleteNode(listHead, secondNode);
	header* newHeader = (header *) secondNode;
	// create new node with the size offset 
	node* newFreeNode = (node *)((size_t) secondNode + size);
	newFreeNode->size = secondNode->size - size; 
	// insert a new node to the list 
	if (secondNode->size - size >= sizeof(node))insertNode(listHead, newFreeNode);
	else size += secondNode->size - size;
	newHeader->size = size;
	// return a pointer
	return (void*) newHeader + sizeof(size_t);
}

// function that frees chunk of memory described by the given pointer
	void
hfree(void* item)
{
	stats.chunks_freed += 1;
	// find the real start of the item
	header* h = (void*) item - sizeof(size_t);
	// create a new node
	node* newNode = (node*) h;
	newNode->size = h->size;
	newNode->next = NULL;
	// insert node into the freeList
	insertNode(listHead,newNode);
	// merge nodes in the list if possible
	merge(listHead->next);
	// unmap nodes in the list if possible
	unmapNode(listHead);
}
