#include <iostream>
#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// THIS VERSION IS ABOUT BUFFER OPTIMIZATION //

#define _Cursor_Delay 350
#define _Tab_Width 4

typedef struct {int32_t x; int32_t y;} __cursor;
//typedef struct {int32_t x; int32_t y; size_t _pos;} ret_cur;
//typedef struct {size_t _start; size_t _end; std::string t_string;} _span;
typedef struct {size_t _start; size_t _end; bool append;} _span2;
//typedef struct {std::string _str; int16_t x; int16_t y; size_t p_loc;} p_cursor;
//typedef struct {int16_t x; int16_t y; std::string ch; bool new_line;} __cursor1;
typedef struct {size_t pos; std::string to_put;} to_insert;

SDL_Surface* prepare_font_atlas(int font_size, int *w, int *h, std::unordered_map<std::string, __cursor> &p_font_map) {

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

    TTF_Font *my_font = TTF_OpenFont(".\\bin\\fonts\\consola.ttf", font_size);

    std::u32string my_string = U" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÇĞİÖŞÜçğıöşü";

    SDL_Surface *source = TTF_RenderUTF8_Blended(my_font, cvt.to_bytes(my_string).c_str(), {230, 230, 230});

    *h = source->h; *w = source->w / my_string.size();
    int count = 0;

    for(auto i : my_string) {
        p_font_map[cvt.to_bytes(i)] = {*w * count++, *w};
    }

    TTF_CloseFont(my_font);

    return source;
}

void _Render(
                std::string &per_buffer,
                std::string &new_buffer,
                std::list<_span2> &char_map1,
                std::list<_span2>::iterator it_cp,
                int _width,
                int _height,
                int s_w,
                int s_h,
                int *x_offset,
                int *y_offset,
                size_t char_pos,
                std::unordered_map<std::string, __cursor> &font_map,
                SDL_Surface *screen,
                SDL_Surface *font_atlas,
                Uint32 key_color
                ) {

    SDL_FillRect(screen, NULL, key_color);

    int16_t t_x = char_pos, t_y = *y_offset;
    std::list<_span2>::iterator temp_it = it_cp;

    while(++temp_it != char_map1.end() && t_y < _height + s_h) {

        size_t str_count = temp_it == it_cp && char_pos ? char_pos + temp_it->_start : temp_it->_start;

        while(str_count < temp_it->_end && t_y < _height + s_h) {

            std::string tt_me;
            std::string &_lkup = temp_it->append ? new_buffer : per_buffer;
            char _ch = _lkup[str_count++];
            tt_me = _ch;


            if((_ch & 0xE0) == 0xC0) {
                tt_me += _lkup[str_count++];
            }else if((_ch & 0xF0) == 0xE0){
                tt_me += _lkup[str_count++];
                tt_me += _lkup[str_count++];
            }else if((_ch & 0xF8) == 0xF0){
                tt_me += _lkup[str_count++];
                tt_me += _lkup[str_count++];
                tt_me += _lkup[str_count++];
            }


            if(font_map.find(tt_me) != font_map.end()){
                SDL_Rect temp_font = {(int) font_map[tt_me].x, 0, s_w, s_h};
                SDL_Rect temp_rect = {t_x, t_y, s_w, s_h};
                SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
            }


            if(!tt_me.compare("\n")){
                t_x = *x_offset;
                t_y += s_h;
            }else if(!tt_me.compare("\t")){

                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( t_x > _width - s_w ) {
                    t_x = t_x + *x_offset - (_width - s_w);
                    t_y += s_h;
                }
            }else {
                t_x += s_w;
                if( t_x > _width - s_w) {
                    t_x = *x_offset;
                    t_y += s_h;
                }
            }
        }
    }


}


int SDL_main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    std::unordered_map<std::string, __cursor> font_map;
    size_t char_pos = 0, new_pos = 0;

    int s_w, s_h, y_offset = 0, _width = 110, _height = 200;//_cur_width = 2;
    SDL_Surface *font_atlas = prepare_font_atlas(18, &s_w, &s_h, font_map);
    int _cur_height = s_h, _cur_width = 2;
    _height = (_height / s_h + 1) * s_h;
    _width = (_width / s_w + 1) * s_w;
    int x_offset = 0; //(s_w + 5) * _Tab_Width;
    bool shift_pressed = false;
    bool shift_was_pressed = false;

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

    Uint32 key_color = SDL_MapRGBA(screen->format, 20, 20, 20, 255);
    //SDL_GetColorKey(screen, &key_color);
    SDL_FillRect(screen, NULL,key_color);

    std::list<_span2> clip_board;
    std::list<_span2> del_me2;
    std::string per_buffer;
    std::string new_buffer;

    if(argc > 1) {
        std::ifstream ofs(argv[1]);
        std::basic_stringstream<char> test_2(std::stringstream::in | std::stringstream::out);

        if(ofs.is_open()) {
            test_2 << ofs.rdbuf();
            per_buffer = test_2.str();
            std::cout << "size of the buffer : " << per_buffer.size() << std::endl;
            del_me2.push_back({0, per_buffer.size(), false});
            ofs.close();
        }

    }

    std::list<_span2>::iterator it_cp = del_me2.begin();
    std::list<_span2>::iterator test_cp = del_me2.insert(it_cp, {0, 0, false});
    size_t off_char = 0;
    std::list<_span2>::iterator sel_cp = test_cp;
    std::list<_span2>::iterator sel_end_cp = test_cp;
    size_t sel_char = 0;
    size_t sel_end_char = 0;


    //inserting the cursor


    //SDL_BlitSurface(font_atlas, NULL, screen, NULL);


    SDL_Event e = {};

    __cursor cr1 = {x_offset, y_offset}, pcr1{0, 0};
    Uint64 last_time = SDL_GetTicks64(), start = 0, lastEvent_Timer= 0, lastEvent_Time = 0;
    int frames = 0;
    bool blink_on = true, lost_focus = false;


    SDL_Rect dest_rect = {cr1.x, cr1.y, _cur_width, _cur_height};
    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, 0, font_map, screen, font_atlas, key_color);
    SDL_UpdateWindowSurface(my_test);

    for(;e.type != SDL_QUIT; SDL_PollEvent(&e)) {

        if((start = SDL_GetTicks64()) > last_time + _Cursor_Delay) {
            last_time = start;
            frames = 0;
            if (blink_on){
                //Inverting the colors of the particular pixels
                Uint32 *_pixels = (Uint32*) screen->pixels;
                for(int i = 0; i < _cur_height; i++)
                    for(int j = 0; j < _cur_width; j++)
                        _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;

                blink_on = false;
                SDL_UpdateWindowSurface(my_test);
            }
            else {
                Uint32 *_pixels = (Uint32*) screen->pixels;
                for(int i = 0; i < _cur_height; i++)
                    for(int j = 0; j < _cur_width; j++)
                        _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;

                blink_on = true;
                SDL_UpdateWindowSurface(my_test);
            }
        }

        lastEvent_Time = lost_focus ? lastEvent_Time : start;
        frames++;
        if(e.type == SDL_KEYDOWN) {
            if(e.key.keysym.sym == SDLK_RETURN) {

                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                if(char_pos > 0){
                    del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, false});
                    it_cp->_start = char_pos + it_cp->_start;
                    char_pos = 0;
                }
                new_buffer += '\n';
                del_me2.insert(it_cp, {new_buffer.size() - 1, new_buffer.size(), true});

                cr1.x = x_offset;
                cr1.y += s_h;

                if(cr1.y > _height - s_h){
                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = off_char;
                    bool is_break = false;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                off_char = x_offset;
                                is_break = true;
                                break;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    off_char = t_x + x_offset - (_width - s_w);
                                    is_break = true;
                                    break;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    off_char = x_offset;
                                    is_break = true;
                                    break;
                                }
                            }
                        }
                        if(is_break) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);
                            x_offset = 0;

                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }

                            break;

                        }
                    }


                    cr1.y = _height - s_h;

                }else
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_BACKSPACE){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                bool isn_append = false;
                std::list<_span2>::iterator tt_it = del_me2.end();
                std::list<_span2>::iterator temp_it = test_cp;
                cr1.x = x_offset; cr1.y = y_offset;

                if(!it_cp->append && char_pos) {
/*
                    for(auto i : del_me2)
                        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                    std::cout << "for the first_1*******\n";
*/
                    size_t old_char = char_pos;

                    if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                        while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);

                    size_t temp_end = it_cp->_end;
                    it_cp->_end = char_pos + it_cp->_start;
                    size_t temp_start = old_char + it_cp->_start;

                    if(it_cp->_start == it_cp->_end) {
                        //std::cout << "am i called?\n";
                        if(temp_start == temp_end)
                            it_cp = del_me2.erase(it_cp);
                        else {
                            it_cp->_start = temp_start;
                            it_cp->_end = temp_end;
                        }
                    }else if(temp_start != temp_end){
                        it_cp = del_me2.insert(++it_cp, {temp_start, temp_end, false});
                        char_pos = 0;
                    }else {
                        std::cout << "triggered\n";
                        tt_it = ++it_cp;
                        char_pos = 0;
                    }

                    //char_pos = 0;

                    //std::cout << char_pos << " " << it_cp->_end << std::endl;


/*
                    for(auto i : del_me2)
                        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                    std::cout << "for the first_2*******\n";
*/

                }else if(--it_cp != test_cp){
                        if(!it_cp->append){
                            char_pos = it_cp->_end - it_cp->_start;
                            size_t old_char = char_pos;
                            if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                            //if(char_pos > 0){
                                size_t temp_end = it_cp->_end;
                                it_cp->_end = char_pos + it_cp->_start;
                                size_t temp_start = old_char + it_cp->_start;

                                if(it_cp->_start == it_cp->_end) {
                                    //std::cout << "am i being called?\n";
                                    if(temp_start == temp_end)
                                        it_cp = del_me2.erase(it_cp);
                                    else {
                                        it_cp->_start = temp_start;
                                        it_cp->_end = temp_end;
                                    }
                                }else if(temp_start != temp_end){
                                    it_cp = del_me2.insert(++it_cp, {temp_start, temp_end, false});
                                    char_pos = 0;
                                }else {
                                    //isn_append = true;
                                    tt_it = ++it_cp;
                                    char_pos = 0;
                                }

                                //std::cout << char_pos << " " << it_cp->_end << std::endl;

/*
                                for(auto i : del_me2)
                                    std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                                std::cout << "for the second*******\n";
*/

                            //}

                        }else {
                            it_cp = del_me2.erase(it_cp);
                            char_pos = 0;
                        }

                }else {

                    if(test_cp != del_me2.begin()){

                        test_cp = del_me2.erase(test_cp);
                        //std::list<_span2>::iterator t__t = test_cp;
/*
                        if(!it_cp->append && char_pos){

                            it_cp = t__t;
                            size_t old_char = char_pos;
                            if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);

                            size_t temp_end = it_cp->_end;
                            it_cp->_end = char_pos + it_cp->_start;
                            size_t temp_start = old_char + it_cp->_start;

                            if(it_cp->_start == it_cp->_end) {
                                //std::cout << "am i called?\n";
                                if(temp_start == temp_end)
                                    it_cp = del_me2.erase(it_cp);
                                else {
                                    it_cp->_start = temp_start;
                                    it_cp->_end = temp_end;
                                }
                            }else if(temp_start != temp_end){
                                it_cp = del_me2.insert(++it_cp, {temp_start, temp_end, false});
                                char_pos = 0;
                            }else {
                                std::cout << "triggered\n";
                                tt_it = ++it_cp;
                                char_pos = 0;
                            }


                        }else */
                        if(!(--it_cp)->append){


                            char_pos = it_cp->_end - it_cp->_start;
                            size_t old_char = char_pos;
                            if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);

                            size_t temp_end = it_cp->_end;
                            it_cp->_end = char_pos + it_cp->_start;
                            size_t temp_start = old_char + it_cp->_start;

                            if(it_cp->_start == it_cp->_end) {
                                //std::cout << "am i being called?\n";
                                if(temp_start == temp_end)
                                    it_cp = del_me2.erase(it_cp);
                                else {
                                    it_cp->_start = temp_start;
                                    it_cp->_end = temp_end;
                                }
                            }else if(temp_start != temp_end){
                                it_cp = del_me2.insert(++it_cp, {temp_start, temp_end, false});
                                char_pos = 0;
                            }else {
                                //isn_append = true;
                                tt_it = ++it_cp;
                                char_pos = 0;
                            }

                        }else {
                            it_cp = del_me2.erase(it_cp);
                            char_pos = 0;
                        }
                            // THIS IS WHERE THE TRICK BEGINS

                            std::list<_span2>::iterator test_this = test_cp;

                            bool is_break = false;

                            while(--test_this != del_me2.begin()) {

                                size_t x_start_pos = test_this->_end; //(!th_count++) ? test_this->_start + char_pos : test_this->_end;
                                size_t tri_count = 0;


                                while(x_start_pos > test_this->_start) {

                                    if(!test_this->append){
                                        tri_count++;
                                        if((per_buffer[--x_start_pos] & 0xFF) > 0x7F){
                                            tri_count++;
                                            while((per_buffer[--x_start_pos] & 0xE0) != 0xC0)
                                                tri_count++;
                                        }

                                        if(per_buffer[x_start_pos] == '\n'){

                                            del_me2.insert(test_this, {test_this->_start, x_start_pos + 1, false});
                                            test_this->_start = x_start_pos + 1;

                                            if(test_this->_start == test_this->_end)
                                                test_this = del_me2.erase(test_this);

                                            test_cp = del_me2.insert(test_this, {0, 0, false});
                                            temp_it = test_cp;

                                            if(test_this == tt_it)
                                                char_pos = tri_count - 1;

                                            is_break = true;
                                            break;
                                        }

                                    }else {

                                        if((new_buffer[--x_start_pos] & 0xFF) > 0x7F)
                                            while((new_buffer[--x_start_pos] & 0xE0) != 0xC0);

                                        if(new_buffer[x_start_pos] == '\n'){

                                            test_cp = del_me2.insert(++test_this, {0, 0, false});

                                            temp_it = test_cp;
                                            is_break = true;
                                            break;
                                        }
                                    }

                                }

                                if(is_break){
                                    off_char = x_offset;
                                    break;
                                }
                            }



                            if(test_this == del_me2.begin()){
                                test_cp = del_me2.begin();
                                temp_it = test_cp;
                            }

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                    }else
                        ++it_cp;
                }



                while(++temp_it != it_cp) {

                    size_t start_pos = 0;

                    size_t end_pos = (temp_it == tt_it) ? char_pos : temp_it->_end - temp_it->_start;


                    while(start_pos < end_pos) {

                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];
                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }

                    }
                }

                if(isn_append)
                    --it_cp;

                if(cr1.y > _height - s_h){

                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = x_offset;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                t_x = x_offset;
                                cr1.y -= s_h;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x = t_x + x_offset - (_width - s_w);
                                    cr1.y -= s_h;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    cr1.y -= s_h;
                                }
                            }

                            if(cr1.y < _height)
                                break;
                        }
                        if(cr1.y < _height) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                //if(temp_it == it_cp)
                                //    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});


                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }
/*
                            for(auto i : del_me2)
                                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                            std::cout << "*******\n";
*/

                            break;
                        }
                    }

                }else
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_DELETE){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                if(it_cp != del_me2.end()){

                    if(!it_cp->append) {
                        size_t temp_start = it_cp->_start;
                        size_t temp_end = it_cp->_start + char_pos;
                        char _ch = per_buffer[char_pos++ + it_cp->_start];
                        if((_ch & 0xE0) == 0xC0)
                            char_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            char_pos++;
                            char_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            char_pos++;
                            char_pos++;
                            char_pos++;
                        }

                        if(it_cp->_start + char_pos < it_cp->_end) {
                            it_cp->_start = it_cp->_start + char_pos;

                            if(temp_start != temp_end)
                                del_me2.insert(it_cp, {temp_start, temp_end, 0});

                        }else {
                                it_cp->_end = temp_end;
                                if(it_cp->_start == it_cp->_end)
                                    it_cp = del_me2.erase(it_cp);
                                else
                                    it_cp++;
                        }

                        char_pos = 0;

                    }else {
                        it_cp = del_me2.erase(it_cp);
                        char_pos = 0;
                    }

                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                }

                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_TAB){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                if(char_pos > 0){
                    del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, false});
                    it_cp->_start = char_pos + it_cp->_start;
                    char_pos = 0;
                }
                new_buffer += '\t';
                del_me2.insert(it_cp, {new_buffer.size() - 1, new_buffer.size(), true});

                cr1.x = (((cr1.x / s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( cr1.x  > _width - s_w) {
                    cr1.x = cr1.x + x_offset - (_width - s_w);
                    cr1.y += s_h;
                }


                if(cr1.y > _height - s_h){
                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = off_char;
                    bool is_break = false;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                off_char = x_offset;
                                is_break = true;
                                break;
                            }else if(_ch == '\t'){

                                t_x = ((( t_x / s_w ) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    off_char = t_x + x_offset - (_width - s_w);
                                    is_break = true;
                                    break;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    off_char = x_offset;
                                    is_break = true;
                                    break;
                                }
                            }
                        }
                        if(is_break) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }

                            break;

                        }
                    }


                    cr1.y = _height - s_h;

                }else
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_LEFT){

                if(!blink_on){
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                if(SDL_GetModState() == KMOD_RSHIFT) {

                    if(!shift_was_pressed){
                        sel_cp = it_cp;
                        sel_char = char_pos;
                        std::cout << "from left\n";
                    }

                    shift_pressed = true;
                    shift_was_pressed = false;
                }else if(shift_was_pressed){
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);
                }


                cr1.x = x_offset; cr1.y = y_offset;
                std::list<_span2>::iterator temp_it = test_cp;
                std::list<_span2>::iterator tt_it = del_me2.end();
                bool isn_append = false;


                if(!it_cp->append && char_pos) {
                    if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                        while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                    if(char_pos > 0){
                        isn_append = true;
                        tt_it = it_cp++;
                    }
                }else if(--it_cp != test_cp){
                        if(!it_cp->append){
                            char_pos = it_cp->_end - it_cp->_start;
                            if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                            if(char_pos > 0){
                                isn_append = true;
                                tt_it = it_cp++;
                            }
                        }

                }else {
                    if(test_cp != del_me2.begin()){

                        test_cp = del_me2.erase(test_cp);
                        std::list<_span2>::iterator t__t = test_cp;


                        if(!(--t__t)->append && !test_cp->append){
                            char_pos = t__t->_end - t__t->_start;
                            t__t->_end = test_cp->_end;
                            test_cp = del_me2.erase(test_cp);

                            it_cp = t__t;
                            if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                            if(char_pos > 0){
                                isn_append = true;
                                tt_it = it_cp++;
                            }
                        }else if(!(--it_cp)->append){
                                char_pos = it_cp->_end - it_cp->_start;
                                if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                                    while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                                if(char_pos > 0){
                                    isn_append = true;
                                    tt_it = it_cp++;
                                }
                        }
                            // THIS IS WHERE THE TRICK BEGINS


                            std::list<_span2>::iterator test_this = test_cp;

                            int32_t th_count = 0;
                            bool is_break = false;

                            while(--test_this != del_me2.begin()) {

                                size_t x_start_pos = (!th_count++) ? test_this->_start + char_pos : test_this->_end;
                                size_t tri_count = 0;


                                while(x_start_pos > test_this->_start) {

                                    if(!test_this->append){
                                        tri_count++;
                                        if((per_buffer[--x_start_pos] & 0xFF) > 0x7F){
                                            tri_count++;
                                            while((per_buffer[--x_start_pos] & 0xE0) != 0xC0)
                                                tri_count++;
                                        }

                                        if(per_buffer[x_start_pos] == '\n'){

                                            del_me2.insert(test_this, {test_this->_start, x_start_pos + 1, false});
                                            test_this->_start = x_start_pos + 1;

                                            if(test_this->_start == test_this->_end)
                                                test_this = del_me2.erase(test_this);

                                            test_cp = del_me2.insert(test_this, {0, 0, false});
                                            temp_it = test_cp;

                                            if(test_this == tt_it)
                                                char_pos = tri_count - 1;

                                            is_break = true;
                                            break;
                                        }

                                    }else {

                                        if((new_buffer[--x_start_pos] & 0xFF) > 0x7F)
                                            while((new_buffer[--x_start_pos] & 0xE0) != 0xC0);

                                        if(new_buffer[x_start_pos] == '\n'){

                                            test_cp = del_me2.insert(++test_this, {0, 0, false});

                                            temp_it = test_cp;
                                            is_break = true;
                                            break;
                                        }
                                    }

                                }

                                if(is_break){
                                    break;
                                    off_char = x_offset;
                                }
                            }

                            if(test_this == del_me2.begin()){
                                test_cp = del_me2.begin();
                                temp_it = test_cp;
                                off_char = x_offset;
                            }

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                    }else
                        ++it_cp;
                }

                cr1.x = off_char;


                while(++temp_it != it_cp) {

                    size_t start_pos = 0;

                    size_t end_pos = (temp_it == tt_it) ? char_pos : temp_it->_end - temp_it->_start;


                    while(start_pos < end_pos) {

                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];
                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }

                    }
                }

                if(isn_append)
                    --it_cp;

                if(cr1.y > _height - s_h){

                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = x_offset;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= s_h;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x = t_x + x_offset - (_width - s_w);
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }

                            if(cr1.y < _height)
                                break;
                        }
                        if(cr1.y < _height) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});


                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }
/*
                            for(auto i : del_me2)
                                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                            std::cout << "*******\n";
*/

                            break;
                        }
                    }

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;

                if(shift_pressed) {

                    Uint32 *_pixels = (Uint32*) screen->pixels;
                        for(int i = 0; i < _cur_height; i++)
                            for(int j = 0; j < s_w; j++)
                                _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;

                    shift_pressed = false;
                    shift_was_pressed = true;

                }else {
                    shift_was_pressed = false;
                }


                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_RIGHT){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                if(SDL_GetModState() == KMOD_RSHIFT) {

                    if(!shift_was_pressed){
                        sel_cp = it_cp;
                        sel_char = char_pos;
                        sel_end_cp = sel_cp;
                        sel_end_char = sel_char;

                    }else if(sel_end_cp != test_cp) {
                        if(sel_cp != sel_end_cp && sel_char != sel_end_char){
                            sel_cp = it_cp;
                            sel_char = char_pos;
                        }
                    }

                    shift_pressed = true;
                    shift_was_pressed = false;



                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < s_w; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;

                }else if(shift_was_pressed){
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);
                }



                if(it_cp != del_me2.end()) {

                    if(!it_cp->append) {
                        char _ch = per_buffer[char_pos++ + it_cp->_start];
                        if((_ch & 0xE0) == 0xC0)
                            char_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            char_pos++;
                            char_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            char_pos++;
                            char_pos++;
                            char_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }

                        if((char_pos + it_cp->_start) >= it_cp->_end){
                            it_cp++;
                            char_pos = 0;
                        }


                    }else {

                        char _ch = new_buffer[it_cp++->_start];

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }
                    }

                }

                if(shift_pressed){
                    if(sel_end_cp == sel_cp){
                        sel_end_cp = it_cp;
                        sel_end_char = char_pos;
                    }
                }

                if(cr1.y > _height - s_h){
                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = off_char;
                    bool is_break = false;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                off_char = x_offset;
                                is_break = true;
                                break;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    off_char = t_x + x_offset - (_width - s_w);
                                    is_break = true;
                                    break;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    off_char = x_offset;
                                    is_break = true;
                                    break;
                                }
                            }
                        }
                        if(is_break) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                if(temp_it == sel_cp){
                                    sel_char -= start_pos;
                                    if(sel_char & 0xff00000000000000)
                                        sel_char = 0;
                                }


                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }

                            if(shift_pressed){

                                int t_x = off_char; int t_y = y_offset;
                                bool isn_append = false;

                                std::list<_span2>::iterator tt_it = del_me2.end();
                                std::list<_span2>::iterator temp_it = test_cp;

                                if(!sel_end_cp->append){
                                    tt_it = sel_end_cp++;
                                    isn_append = true;
                                }

                                bool is_hit = false;

                                while(++temp_it != sel_end_cp)
                                    if(temp_it == sel_cp)
                                        break;

                                if(temp_it == sel_end_cp){
                                    is_hit = true;
                                }

                                temp_it = test_cp;

                                while(++temp_it != sel_end_cp) {

                                    size_t start_pos = 0;

                                    size_t end_pos = (temp_it == tt_it) ? char_pos : temp_it->_end - temp_it->_start;



                                    while(start_pos < end_pos) {

                                        if(!is_hit){
                                            if(temp_it == sel_cp && start_pos >= sel_char)
                                                is_hit = true;
                                        }

                                        if(is_hit){
                                            Uint32 *_pixels = (Uint32*) screen->pixels;
                                            for(int i = 0; i < _cur_height; i++)
                                                for(int j = 0; j < s_w; j++)
                                                    _pixels[(j + t_x) + (t_y + i) * screen->w] = (0xFFFFFFFF - _pixels[(j + t_x) + (t_y + i) * screen->w])  | 0xFF000000;
                                        }
                                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];
                                        if((_ch & 0xE0) == 0xC0)
                                            start_pos++;
                                        else if((_ch & 0xF0) == 0xE0) {
                                            start_pos++;
                                            start_pos++;
                                        }
                                        else if((_ch & 0xF8) == 0xF0){
                                            start_pos++;
                                            start_pos++;
                                            start_pos++;
                                        }

                                        if(_ch == '\n'){
                                            t_x = x_offset;
                                            t_y += s_h;
                                        }else if(_ch == '\t'){

                                            t_x = (((t_x / s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                            if( t_x > _width - s_w ) {
                                                t_x = t_x + x_offset - (_width - s_w);
                                                t_y += s_h;
                                            }
                                        }else {
                                            t_x += s_w;
                                            if( t_x > _width - s_w) {
                                                t_x = x_offset;
                                                t_y += s_h;
                                            }
                                        }

                                    }
                                }

                                if(isn_append)
                                    --sel_end_cp;
                            }


                            break;

                        }
                    }

                    cr1.y = _height - s_h;
                }



                dest_rect.x = cr1.x; dest_rect.y = cr1.y;

                if(shift_pressed) {
                    shift_pressed = false;
                    shift_was_pressed = true;

                }else {
                    shift_was_pressed = false;
                }

                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_UP){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                cr1.x = x_offset; cr1.y = y_offset;
                std::list<_span2>::iterator temp_it = test_cp;
                std::list<_span2>::iterator tt_it = del_me2.end();
                bool isn_append = false, page_passed = false;
                size_t th_count = 0, n_count = 0;

                while(it_cp != del_me2.begin()) {


                    if(it_cp == test_cp){

                        test_cp = del_me2.erase(test_cp);
                        std::list<_span2>::iterator t__t = test_cp;
                        it_cp = test_cp;
                        page_passed = true;


                        if(!(--t__t)->append && !test_cp->append){
                            char_pos = t__t->_end - t__t->_start;
                            t__t->_end = test_cp->_end;
                            del_me2.erase(test_cp);
                            it_cp = t__t;
                            th_count = 0;

                            //if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                            //    while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }else
                            char_pos = 0;
/*
                        for(auto i : del_me2)
                            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "*******\n";
*/
                        th_count = 0;
                    }


                    size_t start_pos = (!th_count++) ? char_pos : it_cp->_end - it_cp->_start;

                    bool is_break = false;

                    while((start_pos + it_cp->_start) > it_cp->_start) {

                        if(!it_cp->append){
                            if((per_buffer[--start_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--start_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }else{
                            if((new_buffer[--start_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((new_buffer[--start_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }

                        char _ch = (!it_cp->append) ? per_buffer[start_pos + it_cp->_start] : new_buffer[start_pos + it_cp->_start];

                        if(_ch == '\n' && ++n_count > 1){
                                off_char = x_offset;
                                is_break = true;
                                break;
                        }

                    }

                    if(is_break){

                        if(page_passed) {

                            if(!it_cp->append){
                                del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + start_pos + 1, false});
                                it_cp->_start = start_pos + 1 + it_cp->_start;
                                if(it_cp->_start == it_cp->_end)
                                    it_cp = del_me2.erase(it_cp);

                                //std::cout << char_pos << " " << start_pos << " " << y_offset <<std::endl;

                                //std::cout << "here is the problem\n";
                                char_pos = 0; //-= (start_pos + 1);

                                test_cp = del_me2.insert(it_cp, {0, 0, false});
                                isn_append = true;
                                tt_it = it_cp++;

                            }else{
                                test_cp = del_me2.insert(++it_cp, {0, 0, false});
                                char_pos = 0;
                            }
/*
                            for(auto i : del_me2)
                                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                            std::cout << "*******\n";
*/
                            temp_it = test_cp;

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                        }else {

                            if( it_cp->_start + start_pos + 1 >= it_cp->_end){
                                if(it_cp->append)
                                    start_pos--;
                                ++it_cp;
                            }
                            if(!it_cp->append){
                                char_pos = start_pos + 1;
                                isn_append = true;
                                tt_it = it_cp++;
                            }else
                                char_pos = 0;
                        }

                        break;
                    }
                    it_cp--;
                }

                if(it_cp == del_me2.begin()){
                    it_cp++;

                    if(page_passed){
                        if(!it_cp->append){
                            isn_append = true;
                            tt_it = it_cp++;
                            //char_pos = 0;
                        }
                        test_cp = del_me2.begin();
                        temp_it = test_cp;
                    }
                    char_pos = 0;
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                }



                while(++temp_it != it_cp) {

                    size_t start_pos = 0;

                    size_t end_pos = (temp_it == tt_it) ? char_pos : temp_it->_end - temp_it->_start;

                    while(start_pos < end_pos) {

                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];
                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }

                    }
                }

                if(isn_append)
                    --it_cp;


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_DOWN){
                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                size_t th_count = 0;

                while(it_cp != del_me2.end()) {

                    size_t start_pos = (!th_count++) ? char_pos : 0;
                    bool is_break = false;

                    while((start_pos + it_cp->_start) < it_cp->_end) {

                        char _ch = (!it_cp->append) ? per_buffer[start_pos++ + it_cp->_start] : new_buffer[start_pos++ + it_cp->_start];

                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                            is_break = true;
                            break;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if(cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }
                    }

                    if(is_break){
                        if(!it_cp->append){
                            char_pos = start_pos;
                            if(it_cp->_start + char_pos >= it_cp->_end){
                                it_cp++;
                                char_pos = 0;
                            }
                        }
                        else{
                            it_cp++;
                            char_pos = 0;
                        }
                        break;
                    }
                    it_cp++;
                }

                if(it_cp == del_me2.end())
                    char_pos = 0;


                if(cr1.y > _height - s_h){

                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = off_char;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= s_h;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x = t_x + x_offset - (_width - s_w);
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }

                            if(cr1.y < _height)
                                break;
                        }
                        if(cr1.y < _height) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});


                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }


                            break;
                        }
                    }

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_PAGEDOWN) {

                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                size_t th_count = 0;
                int line_count = 0;

                while(it_cp != del_me2.end()) {

                    size_t start_pos = (!th_count++) ? char_pos : 0;
                    bool is_break = false;

                    while((start_pos + it_cp->_start) < it_cp->_end) {

                        char _ch = (!it_cp->append) ? per_buffer[start_pos++ + it_cp->_start] : new_buffer[start_pos++ + it_cp->_start];

                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                            if(++line_count > _height / s_h){
                                is_break = true;
                                break;
                            }
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                                if(++line_count > _height / s_h){
                                    is_break = true;
                                    break;
                                }
                            }
                        }else {
                            cr1.x += s_w;
                            if(cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                                if(++line_count > _height / s_h){
                                    is_break = true;
                                    break;
                                }
                            }
                        }
                    }

                    if(is_break){
                        if(!it_cp->append){
                            char_pos = start_pos;
                            if(it_cp->_start + char_pos >= it_cp->_end){
                                it_cp++;
                                char_pos = 0;
                            }
                        }else{
                            it_cp++;
                            char_pos = 0;
                        }
                        break;
                    }
                    it_cp++;
                }

                if(it_cp == del_me2.end())
                    char_pos = 0;


                if(cr1.y > _height - s_h){

                    std::list<_span2>::iterator temp_it = test_cp;
                    int t_x = off_char;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while((start_pos + temp_it->_start) < temp_it->_end) {

                            char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                            if((_ch & 0xE0) == 0xC0)
                                start_pos++;
                            else if((_ch & 0xF0) == 0xE0) {
                                start_pos++;
                                start_pos++;
                            }
                            else if((_ch & 0xF8) == 0xF0){
                                start_pos++;
                                start_pos++;
                                start_pos++;
                            }

                            if(_ch == '\n'){
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= s_h;
                            }else if(_ch == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x = t_x + x_offset - (_width - s_w);
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    off_char = t_x;
                                    cr1.y -= s_h;
                                }
                            }

                            if(cr1.y < _height)
                                break;
                        }
                        if(cr1.y < _height) {

                            if(!temp_it->append){
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                                temp_it->_start = start_pos + temp_it->_start;
                                if(temp_it->_start == temp_it->_end)
                                    temp_it = del_me2.erase(temp_it);
                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, false});
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, false});


                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);
                            x_offset = 0;

                            std::list<_span2>::iterator test_this = test_cp;

                            while(--test_this != del_me2.begin()) {
                                if(test_this->_start == test_this->_end)
                                    break;
                            }

                            if(test_this != del_me2.begin()) {
                                test_this = del_me2.erase(test_this);
                                std::list<_span2>::iterator t__t = test_this;

                                if(!(--t__t)->append && !test_this->append){
                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }


                            break;
                        }
                    }

                }



                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;

            }
            if(e.key.keysym.sym == SDLK_PAGEUP) {

                if(!blink_on) {
                    Uint32 *_pixels = (Uint32*) screen->pixels;
                    for(int i = 0; i < _cur_height; i++)
                        for(int j = 0; j < _cur_width; j++)
                            _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
                }

                cr1.x = x_offset; cr1.y = y_offset;
                std::list<_span2>::iterator temp_it = test_cp;
                std::list<_span2>::iterator t_it_cp = it_cp;
                size_t t_char_pos = char_pos;
                std::list<_span2>::iterator tt_it = del_me2.end();
                bool isn_append = false, page_passed = false;
                size_t th_count = 0, n_count = 0;

                while(it_cp != del_me2.begin()) {

                    if(it_cp == test_cp){

                        it_cp = del_me2.erase(test_cp);
                        std::list<_span2>::iterator t__t = it_cp;
                        page_passed = true;


                        if(!(--t__t)->append && !it_cp->append){
                            char_pos = t__t->_end - t__t->_start;
                            //std::cout << "old Pos = " << t_char_pos << std::endl;
                            if(it_cp == t_it_cp){
                                t_char_pos += (t__t->_end - t__t->_start);
                                //std::cout << "new Pos = " << t_char_pos << std::endl;
                                t_it_cp = t__t;
                            }
                            t__t->_end = it_cp->_end;
                            del_me2.erase(it_cp);
                            it_cp = t__t;
                            th_count = 0;

                            //if((per_buffer[--char_pos + it_cp->_start] & 0xFF) > 0x7F)
                            //    while((per_buffer[--char_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }else
                            char_pos = 0;



/*
                        for(auto i : del_me2)
                            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "*******\n";
*/
                        th_count = 0;
                    }


                    size_t start_pos = (!th_count++) ? char_pos : it_cp->_end - it_cp->_start;

                    bool is_break = false;

                    while((start_pos + it_cp->_start) > it_cp->_start) {



                        if(!it_cp->append){
                            if((per_buffer[--start_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((per_buffer[--start_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }else{
                            if((new_buffer[--start_pos + it_cp->_start] & 0xFF) > 0x7F)
                                while((new_buffer[--start_pos + it_cp->_start] & 0xE0) != 0xC0);
                        }

                        //std::cout << start_pos << std::endl;

                        char _ch = (!it_cp->append) ? per_buffer[start_pos + it_cp->_start] : new_buffer[start_pos + it_cp->_start];

                        if(_ch == '\n'){

                            size_t tt_count = 0;
                            std::list<_span2>::iterator temp_it = it_cp;
                            std::list<_span2>::iterator tt_it = del_me2.end();
                            int t_x = x_offset;
                            off_char = x_offset;
                            int line_count = 0;
                            bool isn_append = false;

                            if(!t_it_cp->append){
                                //std::cout << t_it_cp->_end << " " << it_cp->_end << " " << (t_it_cp == it_cp) << std::endl;
                                tt_it = t_it_cp++;
                                isn_append = true;
                            }

                            while(temp_it != t_it_cp) {

                                size_t _start_pos = (!tt_count++) ? start_pos : 0;
                                size_t end_pos = (temp_it == tt_it) ? t_char_pos : temp_it->_end - temp_it->_start;
                                //std::cout << "is it called\n";
                                //std::cout << _start_pos << " " << end_pos << " " << char_pos << std::endl;

                                while(_start_pos < end_pos) {

                                    char _ch = (!temp_it->append) ? per_buffer[_start_pos++ + temp_it->_start] : new_buffer[_start_pos++ + temp_it->_start];
                                    if((_ch & 0xE0) == 0xC0)
                                        _start_pos++;
                                    else if((_ch & 0xF0) == 0xE0) {
                                        _start_pos++;
                                        _start_pos++;
                                    }
                                    else if((_ch & 0xF8) == 0xF0){
                                        _start_pos++;
                                        _start_pos++;
                                        _start_pos++;
                                    }

                                    if(_ch == '\n'){
                                        t_x = x_offset;
                                        if(++line_count > _height / s_h){
                                            //off_char = t_x;
                                            break;
                                        }
                                    }else if(_ch == '\t'){

                                        t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                        if( t_x > _width - s_w ) {
                                            t_x = t_x + x_offset - (_width - s_w);
                                            if(++line_count > _height / s_h){
                                                //off_char = t_x + x_offset - (_width - s_w);
                                                break;
                                            }
                                        }
                                    }else {
                                        t_x += s_w;
                                        if( t_x > _width - s_w) {
                                            t_x = x_offset;
                                            if(++line_count > _height / s_h){
                                                //off_char = t_x;
                                                break;
                                            }
                                        }
                                    }

                                }

                                //std::cout << line_count << std::endl;
                                if(line_count > _height / s_h)
                                    break;
                                temp_it++;
                            }

                            if(isn_append)
                                --t_it_cp;

                            if(line_count > _height / s_h){
                                is_break = true;
                                break;
                            }
                        }

                    }

                    if(is_break){

                        if(page_passed) {

                            if(!it_cp->append){
                                del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + start_pos + 1, false});
                                it_cp->_start = start_pos + 1 + it_cp->_start;
                                if(it_cp->_start == it_cp->_end)
                                    it_cp = del_me2.erase(it_cp);

                                //std::cout << char_pos << " " << start_pos << " " << y_offset <<std::endl;

                                //std::cout << "here is the problem\n";
                                char_pos = 0; //-= (start_pos + 1);

                                test_cp = del_me2.insert(it_cp, {0, 0, false});
                                isn_append = true;
                                tt_it = it_cp++;

                            }else{
                                test_cp = del_me2.insert(++it_cp, {0, 0, false});
                                char_pos = 0;
                            }
/*
                            for(auto i : del_me2)
                                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                            std::cout << "*******\n";
*/
                            temp_it = test_cp;

                            _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

                        }else {

                            if( it_cp->_start + start_pos + 1 >= it_cp->_end){
                                if(it_cp->append)
                                    start_pos--;
                                ++it_cp;
                            }
                            if(!it_cp->append){
                                char_pos = start_pos + 1;
                                isn_append = true;
                                tt_it = it_cp++;
                            }else
                                char_pos = 0;
                        }

                        break;
                    }
                    it_cp--;
                }


                if(it_cp == del_me2.begin()){
                    it_cp++;
                    off_char = x_offset;

                    if(page_passed){
                        if(!it_cp->append){
                            isn_append = true;
                            tt_it = it_cp++;
                            //char_pos = 0;
                        }
                        test_cp = del_me2.begin();
                        temp_it = test_cp;
                    }
                    char_pos = 0;
                    _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                }


                while(++temp_it != it_cp) {

                    size_t start_pos = 0;

                    size_t end_pos = (temp_it == tt_it) ? char_pos : temp_it->_end - temp_it->_start;

                    while(start_pos < end_pos) {

                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];
                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            cr1.x = x_offset;
                            cr1.y += s_h;
                        }else if(_ch == '\t'){

                            cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( cr1.x > _width - s_w ) {
                                cr1.x = cr1.x + x_offset - (_width - s_w);
                                cr1.y += s_h;
                            }
                        }else {
                            cr1.x += s_w;
                            if( cr1.x > _width - s_w) {
                                cr1.x = x_offset;
                                cr1.y += s_h;
                            }
                        }

                    }
                }

                if(isn_append)
                    --it_cp;


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }
        }


        if(e.type == SDL_TEXTINPUT) {
            std::string t1 = e.text.text;

            if(!blink_on) {
                Uint32 *_pixels = (Uint32*) screen->pixels;
                for(int i = 0; i < _cur_height; i++)
                    for(int j = 0; j < _cur_width; j++)
                        _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
            }


            if(char_pos > 0){
                del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, false});
                it_cp->_start = char_pos + it_cp->_start;
                char_pos = 0;
            }
            new_buffer += t1;
            del_me2.insert(it_cp, {new_buffer.size() - t1.size(), new_buffer.size(), true});

/*
            for(auto i : del_me2)
                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
            std::cout << "*******\n";
*/


            cr1.x += s_w;

            if( cr1.x > _width - s_w) {
                cr1.x = x_offset;
                cr1.y += s_h;
            }

            if(cr1.y > _height - s_h){
                std::list<_span2>::iterator temp_it = test_cp;
                int t_x = off_char;
                bool is_break = false;

                while(++temp_it != del_me2.end()) {

                    size_t start_pos = 0;

                    while((start_pos + temp_it->_start) < temp_it->_end) {

                        char _ch = (!temp_it->append) ? per_buffer[start_pos++ + temp_it->_start] : new_buffer[start_pos++ + temp_it->_start];

                        if((_ch & 0xE0) == 0xC0)
                            start_pos++;
                        else if((_ch & 0xF0) == 0xE0) {
                            start_pos++;
                            start_pos++;
                        }
                        else if((_ch & 0xF8) == 0xF0){
                            start_pos++;
                            start_pos++;
                            start_pos++;
                        }

                        if(_ch == '\n'){
                            off_char = x_offset;
                            is_break = true;
                            break;
                        }else if(_ch == '\t'){

                            t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( t_x > _width - s_w) {
                                off_char = t_x + x_offset - (_width - s_w);
                                is_break = true;
                                break;
                            }
                        }else {
                            t_x += s_w;
                            if(t_x > _width - s_w) {
                                off_char = x_offset;
                                is_break = true;
                                break;
                            }
                        }
                    }
                    if(is_break) {

                        if(!temp_it->append){
                            del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, false});
                            temp_it->_start = start_pos + temp_it->_start;
                            if(temp_it->_start == temp_it->_end)
                                temp_it = del_me2.erase(temp_it);
                            if(temp_it == it_cp)
                                char_pos -= start_pos;

                            test_cp = del_me2.insert(temp_it, {0, 0, false});
                        }else
                            test_cp = del_me2.insert(++temp_it, {0, 0, false});

                        _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);


                        std::list<_span2>::iterator test_this = test_cp;

                        while(--test_this != del_me2.begin()) {
                            if(test_this->_start == test_this->_end)
                                break;
                        }

                        if(test_this != del_me2.begin()) {
                            test_this = del_me2.erase(test_this);
                            std::list<_span2>::iterator t__t = test_this;

                            if(!(--t__t)->append && !test_this->append){
                                t__t->_end = test_this->_end;
                                del_me2.erase(test_this);
                            }

                        }

                        break;

                    }
                }

                cr1.y = _height - s_h;
            }else
                _Render(per_buffer, new_buffer, del_me2, test_cp, _width, _height, s_w, s_h, &x_offset, &y_offset, off_char, font_map, screen, font_atlas, key_color);

            pcr1.x = cr1.x;

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
    SDL_DestroyWindow(my_test);
    TTF_Quit();
    SDL_Quit();


    return EXIT_SUCCESS;
}
