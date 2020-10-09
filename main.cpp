#include <iostream>
#include <thread>
#include <chrono>
#include "creature.h"
#include "world.h"
#include "ui.h"

int main(int, char**) {
    world w;
    ui_imgui ui(w);
    w.w = 1700;
    w.h = 1000;
    w.creatures_percept_radius = 128;
    w.creatures_percept_max = 8;
    w.creatures_speed_max = 50;
    w.creatures_angle_speed_max = math::pi/3;
    w.creatures_max = 128;
    w.best_creatures_count = 8;
    w.foods_max = 32;
    w.creatures_hp_max = 1024*4;
    w.dhp_per_iter = 1;
    w.dhp_speed_ratio = 1;
    w.creatures_radius_min = 2;
    w.creatures_radius_max = 8;
    w.neur_layers_min = 8;
    w.neur_layers_max = 16;
    w.neur_wght_min = -8; 
    w.neur_wght_max = +8; 
    w.neur_thrs_min = -4; 
    w.neur_thrs_max = +4; 
    w.neur_count_min = 16;
    w.neur_count_max = 32;
    w.cps = 120;
    w.time_scale = 20;
    w.start();
    ui.start();
    w.stop();
    return 0;
}
