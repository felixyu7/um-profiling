/* 
 * memory.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW6, Universal Virtual Machine
 * Interface of the memory module of the UM. Provides a representation
 * of the memory of the UM, and contains all functions necessary for
 * creation, management, and freeing of the memory.
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * November 17, 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "seq.h"

typedef struct Main_memory {
        Seq_T mapped;
        Seq_T unmapped;
} *Main_memory;


/*
  Parameters: none
  Returns: memory struct

  Does: creates a memory struct and initializes it to the content
        of the pointer that was passed in
*/
Main_memory init_memory();


/*
  Parameters: memory struct and segment index
  Returns: pointer to a segment

  Does: retrieves a pointer to a particular segment from mapped segments
*/
uint32_t *get_segment(Main_memory mem, uint32_t address);

void put_word(Main_memory mem, uint32_t address, int offset, uint32_t val);

/*
  Parameters: memory struct, segment index, and offset
  Returns: uint32_t value

  Does: retrieves a word at $m[address][offset]
*/
uint32_t get_word(Main_memory mem, uint32_t address, int offset);

/*
  Parameters: memory struct and address of the new program
  Returns: none

  Does: destroys the current 0 segment, and replaces it with the given
  segment
*/
void new_program(Main_memory mem, uint32_t source);

/*
  Parameters: memory struct and int value
  Returns: uint32_t value that represents address to the mapped segment

  Does: maps a segment of given size and it also initializes every
                word in the segment
*/
uint32_t map(Main_memory mem, size_t size);

/*
  Parameters: memory struct and int index
  Returns: none

  Does: unmaps segment at mem[address];
*/
void unmap(Main_memory mem, uint32_t address);


/*
  Parameters: Memory struct
  Returns: none

  Does: frees all memory associated with the memory struct
*/
void delete_mem(Main_memory mem);