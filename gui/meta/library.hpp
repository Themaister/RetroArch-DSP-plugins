#ifndef __LIBRARY_HPP
#define __LIBRARY_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <utility>
#include <stddef.h>

class Library
{
   public:
      Library() : library_handle(NULL) {}

      Library(const char *path)
      {
#ifdef _WIN32
         library_handle = LoadLibrary(path);
#else
         library_handle = dlopen(path, RTLD_LAZY);
#endif
      }

      void operator=(const Library&) = delete;
      Library(Library&& in)
      {
         *this = std::move(in);
      }

      Library& operator=(Library&& in)
      {
         free_lib();
         library_handle = in.library_handle;
         in.library_handle = NULL;
         return *this;
      }

      operator bool()
      {
         return library_handle;
      }

      ~Library()
      {
         free_lib();
      }

      template <class T>
      T sym(const char *sym)
      {
         if (library_handle)
         {
#ifdef _WIN32
            return reinterpret_cast<T>(GetProcAddress(library_handle, sym));
#else
            return reinterpret_cast<T>(dlsym(library_handle, sym));
#endif
         }
         else
            return NULL;
      }

   private:
#ifdef _WIN32
      HMODULE library_handle;
#else
      void *library_handle;
#endif

      void free_lib()
      {
         if (library_handle)
         {
#ifdef _WIN32
            FreeLibrary(library_handle);
#else
            dlclose(library_handle);
#endif
         }
      }
};

#endif
