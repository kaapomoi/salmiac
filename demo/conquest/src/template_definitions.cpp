/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "artisan.cpp"
#include "game.cpp"
#include "neural_net.cpp"
#include "orchestrator.cpp"


template class Orchestrator<40, 40, 6, 2>;
template class Artisan<40, 40, 6, 2>;
template class Game<40, 40, 6, 2>;
