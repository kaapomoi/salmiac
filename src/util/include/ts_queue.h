/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TS_QUEUE_H
#define SALMIAC_TS_QUEUE_H

#include <mutex>
#include <optional>
#include <queue>

namespace sal {


///
/// \brief Thread safe queue
///
template<class T>
class Ts_queue {
public:
    Ts_queue() = default;

    void push(T value) noexcept
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        m_queue.push(std::move(value));
        m_condition_variable.notify_one();
    }

    std::optional<T> pop() noexcept
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_condition_variable.wait(
            lck, [this]() -> bool { return !m_queue.empty() || m_cancel_requested.load(); });
        if (m_queue.empty()) {
            return {};
        }

        T retval{m_queue.front()};
        m_queue.pop();
        return retval;
    }

    void cancel_all() noexcept
    {
        m_cancel_requested.store(true);
        m_condition_variable.notify_all();
    }

private:
    std::atomic_bool m_cancel_requested{false};
    std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condition_variable;
};


} // namespace sal

#endif //SALMIAC_TS_QUEUE_H
