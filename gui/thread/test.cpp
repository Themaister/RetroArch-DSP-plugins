#include "thread.hpp"
#include <functional>
#include <iostream>

void fn(int a)
{
   std::cout << a << std::endl;
}

void fn2()
{
   std::cout << "Hai 2 u" << std::endl;
}

class Foo
{
   public:
      void foo() const { std::cout << "Foo" << std::endl; }
};

int main()
{
   Foo foo;
   Thread a;
   a = Thread(std::bind(fn, 10));
   a.join();
   a = Thread(fn2);
   a.join();
   a = Thread(std::bind(&Foo::foo, &foo));
   a.join();
}
