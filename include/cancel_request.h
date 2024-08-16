//
// Created by dazhiwang on 2024/8/15.
//

#ifndef COROUTINE_CANCEL_REQUEST_H
#define COROUTINE_CANCEL_REQUEST_H

#include <cstdint>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

enum class cancel_state_type : uint8_t {
    NONE,
    CANCELED,
};

class cancel_state {
public:

    void cancel(){
        m_state = static_cast<std::uint32_t> (cancel_state_type::CANCELED);
        for(auto & call : call_backs)
        {
            call();
        }
    };

    [[nodiscard]] bool is_cancelled() const
    {
        return m_state == static_cast<std::uint32_t> (cancel_state_type::CANCELED);
    }

    void register_token(const std::function<void()>& function)
    {
        call_backs.push_back(function);
    }

private:
    std::uint32_t m_state;
    std::vector<std::function<void()>> call_backs;
};

class cancel_token{
public:
    explicit cancel_token(std::shared_ptr<cancel_state> state) : m_state(std::move(state)){};

    bool is_cancelled()
    {
        return m_state && m_state->is_cancelled();
    }

    void register_callback(const std::function<void()>& function)
    {
        m_state->register_token(function);
    }

private:
    std::shared_ptr<cancel_state> m_state;
};

class cancel_register{
public:

    cancel_register(cancel_token token, const std::function<void()>& call_back){
        token.register_callback(call_back);
    }

private:
};

class cancel_request{
public:

    cancel_token token(){
        return cancel_token(m_state);
    }

    void request_cancel()
    {
        if(m_state)
        {
            m_state->cancel();
        }
    }

private:
    std::shared_ptr<cancel_state> m_state;
};




#endif //COROUTINE_CANCEL_REQUEST_H
