#include "DynamicLibrary.hpp"
#include "../RuntimeError.hpp"
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace HollowEngine {

DynamicLibrary::~DynamicLibrary() {
	RunGuarded("Game module unload", [this] { UnLoad(); });
}

bool DynamicLibrary::Load(const char *path) {
	if (path == nullptr) {
		std::cerr << "Path not found\n";
		return false;
	}

	handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

	if (handle == nullptr) {
		std::cerr << dlerror() << '\n';
		return false;
	}

	return true;
}

void *DynamicLibrary::GetSymbol(const char *symbol) {
	if (handle == nullptr || symbol == nullptr) {
		return nullptr;
	}

	dlerror();
	void *address = dlsym(handle, symbol);

	const char *error = dlerror();
	if (error != nullptr) {
		std::cerr << error << '\n';
		return nullptr;
	}

	return address;
}

void DynamicLibrary::UnLoad() {
	if (handle && dlclose(std::exchange(handle, nullptr)) != 0) {
		const char *error = dlerror();
		throw std::runtime_error(error ? error : "dlclose failed");
	}
}

} // namespace HollowEngine
