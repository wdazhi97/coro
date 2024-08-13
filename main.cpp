#include <iostream>
#include <coroutine>

/*struct Promise;
struct Awaiter{
    Awaiter(int i):value(i){}

    bool await_ready() {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> handle){
        std::cout << "Suspend " <<  &handle << " " << value << std::endl;
        return handle;
    }
    void await_resume() {
        std::cout << "resume " << value <<  std::endl;
    };

    int value;
};

struct Task {
    using promise_type = Promise;
    Task(std::coroutine_handle<Promise> h) : m_handler(h){
        std::cout << "init task" << std::endl;
    };

    ~Task() {
        if (m_handler) {
            m_handler.destroy();
        }
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // 允许移动
    Task(Task&& other) noexcept : m_handler(other.m_handler) {
        other.m_handler = nullptr;
    }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_handler) {
                m_handler.destroy();
            }
            m_handler = other.m_handler;
            other.m_handler = nullptr;
        }
        return *this;
    }

    void resume() {
        std::cout << "Resume " << &m_handler << std::endl;
        m_handler.resume();
        m_handler.promise();
    }

    auto operator co_await (){
        return Awaiter{2};
    }

private:
    std::coroutine_handle<Promise> m_handler;
};

struct suspendAlways {
    bool await_ready() {
        return true;
    }
    void await_suspend(std::coroutine_handle<> handle){
        std::cout << "Suspend Always " <<  &handle << std::endl;
    }
    void await_resume() {
        std::cout << "resume Always " << std::endl;
    };
};

struct Promise{
    std::coroutine_handle<Promise> get_return_object(){
        std::cout << "get obj " << std::endl;
        return std::coroutine_handle<Promise>::from_promise(*this);
    }
    suspendAlways initial_suspend() noexcept {
        std::cout << "Init Suspend" <<std::endl;
        return {};}
    std::suspend_always final_suspend() noexcept {
        std::cout << "Final suspend" << std::endl;
        return {};}
    void return_void(){}
    void unhandled_exception() {}
};



Task Test() {
    std::cout << "test" << std::endl;
    auto waiter = Awaiter{1};
    co_await waiter;
    co_return ;
};

Task Test2() {
    std::cout << "test2" << std::endl;
    auto a = Test();
    co_await a;
}*/

namespace coro {

    class Task;

    class suspendFinal {
    public:
        bool await_ready() noexcept {return false;}

        void await_suspend(std::coroutine_handle<> h) noexcept{
            std::cout << "final suspend " << h.address() << std::endl;
            if(!h.done()){
                h.resume();
            }
            else{
                std::cout << h.address() << " has done " << std::endl;
            }
        }

        void await_resume() noexcept {
            std::cout << "await_final resume" << std::endl;
        }
    };

    class suspendAlways {
    public:
        bool await_ready() noexcept {return false;}

        void await_suspend(std::coroutine_handle<> h) noexcept{
            std::cout << "await_suspend always " << h.address() << std::endl;
        }

        void await_resume() noexcept {
            std::cout << "await_suspend resume" << std::endl;
        }
    };

    class promise_base {
    public:

        suspendAlways initial_suspend() noexcept{
            std::cout << "inital suspend" << std::endl;
            return{};
        }

        suspendFinal final_suspend() noexcept {
            std::cout << "final suspend" << std::endl;
            return{};
        }

        void set_continuation(std::coroutine_handle<> continuation){
            m_continuation = continuation;
        }

        void return_void() noexcept {
            std::cout << "return void " << std::endl;
        }

        suspendAlways yield_value(int i){
            std::cout << "yield value" << std::endl;
            value = i;
            return {};
        }

        void unhandled_exception() noexcept {}

        Task get_return_object();

        int getValue(){return value;};

    private:
        int value = 0;
        std::coroutine_handle<> m_continuation;
    };

    class Task {

    public:
        using handle_type =  std::coroutine_handle<promise_base> ;
        using promise_type = promise_base;
        explicit Task(std::coroutine_handle<promise_type> h) : m_handle(h){
            std::cout << "Task Input " << h.address() << " " << m_handle.address() << std::endl;
        }

        auto operator co_await() {
            return AwaitBase{m_handle};
        }

        void resume(){
            m_handle.resume();
        }

        int result(){
            return m_handle.promise().getValue();
        }

        bool done(){
            return m_handle && m_handle.done();
        };

    private:
        class AwaitBase{
        public:
            bool await_ready(){
                bool ready = m_handle && m_handle.done();
                std::cout << "await ready " << ready << std::endl;
                return ready;
            }

            auto await_suspend(std::coroutine_handle<promise_type> awaiting){
                std::cout << "await_suspend continue awaiting " << awaiting.address() << " " << m_handle.address() << std::endl;
                return awaiting;
            }

            void await_resume() noexcept {
                std::cout << "await remuse" <<std::endl;
            }

            explicit AwaitBase(std::coroutine_handle<promise_type> h) :m_handle(h){
                std::cout << "Input " << h.address() << " " << m_handle.address() << " " << this << std::endl;
            }

            explicit AwaitBase(const AwaitBase &) = delete;

            AwaitBase& operator=(const AwaitBase& ) = delete;

        private:
            std::coroutine_handle<promise_type> m_handle;
        };

    private:
        handle_type  m_handle;
    };

    Task promise_base::get_return_object() {
        std::cout << "get obj " << this << std::endl;
        return Task{std::coroutine_handle<promise_base>::from_promise(*this)};
    };
}

class TestObj {
public:

    template<class promise>
    auto operator()(promise promise1) {
        return std::suspend_always{};
    }
};

constexpr TestObj Obj;

coro::Task Test(){
    //init_await
    co_return ; //return void
    //final_await
};

coro::Task Test1(){
    //init_await
    std::cout << "---------------------------------------------------------------" <<std::endl;
    auto a = Test();
    co_await a;
    a.resume();
    std::cout << "pending return void" <<std::endl;
    //return_void
    //final wait
};



int main() {
    auto a = Test1();
    a.resume();
    return 0;
}
