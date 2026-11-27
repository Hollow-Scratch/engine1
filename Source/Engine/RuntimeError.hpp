#pragma once

#include <cstdio>
#include <exception>

namespace HollowEngine {

template <typename Action>
bool RunGuarded(const char *operation, Action &&action) noexcept {
	try {
		action();
		return true;
	} catch (const std::exception &error) {
		std::fprintf(stderr, "%s failed: %s\n", operation, error.what());
	} catch (...) {
		std::fprintf(stderr, "%s failed: unknown exception\n", operation);
	}
	return false;
}

} // namespace HollowEngine
