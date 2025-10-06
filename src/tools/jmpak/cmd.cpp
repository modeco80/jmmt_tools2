#include "cmd.hpp"

namespace jmpak {

	static Command* pCommandHead = nullptr;

	void Command::forEachImpl(bool(*pfn)(Command* command, void* user), void* user) {
		if(pCommandHead == nullptr)
			return;

		Command* pIter = pCommandHead;
		while(pIter != nullptr) {
			if(pfn(pIter, user) == false)
				break;
			pIter = pIter->pNext;
		}
	}

	std::optional<Command*> Command::find(char cmd) {
		Command* pCmd = nullptr;
		forEach([&](Command* pThisCommand) {
			if(pThisCommand->cmd == cmd) {
				pCmd = pThisCommand;
				return false;
			}

			return true;
		});

		if(pCmd)
			return pCmd;
		return std::nullopt;
	};

	Command::Command(char cmd, void(*helpFn)(), i32(*runFn)(int argc, char** argv)) {
		// assign things
		this->cmd = cmd;
		this->helpImpl = helpFn;
		this->runImpl = runFn;

		// Add this command to the list.
		if(pCommandHead == nullptr) {
			pCommandHead = this;
		} else {
			Command* pIter = pCommandHead;
			while(pIter->pNext != nullptr) {
				pIter = pIter->pNext;
			}
			pIter->pNext = this;
		}
	}

}
