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

/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

typedef struct
{
  int size;
  void * nextblock;
  void * prevblock;
} pair_t; 

/************Global Variables*********************************************/
kma_page_t * g_rmap = NULL;

/************Function Prototypes******************************************/
int check_requested_size(kma_size_t size);
void new_page();
void * find_space();
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  if((size + sizeof(kma_page_t*)) > PAGESIZE)
  {
    return NULL;
  }

  if (g_rmap == NULL) { // CREATE NEW PAGE IF THERE IS NO PAGE YET
    new_page();
  } 

  void * ret = find_space();
  return ret; 
}

void
kma_free(void* ptr, kma_size_t size)
{
  ptr = (kma_page_t *) ptr;
  
}

void new_page()
{
  g_rmap = get_page();
  *((kma_page_t**)g_rmap->ptr) = g_rmap;
}

void * find_space(kma_size_t size)
{
  void * ret; 
  pair_t * npair = (pair_t*)(g_rmap->ptr);
  while(npair->size < size)
  {
    if(npair->nextblock == NULL) // There's no more space
    {
      new_page(); // Make a new page 
    }
    npair = npair->nextptr; 
  }
  add_pair((void*)npair+size, npair->size-size); // Add the new pair space
  delete_pair(npair); // Erase what was there before
  return (void*)npair; 
}

void add_pair(void * base, kma_size_t size) 
{
  (pair_t*)base->size = size;
  (pair_t*)base->prev = NULL;
  
  if(base < g_rmap->ptr) // CASE WHERE THE NEW BLOCK IS LESS THAN THE START 
  {
    (pair_t*)((g_rmap->ptr)->prev) = base; // Update the prev of what used to be first node
    base->next = (pair_t*)g_rmap->ptr; // Update the next of new first node
    g_rmap->ptr = (pair_t*)base; // Update the linked list entry pointer in g_rmap
  } else if (base == g_rmap->ptr) // CASE WHERE PAGE IS COMPLETELY EMPTY (PROBABLY NEW PAGE)
  {
    (pair_t*)base->next = NULL; 
  } else  // ALL THE OTHER CASES
  {
    // Find where to put it in the linked list 
    void * temp; 
    (pair_t*) temp = g_rmap->ptr;
    while((pair_t*)temp->nextblock != NULL && temp < base) // Loop till right address is found
    {
      temp = (pair_t*)temp->nextblock; 
    }
    pair_t* tempn = (pair_t*)temp->nextblock;
    if(tempn != NULL) // if the next pointer isn't null, set its prevblock as base
    { 
      tempn->prevblock = base;
    }
    (pair_t*)temp->nextblock = base; // add the base into the linked list
    (pair_t*)base->prevblock = temp; 
    (pair_t*)base->nextblock = tempn;
  }
}

void delete_pair(void * base)
{
  pair_t * ptr = (pair_t*)base;
  pair_t * ptr_prev = ptr->prevblock;
  pair_t * ptr_next = ptr->nextblock;

  if(ptr_prev == NULL && ptr_next == NULL) // There's only one pair
  {
    (pair_t*)g_ptr->ptr = NULL;
  } else if(ptr_next == NULL)
  { 
    ptr_prev->nextblock = NULL;
  } else if (ptr_prev == NULL)
  {
    ptr_prev->next = NULL;
    (pair_t*)g_rmap->ptr = ptr_next;
  } else 
  {
    (pair_t*)ptr_prev->nextblock = ptr_next;
    (pair_t*)ptr_next->prevblock = ptr_prev;
  }
}

int check_requested_size(kma_size_t size)
{
  if((size + sizeof(pair_t*)) > PAGESIZE)
  {
    return 0;
  } 
}

#endif // KMA_RM

