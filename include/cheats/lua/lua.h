#pragma once
#include "../../../hooks/hooks.h"

namespace lua {
	struct ExecutionData {
		std::atomic<bool> waiting{false};
		std::string script;
		std::mutex mutex;
	};

	extern ExecutionData executionData;

	void execute();
	void dumper(const std::string& filename, const std::string& string_to_run);
}