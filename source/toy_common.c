#include "toy_common.h"

//defined separately, as compilation can take several seconds, invalidating the comparisons of __TIME__
static const char* build = __DATE__ " " __TIME__ ";development branch";

const char* Toy_private_version_build() {
	return build;
}
