#include "Engine.hpp"

#include <cstdio>
#include <cstdlib>
#include <exception>

int main() try {
	return HollowEngine::Engine{}.Run();
} catch (const std::exception &error) {
	std::fprintf(stderr, "Engine entry point failed: %s\n", error.what());
	return EXIT_FAILURE;
} catch (...) {
	std::fputs("Engine entry point failed: unknown exception\n", stderr);
	return EXIT_FAILURE;
}
