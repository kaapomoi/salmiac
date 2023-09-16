/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "application.h"
#include "conquest.h"
#include "n_body_sim.h"

int main()
{
    Conquest app;
    app.start();

    app.run();

    app.cleanup();

    return 0;
}
