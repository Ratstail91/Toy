#pragma once

//general utilities
#include "toy_common.h"
#include "toy_console_colors.h"

//basic structures
#include "toy_value.h"
#include "toy_array.h"
#include "toy_stack.h"
#include "toy_bucket.h"
#include "toy_string.h"
//TODO: hashtable

//IR structures and other components
#include "toy_ast.h"
#include "toy_routine.h"

//pipeline
#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_bytecode.h"
#include "toy_vm.h"
