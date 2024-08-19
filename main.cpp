#include <iostream>
#include <coroutine>
#include "include/cancellation_task.hpp"

#include "include/task.hpp"
#include "include/sync_task.hpp"

cppcoro::cancellation_task cancel(cancel_token token) {
    co_return;
}

cppcoro::sync_task<int> test() {
    cancel_request req;
    auto token = req.token();
    co_await cancel(token);
    req.request_cancel();
    co_return 1;
}



int main() {
    auto result = test();
    std::cout << result.result() << std::endl;
    while(1){};
}
