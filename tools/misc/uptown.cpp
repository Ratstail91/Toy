//UPTOWN: Utility to Prepend Text Of Whatever Next

#include <iostream>
#include <fstream>
#include <string>
#include <map>

//protip: This gets more efficient the more files passed in
//protip: pass in the headers first

bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		return -1;
	}

	//cache replacements for all files
	std::map<std::string, std::string> replacements;

	for (int fileCounter = 1; fileCounter < argc; fileCounter++) {
		std::ifstream is; //input stream
		std::string buffer; //store file

		//open
		is.open(argv[fileCounter]);
		if (!is.is_open()) {
			return -1;
		}

		while (!is.eof()) {
			char c = is.peek();

			//skip comments (make it easier)
			if (c == '/') {
				buffer += is.get();

				//single lines
				if (is.peek() == '/') {
					do {
						buffer += is.get();
					} while(is.peek() != '\n');
					continue;
				}

				//multi-line
				if (is.peek() == '*') {
					while (true) {
						do {
							buffer += is.get();
						} while(is.peek() != '*');

						if (is.eof()) { //just in case
							return -1;
						} else {
							buffer += is.get();
							if (is.peek() == '/') {
								buffer += is.get();
								break;
							}
						}
					}
					continue;
				}
			}

			//skip strings (causes issues)
			if (c == '"') {
				do {
					buffer += is.get();
				} while (is.peek() != '"');
				buffer += is.get();
				continue;
			}

			//skip non-words
			if (!is_alpha(c)) {
				buffer += is.get();
				continue;
			}

			//read word
			std::string word = "";
			while(is_alpha(is.peek())) {
				word += is.get();
			}

			//get replacement word, if it doesn't exist
			if (replacements.find(word) == replacements.end()) {
				std::cout << word << " : ";
				std::string prepend = "";

				getline(std::cin, prepend);

				if (prepend.length() == 0) {
					replacements[word] = word;
				}
				else {
					replacements[word] = prepend + word;
				}
			}

			//append the replacement
			buffer += replacements[word];
		}

		//finally
		is.close();

		std::ofstream os;

		os.open(argv[fileCounter]);
		if (!os.is_open()) {
			return -1;
		}

		//bugfix
		if (!buffer.empty()) {
			buffer.pop_back();
		}

		os << buffer;

		os.close();
	}

	return 0;
}