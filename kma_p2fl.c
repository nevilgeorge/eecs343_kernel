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

// struct used as the header to a free list
typedef struct
{
  int size; // size of buffers in this free list
  int used; // number of blocks used
  buffer_header* start; // pointer to the first buffer in the linked list
} free_list_head;

typedef struct
{
  free_list_head buffer_32;
  free_list_head buffer_64;
  free_list_head buffer_128;
  free_list_head buffer_256;
  free_list_head buffer_512;
  free_list_head buffer_1024;
  free_list_head buffer_2048;
  free_list_head buffer_4096;
  free_list_head buffer_8192;
  // main list of next page
  main_list* next_main_list;
} main_list;
/************Global Variables*********************************************/
kma_page_t* entry_point;
/************Function Prototypes******************************************/

void* kma_malloc(kma_size_t size);
void kma_free(void* ptr, kma_size_t size);
void initialize_page(kma_page_t* page_ptr, int size);

/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  // cannot allocate a space larger than a page
  if (size > PAGESIZE) {
	return NULL;
  }

  if (entry_point == NULL) {
	// no page allocated yet, need to allocate first page
	entry_point = getPage();
	initialize_page(entry_point, size);
  } 
}

void
kma_free(void* ptr, kma_size_t size)
{
  ;
}

void
initialize_page(kma_page_t* page_ptr, int size)
{
  // initialize the page by splitting the page into powers of two
  // however, we have to check whether the size needing allocation is greater than 
  // 4096. If so, we should not split the page.
  kma_page_t current_page = *page_ptr;
  main_list* current_main_list = (main_list*)current_page->ptr;

  if (size > PAGESIZE / 2) {
	current_main_list->buffer_32 = NULL;
	current_main_list->buffer_64 = NULL;
	current_main_list->buffer_128 = NULL;
	current_main_list->buffer_256 = NULL;
	current_main_list->buffer_512 = NULL;
	current_main_list->buffer_1024 = NULL;
	current_main_list->buffer_2048 = NULL;
	current_main_list->buffer_4096 = NULL;
	
	current_main_list->buffer_8192.size = 8192; 
	current_main_list->buffer_8192.blocks_left = 1; 
	current_main_list->buffer_8192.start = NULL; 
  } else {
  // if the size requested is less than half PAGESIZE, then we can split the page into a regular power of 2 list
	
	current_main_list->buffer_32.size = 32;
	current_main_list->buffer_64.size = 64;
	current_main_list->buffer_128.size = 128;
	current_main_list->buffer_256.size = 256;
	current_main_list->buffer_512.size = 512;
	current_main_list->buffer_1024.size = 1024;
	current_main_list->buffer_2048.size = 2048;
	current_main_list->buffer_4096.size = 4096;

	current_main_list->buffer_32.used = 0;
	current_main_list->buffer_64.used = 0;
	current_main_list->buffer_128.used = 0;
	current_main_list->buffer_256.used = 0;
	current_main_list->buffer_512.used = 0;
	current_main_list->buffer_1024.used = 0;
	current_main_list->buffer_2048.used = 0;
	current_main_list->buffer_4096.used = 0;

	current_main_list->buffer_32.start = NULL;
	current_main_list->buffer_64.start = NULL;
	current_main_list->buffer_128.start = NULL;
	current_main_list->buffer_256.start = NULL;
	current_main_list->buffer_512.start = NULL;
	current_main_list->buffer_1024.start = NULL;
	current_main_list->buffer_2048.start = NULL;
	current_main_list->buffer_4096.start = NULL;

	//split pages into powers of two
  } 
}

#endif // KMA_P2FL
