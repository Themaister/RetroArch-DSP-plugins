#include "../../ssnes_dsp.h"
#include "../utils.h"
#include <unistd.h>
#include <sys/poll.h>
#include <assert.h>
#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
   assert(argc == 2);

   void *lib = dlopen(argv[1], RTLD_LAZY);
   assert(lib);
   auto plug_init = (const ssnes_dsp_plugin_t *(*)(void))dlsym(lib, "ssnes_dsp_plugin_init");
   assert(plug_init);

   auto driver = plug_init();
   assert(driver);

   ssnes_dsp_info_t info = {
      44100.0,
      44100.0
   };

   auto plug = driver->init(&info);
   assert(plug);

   if (driver->config)
      driver->config(plug);

   int16_t buf[2048];
   float fbuf[512];

   for (;;)
   {
      struct pollfd fds[2];
      memset(fds, 0, sizeof(fds));
      fds[0].fd = 0;
      fds[1].fd = 1;
      fds[0].events = POLLIN;
      fds[1].events = POLLOUT;

      if (poll(fds, 2, 10) < 0)
      {
         perror("poll");
         break;
      }

      if (fds[0].revents & POLLHUP)
         break;
      if (fds[1].revents & POLLHUP)
         break;

      if (!((fds[0].revents & POLLIN) && (fds[1].revents && POLLOUT)))
      {
         if (driver->events)
            driver->events(plug);
         continue;
      }

      if (read(0, buf, 1024) < 1024)
         break;

      for (unsigned i = 0; i < 512; i++)
         fbuf[i] = (float)buf[i] / 0x8000;

      ssnes_dsp_output_t output;
      ssnes_dsp_input_t input = { fbuf, 256 };

      driver->process(plug, &output, &input);

      if (driver->events)
         driver->events(plug);

      assert(output.frames <= 1024);

      audio_convert_float_to_s16(buf, output.samples, output.frames * 2);

      if (write(1, buf, sizeof(int16_t) * output.frames * 2) < (sizeof(int16_t) * output.frames * 2))
         break;
   }

   driver->free(plug);
   dlclose(lib);
}
