#ifndef __LIBRARY_HPP
#define __LIBRARY_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <utility>
#include <stddef.h>
#include <string.h>

class Library
{
   public:
      Library() : library_handle(nullptr) {}

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
         in.library_handle = nullptr;
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
            static_assert(sizeof(T) == sizeof(void*), "Function pointers cannot be cast from void* o.O");
            T ptr;
            void *symbol = dlsym(library_handle, sym);
            // Circumvent illegality of function pointer casts from void*. :)
            memcpy(&ptr, &symbol, sizeof(T));
            return ptr;
#endif
         }
         else
            return nullptr;
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
