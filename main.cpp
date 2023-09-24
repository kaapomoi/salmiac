/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "conquest.h"
#include "n_body_sim.h"
#include "thread_pool.h"

#include <iostream>


int main()
{
    Conquest app;
    //N_body_sim app;
    app.start();

    app.run();

    app.cleanup();

    return 0;
}
