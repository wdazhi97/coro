//
// Created by dazhiwang on 2024/6/27.
//

#ifndef COROUTINE_SHARE_TASK_HPP
#define COROUTINE_SHARE_TASK_HPP
#include <coroutine>
#include <iostream>
namespace coro {

    template<class T>
    class share_task_promise{
        void unhandled_exception(){
            std::terminate();
        }

        std::suspend_always initial_suspend(){
            return {};
        }

        std::suspend_always final_suspend() {
            return {};
        }

        void return_void() {

        }

        void return_value(T && value){
            ::new (static_cast<void* >(std::addressof(m_value))) T(std::forward(value));
        }

        union {
            T m_value;
        };
    };


    template<class T>
    class share_task{
    public:
        using promise_type = share_task_promise<T>;
        using handle_type = std::coroutine_handle<promise_type> ;

    private:

    };
}

#endif //COROUTINE_SHARE_TASK_HPP
