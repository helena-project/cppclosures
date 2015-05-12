//This is the lambda-only way of doing task queues. It might be a little more
//efficient, but it is really difficult to work with lambdas directly (you can't
//return them from functions or store them in non-auto variables).

#include <cstdio>
#include <functional>
#include <string.h>

///////////////////////////////////////////////////////
/// TASK QUEUE SUPPORT
///////////////////////////////////////////////////////
typedef void (*callback_t)();
typedef struct
{
  uint8_t posted;
  callback_t callback;
} task_t;

// All the closure symbols are inspectable in the ELF, we can use
// a two-pass compile to statically size this correctly.
task_t task_queue [10];

void post (callback_t callback, int index)
{
  task_queue[index].posted = 1;
  //This too can be replaced at compile time because it is always
  //the same value. Probably just inject the symbol value into the
  //ELF
  task_queue[index].callback = callback;
}
void run_taskq()
{
  bool run;
  do {
    run = false;
    for (int i=0;i<10;i++)
    {
      if (task_queue[i].posted)
      {
        run = true;
        task_queue[i].posted = 0;
        task_queue[i].callback();
      }
    }
  } while (run);
}

#define POSTCLOSURE(task) \
  static uint8_t task ## _environment_buffer [sizeof(task)]; \
  memcpy(task ## _environment_buffer, &task, sizeof(task)); \
  post([]{ \
    (*((decltype(&task))(task ## _environment_buffer)))  (); \
  }, __COUNTER__);

/////////////////////////////////////////////////////////
/// MAIN EXAMPLE
/////////////////////////////////////////////////////////

//Option 1: template functions
// Lambdas are passed by value, so are
// valid even after the calling function returns
template <class T> void func2(T func)
{
  //Defer the closure to be executed later - copy it to static memory.
  //This is now a "post once" task. i.e. func2 must not be called
  //again before this task is invoked.
  POSTCLOSURE(func)
}
template <class T> void func1(T func)
{
  //Do some stuff, pass the closure around
  func2(func);
}

void example(int test)
{
  int foobar = 5;
  auto printhi = [=]() {
    printf("hi %d, %d\n", test, foobar);
  };
  func1(printhi);

  uint8_t bigarray [100];
  bigarray[0] = test;
  bigarray[1] = 10;
  auto close_big_var = [=]() {
    printf("Close a large array by value:\n");
    printf("some values: %d %d\n",bigarray[0], bigarray[1]);
    printf("Also close a closure: \n>  ");
    printhi();
  };
  POSTCLOSURE(close_big_var)
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
