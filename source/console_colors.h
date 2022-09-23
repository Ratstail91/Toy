#pragma once

//NOTE: you need both font AND background for these to work

//fonts color
#define FONT_BLACK      "\033[30;"
#define FONT_RED        "\033[31;"
#define FONT_GREEN      "\033[32;"
#define FONT_YELLOW     "\033[33;"
#define FONT_BLUE       "\033[34;"
#define FONT_PURPLE     "\033[35;"
#define FONT_DGREEN     "\033[6;"
#define FONT_WHITE      "\033[7;"
#define FONT_CYAN       "\x1b[36m"

//background color
#define BACK_BLACK      "40m"
#define BACK_RED        "41m"
#define BACK_GREEN      "42m"
#define BACK_YELLOW     "43m"
#define BACK_BLUE       "44m"
#define BACK_PURPLE     "45m"
#define BACK_DGREEN     "46m"
#define BACK_WHITE      "47m"

//useful
#define NOTICE FONT_GREEN BACK_BLACK
#define WARN FONT_YELLOW BACK_BLACK   
#define ERROR FONT_RED BACK_BLACK   
#define RESET "\033[0m"
