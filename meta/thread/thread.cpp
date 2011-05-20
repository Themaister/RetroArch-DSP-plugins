#include "thread.hpp"

namespace Internal
{
   void* _Entry(void *data)
   {
      std::shared_ptr<Internal::Callable> *ptr = reinterpret_cast<std::shared_ptr<Internal::Callable>*>(data);

      (*ptr)->run();
   }
}
