/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the McKusick-Karels
 *              algorithm
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
#ifdef KMA_MCK2
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

// struct used as header of each buffer
typedef struct 
{
  void * nextblock;
} buffer_t;

typedef struct
{
  buffer_t* header;
} buffer_list_t;

typedef struct page_node_struct
{
  kma_page_t* page;
  struct page_node_struct* next; 
} page_node_t;

typedef struct pagehead
{
  int num_alloc;
  int size;
  void* ptr;
  void* next_block;
  struct pagehead* next_page; 
} page_header_t; 

typedef struct 
{
  page_header_t header;
  int page_num;
  buffer_list_t freelist[10];
  page_header_t* header;
} mainpage_t;

/************Global Variables*********************************************/
kma_page_t* entrypage;
/************Function Prototypes******************************************/
kma_page_t* init_entrypage(kma_page_t* page);
kma_page_t* init_page(kma_page_t* page, kma_size_t size); 

void* search_bins(kma_size_t size); 
void* search_pages(kma_size_t size);

void remove_allocation(void* ptr);
void clear_bins(kma_page_t* page);

int page_number(long val, long fp_header);
size_t hash(size_t size); 
kma_size_t blk_size(kma_size_t size);

/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  if((size+sizeof(kma_page_t*)) > PAGESIZE) {
    return NULL;
  }
  if(entrypage == NULL) {
    // initialize the entry page
    entrypage = init_entrypage(get_page());
  }

  // Entry page header set 
  mainpage_t* entrypage_header = (mainpage_t*)(entrypage->ptr);
  kma_size_t alloc_size = blksize(size);

  // Search through the bins 
  void* alloc = search_bins(size); 

  if(alloc != NULL) {
    // Bin search was successful
    int page = page_number((long)alloc, (long)entrypage);
    page_header_t* current = (page_header_t*)((long)entrypage_header + (page * PAGESIZE));
  
    // Increment Allocations
    current->num_alloc++;
    entrypage_header->header.num_alloc++;
    alloc_remove(alloc);
    return alloc;
  } else {
    // Bin search was unsuccessful 
    alloc = search_pages(size);
    if(alloc != NULL) {
      // search page successful
      int page = page_number((long)alloc, (long)entrypage_header); 
      page_header_t* current = (page_header_t*)((long)entrypage_header + (page * PAGESIZE));

      // Increment allocations
      current->num_alloc++;
      entrypage_header->header.num_alloc++;

      current->nextblock = (void*)((long)current->next+block + alloc_size);
      
  }
}

void
kma_free(void* ptr, kma_size_t size)
{
  ;
}

#endif // KMA_MCK2
