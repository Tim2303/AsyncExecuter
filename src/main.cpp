/*
 * Main project file
 */

#include <Windows.h>

#include <iostream>
#include <chrono>

#include "async_executer.h"

class MyClass
{
private:
  int b;

public:
  MyClass( void ) = default;

  void func( int A )
  {
    b = A;
    Sleep(100);
  }
}; /* End of 'MyClass' class */

int main( void )
{
  async_executer AExec;

  constexpr int ArrSize = 10;
  MyClass Arr[ArrSize];

  int RandomArr[ArrSize];
  for (int i = 0; i < ArrSize; i++)
    RandomArr[i] = rand();

  // Time for async executing
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < ArrSize; i++)
    AExec.AddExecuteTask(i, &MyClass::func, &Arr[i], RandomArr[i]);
  AExec.WaitForExecuting();
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "Async time (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>
    (end - start).count() << std::endl;

  // Time for simple executing
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < ArrSize; i++)
    Arr[i].func(RandomArr[i]);
  end = std::chrono::high_resolution_clock::now();

  std::cout << "Time (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>
    (end - start).count() << std::endl;

  return 0;
}
