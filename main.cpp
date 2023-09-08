/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "n_body_sim.h"

int main()
{
    N_body_sim app;
    app.start();

    app.run();

    app.cleanup();

    return 0;
}
