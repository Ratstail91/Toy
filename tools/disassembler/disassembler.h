/*
 * disassembler.c
 *
 *  Created on: 10 ago. 2023
 *      Original Author: Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 *
 * Further modified by Kayne Ruse, and added to the Toy Programming Language tool repository.
 */

#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

typedef struct options_s {
    bool alt_format_flag;
    bool group_flag;
} options_t;

typedef enum DIS_OPCODES {
    DIS_OP_EOF,                        //

    // do nothing
    DIS_OP_PASS,                       //

    // basic statements
    DIS_OP_ASSERT,                     //
    DIS_OP_PRINT,                      //

    // data
    DIS_OP_LITERAL,                    //
    DIS_OP_LITERAL_LONG,               //
    DIS_OP_LITERAL_RAW,                //

    // arithmetic operators
    DIS_OP_NEGATE,                     //
    DIS_OP_ADDITION,                   //
    DIS_OP_SUBTRACTION,                //
    DIS_OP_MULTIPLICATION,             //
    DIS_OP_DIVISION,                   //
    DIS_OP_MODULO,                     //
    DIS_OP_GROUPING_BEGIN,             //
    DIS_OP_GROUPING_END,               //

    // variable stuff
    DIS_OP_SCOPE_BEGIN,                //
    DIS_OP_SCOPE_END,                  //

    DIS_OP_TYPE_DECL_removed,          // deprecated
    DIS_OP_TYPE_DECL_LONG_removed,     // deprecated

    DIS_OP_VAR_DECL,                   //
    DIS_OP_VAR_DECL_LONG,              //

    DIS_OP_FN_DECL,                    //
    DIS_OP_FN_DECL_LONG,               //

    DIS_OP_VAR_ASSIGN,                 //
    DIS_OP_VAR_ADDITION_ASSIGN,        //
    DIS_OP_VAR_SUBTRACTION_ASSIGN,     //
    DIS_OP_VAR_MULTIPLICATION_ASSIGN,  //
    DIS_OP_VAR_DIVISION_ASSIGN,        //
    DIS_OP_VAR_MODULO_ASSIGN,          //

    DIS_OP_TYPE_CAST,                  //
    DIS_OP_TYPE_OF,                    //

    DIS_OP_IMPORT,                     //
    DIS_OP_EXPORT_removed,             // deprecated

    // for indexing
    DIS_OP_INDEX,                      //
    DIS_OP_INDEX_ASSIGN,               //
    DIS_OP_INDEX_ASSIGN_INTERMEDIATE,  //
    DIS_OP_DOT,                        //

    // comparison of values
    DIS_OP_COMPARE_EQUAL,              //
    DIS_OP_COMPARE_NOT_EQUAL,          //
    DIS_OP_COMPARE_LESS,               //
    DIS_OP_COMPARE_LESS_EQUAL,         //
    DIS_OP_COMPARE_GREATER,            //
    DIS_OP_COMPARE_GREATER_EQUAL,      //
    DIS_OP_INVERT,                     //

    // logical operators
    DIS_OP_AND,                        //
    DIS_OP_OR,                         //

    // jumps, and conditional jumps (absolute)
    DIS_OP_JUMP,                       //
    DIS_OP_IF_FALSE_JUMP,              //
    DIS_OP_FN_CALL,                    //
    DIS_OP_FN_RETURN,                  //

    // pop the stack at the end of a complex statement
    DIS_OP_POP_STACK,                  //

    //ternary shorthand
    DIS_OP_TERNARY,                    //

    //meta
    DIS_OP_FN_END,                     // different from SECTION_END
    DIS_OP_END_OPCODES,                // mark for end opcodes list. Not valid opcode
    DIS_OP_SECTION_END = 255,
} dis_opcode_t;

typedef enum DIS_LITERAL_TYPE {
    DIS_LITERAL_NULL,       //
    DIS_LITERAL_BOOLEAN,    //
    DIS_LITERAL_INTEGER,    //
    DIS_LITERAL_FLOAT,      //
    DIS_LITERAL_STRING,     //
    DIS_LITERAL_ARRAY,      //
    DIS_LITERAL_DICTIONARY, //
    DIS_LITERAL_FUNCTION,   //
    DIS_LITERAL_IDENTIFIER, //
    DIS_LITERAL_TYPE,       //
    DIS_LITERAL_OPAQUE,     //
    DIS_LITERAL_ANY,        //

    // these are meta-level types - not for general use
    DIS_LITERAL_TYPE_INTERMEDIATE,       // used to process types in the compiler only
    DIS_LITERAL_ARRAY_INTERMEDIATE,      // used to process arrays in the compiler only
    DIS_LITERAL_DICTIONARY_INTERMEDIATE, // used to process dictionaries in the compiler only
    DIS_LITERAL_FUNCTION_INTERMEDIATE,   // used to process functions in the compiler only
    DIS_LITERAL_FUNCTION_ARG_REST,       // used to process function rest parameters only
    DIS_LITERAL_FUNCTION_NATIVE,         // for handling native functions only
    DIS_LITERAL_FUNCTION_HOOK,           // for handling hook functions within literals only
    DIS_LITERAL_INDEX_BLANK,             // for blank indexing i.e. arr[:]
} dis_literal_type_t;

extern void disassemble(const char *filename, options_t config);

#endif /* DISASSEMBLER_H_ */
