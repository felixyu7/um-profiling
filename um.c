/* 
 * um.c
 * COMP40
 * Fall 2019
 *
 * Assignment: HW6, Universal Virtual Machine
 * Main driver program of the UM implementation. Responsible for reading
 * input, calling instructions, and running the program loop. 
 * 
 * Created by Felix J. Yu (fyu04), Micomyiza Theogene (tmicom1)
 * November 17, 2019
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "seq.h"
#include "mem.h"
#include "assert.h"
#include "instructions.h"

typedef struct UM {
        uint32_t *registers;
        Main_memory mem;
        uint32_t prog_counter;
} *UM;


void read_file(FILE *source, UM machine);
UM UM_init(FILE *source);
void execute_inst(UM machine);
void UM_free(UM machine);
void UM_execute(UM machine);
void run_instruction(UM machine, uint32_t inst, bool *status, uint32_t **segment);
bool valid_extension(char *);

bool Bitpack_fitsu(uint64_t n, unsigned width);
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value);

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
        Main_memory mem = init_memory();

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

        uint32_t *segment = get_segment(mem, 0);
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
                                /* unmap, output, input only use 1 register */
                                C_index = Bitpack_getu(inst, 3, 0);
                                C = &((registers)[C_index]);
                        }
                        else if (opcode == 8 || opcode == 12) { 
                                /* map and load_program only use 2 registers */
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

                                /*this retrieves word stored at $m[$r[B]][$r[C]]*/
                                uint32_t word = get_word(mem, *B, *C);

                                /*the set $r[A] receive what returned above*/
                                *A = word;
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

                                /*C can't be 0 because can't divide any number by 0*/
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

                                /*creates new segment with size equal to $r[C] and initializes
                                        each word to 0*/
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
                                        /*register C is loaded with a full 32-bit word in which
                                        every bit is 1*/
                                        *C = 4294967295;
                                }
                                break;
                        case 12:
                                /*pointers shall not be NULL*/
                                assert(B != NULL);
                                assert(C != NULL);

                                /* set new 0 segment to register B */
                                new_program(mem, *B);
                                /* -1 because it iterates again upon return */
                                prog_counter = *C - 1;
                                segment = get_segment(mem, 0);
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
                                fprintf(stderr, "Invalid instruction. opcode %d\n", 
                                        opcode);
                                exit(EXIT_FAILURE);
                                break;
                }

                prog_counter++;
        }

        /* UM_free */
        delete_mem(mem);
        FREE(registers);
        fclose(source);
	exit(EXIT_SUCCESS);
}


/*
  Parameters: file stream of the input file, universal machine representation
  Returns: none

  Does: reads the inputted file, and extracts all the instructions it contains
  and inputs them into the 0 segment
*/
void read_file(FILE *source, UM machine) {
        assert(source != NULL);

        /* seek the end of file to get file size */
        fseek(source, 0L, SEEK_END);
        int size = ftell(source);
        rewind(source);

        uint32_t word = 0;
        unsigned char c;

        /* should be assigned the 0 address */
        int zero = map(machine->mem, size);
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

                put_word(machine->mem, 0, i, word); /* put in 0 segment */
        }
}

/*
  Parameters: file stream of the input file
  Returns: universal machine representation

  Does: initializes a new UM, with the given program
*/
UM UM_init(FILE *source) {

        UM machine = malloc(sizeof(struct UM));
        machine->mem = init_memory();
        /* load file */
        read_file(source, machine);

        machine->registers = calloc(8, sizeof(uint32_t));

        machine->prog_counter = 0;
        return machine;
}

/*
  Parameters: universal machine representation
  Returns: none

  Does: executes the program currently loaded in the UM. It will run through
  all instructions until the program is instructed to halt.
*/
void UM_execute(UM machine) 
{
        uint32_t curr;
        bool status = false;

        uint32_t *segment = get_segment(machine->mem, 0);
        /* while halt has not been called */
        while (status == false) {
                curr = segment[machine->prog_counter + 1];
                run_instruction(machine, curr, &status, &segment);
                machine->prog_counter++;
        }
}

/*
  Parameters: universal machine representation, current instruction, 
  status of the program running or not
  Returns: none

  Does: Calls the associated instruction on the UM. If the program is given
  an instruction that UM does not recognize, then this function WILL EXIT the
  entire run.
*/
void run_instruction(UM machine, uint32_t inst, bool *status, uint32_t **segment)
{
        (void)segment;
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
                        /* unmap, output, input only use 1 register */
                        C_index = Bitpack_getu(inst, 3, 0);
                        C = &((machine->registers)[C_index]);
                }
                else if (opcode == 8 || opcode == 12) { 
                        /* map and load_program only use 2 registers */
                        B_index = Bitpack_getu(inst, 3, 3);
                        C_index = Bitpack_getu(inst, 3, 0);
                        B = &((machine->registers)[B_index]);
                        C = &((machine->registers)[C_index]);
                }
                else {
                        A_index = Bitpack_getu(inst, 3, 6);
                        B_index = Bitpack_getu(inst, 3, 3);
                        C_index = Bitpack_getu(inst, 3, 0);
                        A = &((machine->registers)[A_index]);
                        B = &((machine->registers)[B_index]);
                        C = &((machine->registers)[C_index]);
                }
        }
        uint32_t value;

        switch (opcode) {
                case 0:
                        conditional_move(A, B, C);
                        break;
                case 1:
                        segmented_load(machine->mem, A, B, C);
                        break;
                case 2:
                        segmented_store(machine->mem, A, B, C);
                        break;
                case 3:
                        addition(A, B, C);
                        break;
                case 4:
                        multiplication(A, B, C);
                        break;
                case 5:
                        division(A, B, C);
                        break;
                case 6:
                        bitwise_NAND(A, B, C);
                        break;
                case 7:
                        halt(status);
                        break;
                case 8:
                        map_segment(machine->mem, C, B);
                        break;
                case 9:
                        unmap_segment(machine->mem, C);
                        break;
                case 10:
                        output(C, stdout);
                        break;
                case 11:
                        input(C, stdin);
                        break;
                case 12:
                        load_program(machine->mem, B, C);
                        /* -1 because it iterates again upon return */
                        machine->prog_counter = *C - 1;
                        *segment = get_segment(machine->mem, 0);
                        break;
                case 13:
                        /* special case, re bitpack */
                        A_index = Bitpack_getu(inst, 3, 25);
                        A = &((machine->registers)[A_index]);
                        value = Bitpack_getu(inst, 25, 0);
                        load_value(A, value);
                        break;
                default:
                        fprintf(stderr, "Invalid instruction. opcode %d\n", 
                                opcode);
                        exit(EXIT_FAILURE);
                        break;
        }
}

/*
  Parameters: universal machine representation
  Returns: none

  Does: frees all memory associated with the UM
*/
void UM_free(UM machine) {
        delete_mem(machine->mem);
        FREE(machine->registers);
        FREE(machine);
}

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