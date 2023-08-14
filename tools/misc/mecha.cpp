//MECHA: Markdown Embedded Comment Heuristic Analyzer

/*!
# Handy-Dandy *MECHA* Docs!

The following is the source code for MECHA - a tool for reading in source code, and splicing out the markdown embedded within. It can also spit out chunks of code, though that's a bit uglier.

In theory, this should work correctly on all languages that conform to C-style comments, including Toy.

To build it, simply run `g++ -o mecha mecha.cpp`.

To run the result, you must pass in a series of filenames via the command line.

## License

This tool is considered part of Toy's toolchain, so Toy's zlib license should cover it.

## Header Files

If you're reading this as a comment in the source code, this is what I mean by ugly. Thankfully, this section is outputted correctly in markdown.

!*/

//*!```
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
//!*

/*!```

# Recursive Docs

If you run mecha on it's own source code, it should Just Work, producing a file called mecha_cpp.md
!*/

int main(int argc, char** argv) {
	if (argc <= 1) {
		return -1;
	}

	for (int fileCounter = 1; fileCounter < argc; fileCounter++) {
		std::ifstream is; //input stream
		std::string buffer; //store output file

		//open
		is.open(argv[fileCounter]);
		if (!is.is_open()) {
			return -1;
		}

		while (!is.eof()) {
			//skip until correct characters found
			if (is.get() != '*') {
				continue;
			}

			if (is.get() != '!') {
				continue;
			}

			//found the start of the block - begin reading markdown content
			while (!is.eof()) {
				char c = is.get();

				if (c == '!' && is.peek() == '*') {
					break;
				}

				buffer += c;
			}

			//bugfix
			if (buffer.length() >= 2 && buffer.substr(buffer.length()-2)  == "//") {
				buffer.pop_back();
				buffer.pop_back();
			}
		}

		//finally
		is.close();

		if (buffer.length() == 0) {
			continue;
		}

		std::ofstream os;
		std::string ofname = argv[fileCounter];

		std::replace(ofname.begin(), ofname.end(), '.', '_');
		ofname += ".md";

		os.open(ofname);
		if (!os.is_open()) {
			return -1;
		}

		os << buffer;

		os.close();
	}

	return 0;
}