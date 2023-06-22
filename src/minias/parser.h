#ifndef MINIAS_PARSER_H
#   define MINIAS_PARSER_H

#   include <stddef.h>
#   include <miniisa/bytecode.h>
#   include <miniisa/instruction.h>
#   include "token.h"

#   define PARSER_DATA_MAX_LEN (1024)

typedef enum parser_state {
    /**
     * state of the parser upon initialization
     * ---------------------------------------
     * leads to PARSER_GETTING_INITIAL
     */
    PARSER_INIT = 0,

    /**
     * indicates parser has a finished parsing a statement and terminating here
     * would not result in a syntax error
     * ----------------------------------
     * leads to PARSER_GETTING_INITIAL
     *
     * int parser_terminate(parser_t *p) can only be called when the parser is
     * in this state
     */
    PARSER_DONE = 1,

    /**
     * searching for first part in a statement of code, such as an instruction
     * mnemonic or section directive, before handing it off to their respective
     * states
     * ------
     * leads to PARSER_DETECTING_TYPE if parser is believes this statement
     * is the start of a symbol, data or instruction
     *
     * leads to PARSER_NEEDING_SECTION_NAME if parser believes this statement
     * is the a directive and is looking for the name of the section
     * after the section keyword
     */
    PARSER_GETTING_INITIAL = 2,

    /**
     * detect whether current statement is an instruction, symbol or data
     * ------------------------------------------------------------------
     * if current token matches one of the instruction mnemonics, goto
     * PARSER_FINDING_ARGUMENT
     * if current token is a COLON_TOKEN, this is a symbol, go back to
     * PARSER_DONE
     * if current token is 'data' identifier, goto PARSER_SETTING_DATA
     */
    PARSER_DETECTING_TYPE = 3,

    /**
     * set data for 'data' identifier
     * ------------------------------
     * go to PARSER_DEMANDING_SIZE_COMMA after detecting valid data
     */
    PARSER_SETTING_DATA = 4,

    /**
     * look for argument in instruction
     * --------------------------------
     * must be register, integer or float, go to PARSER_AWAITING_ARG_COMMA
     */
    PARSER_FINDING_ARGUMENT = 5,

    /**
     * awaiting comma after an argument
     * --------------------------------
     * go to PARSER_FINDING_ARGUMENT if comma detected
     * otherwise, if new line detected, go to PARSER_DONE
     */
    PARSER_AWAITING_ARG_COMMA = 6,

    /**
     * get section name after 'section' keyword
     * ----------------------------------------
     * go back to PARSER_DONE when identifier detected
     */
    PARSER_NEEDING_SECTION_NAME = 7,

    /**
     * idk what this is for
     * --------------------
     * goes back to PARSER_DONE when COLON_TOKEN detected
     */
    PARSER_EXPECTING_COLON = 8,

    /**
     * wait for comma after data
     * -------------------------
     * go to PARSER_REQUIRING_SIZE
     */
    PARSER_DEMANDING_SIZE_COMMA = 9,

    /**
     * data type size
     * --------------
     * go back to PARSER_DONE
     */
    PARSER_REQUIRING_SIZE = 10
} parser_state_t;

typedef struct parser {
    token_t prev_token;
    parser_state_t state;
    miniisa_instruction_t instruction;
    uint8_t data[PARSER_DATA_MAX_LEN];
    size_t data_len;
} parser_t;

parser_t *init_parser(parser_t *p);

int parse_one_token(parser_t *p, token_t *t, miniisa_bytecode_t *b);

#endif
