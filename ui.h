#pragma once
#include "world.h"

class ui{
public:
protected:
public:
    virtual ~ui(){};
    virtual void start()=0;
};

#include <SDL.h>
class ui_imgui:public ui{
    SDL_GLContext gl_context;
    SDL_Window* window;
    world &wr;
    struct camera{
        float x, y;
        float zoom;
        template<class T>
        T interp_x(const T&x){ return (x+this->x)*zoom; }
        template<class T>
        T interp_y(const T&y){ return (y+this->y)*zoom; }
    }cam;
public:
    ui_imgui(world &w);
    virtual void start()override;
};
