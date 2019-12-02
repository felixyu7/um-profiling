/* 
 * um.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW7, UM Profiling
 * A profiled version of our previous UM implementation. Designed
 * to maximize performance and speed.
 * Benchmarks (Lab 120):
 * midmark.um - 0.39 s
 * sandmark.umz - 9.70 s
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * December 2, 2019
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "seq.h"
#include "assert.h"

typedef struct Main_memory {
        Seq_T mapped;
        Seq_T unmapped;
} *Main_memory;


bool Bitpack_fitsu(uint64_t n, unsigned width);
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value);


        
/*************************************************************************/
Main_memory init_memory();
uint32_t *get_segment(Main_memory mem, uint32_t address);
void put_word(Main_memory mem, uint32_t address, int offset, uint32_t val);
uint32_t map(Main_memory mem, size_t size);
void unmap(Main_memory mem, uint32_t address);
void new_program(Main_memory mem, uint32_t source);
void delete_mem(Main_memory mem);
void input(uint32_t *C, FILE *I_O_Device);


Except_T Bitpack_Overflow = { "Overflow packing bits" };

int main(int argc, char *argv[])
{
        FILE *source;
        if (argc > 1) {
                source = fopen(argv[1], "rb");

                if (source == NULL){
                    fprintf(stderr, "Error: Cannot open the %s file.\n",
                        argv[1]);

                    exit(EXIT_FAILURE);
                }
        }
        else {
                fprintf(stderr, "Error: Invalid number of arguments.\n");
                exit(EXIT_FAILURE);
        }

        /* UM_init */ 
        Main_memory mem = malloc(sizeof(struct Main_memory));

        mem->mapped = Seq_new(0);

        mem->unmapped = Seq_new(0);
        for (int i = 0; i < 128; i++) {
                Seq_addhi(mem->unmapped, (void *)(uintptr_t)i);
        }

        /* read_file */ 
        /* load file */
        fseek(source, 0L, SEEK_END);
        int size = ftell(source);
        rewind(source);

        uint32_t word = 0;
        unsigned char c;

        /* should be assigned the 0 address */
        int zero = map(mem, size);
        assert(zero == 0);

        for (int i = 0; i < size/4; i++) {
                c = getc(source);
                word = Bitpack_newu(word, 8, 24, (uint64_t)c);
                c = getc(source);
                word = Bitpack_newu(word, 8, 16, (uint64_t)c);
                c = getc(source);
                word = Bitpack_newu(word, 8, 8, (uint64_t)c);
                c = getc(source);
                word = Bitpack_newu(word, 8, 0, (uint64_t)c);

                put_word(mem, 0, i, word); /* put in 0 segment */
        }

        uint32_t *registers = calloc(8, sizeof(uint32_t));

        uint32_t prog_counter = 0;

        /* UM_execute */                
        uint32_t inst;
        bool status = false;

        uint32_t *segment = Seq_get(mem->mapped, 0);
        /* while halt has not been called */
        while (status == false) {
                inst = segment[prog_counter + 1];

                /* run_instruction */
                uint32_t opcode = Bitpack_getu(inst, 4, 28);
                uint32_t A_index = 0;
                uint32_t B_index = 0;
                uint32_t C_index = 0;
                uint32_t *A = NULL;
                uint32_t *B = NULL;
                uint32_t *C = NULL;
                /* load_value is special case */
                if (opcode != 13) {
                        if (opcode == 9 || opcode == 10 || opcode == 11) {
                                /*unmap, output, input only use 1 register*/
                                C_index = Bitpack_getu(inst, 3, 0);
                                C = &((registers)[C_index]);
                        }
                        else if (opcode == 8 || opcode == 12) { 
                                /*map and load_program only use 2 registers*/
                                B_index = Bitpack_getu(inst, 3, 3);
                                C_index = Bitpack_getu(inst, 3, 0);
                                B = &((registers)[B_index]);
                                C = &((registers)[C_index]);
                        }
                        else {
                                A_index = Bitpack_getu(inst, 3, 6);
                                B_index = Bitpack_getu(inst, 3, 3);
                                C_index = Bitpack_getu(inst, 3, 0);
                                A = &((registers)[A_index]);
                                B = &((registers)[B_index]);
                                C = &((registers)[C_index]);
                        }
                }
                uint32_t value;

                switch (opcode) {
                        case 0:
                                /*pointers shall not be NULL*/
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);

                                if (*C != 0) {
                                        *A = *B;
                                }
                                break;
                        case 1:
                                /*pointers shall not be NULL*/
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);

                                /*retrieve word stored at $m[$r[B]][$r[C]]*/
                                uint32_t *temp = get_segment(mem, *B);
                                assert(temp != NULL);
                                /*the set $r[A] receive what returned above*/
                                *A = temp[*C + 1];
                                
                                break;
                        case 2:
                                /*pointers shall not be NULL*/
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);

                                put_word(mem, *A, *B, *C);
                                break;
                        case 3:
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);
                                *A = (*B + *C) % 4294967296;
                                break;
                        case 4:
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);
                                *A = ((*B) * (*C)) % 4294967296;
                                break;
                        case 5:
                                /*pointers shall not be NULL*/
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);

                                /*C can't be 0 because can't divide any
                                                         number by 0*/
                                assert(*C != 0);
                                *A = ((*B) / (*C));
                                break;
                        case 6:
                                /*pointers shall not be NULL*/
                                assert(A != NULL);
                                assert(B != NULL);
                                assert(C != NULL);

                                *A = ~((*B) & (*C));
                                break;
                        case 7:
                                status = true;
                                break;
                        case 8:
                                /*pointers shall not be NULL*/
                                assert(B != NULL);
                                assert(C != NULL);

                                /*creates new segment with size equal to 
                                $r[C] and initializes each word to 0*/
                                *B = map(mem, *C);
                                
                                break;
                        case 9:
                                assert(C != NULL);
                                assert(*C != 0);

                                /*segment $m[$r[C]] is unmapped*/
                                unmap(mem, *C);
                                break;
                        case 10:
                                /*C shall never be Null*/
                                assert(C != NULL);

                                /*only values from 0 to 255 are allowed*/
                                assert(*C <= 255);

                                /*Write 8byte to the stdout*/
                                fprintf(stdout, "%c", *C);
                                break;
                        case 11:
                                /*C shall never be Null*/
                                assert(C != NULL);

                                /*read in an 8 byte value*/
                                char data = fgetc(stdin);
                                if (data != EOF) { 
                                        if (data == 10){ /*catches new line*/
                                                input(C, stdin);
                                        }
                                        else {
                                                *C = data;
                                        }
                                }
                                else { 
                                        /*register C is loaded with a 
                                        full 32-bit word in which every
                                        bit is 1*/
                                        *C = 4294967295;
                                }
                                break;
                        case 12:
                                /*pointers shall not be NULL*/
                                assert(B != NULL);
                                assert(C != NULL);

                                /* set new 0 segment to register B */
                                new_program(mem, *B);
                                /* -1 because it iterates again upon return*/
                                prog_counter = *C - 1;
                                segment = Seq_get(mem->mapped, 0);
                                break;
                        case 13:
                        /* special case, re bitpack */
                                A_index = Bitpack_getu(inst, 3, 25);
                                A = &((registers)[A_index]);
                                value = Bitpack_getu(inst, 25, 0);
                                assert(A != NULL);
                                *A = value;
                                break;
                        default:
                                fprintf(stderr, 
                                    "Invalid instruction. opcode %d\n",
                                                             opcode);
                                exit(EXIT_FAILURE);
                                break;
                }

                prog_counter++;
        }

        /* UM_free */
        delete_mem(mem);
        free(registers);
        fclose(source);
	exit(EXIT_SUCCESS);
}

/*
 * Function: Bitpack_getu
 * Input: 64 bit unsigned integer word, and width, 
 * and least significant bit
 * Returns: uint64_t value of queried bitword
 * Returns bits of length width from the entire word stating at lsb,
 * with default value as 0.
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert((width + lsb) <= 64);
        if (width == 0) {
                return 0;
        }

        uint64_t temp = ~0;
        temp = temp >> (64 - width) << lsb;
        return (word & temp) >> lsb;
}

/*
 * Function: Bitpack_fitsu
 * Input: 64 bit unsigned integer to determine fit, and tested width
 * Returns: Boolean value
 * Should return boolean value, true if word of length width can fit inside
 * unsigned integer. Default value is false if width is 0
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        uint64_t temp = 1;
        if (width == 0) {
                return false;
        }
        if (n >= temp << width) {
                return false;
        }
        else {
                return true;
        }
}

/*
 * Function: Bitpack_newu
 * Parameters: 64 bit unsigned integer word, width of bits, 
 * least significant bit of where to put bits, and 64 bit unsigned integer
 * bit that you want to insert into the word
 * Returns: uint64_t value of new word with added value
 * Adds value with length width to the word starting at lsb. Errors if 
 * value would be longer than the length of word, or if width is longer than
 * 64.
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
        uint64_t value)
{
        assert(width <= 64);
        assert((width + lsb) <= 64);

        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t temp = ~0;
        temp = temp >> (64 - width) << lsb;

        word = (word & ~temp);
        value = value << lsb;

        return (word | value);
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

/*
  Parameters: memory struct, segment index, memory address, value to be put in
  Returns: pointer to a segment

  Does: puts the given value into the given spot in memory
*/
void put_word(Main_memory mem, uint32_t address, int offset, uint32_t val)
{
        uint32_t *temp = get_segment(mem, address);
        /* add one to account for extra space to store size */
        temp[offset + 1] = val;
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
        free(temp); /* free memory associated */
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
void delete_mem(Main_memory mem)
{
        int curr_length = Seq_length(mem->mapped);

        for (int i = 0; i < curr_length; i++) {
                free(get_segment(mem, i));
        }

        Seq_free(&mem->mapped);
        Seq_free(&mem->unmapped);
        free(mem);
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
                *C = 4294967295;
        }
}