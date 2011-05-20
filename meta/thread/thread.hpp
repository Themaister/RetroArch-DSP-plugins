#ifndef __THREAD_HPP
#define __THREAD_HPP

#include <pthread.h>
#include <memory>
#include <utility>

// std::thread isn't ... well functioning yet, 
// so let's make a mock one for now.

namespace Internal
{
   class Callable
   {
      public:
         virtual void run() = 0;
         virtual ~Callable() {}
   };

   extern "C"
   {
      void* _Entry(void *data); // pthread hook
   }
}

class Thread
{
   public:
      Thread() : joinable(false) {}

      template <class T>
      Thread(T fn) : joinable(true)
      {
         callptr = std::make_shared<fn_obj<T>>(fn);
         start();
      }

      // Cannot copy a thread.
      void operator=(const Thread&) = delete;
      Thread(const Thread&) = delete; 

      // Moving on the other hand ...
      Thread(Thread&& in)
      {
         *this = std::move(in);
      }

      Thread& operator=(Thread&& in)
      {
         join();
         m_id = in.m_id;
         joinable = in.joinable;

         callptr = in.callptr;
         in.joinable = false;

         return *this;
      }

      ~Thread()
      {
         join();
      }

      void join()
      {
         if (joinable)
         {
            pthread_join(m_id, NULL);
            joinable = false;
         }
      }

   private:
      pthread_t m_id;
      bool joinable;

      void start()
      {
         if (pthread_create(&m_id, NULL, Internal::_Entry, reinterpret_cast<void*>(&callptr)) < 0)
            joinable = false;
      }

      std::shared_ptr<Internal::Callable> callptr;

      // Function object stuff.
      template <class P>
      class fn_obj : public Internal::Callable
      {
         public:
            fn_obj(P& p) : _p(p) {}
            void run() { _p(); }

         private:
            P _p;
      };
};


#endif
