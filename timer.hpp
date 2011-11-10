#ifndef TIMER_HPP__

#include <time.h>

class Timer
{
   public:
      Timer() : frames(0), time(0.0) {}
      void start()
      {
         last_time = current_time();
      }

      void stop(unsigned processed)
      {
         time += current_time() - last_time;
         frames += processed;
         frames_cnt += processed;

         if (frames_cnt > 100000)
         {
            std::cerr << "frame / s => " << frames / time << std::endl;
            frames_cnt = 0;
         }
      }

   private:
      double current_time()
      {
         struct timespec tv;
         clock_gettime(CLOCK_MONOTONIC, &tv);
         return tv.tv_sec + tv.tv_nsec / 1000000000.0;
      }

      unsigned frames;
      unsigned frames_cnt;
      double time;
      double last_time;
};

#endif

