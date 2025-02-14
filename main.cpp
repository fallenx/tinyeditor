#include "Buffer.hpp"


int SDL_main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();

    int _width = 600; int _height = 600;

    SDL_Window *my_test = SDL_CreateWindow(
        "Test Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        _width,
        _height,
        SDL_WINDOW_RESIZABLE
    );

    SDL_EnableScreenSaver();
    SDL_Surface *screen = SDL_GetWindowSurface(my_test);

    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));


    SDL_Cursor* my_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    SDL_SetCursor(my_cursor);


    Buffer my_first("Deneme", _width, _height);

    SDL_Event e = {};


    Uint64 last_time = SDL_GetTicks64(), start = 0, lastEvent_Timer= 0, lastEvent_Time = 0;
    int frames = 0;
    bool lost_focus = false;

    my_first.Render();

    SDL_BlitSurface(my_first.Get_Screen(), NULL, screen, NULL);

    SDL_UpdateWindowSurface(my_test);

    for(;e.type != SDL_QUIT; SDL_PollEvent(&e)) {

        if((start = SDL_GetTicks64()) > last_time + _Cursor_Delay) {
            last_time = start;
            frames = 0;

            my_first.Render();
            my_first.Cursor_Blink();

            SDL_BlitSurface(my_first.Get_Screen(), NULL, screen, NULL);
            SDL_UpdateWindowSurface(my_test);
        }

        lastEvent_Time = lost_focus ? lastEvent_Time : start;
        frames++;

        if(e.type == SDL_KEYUP) {

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL)
                my_first.set_ctrl(false);


            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                my_first.set_shift(false);


        }


        if(e.type == SDL_KEYDOWN) {

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                if(!my_first.get_ctrl())
                    my_first.set_ctrl(true);

            }

            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                if(!my_first.get_shift())
                    my_first.set_shift(true);

            }


            if(e.key.keysym.sym == SDLK_z) {
                if(my_first.get_ctrl()) {

                    my_first.Undo();
                    last_time = SDL_GetTicks64() - _Cursor_Delay;

                }
            }

            if(e.key.keysym.sym == SDLK_r) {
                if(my_first.get_ctrl()) {

                    my_first.Redo();
                    last_time = SDL_GetTicks64() - _Cursor_Delay;

                }
            }

            if(e.key.keysym.sym == SDLK_x) {

                if(my_first.get_ctrl()) {

                    my_first.Cut();
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                }

            }

            if(e.key.keysym.sym == SDLK_c) {

                if(my_first.get_ctrl()) {

                    my_first.Copy();
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                }

            }

            if(e.key.keysym.sym == SDLK_v) {

                if(my_first.get_ctrl()) {

                    my_first.Paste();
                    last_time = SDL_GetTicks64() - _Cursor_Delay;

                }

            }

            if(e.key.keysym.sym == SDLK_TAB) {

                my_first.Tab();
                last_time = SDL_GetTicks64() - _Cursor_Delay;

            }


            if(e.key.keysym.sym == SDLK_RETURN) {

                my_first.Return();
                last_time = SDL_GetTicks64() - _Cursor_Delay;

            }

            if(e.key.keysym.sym == SDLK_BACKSPACE){

                my_first.Back_Space();
                last_time = SDL_GetTicks64() - _Cursor_Delay;
            }


            if(e.key.keysym.sym == SDLK_LEFT){

                my_first.Left();
                last_time = SDL_GetTicks64() - _Cursor_Delay;
            }

            if(e.key.keysym.sym == SDLK_RIGHT){

                my_first.Right();
                last_time = SDL_GetTicks64() - _Cursor_Delay;
            }

            if(e.key.keysym.sym == SDLK_DOWN){

                my_first.Down();
                last_time = SDL_GetTicks64() - _Cursor_Delay;

            }

            if(e.key.keysym.sym == SDLK_PAGEDOWN){

                my_first.PageDown();
                last_time = SDL_GetTicks64() - _Cursor_Delay;
            }

            if(e.key.keysym.sym == SDLK_UP){

                my_first.Up();
                last_time = SDL_GetTicks64() - _Cursor_Delay;

            }

            if(e.key.keysym.sym == SDLK_PAGEUP){

                my_first.PageUp();
                last_time = SDL_GetTicks64() - _Cursor_Delay;

            }

        }

        if(e.type == SDL_TEXTINPUT) {

            std::string t1 = e.text.text;
            my_first.Text_Input(t1);
            last_time = SDL_GetTicks64() -_Cursor_Delay;
        }


        if(e.type == SDL_WINDOWEVENT) {

            switch (e.window.event) {

                case SDL_WINDOWEVENT_FOCUS_LOST:
                    lost_focus = true;
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    lost_focus = false;
                    break;
                case SDL_WINDOWEVENT_RESIZED:

                    _width = e.window.data1;
                    _height = e.window.data2;

                    if(_height < 100)
                        _height = 100;

                    if(_width < 400)
                        _width = 400;

                    SDL_FreeSurface(screen);
                    SDL_SetWindowSize(my_test, _width, _height);
                    screen = SDL_GetWindowSurface(my_test);

                    my_first.Resize_Buffer(_width, _height);
                    //my_first.Render();
                    last_time = SDL_GetTicks64() -_Cursor_Delay;

                    break;
            }


        }

        if(lost_focus) {
            if((lastEvent_Timer = SDL_GetTicks64()) > lastEvent_Time + 3000 && !my_first.Get_Blink()) {
                while(SDL_WaitEvent(&e)) {
                    if(e.type == SDL_WINDOWEVENT){
                        if(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
                            lost_focus = false;
                            break;
                        }
                    }
                }
            }
        }

        SDL_Delay(1);
    }


    SDL_DestroyWindow(my_test);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}

