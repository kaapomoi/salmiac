/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TS_QUEUE_H
#define SALMIAC_TS_QUEUE_H

#include <mutex>
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
        m_queue.push(value);
        m_condition_variable.notify_one();
    }

    T pop() noexcept
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_condition_variable.wait(lck, [this]() -> bool { return !m_queue.empty(); });
        T retval{m_queue.front()};
        m_queue.pop();
        return retval;
    }

private:
    std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condition_variable;
};


} // namespace sal

#endif //SALMIAC_TS_QUEUE_H
