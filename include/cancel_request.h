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
#include <ranges>

enum class cancel_state_type : uint8_t {
    NONE,
    CANCELED,
};

class call_back_state{
public:
    std::function<void()> call_back;
    bool is_valid;
};

class cancel_state {
public:

    void cancel(){
        m_state = static_cast<std::uint32_t> (cancel_state_type::CANCELED);
        std::cout << "cancel_state" << std::endl;
        for(auto call : std::views::reverse(call_backs)) {
            if(call.is_valid)
            {
                call.call_back();
            }
        }
    };

    [[nodiscard]] bool is_cancelled() const
    {
        return m_state == static_cast<std::uint32_t> (cancel_state_type::CANCELED);
    }

    int register_token(const std::function<void()>& function)
    {
        call_backs.push_back({function,true});
        return call_backs.size() - 1;
    }

    void set_call_invalid(int index)
    {
        if(index > 0 && index < call_backs.size())
        {
            call_backs[index].is_valid = false;
        }
    }

private:
    std::uint32_t m_state;
    std::vector<call_back_state> call_backs;
};

class cancel_token{
public:
    explicit cancel_token(std::shared_ptr<cancel_state> state) : m_state(std::move(state)){};

    bool is_cancelled()
    {
        return m_state && m_state->is_cancelled();
    }

    int register_callback(const std::function<void()>& function)
    {
        return m_state->register_token(function);
    }

    void set_call_invalid(int index)
    {
        m_state->set_call_invalid(index);
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

    cancel_request()
    {
        m_state = std::make_shared<cancel_state>();
    }

    void request_cancel()
    {
        if(m_state && !m_state->is_cancelled())
        {
            m_state->cancel();
        }
    }

private:
    std::shared_ptr<cancel_state> m_state;
};




#endif //COROUTINE_CANCEL_REQUEST_H
