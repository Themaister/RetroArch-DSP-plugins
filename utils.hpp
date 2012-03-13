#ifndef UTILS_HPP__
#define UTILS_HPP__

#include <algorithm>
#include <string>
#include <sstream>
#include <utility>

namespace Utils
{
   template <class T>
   auto begin(T &t) -> decltype(t.begin())
   {
      return t.begin();
   }

   template <class T>
   auto end(T &t) -> decltype(t.end())
   {
      return t.end();
   }

   template <class T, size_t S>
   T* begin(T (&ptr)[S])
   {
      return &ptr[0];
   }

   template <class T, size_t S>
   T* end(T (&ptr)[S])
   {
      return &ptr[S];
   }

   template <class T, size_t S, size_t Z>
   T* begin(T (&ptr)[S][Z])
   {
      return &ptr[0][0];
   }

   template <class T, size_t S, size_t Z>
   T* end(T (&ptr)[S][Z])
   {
      return &ptr[S][0];
   }

   template <class T, class U>
   void set_all(T &t, const U &u)
   {
      std::fill(begin(t), end(t), u);
   }

   template <class T>
   std::string join(T&& t)
   {
      std::ostringstream str;
      str << std::forward<T>(t);
      return str.str();
   }

   template <class T, class... U>
   std::string join(T&& t, U&&... u)
   {
      std::ostringstream str;
      str << std::forward<T>(t) << join(std::forward<U>(u)...);
      return str.str();
   }
}

#endif

