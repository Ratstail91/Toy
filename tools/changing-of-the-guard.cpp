//changing of the guard - made for @hyperiondev's microcontrollers
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

std::string convert(std::string str) {
	str = str.substr(str.find_last_of("\\/")+1);
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	std::replace(str.begin(), str.end(), '.', '_');
	return str;
}

std::string convertToGuardStart(std::string str) {
	str = convert(str);
	return "#ifndef " + str + "\n#define " + str + "\n";
}

std::string convertToGuardEnd(std::string str) {
	str = convert(str);
	return "\n#endif //" + str + "\n";
}

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
			std::string top; //I dislike C++
			getline(is, top);

			//check for pragma guard
			if (top == "#pragma once") {
				top = convertToGuardStart(argv[fileCounter]);
				getline(is, buffer, '\0');
				buffer += convertToGuardEnd(argv[fileCounter]);
			}
			else {
				top += "\n";
				getline(is, buffer, '\0');
			}

			buffer = top + buffer;
		}

		//finally
		is.close();

		std::ofstream os;

		os.open(argv[fileCounter]);
		if (!os.is_open()) {
			return -1;
		}

		os << buffer;

		os.close();
	}

	return 0;
}
