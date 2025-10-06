#include <cstdio>

#include "cmd.hpp"

/// Usage function.
void mainUsage(char* progname) {
	std::printf(
	"jmpak - LibJMMT package tool (c) 2025 modeco80\n"
	"Usage: %s [command] [arguments...]\n"
	"Commands:\n",

	progname);

	// Call help of each command.
	jmpak::Command::forEach([](jmpak::Command* cmd) {
		cmd->help();
		return true;
	});
}

int main(int argc, char** argv) {
	// we need a command to run
	if(argc < 2) {
		mainUsage(argv[0]);
		return 1;
	}

	// Run the command.
	if(auto cmd = jmpak::Command::find(argv[1][0]); cmd.has_value()) {
		auto ret = (*cmd)->run(argc - 2, argv + 2);
		if(ret != 0) {
			mainUsage(argv[0]);
		}
		return ret;
	}

	printf("Error: Unknown command '%c'.\n", argv[1][0]);
	mainUsage(argv[0]);
	return 1;
}
