#include <cstdio>

// commands
namespace jmpak {
	int commandExtractFile(int argc, char** argv);
	int commandExtractPackage(int argc, char** argv);
	int commandList(int argc, char** argv);
	int commandListPaks();
}

void usage(char* progname) {
	std::printf(
		"jmpak: jmmt package tool\n"
		"Usage: %s [command] [arguments...]\n",
		progname
	);
}

int main(int argc, char** argv) {
	// we need a command to run
	if(argc < 2) {
		usage(argv[0]);
		return 1;
	}

	switch(argv[1][0]) {
		case 'e':
			break;
		case 'x':
			break;
		case 'l':
			return jmpak::commandList(argc-2, argv+2);
		case 'L':
			return jmpak::commandListPaks();
	}

	printf("unimplemented command %c\n", argv[1][0]);
	usage(argv[0]);
	return 1;
}
