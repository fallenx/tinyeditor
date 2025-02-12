#include "Model.hpp"

// THIS VERSION IS ABOUT PIECE TABLE implementation  ( (sort of :) ) //

#define _Cursor_Delay 450
#define _Tab_Width 4


typedef struct {int32_t x; int32_t y;} __cursor;
typedef struct {size_t offset; size_t length;} piece;


SDL_Surface* prepare_font_atlas(int font_size, int *w, int *h, int *font_ascent, std::unordered_map<std::string, __cursor> &p_font_map) {

    float dpi_v, dpi_h;
    SDL_GetDisplayDPI(0, NULL, &dpi_h, &dpi_v);
    TTF_Font *my_font = TTF_OpenFontDPI(".\\fonts\\consola.ttf", font_size, dpi_h, dpi_v);
    //TTF_Font *my_font = TTF_OpenFont(".\\fonts\\consola.ttf", font_size);

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
                int _cur_height,
                int x_offset,
                int y_offset,
                std::unordered_map<std::string, __cursor> &font_map,
                SDL_Surface *screen,
                SDL_Surface *font_atlas,
                Uint32 key_color
                ) {

    SDL_FillRect(screen, NULL, key_color);

    int line_offset = 7 * s_w;
    int line_offset_rect = 6 * s_w;

    int t_x = line_offset + x_offset, t_y = y_offset;
    bool render_batch = false;

    SDL_Surface *Cursor = SDL_CreateRGBSurface(0, s_w , _cur_height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_FillRect(Cursor, NULL, SDL_MapRGBA(Cursor->format, 29, 242, 10, 180));

    SDL_Rect line_rect = {0, 0, line_offset_rect, _height};

    SDL_FillRect(screen, &line_rect, SDL_MapRGBA(screen->format, 30, 32, 36, 255));


    size_t _line_count = my_table.line_map[my_table.head->offset];

    std::string _linec = std::to_string(_line_count);


    if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {

        render_batch = true;

        for(auto it = my_table.head; it != my_table.piece_map.end() && t_y < _height + _cur_height; ++it) {

            for(size_t Pos = 0; Pos < it->length; Pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos])) {

                if(it == my_table.batch_start.first && Pos == my_table.batch_start.second)
                    render_batch = false;

                if(!my_table.buffer.substr(it->offset + Pos, UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos])).compare("\r\n")){
                    t_y += _cur_height;
                    t_x = line_offset + x_offset;
                }else
                    t_x += s_w;
            }

            if(it == my_table.batch_start.first && it->length == my_table.batch_start.second)
                render_batch = false;

        }

        t_x = line_offset + x_offset; t_y = y_offset;

    }

    for(auto it = my_table.head; it != my_table.piece_map.end() && t_y < _height + _cur_height; ++it) {

        for(size_t Pos = 0; Pos < it->length; Pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos])) {

            std::string tt_me = my_table.buffer.substr(it->offset + Pos, UTF8_CHAR_LEN(my_table.buffer[it->offset + Pos]));

            if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {
                if(it == my_table.batch_start.first && Pos == my_table.batch_start.second)
                    render_batch = true;
                else if(it == my_table.batch_end.first && Pos == my_table.batch_end.second)
                    render_batch = false;
            }

            if(font_map.find(tt_me) != font_map.end() && t_x >= line_offset && t_x < _width && t_y >= 0){
                SDL_Rect temp_font = {(int) font_map[tt_me].x, 0, s_w, _cur_height};
                SDL_Rect temp_rect = {t_x, t_y, s_w, _cur_height};
                SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
            }


            if(render_batch && t_x >= line_offset && t_x < _width && t_y >= 0) {
                SDL_Rect temp_rect = {t_x, t_y, s_w, _cur_height};
                SDL_BlitSurface(Cursor, NULL, screen, &temp_rect);
            }


            if(!tt_me.compare("\r\n")){
                t_y += _cur_height;
                t_x = line_offset + x_offset;

                my_table.line_map[it->offset] = _line_count;
                _linec = std::to_string(++_line_count);

               // for(auto i : my_table.line_map)
               //     std::cout << "offset " << i.first << " " << i.second << "\n";
               // std::cout << "-----\n";

                for(size_t i = 0; i < _linec.size(); i++) {
                    SDL_Rect temp_font = {(int) font_map[_linec.substr(i,1)].x, 0, s_w, _cur_height};
                    SDL_Rect temp_rect = {(6 - (int) (_linec.size() - i)) * s_w, t_y, s_w, _cur_height};
                    SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
                }

            }else
                t_x += s_w;
        }

        if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {

            if(it == my_table.batch_start.first && it->length == my_table.batch_start.second)
                render_batch = true;
            else if(it == my_table.batch_end.first && it->length == my_table.batch_end.second)
                render_batch = false;
        }

    }

    while((t_y += _cur_height) < _height + _cur_height) {
        SDL_Rect temp_font = {(int) font_map["~"].x, 0, s_w, _cur_height};
        SDL_Rect temp_rect = {line_offset + x_offset, t_y, s_w, _cur_height};
        SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
    }


    SDL_FreeSurface(Cursor);

}

__cursor find_cursor(Model &my_table, int s_w, int _cur_height, int x_offset, int y_offset, int _width, int _height) {

    int line_offset = 7 * s_w;

    __cursor t_cr1 = {line_offset + x_offset, y_offset};

    if(my_table.piece_map.size()) {

        auto it = my_table.head;

        do{

            if(it == my_table.piece_map.end()) {
                t_cr1.y = _height + _cur_height;
                break;
            }

            size_t length = (it == my_table.it) ? (it->offset + my_table.Pos) : (it->offset + it->length);

            for(size_t Pos = it->offset; Pos < length; Pos += UTF8_CHAR_LEN(my_table.buffer[Pos])) {

                if(!my_table.buffer.substr(Pos, UTF8_CHAR_LEN(my_table.buffer[Pos])).compare("\r\n")){
                    t_cr1.y += _cur_height;
                    t_cr1.x = line_offset + x_offset; // to-do indentation to go back
                }else
                    t_cr1.x += s_w;
            }

        }while(it++ != my_table.it && t_cr1.y <= _height + _cur_height);

    }

    return t_cr1;
}


void find_page_head(Model &my_table, int _cur_height) {

    auto t = my_table.it;

    if(t != my_table.piece_map.begin()) {

        while(--t != my_table.piece_map.begin()) {

            if(!my_table.buffer.substr(t->offset, 2).compare("\r\n")) {
                my_table.head = t;
                break;
            }
        }

        if(t == my_table.piece_map.begin())
             my_table.head = my_table.piece_map.begin();

    }else
        my_table.head = my_table.piece_map.begin();

}

int page_upwards(Model &my_table, int _cur_height) {

    int t_cur = 0;

    if(my_table.head != my_table.piece_map.begin()) {

        auto t = my_table.head;

        while(--t != my_table.piece_map.begin()) {

            if(!my_table.buffer.substr(t->offset, 2).compare("\r\n")) {
                my_table.head = t;
                break;
            }
        }

        if(t == my_table.piece_map.begin())
            my_table.head = my_table.piece_map.begin();

        t_cur = _cur_height;
    }

    return t_cur;
}

void page_downwards(Model &my_table, int _cur_height) {

    auto t = my_table.head;

    while(++t != my_table.piece_map.end()) {

        if(!my_table.buffer.substr(t->offset, 2).compare("\r\n")) {
            my_table.head = t;
            break;
        }
    }
}

void find_x(__cursor *to_find, int *x_offset, int _width, int s_w) {

    int line_offset = 7 * s_w;

    if(to_find->x > (_width - s_w)) {
        while(to_find->x >= (_width >> 1)){
            to_find->x -= s_w;
            *x_offset -= s_w;
        }
    }else if(to_find->x < line_offset ) {
        while(*x_offset < 0 && to_find->x <= (_width >> 1)) {
            to_find->x += s_w;
            *x_offset += s_w;
        }
    }
}

bool batch_delete(Model &my_table, __cursor *cr1, int s_w, int _cur_height, int *x_offset, int y_offset, int _width, int _height, int t_height) {

    if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {


        my_table.it = my_table.batch_end.first;
        my_table.Pos = my_table.batch_end.second;

        auto it = my_table.batch_start.first;
        size_t batch_length = 0;

        do {
            size_t length = (it == my_table.batch_end.first) ? my_table.batch_end.second : it->length;
            size_t s_pos = (it == my_table.batch_start.first) ? my_table.batch_start.second : 0;
            for(size_t t_pos = s_pos; t_pos < length; t_pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + t_pos]))
                batch_length++;
        }while(it++ != my_table.batch_end.first);

        while(batch_length--)
            my_table.delete_text(false);

        __cursor to_find = find_cursor(my_table, s_w, _cur_height, *x_offset, y_offset, _width, _height);

        if(to_find.y > (t_height - _cur_height)) {

           find_page_head(my_table, _cur_height);

           to_find.y = -_cur_height;

           while(to_find.y < _height>>1 && my_table.head != my_table.piece_map.begin()) {
                to_find.y = page_upwards(my_table, _cur_height);
                to_find = find_cursor(my_table, s_w, _cur_height, *x_offset, y_offset, _width, _height);
           }

           to_find = find_cursor(my_table, s_w, _cur_height, *x_offset, y_offset, _width, _height);
        }

        find_x(&to_find, x_offset, _width, s_w);

        cr1->x = to_find.x;
        cr1->y = to_find.y;


        my_table.batch_start.first = my_table.it;
        my_table.batch_start.second = my_table.Pos;
        my_table.batch_end.first = my_table.it;
        my_table.batch_end.second = my_table.Pos;

        return true;
    }
    return false;
}

typedef struct {
    SDL_Window *_my_win;
    SDL_Surface *screen;
    Model &my_table;
    int *_width;
    int *_height;
    int s_w;
    int _cur_height;
    int *x_offset;
    int y_offset;
    std::unordered_map<std::string, __cursor> &font_map;
    SDL_Surface *font_atlas;
    Uint32 key_color;
}DATA, *PDATA;

int resizingEventWatcher(void* data, SDL_Event* event) {

  if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {

    SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);

    if (win == (SDL_Window*) ((PDATA) data)->_my_win) {

        *((PDATA) data)->_width = event->window.data1;
        *((PDATA) data)->_height = event->window.data2;

        SDL_FreeSurface(SDL_GetWindowSurface(win));

        SDL_SetWindowSize(win, event->window.data1, event->window.data2);

        ((PDATA) data)->screen = SDL_GetWindowSurface(win);

        _Render(((PDATA) data)->my_table, event->window.data1, event->window.data2, ((PDATA) data)->s_w, ((PDATA) data)->_cur_height,
                 *((PDATA) data)->x_offset, ((PDATA) data)->y_offset, ((PDATA) data)->font_map,
                SDL_GetWindowSurface(win), ((PDATA) data)->font_atlas, ((PDATA) data)->key_color);

        SDL_UpdateWindowSurface(win);

    }

  }

    return 0;
}

int SDL_main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    std::unordered_map<std::string, __cursor> font_map;
    bool ctrl_pressed = false, shift_pressed = false;
    int s_w, s_h, font_ascent, _width = 600, _height = 600, x_offset = 0;  // 117 15
    SDL_Surface *font_atlas = prepare_font_atlas(13, &s_w, &s_h, &font_ascent, font_map);
    int _cur_height = s_h + font_ascent, _cur_width = 1;
    int t_height = (_height / _cur_height) * _cur_height;
    //_width = (_width / s_w + 1) * s_w;

    int y_offset = -_cur_height;

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

    DATA my_data = {my_test, screen, my_table, &_width, &_height, s_w, _cur_height, &x_offset, y_offset, font_map, font_atlas, key_color};
    SDL_AddEventWatch(resizingEventWatcher, &my_data);


    //inserting the cursor

    __cursor cr1 = {7 * s_w, 0};


    SDL_Cursor* my_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    SDL_SetCursor(my_cursor);



    SDL_Event e = {};


    Uint64 last_time = SDL_GetTicks64(), start = 0, lastEvent_Timer= 0, lastEvent_Time = 0;
    int frames = 0;
    bool blink_on = true, lost_focus = false;


    SDL_Rect dest_rect = {cr1.x, cr1.y, _cur_width, _cur_height};
    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);
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

            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                my_table.batch_start = {my_table.it, my_table.Pos};
                my_table.batch_end = {my_table.it, my_table.Pos};
                shift_pressed = false;
            }

        }


        if(e.type == SDL_KEYDOWN) {

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                if(!ctrl_pressed)
                    ctrl_pressed = true;
            }

            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                if(!shift_pressed) {
                    my_table.batch_start = {my_table.it, my_table.Pos};
                    my_table.batch_end = {my_table.it, my_table.Pos};
                    shift_pressed = true;
                }
            }


            if(e.key.keysym.sym == SDLK_z) {
                if(ctrl_pressed) {
                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    my_table.undo();


                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    if(to_find.y > (t_height - _cur_height)) {

                       find_page_head(my_table, _cur_height);

                       to_find.y = -_cur_height;

                       while(to_find.y < _height>>1 && my_table.head != my_table.piece_map.begin()) {
                            to_find.y = page_upwards(my_table, _cur_height);
                            to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);
                       }

                       to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    }

                    find_x(&to_find, &x_offset, _width, s_w);

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

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

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    if(to_find.y > (t_height - _cur_height)) {

                       find_page_head(my_table, _cur_height);

                       to_find.y = -_cur_height;

                       while(to_find.y < _height>>1 && my_table.head != my_table.piece_map.begin()) {
                            to_find.y = page_upwards(my_table, _cur_height);
                            to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);
                       }

                       to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);
                    }

                    find_x(&to_find, &x_offset, _width, s_w);

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);


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

                batch_delete(my_table, &cr1, s_w, _cur_height, &x_offset, y_offset, _width, _height, t_height);

                my_table.insert_text("\r\n");

                __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                find_x(&to_find, &x_offset, _width, s_w);

                cr1.x = to_find.x;
                cr1.y = to_find.y;

                if(cr1.y > t_height - _cur_height) {

                    page_downwards(my_table, _cur_height);

                    cr1.y = t_height - _cur_height;
                }

                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_BACKSPACE){

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                int line_offset = 7 * s_w;

                if (!batch_delete(my_table, &cr1, s_w, _cur_height, &x_offset, y_offset, _width, _height, t_height)) {

                    if(!cr1.y && !x_offset &&  cr1.x == line_offset)
                        cr1.y = page_upwards(my_table, _cur_height);


                    if(my_table.delete_text(false)) {

                        __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                        find_x(&to_find, &x_offset, _width, s_w);

                        cr1.x = to_find.x;
                        cr1.y = to_find.y;

                    }
                }

                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }


            if(e.key.keysym.sym == SDLK_LEFT){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                int line_offset = 7 * s_w;
                bool is_continue = false;

                if(my_table.it == my_table.batch_start.first && my_table.Pos == my_table.batch_start.second)
                    is_continue = true;

                if(!cr1.y && !x_offset && cr1.x == line_offset)
                    cr1.y = page_upwards(my_table, _cur_height);

                if(my_table.left()){

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_start = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_end = {my_table.it, my_table.Pos};
                    }

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    find_x(&to_find, &x_offset, _width, s_w);

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

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

                bool is_continue = false;

                if(my_table.it == my_table.batch_end.first && my_table.Pos == my_table.batch_end.second)
                    is_continue = true;

                if(my_table.right()) {

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_end = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_start = {my_table.it, my_table.Pos};
                    }


                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    find_x(&to_find, &x_offset, _width, s_w);

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    if(cr1.y > t_height - _cur_height) {

                        page_downwards(my_table, _cur_height);

                        cr1.y = t_height - _cur_height;
                    }


                    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);
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

                bool is_continue = false;

                __cursor old_cr = {cr1.x, cr1.y};

                while(my_table.right()) {

                    my_table.left();
                    if(my_table.it == my_table.batch_end.first && my_table.Pos == my_table.batch_end.second)
                        is_continue = true;
                    my_table.right();

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_end = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_start = {my_table.it, my_table.Pos};
                    }

                    if(find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height).y - old_cr.y > _cur_height) {
                        my_table.left();

                        if(my_table.it == my_table.batch_end.first && my_table.Pos == my_table.batch_end.second)
                            is_continue = true;

                        if(shift_pressed) {
                            if(is_continue)
                                my_table.batch_end = {my_table.it, my_table.Pos};
                            else
                                my_table.batch_start = {my_table.it, my_table.Pos};
                        }
                        break;
                    }

                    cr1 = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    if(cr1.x == old_cr.x)
                        break;
                }

                find_x(&cr1, &x_offset, _width, s_w);

                if(cr1.y > t_height - _cur_height) {

                    page_downwards(my_table, _cur_height);

                    cr1.y = t_height - _cur_height;
                }

                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_PAGEDOWN){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                bool is_continue = false;

                __cursor old_cr = {cr1.x, cr1.y};
                bool page_passed = false;

                while(my_table.right()) {

                    my_table.left();
                    if(my_table.it == my_table.batch_end.first && my_table.Pos == my_table.batch_end.second)
                        is_continue = true;
                    my_table.right();

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_end = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_start = {my_table.it, my_table.Pos};
                    }

                    cr1 = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);


                    if(cr1.y > t_height - _cur_height) {

                        do {
                            page_downwards(my_table, _cur_height);
                        }while(find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height).y >= _cur_height);

                        page_passed = true;
                        cr1 = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);
                    }


                    if(page_passed && cr1.y > old_cr.y) {

                        my_table.left();

                        if(my_table.it == my_table.batch_end.first && my_table.Pos == my_table.batch_end.second)
                            is_continue = true;

                        if(shift_pressed) {
                            if(is_continue)
                                my_table.batch_end = {my_table.it, my_table.Pos};
                            else
                                my_table.batch_start = {my_table.it, my_table.Pos};
                        }

                        cr1 = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                        break;
                    }

                    if(page_passed && cr1.x == old_cr.x && cr1.y == old_cr.y)
                       break;

                }

                find_x(&cr1, &x_offset, _width, s_w);


                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_UP){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                bool is_continue = false;

                int line_offet = 7 * s_w;

                __cursor to_find = {line_offet, 0};


                if(!cr1.y)
                    cr1.y = page_upwards(my_table, _cur_height);


                while(my_table.left()) {

                    my_table.right();
                    if(my_table.it == my_table.batch_start.first && my_table.Pos == my_table.batch_start.second)
                        is_continue = true;
                    my_table.left();

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_start = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_end = {my_table.it, my_table.Pos};
                    }

                    to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    if(to_find.y < cr1.y && to_find.x <= cr1.x)
                        break;
                }

                find_x(&to_find, &x_offset, _width, s_w);


                cr1.x = to_find.x;
                cr1.y = to_find.y;

                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_PAGEUP){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                bool is_continue = false;
                bool page_passed = false;

                __cursor to_find = {cr1.x, cr1.y};

                int line_offset = 7 * s_w;


                while(my_table.left()) {

                    my_table.right();
                    if(my_table.it == my_table.batch_start.first && my_table.Pos == my_table.batch_start.second)
                        is_continue = true;
                    my_table.left();

                    if(!to_find.y && to_find.x == line_offset + x_offset) {

                        do {
                            if(!page_upwards(my_table, _cur_height))
                                break;

                        }while( find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height).y < t_height - _cur_height);


                        page_passed = true;
                    }

                    if(shift_pressed) {
                        if(is_continue)
                            my_table.batch_start = {my_table.it, my_table.Pos};
                        else
                            my_table.batch_end = {my_table.it, my_table.Pos};
                    }

                    to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);


                    if(page_passed && to_find.y == cr1.y && to_find.x <= cr1.x)
                        break;
                }

                find_x(&to_find, &x_offset, _width, s_w);


                cr1.x = to_find.x;
                cr1.y = to_find.y;

                _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);

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

            batch_delete(my_table, &cr1, s_w, _cur_height, &x_offset, y_offset, _width, _height, t_height);

            std::string t1 = e.text.text;

            size_t _count = 0;

            while(_count < t1.size()) {

                my_table.insert_text(t1.substr(_count, UTF8_CHAR_LEN(t1[_count])));
                _count += UTF8_CHAR_LEN(t1[_count]);
            }


            __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

            find_x(&to_find, &x_offset, _width, s_w);

            cr1.x = to_find.x;
            cr1.y = to_find.y;

            _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);


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

                    if(_height < 4 * _cur_height)
                        _height = 4 * _cur_height;

                    if(_width < 40 * s_w)
                        _width = 40 * s_w;

                    t_height = (_height / _cur_height) * _cur_height;

                    SDL_FreeSurface(screen);

                    SDL_SetWindowSize(my_test, _width, _height);

                    screen = SDL_GetWindowSurface(my_test);

                    __cursor to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    if(to_find.y > (t_height - _cur_height)) {

                        find_page_head(my_table, _cur_height);
                        to_find.y = -_cur_height;

                       while(to_find.y < _height>>1 && my_table.head != my_table.piece_map.begin()) {
                            to_find.y = page_upwards(my_table, _cur_height);
                            to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);
                       }

                        to_find = find_cursor(my_table, s_w, _cur_height, x_offset, y_offset, _width, _height);

                    }

                    find_x(&to_find, &x_offset, _width, s_w);

                    cr1.x = to_find.x;
                    cr1.y = to_find.y;

                    _Render(my_table, _width, _height, s_w, _cur_height, x_offset, y_offset, font_map, screen, font_atlas, key_color);


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

