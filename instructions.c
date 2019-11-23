/* 
 * instructions.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW6, Universal Virtual Machine
 * Implementation of the instructions module of the UM. Provides all
 * necessary computations and functions for the supported UM instructions.
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * November 17, 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include "instructions.h"
#include "assert.h"


const long MOD_CONST= 4294967296; /*2^32*/


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: add values in register B and register C,
                then store results in A
*/
void addition(uint32_t *A, uint32_t *B, uint32_t *C)
{        

        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        *A = (*B + *C) % MOD_CONST;
}



/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: multiply values in register B and register C,
                then store results in A
*/
void multiplication(uint32_t *A, uint32_t *B, uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        *A = ((*B) * (*C)) % MOD_CONST;
}



/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: divides values in register B and register C,
                then store results in A
*/
void division(uint32_t *A, uint32_t *B, uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        /*C can't be 0 because can't divide any number by 0*/
        assert(*C != 0);
        *A = ((*B) / (*C));
}


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: performs Bitwise NAND for values in register B 
                and register C, then load results in regsiter A 
*/
void bitwise_NAND(uint32_t *A, uint32_t *B, uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        *A = ~((*B) & (*C));
}


/*
  Parameters: pointers to registers A,B,C
  Returns: none

  Does: if content of register C != 0, then move content of
          register B into register A
*/
void conditional_move(uint32_t *A, uint32_t *B, uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        if (*C != 0)
        {
                *A = *B;
        }
}


/*
  Parameters: pointer to register C
  Returns: none

  Does: the value in register C is written to the I/O
                device. and the value is between 0 -> 255
*/
void output(uint32_t *C, FILE *I_O_Device)
{
        /*C shall never be Null*/
        assert(C != NULL);

        /*only values from 0 to 255 are allowed*/
        assert(*C <= 255);

        /*Write 8byte to the stdout*/
        fprintf(I_O_Device, "%c", *C);
        //printf("%d\n",*C);
}


/*
  Parameters: pointer to register C
  Returns: none

  Does: receives input from I/O device, then loads the value
                into register C. the value shall be between 0 -> 255
*/
void input(uint32_t *C, FILE *I_O_Device)
{
        (void) I_O_Device;
        /*C shall never be Null*/
        assert(C != NULL);

        /*read in an 8 byte value*/
        char data = fgetc(I_O_Device);
        if(data != EOF){ 
                if(data == 10){ /*catches new line*/
                        input(C, I_O_Device);
                }
                else{
                        *C = data;
                }
        }
        else { 
                /*register C is loaded with a full 32-bit word in which
                 every bit is 1*/
                *C = MOD_CONST - 1;
        }
}

/*
  Parameters: pointer to register A and 32 bit word
  Returns: none

  Does: This function is called only when the opcode
  is 13. Then, its loads into register A
*/
void load_value(uint32_t *A, uint32_t value)
{
        assert(A != NULL);
        *A = value;
}


/*
  Parameters: pointer to register C, B, and memory struct
  Returns: none

  Does: segment m[r[B]] is duplicated, and duplicate replaces
                m[0]. Then, program counter points to m[0][r[C]]
*/
void load_program(Main_memory um_memory, uint32_t *B,  uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(B != NULL);
        assert(C != NULL);

        /* set new 0 segment to register B */
        new_program(um_memory, *B);
}



/*
  Parameters: um memory struct, and pointers to registers A,B,C
  Returns: none

  Does: loads word from $m[$r[B]][$r[C]] into $r[A]
*/
void segmented_load(Main_memory memory,uint32_t *A,uint32_t *B,uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        /*this retrieves word stored at $m[$r[B]][$r[C]]*/
        uint32_t word = get_word(memory, *B, *C);

        /*the set $r[A] receive what returned above*/
        *A = word;
}


/*
  Parameters: um memory struct, and pointers to registers A,B,C
  Returns: none

  Does: loads $r[C] into $m[$r[A]][$r[B]]
*/
void segmented_store(Main_memory memory,uint32_t *A,uint32_t *B,uint32_t *C)
{
        /*pointers shall not be NULL*/
        assert(A != NULL);
        assert(B != NULL);
        assert(C != NULL);

        put_word(memory, *A, *B, *C);
}



/*
  Parameters: um memory struct, and pointers to registers A,B,C
  Returns: none

  Does: A new segment is created with a number of words equal
                to the values in register C. and initializes them to 0
*/
void map_segment(Main_memory um_memory, uint32_t *C, uint32_t *B)
{

        /*pointers shall not be NULL*/
        assert(B != NULL);
        assert(C != NULL);

        /*creates new segment with size equal to $r[C] and initializes
                each word to 0*/
        *B = map(um_memory, *C);
}


/*
  Parameters: um memory struct, and pointer to register C
  Returns: none

  Does: unmaps Segment in register C, 
*/
void unmap_segment(Main_memory um_memory, uint32_t *C)
{
        assert(C != NULL);
        assert(*C != 0);

        /*segment $m[$r[C]] is unmapped*/
        unmap(um_memory, *C);
}


/*
  Parameters: bool pointer
  Returns: none

  Does: computation stops
*/
void halt(bool *status)
{
        assert(status != NULL);
        *status = true;
}