/* 
 * memory.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW6, Universal Virtual Machine
 * Implementation of the memory module of the UM. Provides a representation
 * of the memory of the UM, and contains all functions necessary for
 * creation, management, and freeing of the memory.
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * November 17, 2019
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "seq.h"
#include "memory.h"
#include "assert.h"
#include "mem.h"


/*
  Parameters: none
  Returns: memory struct

  Does: creates a memory struct and initializes it to the content
        of the pointer that was passed in
*/
Main_memory init_memory()
{
        Main_memory ret = malloc(sizeof(struct Main_memory));

        ret->mapped = Seq_new(0);

        ret->unmapped = Seq_new(0);
        for (int i = 0; i < 128; i++) {
                Seq_addhi(ret->unmapped, (void *)(uintptr_t)i);
        }

        return ret;
}



/*
  Parameters: memory struct and segment index
  Returns: pointer to a segment

  Does: retrieves a pointer to a particular segment from mapped segments
*/
uint32_t *get_segment(Main_memory mem, uint32_t address) 
{
        return (uint32_t *)Seq_get(mem->mapped, address);
}


void put_word(Main_memory mem, uint32_t address, int offset, uint32_t val)
{
        uint32_t *temp = get_segment(mem, address);
        /* add one to account for extra space to store size */
        temp[offset + 1] = val;
}

/*
  Parameters: memory struct, segment index, and offset
  Returns: uint32_t value

  Does: retrieves a word at $m[address][offset]
*/
uint32_t get_word(Main_memory mem, uint32_t address, int offset)
{
        uint32_t *temp = get_segment(mem, address);
        assert(temp != NULL);
        /* add one to account for extra space to store size */
        return temp[offset + 1];
}

/*
  Parameters: memory struct and int value
  Returns: uint32_t value that represents address to the mapped segment

  Does: maps a segment of given size and it also initializes every
                word in the segment
*/
uint32_t map(Main_memory mem, size_t size)
{
        int curr_length = Seq_length(mem->mapped);

        /* if there are no more unmapped addresses, make more */
        if (Seq_length(mem->unmapped) == 0) {
                for (int i = curr_length; i < curr_length + 128; i++) {
                        Seq_addhi(mem->unmapped, (void *)(uintptr_t)i);
                }
        }

        uint32_t *new_seg = calloc(size + 1, sizeof(uint32_t));
        assert(new_seg);
        new_seg[0] = size; /* tracks size of array */
        uint32_t address = (uint32_t)(uintptr_t)Seq_remlo(mem->unmapped);

        /* check that address does not go out of bounds in the mapped seq */
        if ((int)address >= curr_length) {
                Seq_addhi(mem->mapped, new_seg);
        }
        else {
                Seq_put(mem->mapped, address, new_seg);
        }

        return address;
}


/*
  Parameters: memory struct and int index
  Returns: none

  Does: unmaps segment at mem[address];
*/
void unmap(Main_memory mem, uint32_t address) 
{
        uint32_t *temp = get_segment(mem, address);
        assert(temp != NULL);
        FREE(temp); /* free memory associated */
        Seq_put(mem->mapped, address, NULL); /* remove from mapped seq */

        Seq_addlo(mem->unmapped, (void *)(uintptr_t)address);
}

/*
  Parameters: memory struct and address of the new program
  Returns: none

  Does: destroys the current 0 segment, and replaces it with the given
  segment
*/
void new_program(Main_memory mem, uint32_t source)
{      
        /* if source program is already the 0 segment */
        if (source == 0) {
                return;
        }

        uint32_t *new_program = Seq_get(mem->mapped, source);
        uint32_t size = new_program[0];

        /* create a new brand new 0 segment with the new size */
        unmap(mem, 0);

        /* should be assigned the 0 address */
        int zero = map(mem, size);
        assert(zero == 0);

        uint32_t *zero_segm = Seq_get(mem->mapped, 0);

        /* copy memory over from new program to the 0 segment */
        for (int i = 0; i < (int)(size + 1); i++) {
                zero_segm[i] = new_program[i];
        }
}

/*
  Parameters: Memory struct
  Returns: none

  Does: frees all memory associated with the memory struct
*/
void delete_mem(Main_memory mem) {
        int curr_length = Seq_length(mem->mapped);

        for (int i = 0; i < curr_length; i++) {
                free(get_segment(mem, i));
        }

        Seq_free(&mem->mapped);
        Seq_free(&mem->unmapped);
        FREE(mem);
}