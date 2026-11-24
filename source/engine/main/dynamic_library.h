#ifndef HOLLOWENGINE_DYNAMIC_LIBRARY_H
#define HOLLOWENGINE_DYNAMIC_LIBRARY_H

typedef struct DynamicLibrary {
	void *handle;
} DynamicLibrary;

bool  DynamicLibraryLoad(DynamicLibrary *lib, const char *path);
void *DynamicLibraryGetSymbol(DynamicLibrary *lib, const char *name);
void  DynamicLibraryUnload(DynamicLibrary *lib);

#endif