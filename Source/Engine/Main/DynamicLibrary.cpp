#include "DynamicLibrary.hpp"
#include <dlfcn.h>
#include <iostream>

namespace HollowEngine {

DynamicLibrary::~DynamicLibrary() { UnLoad(); }

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
	if (handle != nullptr) {
		dlclose(handle);
		handle = nullptr;
	}
}

} // namespace HollowEngine
