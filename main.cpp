/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "conquest.h"
#include "n_body_sim.h"
#include "thread_pool.h"

#include <iostream>

std::mutex m;

void task()
{
    {
        std::lock_guard<std::mutex> lg(m);
        std::cout << "Thread id: " << std::this_thread::get_id() << " started" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds{1});

    std::lock_guard<std::mutex> lg(m);
    std::cout << "Thread id: " << std::this_thread::get_id() << " finished" << std::endl;
}

int main()
{
    // Conquest app;
    // //N_body_sim app;
    // app.start();

    // app.run();

    // app.cleanup();


    std::cout << "Creating a thread pool with " << std::thread::hardware_concurrency() << " threads"
              << std::endl;
    sal::Thread_pool<std::function<void()>> pool{8};

    for (int i = 0; i < 20; ++i)
        pool.insert(task);

    std::this_thread::sleep_for(std::chrono::seconds{1});

    return 0;
}
