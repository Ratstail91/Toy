#pragma once

#include "common.h"

//for processing the command line arguments
typedef struct {
	bool error;
	bool help;
	bool version;
	char* filename;
	char* source;
	bool verbose;
	int optimize;
} Command;

extern Command command;

void initCommand(int argc, const char* argv[]);

void usageCommand(int argc, const char* argv[]);
void helpCommand(int argc, const char* argv[]);
void copyrightCommand(int argc, const char* argv[]);

void dissectBytecode(const char* tb, int size);