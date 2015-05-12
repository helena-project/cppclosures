//Option 2: std::function
// std::function by default copies the lambda into dynamically allocated
// memory, but it can be told to rather use a reference to the lambda.
// this is a little bit like using a pointer to the lambda directly
// except that the type is known so you can declare variables to store
// it in. (The type of lambdas is unknown unless it does not close any
// variables, then it can be cast to a function pointer)

#include <cstdio>
#include <functional>
#include <string.h>

///////////////////////////////////////////////////////
/// TASK QUEUE SUPPORT
///////////////////////////////////////////////////////
typedef struct
{
  uint8_t posted;
  std::function<void()> callback;
} task_t;

// All the closure symbols are inspectable in the ELF, we can use
// a two-pass compile to statically size this correctly.
task_t task_queue [10];

void post (std::function<void()> callback, int index)
{
  task_queue[index].posted = 1;
  //This too can be replaced at compile time because it is always
  //the same value. Probably just inject the symbol value into the
  //ELF
  task_queue[index].callback = callback;
}
void run_taskq()
{
  for (int i=0;i<10;i++)
  {
    if (task_queue[i].posted)
    {
      task_queue[i].posted = 0;
      task_queue[i].callback();
    }
  }
}

#define POSTFUN(task) \
  post(task, __COUNTER__)

// Convert a lambda into a global lifetime std::function
// this (ab)uses a property of the lambdas: every lambda is a unique
// type. Hence every time this function is called, it is a unique
// template instantiation so creates a unique static memory allocation.
template <typename F, typename T>
std::function<F> to_static(T&& func)
{
  static uint8_t sbuf [sizeof(func)];
  memcpy(sbuf, &func, sizeof(func));
  return std::function<F>( std::ref((*((decltype(&func))(sbuf)))));
}

/////////////////////////////////////////////////////////
/// MAIN EXAMPLE
/////////////////////////////////////////////////////////

void call_sync(std::function<void()> func)
{
  func();
}
void funcb_2(std::function<void()> func)
{
  POSTFUN(func);
}
void funcb_1(std::function<void()> func)
{
  funcb_2(func);
}

void example(int test)
{
  int foobar = 5;
  auto printhi = [=]() {
    printf("hi %d, %d\n", test, foobar);
  };

  //We can pass stack allocated lambdas as std::function with no allocs
  //if we use ref. Naturally if this stack frame dies then this reference
  //is bad.
  call_sync(std::ref(printhi));

  //We can also fairly trivially define a function that takes a stack
  //allocated lambda, and turns it into a std::function backed by unique
  //static memory.
  std::function<void()> myfunc = to_static<void()> (printhi);
  // This could also have been:
  auto myfunc2 = to_static<void()> (printhi);

  //With _this_ std::function, we can call async functions
  funcb_1(myfunc);

  //We can also do this with large closures without dynamic memory alloc
  uint8_t bigarray [100];
  bigarray[0] = 4;
  bigarray[1] = 10;
  auto close_big_var = [=]() {
    printf("Close a large array by value:\n");
    printf("some values: %d %d\n",bigarray[0], bigarray[1]);
    printf("Also close a closure: \n");
    printhi();
  };
  POSTFUN(to_static<void()> (close_big_var));
}

int main ()
{
  printf("run with 5:\n");
  example(5);
  run_taskq();
  printf("run with 6:\n");
  example(6);
  run_taskq();
}
