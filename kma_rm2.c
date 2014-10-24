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
} k_free_block; 

/************Global Variables*********************************************/
kma_page_t * g_rmap = NULL;

/************Function Prototypes******************************************/
int check_requested_size(kma_size_t size);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  if((size + sizeof(kma_page_t*)) > PAGESIZE)
  {
    return NULL;
  }
 // USING FIRST FIT
 // check if next_free_page is large enough to hold the new kma_page_t and size of the allocated memory
 // if yes, store there
 // if not, then find next free space
 // go through entire memory and coalesce memory

  kma_page_t * map;
  if (pool == NULL) { // CREATE NEW PAGE 
    g_rmap = get_page();
    *((kma_page_t**)g_rmap->ptr) = g_rmap;
    map = g_rmap;
    k_free_block * fb; // Create a new free_block struct in stack
    fb->size = PAGESIZE-size; // Initialize the free block struct
    fb->nextblock = NULL;
    fb->prevblock = NULL;
    memcpy(g_rmap, fb, sizeof(k_free_block)); // Copy the thing into the page 
    return &g_rmap + sizeof(g_rmap->ptr); // Return the address of the allocated block
  } else {
    // Page already exists 
    k_free_block * fb = g_rmap->ptr;
    while(fb->size < size) {
      if(fb->nextblock == NULL) {
        // Request a new page & update everything else
      }
      fb = fb->nextblock;
    }
    fb->size = fb->size - size;
    fb->next = NULL;
    fb->prev = NULL;
    memcpy(g_rmap, ); 
  }
  while(map->size < size) {
    map = map -> ptr; // loop through the linked list till there's big enough space
     
  }
  
  g_rmap->ptr = map->ptr + size;
  g_rmap->size = map->size - size;
  return map->ptr + sizeof(kma_page_t*);
}

void
kma_free(void* ptr, kma_size_t size)
{
  ptr = (kma_page_t *) ptr;
  
}

int check_requested_size(kma_size_t size)
{
  if((size + sizeof(kma_page_t *)) > PAGESIZE)
  {
    return 0;
  } 
}

#endif // KMA_RM

