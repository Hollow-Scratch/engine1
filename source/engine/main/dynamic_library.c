
#include "main/dynamic_library.h"

#include <dlfcn.h>
#include <stddef.h>

bool DynamicLibraryLoad(DynamicLibrary *lib, const char *path) {
	if (lib == NULL || path == NULL) {
		return false;
	}

	lib->handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

	return lib->handle != NULL;
}

void *DynamicLibraryGetSymbol(DynamicLibrary *lib, const char *name) {
	if (lib == NULL || lib->handle == NULL || name == NULL) {
		return NULL;
	}

	dlerror();

	void *symbol = dlsym(lib->handle, name);

	if (dlerror() != NULL) {
		return NULL;
	}

	return symbol;
}

void DynamicLibraryUnload(DynamicLibrary *lib) {
	if (lib == NULL || lib->handle == NULL) {
		return;
	}

	dlclose(lib->handle);
	lib->handle = nullptr;
}
