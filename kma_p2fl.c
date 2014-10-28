/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the power-of-two free list
 *             algorithm
 *    Author: Stefan Birrer
 *    Copyright: 2004 Northwestern University
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    Revision 1.2  2009/10/31 21:28:52  jot836
 *    This is the current version of KMA project 3.
 *    It includes:
 *    - the most up-to-date handout (F'09)
 *    - updated skeleton including
 *        file-driven test harness,
 *        trace generator script,
 *        support for evaluating efficiency of algorithm (wasted memory),
 *        gnuplot support for plotting allocation and waste,
 *        set of traces for all students to use (including a makefile and README of the settings),
 *    - different version of the testsuite for use on the submission site, including:
 *        scoreboard Python scripts, which posts the top 5 scores on the course webpage
 *
 *    Revision 1.1  2005/10/24 16:07:09  sbirrer
 *    - skeleton
 *
 *    Revision 1.2  2004/11/05 15:45:56  sbirrer
 *    - added size as a parameter to kma_free
 *
 *    Revision 1.1  2004/11/03 23:04:03  sbirrer
 *    - initial version for the kernel memory allocator project
 *
 ***************************************************************************/
#ifdef KMA_P2FL
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

// struct used as the header of each buffer
typedef struct
{
   void* nextblock;
} buffer_header;

// struct to keep track of pages allocated to a certain free list
typedef struct page_node_struct
{
   kma_page_t* page;
   struct page_node_struct* next;
} page_node;

// struct used as the header to a free list
typedef struct
{
  int size; // size of buffers in this free list
  int used; // number of blocks used
  buffer_header* start; // pointer to the first buffer in the linked list
  page_node* page_list;
} free_list;

typedef struct
{
  free_list buffer_32;
  free_list buffer_64;
  free_list buffer_128;
  free_list buffer_256;
  free_list buffer_512;
  free_list buffer_1024;
  free_list buffer_2048;
  free_list buffer_4096;
  free_list buffer_8192;
  int used;
  free_list kpages;
} main_list;
/************Global Variables*********************************************/
kma_page_t* entry_point;
/************Function Prototypes******************************************/

void* kma_malloc(kma_size_t size);
void kma_free(void* ptr, kma_size_t size);
void initialize_page(kma_page_t* page_ptr);
void* find_buffer_from_free_list(free_list* list);
void allocate_buffers_to_list(free_list* list);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  // cannot allocate a space larger than a page
 // if (size > PAGESIZE) {
//	return NULL;
  //}

  if (entry_point == NULL) {
	// no page allocated yet, need to allocate first page
	entry_point = get_page();
	initialize_page(entry_point);
  }

  main_list* mainlist = (main_list*)entry_point->ptr;
  int total_space = size + sizeof(buffer_header);
  free_list* target = NULL;

  // find the right free list
  if (total_space <= 32) {
	target = &mainlist->buffer_32;
  } else if (total_space <= 64) {
	target = &mainlist->buffer_64;
  } else if (total_space <= 128) {
	target = &mainlist->buffer_128;
  } else if (total_space <= 256) {
	target = &mainlist->buffer_256;
  } else if (total_space <= 512) {
	target = &mainlist->buffer_512;
  } else if (total_space <= 1024) {
	target = &mainlist->buffer_1024;
  } else if (total_space <= 2048) {
	target = &mainlist->buffer_2048;
  } else if (total_space <= 4096) {
	target = &mainlist->buffer_4096;
  } else if (total_space <= 8192) {
	target = &mainlist->buffer_8192;
  }

  // if target was not found in the mainlist, give it a buffer
  if (target != NULL) {
	void* mem_pointer = find_buffer_from_free_list(target);
	return mem_pointer;
  } else {
	return NULL;
  } 
}

void*
find_buffer_from_free_list(free_list* list)
{

  if (list->start == NULL) {
 	// all buffers of this size have been already allocated
	// so we need to allocated more buffers to that free list
	allocate_buffers_to_list(list);
  }

  buffer_header* buf = list->start;
  // set the start of the list to point to the next free block
  list->start = (buffer_header*)buf->nextblock;
  // set the next pointer of the now removed buffer to point back at the list
  buf->nextblock = (void*)list;
  // increment the used count of list
  list->used++;
  main_list* temp = (main_list*)entry_point->ptr;
  if (list != &temp->kpages) {
	temp->used++;
  }
  // return a pointer to the free space in the buffer
  return (void*)buf + sizeof(buffer_header);
}

void
allocate_buffers_to_list(free_list* list)
{
  // need to allocate more space
  // find what size the buffers need to be
  int size = list->size;
  // allocate a new page to this free list
  kma_page_t* new_page = get_page();
  // find the number of buffers we can get from the new page
  int divisions = PAGESIZE / size;
  // grab the start pointer of the new page
  void* page_start = new_page->ptr;
  int i;
  // make a free list of buffers of size 
  for (i = 0; i < divisions; i++)
  {
	// create a new buffer
	buffer_header* buf = (buffer_header*)(page_start + i * size);
	// set the nextblock to point to the buf pointed to by list->start
	buf->nextblock = list->start;
	// set the start to the most recently created buf
	list->start = buf;
  }
  
  // add page to free list of pages in order to keep track of it
  main_list* temp = (main_list*)entry_point->ptr;
  page_node* p_node = (page_node*)find_buffer_from_free_list(&(temp->kpages));
  p_node->page = new_page;
  // add p_node to the linked list of pages 
  p_node->next = list->page_list;
  list->page_list = p_node;

}

void
kma_free(void* ptr, kma_size_t size)
{
  // get to the beginning of the block by subtracting the size of the buffer_header from the pointer passed in
  buffer_header* buf = (buffer_header*)((void*)ptr - sizeof(buffer_header));
  // return the buffer to the free list we first removed it from
  // when we removed it, we set the next pointer to point to the list it originally came from
  free_list* list = (free_list*)buf->nextblock;
  // adding it back to the linked list
  buf->nextblock = list->start;
  list->start = buf;
  list->used--;

  if (list->used == 0) {
	list->start = NULL;
	// start from the top, traverse the linked list of pages and free them
	// this is done when a certain free list is empty
	page_node* temp = list->page_list;
	while (temp != NULL) {
		free_page(temp->page);
		temp = temp->next;
	}
	// all buffers of this free list have been freed
	list->page_list = NULL;
  }

  // decrease the count from the main list too
  main_list* temp_main_list = (main_list*)entry_point->ptr;
  temp_main_list->used--;
  // if the main list is empty, we should free the pages that are used to hold the page_nodes
  if (temp_main_list->used == 0) {
	page_node* temp_page_node = temp_main_list->kpages.page_list;
	while (temp_page_node->next != NULL) {
		free_page(temp_page_node->page);
		temp_page_node = temp_page_node->next;
	}
	free_page(temp_page_node->page);
	entry_point = NULL;
  }
}

void
initialize_page(kma_page_t* page_ptr)
{
  // initialize the page if it hasn't been initialized before.
  // This involves setting the main fields of each struct.

  kma_page_t* current_page = page_ptr; 
  main_list* current_main_list = (main_list*)current_page->ptr;
  int i;
	
	current_main_list->buffer_32.size = 32;
	current_main_list->buffer_64.size = 64;
	current_main_list->buffer_128.size = 128;
	current_main_list->buffer_256.size = 256;
	current_main_list->buffer_512.size = 512;
	current_main_list->buffer_1024.size = 1024;
	current_main_list->buffer_2048.size = 2048;
	current_main_list->buffer_4096.size = 4096;
	current_main_list->buffer_8192.size = 8192; 
	current_main_list->kpages.size = sizeof(page_node) + sizeof(buffer_header);

	current_main_list->buffer_32.used = 0;
	current_main_list->buffer_64.used = 0;
	current_main_list->buffer_128.used = 0;
	current_main_list->buffer_256.used = 0;
	current_main_list->buffer_512.used = 0;
	current_main_list->buffer_1024.used = 0;
	current_main_list->buffer_2048.used = 0;
	current_main_list->buffer_4096.used = 0;
	current_main_list->buffer_8192.used = 0;
	current_main_list->kpages.used = 0;

	current_main_list->buffer_32.page_list = NULL;
	current_main_list->buffer_64.page_list = NULL;
	current_main_list->buffer_128.page_list = NULL;
	current_main_list->buffer_256.page_list = NULL;
	current_main_list->buffer_512.page_list = NULL;
	current_main_list->buffer_1024.page_list = NULL;
	current_main_list->buffer_2048.page_list = NULL;
	current_main_list->buffer_4096.page_list = NULL;
	current_main_list->buffer_8192.page_list = NULL;
	current_main_list->kpages.page_list = NULL;

	current_main_list->buffer_32.start = NULL;
	current_main_list->buffer_64.start = NULL;
	current_main_list->buffer_128.start = NULL;
	current_main_list->buffer_256.start = NULL;
	current_main_list->buffer_512.start = NULL;
	current_main_list->buffer_1024.start = NULL;
	current_main_list->buffer_2048.start = NULL;
	current_main_list->buffer_4096.start = NULL;
	current_main_list->buffer_8192.start = NULL; 
	current_main_list->kpages.start = NULL;


	// divide the remaining space into buffers that hold page_nodes
	// the first page is used to hold the main list and the page_nodes
	int remaining_space = PAGESIZE - sizeof(main_list);
	void* start_point = current_page->ptr + sizeof(main_list);
	int count = remaining_space / (sizeof(page_node) + sizeof(buffer_header));
	buffer_header* buf;
	
	for (i = 0; i < count; i++) {
		buf = (buffer_header*)(start_point + i * (sizeof(page_node) + sizeof(buffer_header)));
		buf->nextblock = current_main_list->kpages.start;
		current_main_list->kpages.start = buf;
	}

	main_list* temp = (main_list*)entry_point->ptr;
	page_node* p_node = (page_node*)find_buffer_from_free_list(&temp->kpages);
	p_node->page = current_page;
	p_node->next = current_main_list->kpages.page_list;
	current_main_list->kpages.page_list = p_node;
	
}

#endif // KMA_P2FL
