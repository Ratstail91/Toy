#include <stdio.h>
#include <string.h>

int getFilePath(char* dest, const char* src) {
	//extract the directory from src, and store it in dest
#if defined(_WIN32) || defined(_WIN64)
	char* p = strrchr(src, '\\');
#else
	char* p = strrchr(src, '/');
#endif

	int len = p != NULL ? p - src + 1 : 0;
	strncpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

int getFileName(char* dest, const char* src) {
	//extract the directory from src, and store it in dest
#if defined(_WIN32) || defined(_WIN64)
	char* p = strrchr(src, '\\');
#else
	char* p = strrchr(src, '/');
#endif

	//if we're not at the end of the string, skip the slash
	if (*p != '\0') {
		p++;
	}

	int len = strlen(p);
	strncpy(dest, p, len);
	dest[len] = '\0';

	return len;
}

#define APPEND(dest, src) \
	strncpy((dest) + (strlen(dest)), (src), strlen((src)) + 1);

#if defined(_WIN32) || defined(_WIN64)
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '/' ? '\\' : str[i];
#else
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '\\' ? '/' : str[i];
#endif

int main() {
	//check the platform
	printf("Platform: ");
#if defined(__linux__)
	printf("Linux");
#elif defined(_WIN64)
	printf("Win64");
#elif defined(_WIN32)
	printf("Win32");
#elif defined(__APPLE__)
	printf("macOS");
#else
	printf("Unknown");
#endif

	printf("\n");

	//run each test
    {
        char src[256] = "../folder/file.txt";
        char dest[256];
		FLIPSLASH(src);
        getFilePath(dest, src);
        printf("Path: %s\n", dest);
    }
    
    {
        char src[256] = "../folder/file.txt";
        char dest[256];
		FLIPSLASH(src);
        getFileName(dest, src);
        printf("Name: %s\n", dest);
    }
    
    {
        char src[256] = "../folder/file.txt";
        char dest[256];
		FLIPSLASH(src);
        getFilePath(dest, src);
        APPEND(dest, "target.txt");
        FLIPSLASH(dest);
        printf("Target: %s\n", dest);
    }

    return 0;
}