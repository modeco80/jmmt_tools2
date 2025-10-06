#pragma once
#include <mco/base_types.hpp>
#include <optional>

namespace jmpak {

	/// This class wraps commands. Rather than using regular vtables, we just have a concrete "Command"
	/// class which has function pointers. A rather C-style interface, but it works for what we need.
	class Command {
		Command* pNext;

		// data
		char cmd;
		void (*helpImpl)();
		i32 (*runImpl)(int argc, char** argv);

	   public:
		static std::optional<Command*> find(char cmd);

		static void forEachImpl(bool (*pfn)(Command* command, void* user), void* user);

		template <class F>
		static void forEach(F&& fn) {
			struct CbState {
				F fn;
			} state = { static_cast<F&&>(fn) };

			forEachImpl([](Command* command, void* user) {
				return reinterpret_cast<CbState*>(user)->fn(command);
			},
						&state);
		}

		Command(char cmd, void (*helpFn)(), i32 (*runFn)(int argc, char** argv));

		void help() {
			return helpImpl();
		}

		i32 run(int argc, char** argv) {
			return runImpl(argc, argv);
		}
	};

} // namespace jmpak
