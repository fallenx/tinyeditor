#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Model.hpp"

// THIS VERSION IS ABOUT PIECE TABLE implementation  ( (sort of :) ) //

#define _Cursor_Delay 450
#define _Tab_Width 4

inline int UTF8_CHAR_LEN(char byte) {return byte == 0x0d ? 2 : (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1; }

typedef struct {int32_t x; int32_t y;} __cursor;
typedef struct {size_t offset; size_t length;} piece;


SDL_Surface* prepare_font_atlas(int font_size, int *w, int *h, int *font_ascent, std::unordered_map<std::string, __cursor> &p_font_map) {

    float dpi_v, dpi_h;
    SDL_GetDisplayDPI(0, NULL, &dpi_h, &dpi_v);
    TTF_Font *my_font = TTF_OpenFontDPI(".\\fonts\\consola.ttf", font_size, dpi_h, dpi_v);

    std::string my_string = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    my_string += "ÇĞİÖŞÜçğıöşü"; /// Turkish
    my_string += "АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя"; /// Russian
    my_string += "«»’©"; /// Specials

    SDL_Surface *source = TTF_RenderUTF8_Blended(my_font, my_string.c_str(), {29, 242, 10});

    TTF_SizeUTF8(my_font, my_string.c_str(), w, h);

    uint32_t count = 0;
    int32_t main_cnt = 0;

    while(count < my_string.size()) {
        count += UTF8_CHAR_LEN(my_string[count]);
        main_cnt++;
    }

    *w /= main_cnt;

    *font_ascent = TTF_FontLineSkip(my_font) - *h;

    main_cnt = 0; count = 0;

    while(count < my_string.size()) {
        int new_count = UTF8_CHAR_LEN(my_string[count]);
        p_font_map[my_string.substr(count, new_count)] = {*w * main_cnt++, 0};
        count += new_count;
    }

    TTF_CloseFont(my_font);

    return source;
}


void _Render(
                Model &my_table,
                int _width,
                int _height,
                int s_w,
                int s_h,
                int x_offset,
                int y_offset,
                int font_ascent,
                std::unordered_map<std::string, __cursor> &font_map,
                SDL_Surface *screen,
                SDL_Surface *font_atlas,
                Uint32 key_color
                ) {

    SDL_FillRect(screen, NULL, key_color);

    int t_x = x_offset, t_y = y_offset;



    for(auto it = my_table.piece_map.begin(); it != my_table.piece_map.end(); ++it) {

        for(size_t Pos = 0; Pos < it->length; Pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos])) {

            std::string tt_me = my_table.buffer.substr(it->offset + Pos, UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos]));

            if(font_map.find(tt_me) != font_map.end() && t_x >= 0 && t_x < _width){
                SDL_Rect temp_font = {(int) font_map[tt_me].x, 0, s_w, s_h};
                SDL_Rect temp_rect = {t_x, t_y + font_ascent, s_w, s_h};
                SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
            }

            if(!tt_me.compare("\r\n")){
                t_y += (s_h + font_ascent);
                t_x = x_offset;
            }else
                t_x += s_w;
        }

    }

}

__cursor find_cursor(Model &my_table, int s_w, int _cur_height, int *x_offset, int *y_offset, int _width, int _height) {

    __cursor t_cr1 = {*x_offset, *y_offset};

    if(my_table.piece_map.size()) {

        auto it = my_table.piece_map.begin();

        do{

            size_t length = (it == my_table.it) ? (it->offset + my_table.Pos) : (it->offset + it->length);

            for(size_t Pos = it->offset; Pos < length; Pos += UTF8_CHAR_LEN(my_table.buffer[Pos])) {

                if(!my_table.buffer.substr(Pos, UTF8_CHAR_LEN(my_table.buffer[Pos])).compare("\r\n")){
                    t_cr1.y += _cur_height;
                    t_cr1.x = *x_offset; // to-do indentation to go back
                }else
                    t_cr1.x += s_w;
            }

        }while(it++ != my_table.it);
    }

    return t_cr1;
}

int SDL_main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    std::unordered_map<std::string, __cursor> font_map;
    bool ctrl_pressed = false;
    int s_w, s_h, font_ascent, _width = 600, _height = 600, x_offset = 0, y_offset = 0;   // 117 15
    SDL_Surface *font_atlas = prepare_font_atlas(14, &s_w, &s_h, &font_ascent, font_map);
    int _cur_height = s_h + font_ascent, _cur_width = s_w;
    _height = (_height / (s_h + font_ascent) + 1) * (s_h + font_ascent);
    _width = (_width / s_w + 1) * s_w;

    //size_t Pos = 0;

    SDL_Window *my_test = SDL_CreateWindow(
        "Test Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        _width,
        _height,
        SDL_WINDOW_RESIZABLE
        );

    // Create Cursor

    SDL_EnableScreenSaver();
    SDL_Surface *screen = SDL_GetWindowSurface(my_test);

    SDL_Surface *Cursor = SDL_CreateRGBSurface(0, _cur_width , _cur_height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_Surface *Temp_Surface = SDL_CreateRGBSurfaceWithFormat(0, Cursor->w , Cursor->h, 32, screen->format->format);


    Uint32 Cursor_Color = SDL_MapRGBA(Cursor->format, 29, 242, 10, 180);

    SDL_FillRect(Cursor, NULL, Cursor_Color);


    Uint32 key_color = SDL_MapRGBA(screen->format, 0, 0, 0, 255);
    SDL_FillRect(screen, NULL, key_color);

    Model my_table;

    //inserting the cursor

    __cursor cr1 = {0, 0};


    SDL_Cursor* my_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    SDL_SetCursor(my_cursor);



    SDL_Event e = {};


    Uint64 last_time = SDL_GetTicks64(), start = 0, lastEvent_Timer= 0, lastEvent_Time = 0;
    int frames = 0;
    bool blink_on = true, lost_focus = false;


    SDL_Rect dest_rect = {cr1.x, cr1.y, _cur_width, _cur_height};
    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);
    SDL_UpdateWindowSurface(my_test);

    for(;e.type != SDL_QUIT; SDL_PollEvent(&e)) {

        if((start = SDL_GetTicks64()) > last_time + _Cursor_Delay) {
            last_time = start;
            frames = 0;
            if (blink_on){
                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                SDL_BlitSurface(screen, &temp_rect, Temp_Surface, NULL);
                SDL_BlitSurface(Cursor, NULL, screen, &temp_rect);

                blink_on = false;

                SDL_UpdateWindowSurface(my_test);
            }
            else {
                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);

                blink_on = true;

                SDL_UpdateWindowSurface(my_test);
            }
        }

        lastEvent_Time = lost_focus ? lastEvent_Time : start;
        frames++;

        if(e.type == SDL_KEYUP) {

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL)
                ctrl_pressed = false;

        }


        if(e.type == SDL_KEYDOWN) {

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                if(!ctrl_pressed)
                    ctrl_pressed = true;
            }


            if(e.key.keysym.sym == SDLK_z) {
                if(ctrl_pressed) {
                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    my_table.undo();

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.x > (_width - s_w)) {
                        while(to_find.x >= (_width >> 1)){
                            to_find.x -= s_w;
                            x_offset -= s_w;
                        }
                    }else if(to_find.x < 0) {
                        while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                            to_find.x += s_w;
                            x_offset += s_w;
                        }
                    }

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;
                }
            }

            if(e.key.keysym.sym == SDLK_r) {
                if(ctrl_pressed) {
                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    my_table.redo();

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.x > (_width - s_w)) {
                        while(to_find.x >= (_width >> 1)){
                            to_find.x -= s_w;
                            x_offset -= s_w;
                        }
                    }else if(to_find.x < 0) {
                        while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                            to_find.x += s_w;
                            x_offset += s_w;
                        }
                    }

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);


                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;
                }
            }


            if(e.key.keysym.sym == SDLK_RETURN) {

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                my_table.insert_text("\r\n");

                __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                if(to_find.x > (_width - s_w)) {
                    while(to_find.x >= (_width >> 1)){
                        to_find.x -= s_w;
                        x_offset -= s_w;
                    }
                }else if(to_find.x < 0) {
                    while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                        to_find.x += s_w;
                        x_offset += s_w;
                    }
                }

                cr1.x = to_find.x;
                cr1.y = to_find.y;

                _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_BACKSPACE){

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(my_table.delete_text(false)) {

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.x > (_width - s_w)) {
                        while(to_find.x >= (_width >> 1)){
                            to_find.x -= s_w;
                            x_offset -= s_w;
                        }
                    }else if(to_find.x < 0) {
                        while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                            to_find.x += s_w;
                            x_offset += s_w;
                        }
                    }

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }


            if(e.key.keysym.sym == SDLK_LEFT){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(my_table.left()){
                    int old_x_offset = x_offset;

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.x > (_width - s_w)) {
                        while(to_find.x >= (_width >> 1)){
                            to_find.x -= s_w;
                            x_offset -= s_w;
                        }
                    }else if(to_find.x < 0) {
                        while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                            to_find.x += s_w;
                            x_offset += s_w;
                        }
                    }

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    if(old_x_offset != x_offset)
                        _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                }

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_RIGHT){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(my_table.right()) {
                    int old_x_offset = x_offset;

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.x > (_width - s_w)) {
                        while(to_find.x >= (_width >> 1)){
                            to_find.x -= s_w;
                            x_offset -= s_w;
                        }
                    }else if(to_find.x < 0) {
                        while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                            to_find.x += s_w;
                            x_offset += s_w;
                        }
                    }

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    if(old_x_offset != x_offset)
                        _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_DOWN){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                int old_x_offset = x_offset;
                __cursor to_find = {cr1.x, cr1.y};

                while(my_table.right()) {

                    to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.y > cr1.y) {

                        if(to_find.x >= cr1.x)
                            break;

                        __cursor to_find_2 = {to_find.x, to_find.y};

                        while(my_table.right()) {

                            to_find_2 = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                            if(to_find_2.y > to_find.y) {
                                my_table.left();
                                to_find_2 = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);
                                break;
                            }

                            if(to_find_2.x >= cr1.x)
                                break;

                        }

                        to_find.x = to_find_2.x;
                        to_find.y = to_find_2.y;
                        break;
                    }
                }

                if(to_find.x > (_width - s_w)) {
                    while(to_find.x >= (_width >> 1)){
                        to_find.x -= s_w;
                        x_offset -= s_w;
                    }
                }else if(to_find.x < 0) {
                    while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                        to_find.x += s_w;
                        x_offset += s_w;
                    }
                }

                cr1.x = to_find.x;
                cr1.y = to_find.y;


                if(old_x_offset != x_offset)
                    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }



            if(e.key.keysym.sym == SDLK_UP){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                int old_x_offset = x_offset;
                __cursor to_find = {};

                while(my_table.left()) {

                    to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

                    if(to_find.y < cr1.y && to_find.x <= cr1.x)
                        break;
                }

                if(to_find.x > (_width - s_w)) {
                    while(to_find.x >= (_width >> 1)){
                        to_find.x -= s_w;
                        x_offset -= s_w;
                    }
                }else if(to_find.x < 0) {
                    while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                        to_find.x += s_w;
                        x_offset += s_w;
                    }
                }

                cr1.x = to_find.x;
                cr1.y = to_find.y;


                if(old_x_offset != x_offset)
                    _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

        }

        if(e.type == SDL_TEXTINPUT) {

            if(!blink_on) {
                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
            }

            std::string t1 = e.text.text;

            size_t _count = 0;

            while(_count < t1.size()) {

                my_table.insert_text(t1.substr(_count, UTF8_CHAR_LEN(t1[_count])));
                _count += UTF8_CHAR_LEN(t1[_count]);
            }


            __cursor to_find = find_cursor(my_table, s_w, _cur_height, &x_offset, &y_offset, _width, _height);

            if(to_find.x > (_width - s_w)) {
                while(to_find.x >= (_width >> 1)){
                    to_find.x -= s_w;
                    x_offset -= s_w;
                }
            }else if(to_find.x < 0) {
                while(x_offset < 0 && to_find.x <= (_width >> 1)) {
                    to_find.x += s_w;
                    x_offset += s_w;
                }
            }

            cr1.x = to_find.x;
            cr1.y = to_find.y;


            _Render(my_table, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, font_map, screen, font_atlas, key_color);


            dest_rect.x = cr1.x; dest_rect.y = cr1.y;
            last_time = SDL_GetTicks64() -_Cursor_Delay;
            blink_on = true;
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

                    if(!blink_on) {
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() -_Cursor_Delay;
                    blink_on = true;

                    break;
            }


        }

        if(lost_focus) {
            if((lastEvent_Timer = SDL_GetTicks64()) > lastEvent_Time + 3000 && !blink_on) {
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

    SDL_FreeSurface(font_atlas);
    SDL_FreeSurface(Cursor);
    SDL_FreeSurface(Temp_Surface);
    SDL_DestroyWindow(my_test);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}

