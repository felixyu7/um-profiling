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
#include <bitpack.h>
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
void run_instruction(UM machine, uint32_t inst, bool *status);
bool valid_extension(char *);

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
        UM machine;
        machine = UM_init(source);
        UM_execute(machine);
        UM_free(machine);
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

        /* while halt has not been called */
        while (status == false) {
                curr = get_word(machine->mem, 0, machine->prog_counter);
                run_instruction(machine, curr, &status);
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
void run_instruction(UM machine, uint32_t inst, bool *status)
{
        uint32_t opcode = Bitpack_getu(inst, 4, 28);
        uint32_t A_index = Bitpack_getu(inst, 3, 6);
        uint32_t B_index = Bitpack_getu(inst, 3, 3);
        uint32_t C_index = Bitpack_getu(inst, 3, 0);
        uint32_t *A = &((machine->registers)[A_index]);
        uint32_t *B = &((machine->registers)[B_index]);
        uint32_t *C = &((machine->registers)[C_index]);
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