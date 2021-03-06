#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <thread>
#include <random>
#include <iostream>
#include <chrono>
#include <execution>
#include "creature.h"
#include "utils.h"

class world{
    using m_creatures_t = std::vector<std::shared_ptr<creature>>;
    using creatures_t = std::vector<creature>;
    using m_foods_t = std::vector<std::shared_ptr<food>>;
    using foods_t = std::vector<food>;
public:
    world();
    ~world();

    void start();
    void stop();
    auto creatures() -> m_creatures_t;
    auto creatures_count() -> size_t;
    auto foods() -> foods_t;
    auto foods_count() -> size_t;

    property<size_t> foods_max, creatures_max;
    property<size_t> w, h;
    property<size_t> best_creatures_count = 1;
    property<size_t> creatures_percept_radius;
    property<size_t> creatures_percept_max;
    property<float> creatures_hp_max;
    property<float> creatures_speed_max;
    property<float> creatures_angle_speed_max;
    property<float> creatures_radius_min, creatures_radius_max;
    property<float> dhp_per_iter, dhp_speed_ratio;

    property<float> neur_wght_min, neur_wght_max;
    property<float> neur_thrs_min, neur_thrs_max;
    property<size_t> neur_layers_min, neur_layers_max;
    property<size_t> neur_count_min, neur_count_max;
    property<size_t> cps = 120;
    property<double> time_scale;
protected:
    void p_fill_world();
    void p_fill_world(const m_creatures_t &best);
    void p_thr_func();
    void p_refill_foods();
    void p_per_creature_func(std::shared_ptr<creature> cr);

    std::random_device rd;
    std::mt19937 gen;
    std::atomic_bool m_started, m_stop_requested;
    std::mutex m_mt;
    std::thread m_thr;
    m_foods_t m_foods;
    m_creatures_t m_creatures;
    std::chrono::steady_clock::time_point m_last_iter;
};

inline world::world(){
    m_started = false;
    m_stop_requested = false;
    gen = std::mt19937(rd());
}
inline world::~world(){
    m_stop_requested = true;
    if(m_thr.joinable()){
        m_thr.join();
    }
}

inline void world::p_fill_world(const m_creatures_t &best){
    if(best.size()==0){
        throw std::runtime_error("no best creatures to populate world");
    }
    m_creatures.clear();
    m_creatures.reserve(creatures_max);
    m_foods.clear();
    p_refill_foods();

    size_t children_per_best = creatures_max()/best.size();
    std::uniform_real_distribution<float> dist_x(0, w());
    std::uniform_real_distribution<float> dist_y(0, h());
    std::uniform_real_distribution<float> dist_wght(neur_wght_min(), neur_wght_max());
    std::uniform_real_distribution<float> dist_thrs(neur_thrs_min(), neur_thrs_max());
    std::uniform_int_distribution<size_t> dist_lr(neur_layers_min(), neur_layers_max());
    std::uniform_int_distribution<size_t> dist_nr(neur_count_min(), neur_count_max());
    std::uniform_real_distribution<float> dist_chance(0, 1);
    float change_wght_chance = 0.05;
    float change_wght_percent = 0.02;
    float change_thrs_chance = 0.05;
    float change_thrs_percent = 0.02;
    float add_neur_chance = 0.5;
    float add_layer_chance = 0.1;
    float del_neur_perc = 0.5;
    for(size_t i=0; i<best.size(); i++){
        auto &cr = best.at(i);
        for(size_t j=0; j<children_per_best; j++){
            auto child = std::make_shared<creature>(*cr, gen,
                change_wght_chance, change_wght_percent,
                change_thrs_chance, change_thrs_percent,
                add_neur_chance, del_neur_perc,
                dist_wght, dist_thrs);
            child->speed = 0;
            child->angle = dist_x(gen);
            child->angle_speed = 0;
            child->hp = creatures_hp_max;
            child->x = dist_x(gen);
            child->y = dist_y(gen);
            child->radius = cr->radius;
            m_creatures.emplace_back(std::move(child));
        }
    }
    m_last_iter = std::chrono::steady_clock::now();
}
inline void world::p_fill_world(){
    m_creatures.clear();
    m_creatures.reserve(creatures_max);
    m_foods.clear();
    p_refill_foods();

    std::uniform_real_distribution<float> dist_x(0, w());
    std::uniform_real_distribution<float> dist_y(0, h());
    std::uniform_real_distribution<float> dist_wght(neur_wght_min(), neur_wght_max());
    std::uniform_real_distribution<float> dist_thrs(neur_thrs_min(), neur_thrs_max());
    std::uniform_real_distribution<float> dist_radius(creatures_radius_min(), creatures_radius_max());
    std::uniform_int_distribution<size_t> dist_lr(neur_layers_min(), neur_layers_max());
    std::uniform_int_distribution<size_t> dist_nr(neur_count_min(), neur_count_max());
    const size_t size_ins = 5;
    const size_t size_outs = 3;

    neur::layer<float>::act_t act = [](auto &out, auto& arg){
        out = std::abs(out) >= *arg.begin()? out : 0;
    };
    static auto lbd_gen_lr = [this, &dist_wght, &dist_thrs, &act](auto &nrs, auto &nrs_prev, auto &lr_num, auto &br){
        xt::xarray<float> wghts = xt::zeros<float>({nrs_prev, nrs});
        xt::xarray<float> thrss = xt::zeros<float>({(size_t)1, nrs});
        std::generate(wghts.begin(), wghts.end(), [this, &dist_wght](){ return dist_wght(gen); });
        std::generate(thrss.begin(), thrss.end(), [this, &dist_thrs](){ return dist_thrs(gen); });
        neur::layer<float> lr(std::move(wghts));
        lr.set_activation(act);
        lr.set_activation_args(std::move(thrss));
        br.insert(br.begin()+lr_num, std::move(lr));
        nrs_prev = nrs;
    };
    for(size_t i=0; i<creatures_max; i++){
        auto lrs = dist_lr(gen);
        size_t nrs_prev = size_ins;
        neur::brain<float> br;
        br.reserve(lrs);
        for(size_t l=0; l<lrs; l++){
            auto nrs = dist_nr(gen);
            lbd_gen_lr(nrs, nrs_prev, l, br);
        }
        lbd_gen_lr(size_outs, nrs_prev, lrs, br);

        auto cr = std::make_shared<creature>(std::move(br));
        cr->speed = 0;
        cr->angle = dist_x(gen);
        cr->angle_speed = 0;
        cr->hp = creatures_hp_max;
        cr->x = dist_x(gen);
        cr->y = dist_y(gen);
        cr->radius = dist_radius(gen);
        this->m_creatures.emplace_back(std::move(cr));
    }
    m_last_iter = std::chrono::steady_clock::now();
}
inline void world::p_thr_func(){
    while(!m_stop_requested){
        auto iter_time0 = std::chrono::steady_clock::now();
        auto frame_gap = std::chrono::milliseconds(1000 / cps);
        std::for_each(std::execution::par, m_creatures.begin(), m_creatures.end(),
            [this](auto &&cr){ this->p_per_creature_func(cr); });
        std::vector<std::shared_ptr<creature>> best_creatures;
        {
            std::unique_lock<std::mutex> lock(m_mt);

            std::sort(m_creatures.begin(), m_creatures.end(),
                [](auto &&cr0, auto &&cr1){ return cr0->hp() > cr1->hp(); });

            std::copy(m_creatures.begin(), m_creatures.begin()+std::min(best_creatures_count(), m_creatures.size()),
                std::back_inserter(best_creatures));

            auto it = std::find_if(m_creatures.begin(), m_creatures.end(),
                [](auto &&cr){ return cr->hp() <= 0.f; });
            m_creatures.erase(it, m_creatures.end());

            p_refill_foods();
        }
        m_last_iter = std::chrono::steady_clock::now();
        
        std::this_thread::sleep_until(iter_time0+frame_gap);
        if(m_creatures.size() <= best_creatures.size()){
            std::unique_lock<std::mutex> lock(m_mt);
            p_fill_world(best_creatures);
        }else{
            bool should_refill = std::all_of(m_creatures.begin(), m_creatures.end(), [](auto &&cr){ return cr->hp() == 0;});
            if(should_refill){
                p_fill_world();
            }
        }
    }
}
inline void world::p_refill_foods(){
    auto it_fd = m_foods.begin();
    while(it_fd != m_foods.end()){
        auto &fd = *it_fd;
        if(fd->eaten()){
            it_fd = m_foods.erase(it_fd);
        }else{
            it_fd++;
        }
    }
    std::uniform_real_distribution<float> dist_x(0, w());
    std::uniform_real_distribution<float> dist_y(0, h());
    m_foods.reserve(foods_max);
    while (m_foods.size() < foods_max){
        auto fd = new food();
        fd->x() = dist_x(this->gen);
        fd->y() = dist_y(this->gen);
        fd->radius = creatures_radius_min();
        fd->eaten = false;
        m_foods.emplace_back(fd);
    }
}
inline void world::p_per_creature_func(std::shared_ptr<creature> cr){
    std::vector<std::shared_ptr<world_object>> filtered;
    filtered.reserve(creatures_percept_max());
    auto fill_filtered = [this, &cr, &filtered](auto &&trg){
        for(size_t i=0; i<trg.size(); i++){
            if(filtered.size() == creatures_percept_max){
                return;
            }
            auto &el = trg.at(i);
            if(math::pif(cr->x(), cr->y(), el->x(), el->y()) < creatures_percept_radius){
                filtered.emplace_back(el);
            }
        }
    };
    //fill_filtered(m_creatures);
    fill_filtered(m_foods);

    auto now = std::chrono::steady_clock::now();
    auto time_passed_delta = now - m_last_iter;
    auto time_passed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_passed_delta);
    auto time_passed_s = time_passed_ms.count()/1000.;
    time_passed_s *= time_scale;
    cr->percieve(filtered);
    cr->act();

    cr->angle_speed = std::clamp(cr->angle_speed(), -creatures_angle_speed_max, +creatures_angle_speed_max);
    cr->speed = std::clamp(cr->speed(), -creatures_speed_max, +creatures_speed_max);
    if(std::abs(cr->speed()) <= std::numeric_limits<typename decltype(cr->speed)::value_type>::epsilon()*20){
        cr->speed() = 0;
    }

    cr->angle() += cr->angle_speed()*time_passed_s;
    cr->angle = std::fmod(cr->angle, 2*math::pi);
    if(cr->angle < 0){ cr->angle() += 2*math::pi;}

    cr->x() += cr->speed()*time_passed_s*std::cos(cr->angle());
    cr->y() += cr->speed()*time_passed_s*std::sin(cr->angle());
    cr->x() = std::fmod(cr->x(), (float)w());
    cr->y() = std::fmod(cr->y(), (float)h());
    if(cr->x() < 0) cr->x() += w();
    if(cr->y() < 0) cr->y() += h();
    static auto food_lbd = [&cr](auto obj){
        auto cast = std::dynamic_pointer_cast<food>(obj);
        if(!cast){
            return false;
        }
        auto dist = math::pif(cast->x, cast->y, cr->x, cr->y);
        if(dist > (cr->radius()+cast->radius()) || cast->eaten()){
            return false;
        }
        return true;
    };
    auto fd_it = std::find_if(filtered.begin(), filtered.end(), food_lbd);
    if(fd_it != filtered.end()){
        auto &fd = *fd_it;
        fd->eaten() = true;
        cr->hp() += 100;
    }
    cr->hp() -= dhp_per_iter();
    cr->hp() -= std::abs(cr->speed())*dhp_speed_ratio()/std::abs(creatures_speed_max()); //dhp will be proportioned to creature speed
    cr->hp() -= std::abs(cr->angle_speed())*dhp_speed_ratio()/std::abs(creatures_angle_speed_max()); //dhp will be proportioned to creature speed
}
inline void world::start(){
    p_fill_world();
    m_started = true;
    m_thr = std::thread([this](){ p_thr_func(); });
}
inline void world::stop(){
    m_started = false;
}
inline auto world::creatures() -> m_creatures_t{
    m_creatures_t copy;
    {
        std::unique_lock<std::mutex> lock(m_mt);
        copy = m_creatures;
        /*
        std::transform(m_creatures.begin(), m_creatures.end(), std::back_inserter(copy),
            [](auto &&ptr){ return *ptr; });
        */
    }
    return copy;
}
inline auto world::creatures_count() -> size_t { return m_creatures.size(); }
inline auto world::foods() -> foods_t{
    foods_t copy;
    {
        std::unique_lock<std::mutex> lock(m_mt);
        std::transform(m_foods.begin(), m_foods.end(), std::back_inserter(copy),
            [](auto &&ptr){ return *ptr; });
    }
    return copy;
}
inline auto world::foods_count() -> size_t { return m_foods.size(); }