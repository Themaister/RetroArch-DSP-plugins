#ifndef TIMER_HPP__

#include <time.h>

class Timer
{
   public:
      Timer() : frames(0), cycles(0) {}
      void start()
      {
         last_cycles = current_time();
      }

      void stop(unsigned processed)
      {
         cycles += current_time() - last_cycles;
         frames += processed;
         frames_cnt += processed;

         if (frames_cnt > 100000)
         {
            std::cerr << "Cycles / frame => " << cycles / frames << std::endl;
            frames_cnt = 0;
         }
      }

   private:
      uint64_t current_time() // Unsafe, but good enough for our purposes ... ;)
      {
         uint32_t lo, hi;
         asm volatile (
               "xorl %%eax, %%eax\n\t"
               "cpuid\n\t"
               ::: "%eax", "%ebx", "%ecx", "%edx"); // Serialize pipelines.

         asm volatile ( "rdtsc\n\t" : "=a"(lo), "=d"(hi) );
         return ((uint64_t)hi << 32) | lo;
      }

      unsigned frames;
      unsigned frames_cnt;
      uint64_t cycles;
      double last_cycles;
};


#endif

