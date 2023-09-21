/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#ifndef SALMIAC_THREAD_POOL_H
#define SALMIAC_THREAD_POOL_H

#include "ts_queue.h"

#include <thread>

namespace sal {

template<class F>
class Thread_pool {
public:
    static constexpr std::size_t min_thread_count{1};

    /// \note Min thread count is one
    /// \note Max thread count will be what the hardware supports
    Thread_pool(std::size_t const n_threads) noexcept
    {
        std::size_t const hardware_thread_count{std::thread::hardware_concurrency()};
        /// Size will be atleast one and at most what the hardware has.
        std::size_t const thread_count =
            std::max(std::min(n_threads, hardware_thread_count), min_thread_count);

        for (std::size_t i{0}; i < thread_count; i++) {
            m_threads.push_back(std::jthread{&Thread_pool<F>::execute_worker, this});
        }
    }

    void insert(F&& f) noexcept { m_job_queue.push(std::move(f)); }

    void cancel_all() noexcept
    {
        m_should_close.store(true);
        m_job_queue.cancel_all();
    }

private:
    void execute_worker() noexcept
    {
        while (!m_should_close.load()) {
            std::optional<F> job{m_job_queue.pop()};
            // Check for nullopt
            if (job) {
                job.value()();
            }
        }
    }

    std::atomic_bool m_should_close{false};
    Ts_queue<F> m_job_queue;
    std::vector<std::jthread> m_threads;
};

} // namespace sal

#endif //SALMIAC_LOG_H
