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
}