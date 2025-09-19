# 源码
```c++
#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>
 
auto switch_to_new_thread(std::jthread& out)
{
    struct awaitable
    {
        std::jthread* p_out;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            std::jthread& out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output jthread parameter not empty");
            out = std::jthread([h] { h.resume(); });
            // Potential undefined behavior: accessing potentially destroyed *this
            // std::cout << "New thread ID: " << p_out->get_id() << '\n';
            std::cout << "New thread ID: " << out.get_id() << '\n'; // this is OK
        }
        void await_resume() {}
    };
    return awaitable{&out};
}
 
struct task
{
    struct promise_type
    {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
 
task resuming_on_new_thread(std::jthread& out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);
    // awaiter destroyed here
    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}
 
int main()
{
    std::jthread out;
    resuming_on_new_thread(out);
}
```
-----------------
# 翻译后等价代码
```c++
/*************************************************************************************
 * NOTE: The coroutine transformation you've enabled is a hand coded transformation! *
 *       Most of it is _not_ present in the AST. What you see is an approximation.   *
 *************************************************************************************/
#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

awaitable switch_to_new_thread(std::jthread & out)
{
  struct awaitable
  {
    std::jthread * p_out;
    inline bool await_ready()
    {
      return false;
    }
    
    inline void await_suspend(std::coroutine_handle<void> h)
    {
      std::jthread & out = *this->p_out;
      if(out.joinable()) {
        throw std::runtime_error(std::runtime_error("Output jthread parameter not empty"));
      } 
      
            
      class __lambda_17_32
      {
        public: 
        inline /*constexpr */ void operator()() const
        {
          h.resume();
        }
        
        private: 
        std::coroutine_handle<void> h;
        public: 
        // inline /*constexpr */ __lambda_17_32 & operator=(const __lambda_17_32 &) /* noexcept */ = delete;
        // inline /*constexpr */ __lambda_17_32(__lambda_17_32 &&) noexcept = default;
        __lambda_17_32(const std::coroutine_handle<void> & _h)
        : h{_h}
        {}
        
      } __lambda_17_32{h};
      
      out.operator=(std::jthread(std::jthread(__lambda_17_32)));
      std::operator<<(std::operator<<(std::operator<<(std::cout, "New thread ID: "), out.get_id()), '\n');
    }
    
    inline void await_resume()
    {
    }
    
  };
  
  return awaitable{&out};
}

struct task
{
  struct promise_type
  {
    inline task get_return_object()
    {
      return {};
    }
    
    inline std::suspend_never initial_suspend()
    {
      return {};
    }
    
    inline std::suspend_never final_suspend() noexcept
    {
      return {};
    }
    
    inline void return_void()
    {
    }
    
    inline void unhandled_exception()
    {
    }
    
    // inline constexpr promise_type() noexcept = default;
  };
  
};


struct __resuming_on_new_threadFrame
{
  void (*resume_fn)(__resuming_on_new_threadFrame *);
  void (*destroy_fn)(__resuming_on_new_threadFrame *);
  std::__coroutine_traits_impl<task>::promise_type __promise;
  int __suspend_index;
  bool __initial_await_suspend_called;
  std::jthread & out;
  std::suspend_never __suspend_39_6;
  awaitable __suspend_42_14;
  std::suspend_never __suspend_39_6_1;
};

task resuming_on_new_thread(std::jthread & out)
{
  /* Allocate the frame including the promise */
  /* Note: The actual parameter new is __builtin_coro_size */
  __resuming_on_new_threadFrame * __f = reinterpret_cast<__resuming_on_new_threadFrame *>(operator new(sizeof(__resuming_on_new_threadFrame)));
  __f->__suspend_index = 0;
  __f->__initial_await_suspend_called = false;
  __f->out = std::forward<std::jthread &>(out);
  
  /* Construct the promise. */
  new (&__f->__promise)std::__coroutine_traits_impl<task>::promise_type{};
  
  /* Forward declare the resume and destroy function. */
  void __resuming_on_new_threadResume(__resuming_on_new_threadFrame * __f);
  void __resuming_on_new_threadDestroy(__resuming_on_new_threadFrame * __f);
  
  /* Assign the resume and destroy function pointers. */
  __f->resume_fn = &__resuming_on_new_threadResume;
  __f->destroy_fn = &__resuming_on_new_threadDestroy;
  
  /* Call the made up function with the coroutine body for initial suspend.
     This function will be called subsequently by coroutine_handle<>::resume()
     which calls __builtin_coro_resume(__handle_) */
  __resuming_on_new_threadResume(__f);
  
  
  return __f->__promise.get_return_object();
}

/* This function invoked by coroutine_handle<>::resume() */
void __resuming_on_new_threadResume(__resuming_on_new_threadFrame * __f)
{
  try 
  {
    /* Create a switch to get to the correct resume point */
    switch(__f->__suspend_index) {
      case 0: break;
      case 1: goto __resume_resuming_on_new_thread_1;
      case 2: goto __resume_resuming_on_new_thread_2;
      case 3: goto __resume_resuming_on_new_thread_3;
    }
    
    /* co_await insights.cpp:39 */
    __f->__suspend_39_6 = __f->__promise.initial_suspend();
    if(!__f->__suspend_39_6.await_ready()) {
      __f->__suspend_39_6.await_suspend(std::coroutine_handle<task::promise_type>::from_address(static_cast<void *>(__f)).operator std::coroutine_handle<void>());
      __f->__suspend_index = 1;
      __f->__initial_await_suspend_called = true;
      return;
    } 
    
    __resume_resuming_on_new_thread_1:
    __f->__suspend_39_6.await_resume();
    std::operator<<(std::operator<<(std::operator<<(std::cout, "Coroutine started on thread: "), std::this_thread::get_id()), '\n');
    
    /* co_await insights.cpp:42 */
    __f->__suspend_42_14 = switch_to_new_thread(__f->out);
    if(!__f->__suspend_42_14.await_ready()) {
      __f->__suspend_42_14.await_suspend(std::coroutine_handle<task::promise_type>::from_address(static_cast<void *>(__f)).operator std::coroutine_handle<void>());
      __f->__suspend_index = 2;
      return;
    } 
    
    __resume_resuming_on_new_thread_2:
    __f->__suspend_42_14.await_resume();
    std::operator<<(std::operator<<(std::operator<<(std::cout, "Coroutine resumed on thread: "), std::this_thread::get_id()), '\n');
    /* co_return insights.cpp:39 */
    __f->__promise.return_void()/* implicit */;
    goto __final_suspend;
  } catch(...) {
    if(!__f->__initial_await_suspend_called) {
      throw ;
    } 
    
    __f->__promise.unhandled_exception();
  }
  
  __final_suspend:
  
  /* co_await insights.cpp:39 */
  __f->__suspend_39_6_1 = __f->__promise.final_suspend();
  if(!__f->__suspend_39_6_1.await_ready()) {
    __f->__suspend_39_6_1.await_suspend(std::coroutine_handle<task::promise_type>::from_address(static_cast<void *>(__f)).operator std::coroutine_handle<void>());
    __f->__suspend_index = 3;
    return;
  } 
  
  __resume_resuming_on_new_thread_3:
  __f->destroy_fn(__f);
}

/* This function invoked by coroutine_handle<>::destroy() */
void __resuming_on_new_threadDestroy(__resuming_on_new_threadFrame * __f)
{
  /* destroy all variables with dtors */
  __f->~__resuming_on_new_threadFrame();
  /* Deallocating the coroutine frame */
  /* Note: The actual argument to delete is __builtin_coro_frame with the promise as parameter */
  operator delete(static_cast<void *>(__f), sizeof(__resuming_on_new_threadFrame));
}


int main()
{
  std::jthread out = std::jthread();
  resuming_on_new_thread(out);
  return 0;
}

```