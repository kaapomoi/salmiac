/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "bbmania.h"
#include "bbmania_server.h"
#include "thread_pool.h"

#include <iostream>


int main(std::int32_t const argc, char const* const* argv)
{
    //Conquest app;
    if (argc > 1) {
        if (strcmp(argv[1], "--server") == 0) {
            bbm::Bbmania_server server;

            server.run();
        }
    }
    else {
        bbm::Bbmania app;
        app.start();

        app.run();

        app.cleanup();
    }

    return 0;
}
