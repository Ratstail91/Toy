#include <stdlib.h>

#include "cargs.h"
#include "disassembler.h"

static struct cag_option options[] = {
        {
                .identifier = 'a',
                .access_letters = "a",
                .access_name = NULL,
                .value_name = NULL,
                .description = "Alternate format"
        }, {
                .identifier = 'g',
                .access_letters = "g",
                .access_name = NULL,
                .value_name = NULL,
                .description = "Group literals with functions"
        }, {
                .identifier = 'h',
                .access_letters = "h",
                .access_name = "help",
                .description = "Shows the command help"
        }
};

int main(int argc, char *argv[]) {
	char identifier;
	cag_option_context context;
	options_t config = { false, false };

	cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
	while (cag_option_fetch(&context)) {
		identifier = cag_option_get(&context);
		switch (identifier) {
		case 'a':
			config.alt_format_flag = true;
			break;
		case 'g':
		    config.group_flag = true;
		    config.alt_format_flag = true;
		    break;
		case 'h':
			printf("Usage: disassembler [OPTION] file\n");
			cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
			return EXIT_SUCCESS;
		}
	}

	disassemble(argv[context.index], config);

	return EXIT_SUCCESS;
}
