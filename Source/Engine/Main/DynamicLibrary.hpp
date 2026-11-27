#ifndef HOLLOWENGINE_DYNAMICLIBRARY_HEADER
#define HOLLOWENGINE_DYNAMICLIBRARY_HEADER

namespace HollowEngine {

class DynamicLibrary {
  public:
	DynamicLibrary() = default;
	~DynamicLibrary();

	DynamicLibrary(const DynamicLibrary &) = delete;
	DynamicLibrary &operator=(const DynamicLibrary &) = delete;

	bool Load(const char *path);
	void *GetSymbol(const char *Symbol);
	void UnLoad();

  private:
	void *handle = nullptr;
};
} // namespace HollowEngine

#endif
