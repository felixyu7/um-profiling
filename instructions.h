/* 
 * instructions.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW6, Universal Virtual Machine
 * Interface of the instructions module of the UM. Provides all
 * necessary computations and functions for the supported UM instructions.
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * November 17, 2019
 */

#ifndef INSTRUCTIONS_INCLUDED
#define INSTRUCTIONS_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: if content of register C != 0, then move
                register B into register A
*/
void conditional_move(uint32_t *A, uint32_t *B, uint32_t *C);


/*
  Parameters: Main_memory and pointers to registers A,B,C
  Returns: none

  Does: loads word from $m[$r[B]][$r[C]] into $r[A]
*/
void segmented_load(Main_memory memory,uint32_t *A,uint32_t *B,uint32_t *C);

/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: loads $r[C] into $m[$r[A]][$r[B]]
*/
void segmented_store(Main_memory memory,uint32_t *A,uint32_t *B,uint32_t *C);


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: add values in register B and register C,
                then store results in A
*/
void addition(uint32_t *A, uint32_t *B, uint32_t *C);


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: multiply values in register B and register C,
                then store results in A
*/
void multiplication(uint32_t *A, uint32_t *B, uint32_t *C);


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: divides values in register B and register C,
                then store results in A
*/
void division(uint32_t *A, uint32_t *B, uint32_t *C);



/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: performs Bitwise NAND for values in register B 
                and register C, then load results in regsiter A 
*/
void bitwise_NAND(uint32_t *A, uint32_t *B, uint32_t *C);



/*
  Parameters: bool pointer
  Returns: none

  Does: computation stops
*/
void halt(bool *status);


/*
  Parameters: um memory struct, and pointers to registers A,B,C
  Returns: none

  Does: A new segment is created with a number of words equal
                to the values in register C. and initializes them to 0
*/
void map_segment(Main_memory um_memory, uint32_t *C, uint32_t *B);


/*
  Parameters: um memory struct, and pointer to register C
  Returns: none

  Does: unmaps Segment in register C, 
*/
void unmap_segment(Main_memory um_memory, uint32_t *C);


/*
  Parameters: pointer to register C
  Returns: none

  Does: the value in register C is written to the I/O
                device. and the value is between 0 -> 255
*/
void output(uint32_t *C, FILE *);


/*
  Parameters: pointer to register C
  Returns: none

  Does: receives input from I/O device, then loads the value
                into register C. the value shall be between 0 -> 255
*/
void input(uint32_t *C, FILE *);


/*
  Parameters: um memory struct, pointer to register C, B
  Returns: none

  Does: segment m[r[B]] is duplicated, and duplicate replaces
                m[0]. Then, program counter points to m[0][r[C]]
*/
void load_program(Main_memory um_memory, uint32_t *B,  uint32_t *C);

/*
  Parameters: pointer to register A and 32 bit word
  Returns: none

  Does: This function is called only when the opcode
  is 13. Then, its loads into register A
*/
void load_value(uint32_t *A, uint32_t value);


#endif 