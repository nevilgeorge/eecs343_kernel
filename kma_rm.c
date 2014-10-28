/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the resource map algorithm
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
#ifdef KMA_RM
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

/*** pair_t *****/
/* Used to keep the 
   pair <base, size> 
   resource map entries.
   Has a linked list form */
typedef struct
{
  int size;
  void * nextblock;
  void * prevblock;
} pair_t; 
/****************/

/*** page_info_t ***/
/* Contains necessary 
   information about 
   the page. 
   Makes life easier
   when freeing & 
   coalescing pages. */
typedef struct
{
  int buffer_count;
  int page_count;
  pair_t * entry;
  void * page;
} page_info_t;
/**************/

/************Global Variables*********************************************/
kma_page_t * g_rmap = NULL;

/************Function Prototypes******************************************/
void new_page();
void * find_space(kma_size_t size);
void add_pair(void * base, kma_size_t size);
void delete_pair(void * base);
void coalesce(void * ptr);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

/****************************************************************************
 * Name: kma_malloc *
 * Purpose: kernel memory allocator (malloc)
 * Description: Returns pointer to a block of memory upon request
****************************************************************************/
void* kma_malloc(kma_size_t size)
{
  if((size + sizeof(kma_page_t*)) > PAGESIZE)
  {
    return NULL;
  }
  
  if(size < sizeof(kma_page_t)) 
  {
    size = sizeof(kma_page_t);
  }

  if (g_rmap == NULL) { // CREATE NEW PAGE IF THERE IS NO PAGE YET
    g_rmap = get_page();
    new_page(g_rmap);
  }

  void * ret = find_space(size);
  page_info_t * basepage = BASEADDR(ret);
  basepage->buffer_count++;
  return ret;
}

/***************************************************************************
 * Name: kma_free 
 * Input: void * pointer_to_block_to_be_freed, kma_size_t size_of_block
 * Output: None
 * Description: Frees the memory block of a given pointer
 **************************************************************************/
void kma_free(void* ptr, kma_size_t size)
{
  add_pair(ptr, size); // New pair of free memory
  coalesce(ptr); // Coalesce the memory space
}

/***************************************************************************
 * Name: new_page
 * Input: kma_page_t * pointer_to_new_page
 * Output: none
 * Purpose: Set up a new page
 **************************************************************************/
void new_page(kma_page_t* page)
{
  *((kma_page_t**)page->ptr) = page; 
  page_info_t * pageinfo = (page_info_t *)(page->ptr);
  pageinfo->page_count = 0;
  pageinfo->buffer_count = 0;
  pageinfo->entry = (pair_t*)((long int)pageinfo + sizeof(page_info_t));
  pageinfo->page = page;
  add_pair((void*)(pageinfo->entry), (PAGESIZE-sizeof(page_info_t)));
}

void* find_space(kma_size_t size)
{
  page_info_t * pageinfo = (page_info_t*)(g_rmap->ptr);
  pair_t * npair = (pair_t*)(pageinfo->entry); 
  while(npair != NULL)
  {
    if(npair->size < size) // Not enough space 
    {
      npair = (pair_t*)(npair->nextblock);
      continue;
    } else if (npair->size == size || npair->size - size < sizeof(pair_t)) // Perfect fit
    {
      delete_pair(npair); 
      return ((void*)npair);
    } else {
      add_pair((void*)((long int)npair + size), (npair->size - size)); 
      delete_pair(npair); 
      return ((void*)npair);
    }  
  }
  // No more space left in the page: Get a new page
  kma_page_t* newpage = get_page();
  new_page(newpage);
  pageinfo->page_count++;
  return find_space(size); 
}

void add_pair(void * base, kma_size_t size) 
{
  page_info_t * pageinfo = (page_info_t*)(g_rmap->ptr);
  void * entry = (pair_t*)(pageinfo->entry);
  
  ((pair_t*)base)->size = size;
  ((pair_t*)base)->prevblock = NULL;
  
  if(base < entry) // CASE WHERE THE NEW BLOCK IS LESS THAN THE ENTRY TO LIST 
  {
    ((pair_t*)entry)->prevblock = base; // Update the prev of what used to be first node
    ((pair_t*)base)->nextblock = entry; // Update the next of new first node
    pageinfo->entry = (pair_t*)base; // Update the linked list entry pointer in g_rmap
  } else if (base == entry) // CASE WHERE PAGE IS COMPLETELY EMPTY (PROBABLY NEW PAGE)
  {
    ((pair_t*)base)->nextblock = NULL; 
  } else  // ALL THE OTHER CASES
  {
    // Find where to put it in the linked list 
    while(((pair_t*)entry)->nextblock != NULL && entry < base) // Loop till right address is found
    {
      entry = ((pair_t*)entry)->nextblock; 
    }
    void * entry_next = ((pair_t*)entry)->nextblock;
    if(entry_next != NULL) // if the next block isn't null, set its prevblock as base
    { 
      ((pair_t*)entry_next)->prevblock = base; 
    }
    ((pair_t*)entry)->nextblock = base; // add the base into the linked list
    ((pair_t*)base)->prevblock = entry;
    ((pair_t*)base)->nextblock = entry_next;
  }
}

void delete_pair(void * base)
{
  void * ptr = (pair_t*)base;
  void * ptr_prev = ((pair_t*)ptr)->prevblock;
  void * ptr_next = ((pair_t*)ptr)->nextblock;
  if(ptr_prev == NULL && ptr_next == NULL) // There's only one pair
  {
    page_info_t * pageinfo = (page_info_t*)(g_rmap->ptr);
    pageinfo->entry = NULL;
    g_rmap = NULL;
    return;
  } else if(ptr_next == NULL) // The node is last node in the list 
  { 
    ((pair_t*)ptr_prev)->nextblock = NULL;
  } else if (ptr_prev == NULL) // The node is first node in the list 
  { 
    ((pair_t*)ptr_next)->prevblock = NULL;
    page_info_t * pageinfo = (page_info_t*)(g_rmap->ptr);
    pageinfo->entry = ptr_next; // Update the global entry into the list 
    return;
  } else  // The node is in between: typical linked list node removal 
  {
    pair_t * temp1 = ((pair_t*)ptr)->prevblock;
    pair_t * temp2 = ((pair_t*)ptr)->nextblock;
    temp1->nextblock = temp2;
    temp2->prevblock = temp1;
    return;
  }
}

void coalesce(void * ptr)
{
  // Coalesce appropriate free memory blocks
  // Also frees the page if necessary 
  page_info_t* basepage = BASEADDR(ptr);
  basepage->buffer_count--;

  page_info_t* firstpage = (page_info_t*)(g_rmap->ptr);
  page_info_t* lastpage;
  int end = firstpage->page_count;
  int flag = 1;

  while(flag)
  {
    lastpage = ((page_info_t*)((long int)firstpage + end * PAGESIZE));
    flag = 0;
    if(lastpage->buffer_count == 0)
    {
      flag = 1;
      pair_t * temp;
      for(temp = firstpage->entry; temp != NULL; temp = temp->nextblock)
      {
        if(BASEADDR(temp) == lastpage)
          delete_pair(temp);
      }
      flag = 1;
      if(lastpage == firstpage)
      {
        flag = 0;
        g_rmap = NULL;
      }
      free_page((kma_page_t*)(lastpage->page));
      if(g_rmap != NULL)
        firstpage->page_count--;
    }
    end--;
  }
}

#endif // KMA_RM

