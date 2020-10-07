#pragma once
#include <xtensor/xarray.hpp>
#include <random>
#include <cmath>
#include "brain.h"
#include "property.h"

class world_object{
public:
    using coord_t = float;

    virtual ~world_object(){}
    property<coord_t> x = 0;
    property<coord_t> y = 0;
    property<bool> eaten = false;
protected:
};

class food:public world_object{
public:
    property<coord_t> radius = 16;
};

class creature:public world_object{
public:
    using brain_t = neur::brain<float>;

    creature(brain_t &&br);
    creature(const creature &rhs);
    creature(creature &&rhs);
    creature(const creature &parent, std::mt19937 &gen,
        float change_wght_chance, float change_wght_percent,
        float change_thrs_chance, float change_thrs_percent,
        float add_neur_chance, float del_neur_perc,
        std::uniform_real_distribution<float> &dist_wght,
        std::uniform_real_distribution<float> &dist_thrs);

    void percieve(const std::vector<std::shared_ptr<world_object>> &objs);
    void act();
    property<float> hp;
    property<coord_t> speed;
    property<float> angle;
    property<float> radius;
protected:
    struct decision{
        coord_t d_sp;
        float d_angle;
    };
    std::vector<decision> m_dec;
    neur::brain<float> m_br;
};

inline creature::creature(brain_t &&br)
    :m_br(std::move(br))
{
}
inline creature::creature(const creature &rhs){
    this->m_br = rhs.m_br;
    this->x = rhs.x;
    this->y = rhs.y;
    this->angle = rhs.angle;
    this->speed = rhs.speed;
    this->hp = rhs.hp;
    this->radius = rhs.radius;
}
inline creature::creature(creature &&rhs){
    this->m_br = std::move(rhs.m_br);
    this->x = std::move(rhs.x);
    this->y = std::move(rhs.y);
    this->angle = std::move(rhs.angle);
    this->speed = std::move(rhs.speed);
    this->hp = std::move(rhs.hp);
    this->radius = std::move(rhs.radius);
}
inline creature::creature(const creature &parent, std::mt19937 &gen,
    float change_wght_chance, float change_wght_percent,
    float change_thrs_chance, float change_thrs_percent,
    float add_neur_chance, float del_neur_perc,
    std::uniform_real_distribution<float> &dist_wght,
    std::uniform_real_distribution<float> &dist_thrs)
{
    std::uniform_real_distribution<float> dist_chance(0,1);
    auto &br = parent.m_br;
    this->m_br.clear();

    for(auto &lr:br){
        neur::layer<float> copy(lr);
        for(auto &weight:copy){
            if(dist_chance(gen) <= change_wght_chance){
                weight += weight * change_wght_percent;
            }
        }
        for(auto &thrs:copy.activation_args()){
            if(dist_chance(gen) <= change_thrs_chance){
                thrs += thrs * change_thrs_percent;
            }
        }
        this->m_br.container().emplace_back(std::move(copy));
    }
}

inline void creature::percieve(const std::vector<std::shared_ptr<world_object>> &objs){
    xt::xarray<float> in_vec;
    in_vec.resize({1, 5});
    static const auto pi = std::atan(1)*4;
    for(auto &obj:objs){
        auto dx = obj->x - x;
        auto dy = obj->y - y;
        auto angle_obj = std::atan(dy/dx);
        if(dx > 0){
            if(dy < 0){
                angle_obj = 2*pi+angle_obj;
            }
        }else{
            angle_obj = 1*pi+angle_obj;
        }
        auto angle_rel = angle_obj - angle();
        if(angle_rel < -pi) angle_rel += 2*pi;
        if(angle_rel > +pi) angle_rel -= 2*pi;
        *(in_vec.data()+0) = angle_rel; //delta angle
        *(in_vec.data()+1) = std::sqrt(std::pow(dx, 2) + std::pow(dy, 2)); //distance
        *(in_vec.data()+2) = 1./(hp+1); //reverse hp to stimulate act
        *(in_vec.data()+3) = 0;
        *(in_vec.data()+4) = 0;
        xt::xarray<float> out = m_br.process(in_vec);
        auto dec = decision{(coord_t)out.at(0), (float)out.at(1)};
        m_dec.emplace_back(std::move(dec));
    }
}
inline void creature::act(){
    if(m_dec.empty()){
        return;
    }
    auto &dec = m_dec.front(); //TODO:choose decision properly
    this->speed() += dec.d_sp;
    this->angle() += dec.d_angle;
    m_dec.clear();
}