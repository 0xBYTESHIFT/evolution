#include "ui.h"

#include <GL/glew.h>
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

ui_imgui::ui_imgui(world &w)
    :gl_context(NULL),
    wr(w)
{
    cam.x = 0;
    cam.y = 0;
    cam.zoom = 1;
    if (SDL_Init(SDL_INIT_VIDEO |
        SDL_INIT_TIMER |
        SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        abort();
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = glewInit() != GLEW_OK;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        abort();
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void ui_imgui::start(){
    bool done = false;
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    float accel=1;
    bool accel_reset = false;
    auto& cps = wr.cps();
    auto& fds_max = wr.foods_max();
    auto& crs_max = wr.creatures_max();
    auto& wr_w = wr.w();
    auto& wr_h = wr.h();
    auto& crs_perc_rad = wr.creatures_percept_radius();
    auto& crs_perc_max = wr.creatures_percept_max();
    auto& crs_hp_max = wr.creatures_hp_max();
    auto& crs_speed_max = wr.creatures_speed_max();
    auto& crs_rad_min = wr.creatures_radius_min();
    auto& crs_rad_max = wr.creatures_radius_max();
    auto& wr_dhp = wr.dhp_per_iter();

    while(!done){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
            }
            if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_DOWN){   cam.y+=accel; accel*=1.05; }
                else if(event.key.keysym.sym == SDLK_UP){cam.y-=accel; accel*=1.05; }
                else if(event.key.keysym.sym == SDLK_LEFT){  cam.x-=accel; accel*=1.05; }
                else if(event.key.keysym.sym == SDLK_RIGHT){ cam.x+=accel; accel*=1.05; }
            }else{
                accel_reset = true;
            }
        }
        if(accel_reset){
            accel = 1;
            accel_reset = false;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        auto drawList = ImGui::GetBackgroundDrawList();
        {
            drawList->AddRect({cam.interp_x(0.f), cam.interp_y(0.f)},
                {cam.interp_x((float)wr.w()), cam.interp_y((float)wr.h())}, 0xFFFFFFFF);
        }

        ImGui::Begin("UI");
        ImGui::Text("Time per frame %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::InputScalar ("cps", ImGuiDataType_U64, &cps);
        ImGui::InputScalar ("World w", ImGuiDataType_U64, &wr_w);
        ImGui::InputScalar ("World h", ImGuiDataType_U64, &wr_h);
        ImGui::InputScalar ("Max creatures", ImGuiDataType_U64, &crs_max);
        ImGui::InputScalar ("Max food", ImGuiDataType_U64, &fds_max);
        ImGui::InputScalar ("Hp", ImGuiDataType_Float, &crs_hp_max);
        ImGui::InputScalar ("Hp delta", ImGuiDataType_Float, &wr_dhp);
        ImGui::InputScalar ("Speed max", ImGuiDataType_Float, &crs_speed_max);

        auto crs = wr.creatures();
        auto fds = wr.foods();
        auto cr_clr = 0xFFFFFFFF;
        auto fd_clr = 0x80EE80FF;
        size_t i=0;
        for(const auto &cr_ptr:crs){
            auto &cr = *cr_ptr;
            auto txt_speed = std::to_string(cr.speed());
            auto txt_angle = std::to_string(cr.angle());
            auto x_ = cam.interp_x(cr.x());
            auto y_ = cam.interp_y(cr.y());
            drawList->AddCircle({x_, y_}, cr.radius(), cr_clr);
            drawList->AddLine({x_, y_}, {x_+cr.radius()*std::cos(cr.angle()), y_+cr.radius()*std::sin(cr.angle())}, cr_clr);
            i++;
        }
        for(const auto &fd:fds){
            auto x_ = cam.interp_x(fd.x());
            auto y_ = cam.interp_y(fd.y());
            drawList->AddCircle({x_, y_}, fd.radius, fd_clr);
        }
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
