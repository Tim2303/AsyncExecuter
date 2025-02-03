/*
 * Module for asynchronous command execution
 */

#ifndef __async_executer_h_
#define __async_executer_h_

#include <functional>
#include <thread>
#include <map>

/* Async executer class */
template<typename Index = int>
class async_exec
{
public:
  /* Async execution status enum */
  enum class status
  {
    AE_STATUS_DONE = 0,
    AE_STATUS_EXECUTING = 1,
    AE_STATUS_ERROR = 2,
  }; /* End of 'status' enum class */

private:

  class entry
  {
  private:
    // Thread for function executing
    std::thread ExecuteThread{};

    // Current thread status
    status Status = status::AE_STATUS_EXECUTING;

    template<typename Func>
    void Executer( Func FuncPtr )
    {
      try
      {
        Status = status::AE_STATUS_EXECUTING;
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
      ExecuteThread.detach();
    } /* End of 'Execute' function */

    constexpr status GetStatus( void ) const noexcept
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

    auto Binded = std::bind(FuncPtr, Argts...);

    Executers[Ind] = entry();
    Executers[Ind].Execute(Binded);
  } /* End of 'AddExecuteTask' function */

  constexpr void WaitForExecuting(void) const noexcept
  {
    bool IsInProgress = true;

    while (IsInProgress)
    {
      IsInProgress = false;
      for (const auto& Exe : Executers)
        if (Exe.second.GetStatus() == status::AE_STATUS_EXECUTING)
          IsInProgress = true;
    }
  } /* End of 'WaitForExecuting' function */

  /* Wait for all execute tasts to stop function.
   * ARGUMENTS:
   *   - thread index:
   *       Index Ind;
   * RETURNS:
   *   (status) thread status.
   */
  constexpr status GetThreadExecuteStatus(Index Ind) const noexcept
  {
    return Executers[Ind].second.GetStatus();
  } /* End of 'GetThreadExecuteStatus' function */

  /* Get percentage for executer threads progress function.
   * ARGUMENTS: None.
   * RETURNS:
   *   (std::tuple<float, float, float>) progresses 'done', 'executing' and 'error'.
   */
  constexpr std::tuple<float, float, float> GetWorkPercentage(void) const noexcept
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
  } /* End of 'GetWorkPercentage' function */

}; /* End of 'async_exec' class */

#endif /* __async_executer_h_ */
