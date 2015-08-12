#ifndef DYNAMIC_LINKING_LOADER_H_
#define DYNAMIC_LINKING_LOADER_H_

#include <dlfcn.h>

#include <string>

/// C++ RAII interface to the Linux Dynamic Linking Loader
/// i.e. dladdr, dlclose, dlerror, dlopen, dlsym, dlvsym
class DynamicLinkingLoader {
 public:
  /// Construct DynamicLinkingLoader to load library_file_name
  DynamicLinkingLoader(const std::string & library_file_name) {
    // Make sure library_file_name is actually treated as a file path
    // by dlopen (instead of searching a bunch of env variables such as
    // DT_RUNPATH, DT_RPATH, and LD_LIBRARY_PATH). Check man dlopen for details.

    // Check if we don't have a '/' in library_file_name. If so, it's already a path.
    // If not, prepend "./" to force it to be treated like a relative path.
    std::string file_name_for_dlopen(library_file_name);
    if (library_file_name.find('/') == std::string::npos) {
      file_name_for_dlopen.insert(file_name_for_dlopen.begin(), {'.', '/'}); 
    }

    // Now call dlopen eagerly (RTLD_NOW)
    handle_ = dlopen(file_name_for_dlopen.c_str(), RTLD_NOW);
    if (handle_ == nullptr) {
      throw std::runtime_error(dlerror());
    }

    // Clear out dlerror()
    dlerror();
  }

  /// Destructor for DynamicLinkingLoader
  ~DynamicLinkingLoader() {
    try {
      std::cerr << "Calling dlclose in DynamicLinkingLoader's destructor" << std::endl;
      int ret = dlclose(handle_);
      if (ret != 0) {
        throw std::runtime_error(dlerror());
      }
      dlerror();
    } catch (const std::exception & e) {
      std::cerr << "Caught exception in destructor " << e.what() << std::endl;
    }
  }

  /// Delete all copy constructors, move constructors, and
  /// copy assignment, move assignment operators
  DynamicLinkingLoader(const DynamicLinkingLoader &) = delete;
  DynamicLinkingLoader(DynamicLinkingLoader &&) = delete;
  DynamicLinkingLoader & operator=(const DynamicLinkingLoader &) = delete;
  DynamicLinkingLoader & operator=(DynamicLinkingLoader &&) = delete;

 private:
  /// Opaque handle to library_file_name
  void * handle_ = nullptr;
};

#endif  // DYNAMIC_LINKING_LOADER_H_