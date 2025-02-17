/*
 * Module for asynchronous commands execution
 */

#ifndef __async_executer_h_
#define __async_executer_h_

#include <functional>
#include <thread>
#include <atomic>
#include <map>

/* Async executer class */
template<typename Index = int>
class async_executer
{
public:
  /* Default class constructor */
  async_executer( void ) = default;
  async_executer( const async_executer &A ) = delete;

  /* Async execution status enum */
  enum class status
  {
    AE_STATUS_DONE = 0,
    AE_STATUS_EXECUTING = 1,
    AE_STATUS_ERROR = 2,
  }; /* End of 'status' enum class */

private:

  /* Executing thread class */
  class entry
  {
    friend class async_executer;
  private:
    // Thread for function executing
    std::thread ExecuteThread{};

    // Current thread status
    std::atomic<status> Status = status::AE_STATUS_EXECUTING;

    template<typename Func>
    void Executer( Func FuncPtr )
    {
      try
      {
        FuncPtr();
      } catch (...)
      {
        Status = status::AE_STATUS_ERROR;
      }

      if (Status == status::AE_STATUS_EXECUTING)
        Status = status::AE_STATUS_DONE;
    } /* End of 'Executer' function */

  public:
    entry( void ) = default;

    template<typename Func>
    void Execute( Func FuncPtr )
    {
      ExecuteThread = std::thread(&entry::Executer<Func>, this, FuncPtr);
    } /* End of 'Execute' function */

    status GetStatus( void ) const noexcept
    {
      return Status;
    } /* End of 'GetStatus' function */

  }; /* End of 'entry' class */

  // Execute threads map
  std::map<Index, entry> Executers;

public:

  template<typename Func, typename... Args>
  constexpr void AddExecuteTask( Index Ind, Func FuncPtr, Args&&... Argts )
  {
    if (Executers.find(Ind) != Executers.end())
      return;

    Executers[Ind].Execute(std::bind(FuncPtr, Argts...));
  } /* End of 'AddExecuteTask' function */

  void WaitForExecuting( void ) noexcept
  {
    bool IsInProgress = true;

    while (IsInProgress)
    {
      IsInProgress = false;
      for (const auto& Exe : Executers)
        if (Exe.second.GetStatus() == status::AE_STATUS_EXECUTING)
          IsInProgress = true;
    }

    for (auto &exec : Executers)
      if (exec.second.ExecuteThread.joinable())
        exec.second.ExecuteThread.join();

    Executers.clear();
  } /* End of 'WaitForExecuting' function */

  status GetThreadStatus( Index Ind ) const noexcept
  {
    return Executers[Ind].second.GetStatus();
  } /* End of 'GetThreadStatus' function */

  /* Returns 3 parameters: percentage of
   * 0) Done threads
   * 1) Executing threads
   * 2) Error threads
   */
  std::tuple<float, float, float> GetPercentage( void ) const noexcept
  {
    if (Executers.size() == 0)
      return std::tuple<float, float, float>{};

    float DoneExecError[3]{}, All = 0;
    for (const auto& Exe : Executers)
      DoneExecError[static_cast<int>(Exe.second.GetStatus())]++, All++;

    return std::tuple<float, float, float>
    {
      DoneExecError[0] / All,
      DoneExecError[1] / All,
      DoneExecError[2] / All
    };
  } /* End of 'GetPercentage' function */

}; /* End of 'async_executer' class */

#endif /* __async_executer_h_ */
