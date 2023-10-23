#include <iostream>
#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <tinyfiledialogs.h>

// THIS VERSION IS ABOUT BUFFER OPTIMIZATION //

#define _Cursor_Delay 450
#define _Tab_Width 4
//#define UTF8_CHAR_LEN(byte) ((byte) == 0x0d) ? 2 : (( 0xE5000000 >> (( (byte) >> 3 ) & 0x1e )) & 3 ) + 1

inline int UTF8_CHAR_LEN(char byte) {return byte == 0x0d ? 2 : (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1; }

typedef struct {int32_t x; int32_t y;} __cursor;
typedef struct {size_t _start; size_t _end; bool append;} _span2;

char new_line = '\r', ctrl_line = '\n'; int step_count = 2;


typedef struct {SDL_Window *_my_win;
                std::string &per_buffer;
                std::list<_span2> &del_me2;
                std::list<std::list<_span2>::iterator> &remit_list;
                std::list<char> &undo_type;
                std::list<_span2>::iterator *test_cp;
                std::list<_span2>::iterator *it_cp;
                std::list<_span2>::iterator *sel_cp;
                std::list<_span2>::iterator *sel_end_cp;
                std::list<std::list<_span2>::iterator>::iterator &remit_it;
                std::list<char>::iterator &undo_type_it;
                size_t *char_pos;
                size_t *sel_char;
                size_t *sel_end_char;
                __cursor &cr1;
                int *_width;
                int *_height;
                int s_w;
                int s_h;
                int _cur_height;
                int *x_offset;
                int *y_offset;
                int font_ascent;
                size_t *off_char;
                std::unordered_map<std::string, __cursor> &font_map;
                SDL_Surface **screen;
                SDL_Surface *font_atlas;
                Uint32 key_color;
                } _DATA, *P_DATA;


bool check_sel_cursor(std::string &per_buffer, std::list<_span2> &del_me2, std::list<_span2>::iterator sel_cp,
                    std::list<_span2>::iterator sel_end_cp, size_t sel_char, size_t sel_end_char, __cursor cr1,
                    int x_offset, int s_w, int s_h, int _width, int _height) {


        std::list<_span2>::iterator temp_it = sel_cp;

        do {

            size_t start_pos = (temp_it == sel_cp) ? sel_cp->_start + sel_char : temp_it->_start;
            size_t end_pos = (temp_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : temp_it->_end;

            while(start_pos < end_pos && temp_it->append) {

                if(per_buffer[start_pos] == new_line){
                    cr1.x = x_offset;
                    cr1.y += s_h;
                }else if(per_buffer[start_pos] == '\t'){

                    cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                    if( cr1.x > _width - s_w ) {
                        cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                        cr1.x = s_w * (cr1.x / s_w);
                        cr1.y += s_h;
                    }
                }else {
                    cr1.x += s_w;
                    if( cr1.x > _width - s_w) {
                        cr1.x = x_offset;
                        cr1.y += s_h;
                    }
                }

                if(cr1.y > _height - s_h)
                    return true;

                start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

            }


    }while(temp_it++ != sel_end_cp);

    return false;

}


bool check_cursor(std::string &per_buffer, std::list<_span2> &paste_board, __cursor &cr1,
                    int x_offset, int s_w, int s_h, int _width, int _height) {


        std::list<_span2>::iterator temp_it = paste_board.begin();

        while(temp_it != paste_board.end()) {

            size_t start_pos = temp_it->_start;

            while(start_pos < temp_it->_end && temp_it->append) {

                if(per_buffer[start_pos] == new_line){
                    cr1.x = x_offset;
                    cr1.y += s_h;
                }else if(per_buffer[start_pos] == '\t'){

                    cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                    if( cr1.x > _width - s_w ) {
                        cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                        cr1.x = s_w * (cr1.x / s_w);
                        cr1.y += s_h;
                    }
                }else {
                    cr1.x += s_w;
                    if( cr1.x > _width - s_w) {
                        cr1.x = x_offset;
                        cr1.y += s_h;
                    }
                }

                if(cr1.y > _height - s_h)
                    return true;

                start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

            }

            temp_it++;
    }

    return false;

}



void adv_cursor_while(std::string &per_buffer, std::list<_span2>::iterator temp_it,
                       std::list<_span2>::iterator it_cp, __cursor &cr1,
                       size_t char_pos, int off_char, int y_offset, int s_w, int s_h, int _width) {

    cr1.x = off_char; cr1.y = y_offset;

        do {
   // while(temp_it != it_cp) {

        size_t start_pos = 0;

        size_t end_pos = (temp_it == it_cp) ? char_pos : temp_it->_end - temp_it->_start;

        while(start_pos < end_pos && temp_it->append) {

            if(per_buffer[temp_it->_start + start_pos] == new_line){
                cr1.x = 0;
                cr1.y += s_h;
            }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( cr1.x > _width - s_w ) {
                    cr1.x %= (_width - s_w); // cr1.x + 0 - (_width - s_w);
                    cr1.x = s_w * (cr1.x / s_w);
                    cr1.y += s_h;
                }
            }else {
                cr1.x += s_w;
                if( cr1.x > _width - s_w) {
                    cr1.x = 0;
                    cr1.y += s_h;
                }
            }

            start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);

        }

    }while(temp_it++ != it_cp);

}

void adv_cursor(std::string &per_buffer, __cursor &cr1, size_t _start, size_t char_pos, int x_offset, int s_w, int s_h, int _width) {


    if(per_buffer[_start + char_pos] == new_line){
        cr1.x = x_offset;
        cr1.y += s_h;
    }else if(per_buffer[_start + char_pos] == '\t'){

        cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

        if( cr1.x > _width - s_w ) {
            cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
            cr1.x = s_w * (cr1.x / s_w);
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

SDL_Surface* prepare_font_atlas(int font_size, int *w, int *h, int *font_ascent, std::unordered_map<std::string, __cursor> &p_font_map) {

    //float dpi_v, dpi_h;
    //SDL_GetDisplayDPI(0, NULL, &dpi_h, &dpi_v);
    TTF_Font *my_font = TTF_OpenFont(".\\fonts\\RobotoMono-Regular.ttf", font_size);//, dpi_h, dpi_v);

    std::string my_string = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    my_string += "ÇĞİÖŞÜçğıöşü"; /// Turkish
    my_string += "АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя"; /// Russian
    my_string += "«»’“”"; /// Specials

    SDL_Surface *source = TTF_RenderUTF8_Blended(my_font, my_string.c_str(), {230, 230, 230});

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
                std::string &per_buffer,
                std::list<_span2> &del_me2,
                std::list<_span2>::iterator it_cp,
                int _width,
                int _height,
                int s_w,
                int s_h,
                int x_offset,
                int y_offset,
                int font_ascent,
                size_t off_char,
                std::unordered_map<std::string, __cursor> &font_map,
                SDL_Surface *screen,
                SDL_Surface *font_atlas,
                Uint32 key_color
                ) {

    SDL_FillRect(screen, NULL, key_color);

    int16_t t_x = off_char, t_y = y_offset;
    std::list<_span2>::iterator temp_it = it_cp;

    while(++temp_it != del_me2.end() && t_y < _height) {

        size_t str_count = temp_it->_start;

        while(str_count < temp_it->_end && t_y < _height && temp_it->append) {


            int new_count = UTF8_CHAR_LEN(per_buffer[str_count]);
            std::string tt_me = per_buffer.substr(str_count, new_count);
            str_count += new_count;


            if(font_map.find(tt_me) != font_map.end()){
                SDL_Rect temp_font = {(int) font_map[tt_me].x, 0, s_w, s_h};
                SDL_Rect temp_rect = {t_x, t_y + font_ascent, s_w, s_h};
                SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
            }


            if(tt_me[0] == new_line){
                t_x = x_offset;
                t_y += (s_h + font_ascent);
            }else if(tt_me[0] == '\t'){

                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( t_x > _width - s_w ) {
                    t_x %= (_width - s_w);// t_x + x_offset - (_width - s_w);
                    t_x = s_w * (t_x / s_w);
                    t_y += (s_h + font_ascent);
                }
            }else {
                t_x += s_w;
                if( t_x > _width - s_w) {
                    t_x = x_offset;
                    t_y += (s_h + font_ascent);
                }
            }

        }
    }

    //std::cout << "list bucket size " << del_me2.size() << " " <<(float) (del_me2.size() * sizeof(_span2)) / 1024.0f <<  " kbytes." << std::endl;


}


void _Sel_Render(
                std::string &per_buffer,
                std::list<_span2> &del_me2,
                std::list<_span2>::iterator test_cp,
                std::list<_span2>::iterator sel_cp,
                std::list<_span2>::iterator sel_end_cp,
                int _width,
                int _height,
                int s_w,
                int s_h,
                int _cur_height,
                int x_offset,
                int y_offset,
                size_t off_char,
                size_t sel_char,
                size_t sel_end_char,
                SDL_Surface *screen
                ) {

/*
    std::cout << "---- Rendered----\n";
    for(auto i : del_me2)
        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
    std::cout << "*******\n";
*/
    //std::string &_lkup = sel_cp->append ? new_buffer : per_buffer;
    //std::string &_lkup2 = sel_end_cp->append ? new_buffer : per_buffer;
    //std::cout << "sel_cp = " << per_buffer[sel_cp->_start + sel_char] << std::endl;
    //std::cout << "sel_char = " << sel_cp->_start << " " << sel_char << std::endl;
    //std::cout << "sel_end_cp = " << per_buffer[sel_end_cp->_start + sel_end_char] << std::endl;
    //std::cout << "sel_end_char = " << sel_end_cp->_start << " " << sel_end_char << std::endl;
    //std::cout << "---------------------------\n";


    int16_t t_x = off_char, t_y = y_offset;
    bool _start = false, _break = false, is_first = true;

    _cur_height -= s_h;

    SDL_Surface *sel_surface = SDL_CreateRGBSurface(0, s_w, _cur_height + s_h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);

    Uint32 Sel_Color = SDL_MapRGBA(sel_surface->format, 250, 250, 250, 120);

    SDL_FillRect(sel_surface, NULL, Sel_Color);



    SDL_BlendMode screen_blend_mode = {};

    //SDL_SetSurfaceBlendMode(sel_surface, SDL_BLENDMODE_BLEND);
    //SDL_SetSurfaceBlendMode(screen, screen_blend_mode);

    SDL_GetSurfaceBlendMode(sel_surface, &screen_blend_mode);


    //std::cout << "screen blendmode = " << screen_blend_mode << std::endl;


    std::list<_span2>::iterator temp_it = sel_cp;

    do {
        if(temp_it == test_cp) {

            int16_t old_x = x_offset;

            for(; old_x < t_x && old_x < _width - s_w; old_x += s_w) {
                SDL_Rect dest_rect = {old_x, t_y, s_w, _cur_height};
                SDL_BlitSurface(sel_surface, NULL, screen, &dest_rect);
            }

            _start = true;
            break;
        }
        if(temp_it == sel_end_cp)
            break;
    }while(++temp_it != del_me2.end());

    temp_it = test_cp;

    while(++temp_it != del_me2.end() && t_y < _height + s_h) {

        size_t str_count = temp_it->_start;
        //std::string &_lkup = temp_it->append ? new_buffer : per_buffer;

        while(str_count < temp_it->_end && t_y < _height && temp_it->append) {

            char _ch = per_buffer[str_count];

            if(temp_it == sel_end_cp && str_count >= (sel_end_cp->_start + sel_end_char)){
                _break = true;
                break;
            }else if(temp_it == sel_cp && str_count == (sel_cp->_start + sel_char))
                _start = true;

            if(_start) {

                SDL_Rect dest_rect = {t_x, t_y, s_w, _cur_height + s_h};
                SDL_BlitSurface(sel_surface, NULL, screen, &dest_rect);

            }

            if(_ch == new_line){
                t_x = x_offset;
                t_y += s_h;
            }else if(_ch == '\t'){

                    int16_t old_x = t_x + s_w;

                    t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                    if(_start) {
                        for(; old_x < t_x && old_x < _width - s_w; old_x += s_w) {
                            SDL_Rect dest_rect = {old_x, t_y, s_w, _cur_height + s_h};
                            SDL_BlitSurface(sel_surface, NULL, screen, &dest_rect);
                        }
                    }


                    if( t_x > _width - s_w) {
                        old_x = x_offset;
                        t_x %= (_width - s_w); // t_x + x_offset - (_width - s_w);
                        t_x = s_w * (t_x / s_w);
                        t_y += s_h;
                        if(_start) {
                            for(; old_x < t_x && old_x < _width - s_w; old_x += s_w) {
                                SDL_Rect dest_rect = {old_x, t_y, s_w, _cur_height + s_h};
                                SDL_BlitSurface(sel_surface, NULL, screen, &dest_rect);
                            }
                        }
                    }


            }else {
                t_x += s_w;
                if( t_x > _width - s_w) {
                    t_x = x_offset;
                    t_y += s_h;
                }
            }

            str_count += UTF8_CHAR_LEN(per_buffer[str_count]);
        }
        if(_break)
            break;
    }

    SDL_FreeSurface(sel_surface);

}

void _Selection_Down(
                const std::list<_span2> &del_me2,
                const std::list<_span2>::iterator test_cp,
                const std::list<_span2>::iterator sel_it_temp,
                std::list<_span2>::iterator *sel_cp,
                std::list<_span2>::iterator *sel_end_cp,
                const size_t sel_char_temp,
                size_t *sel_char,
                size_t *sel_end_char
                )
{

    bool breaked = false;

    std::list<_span2>::iterator sel_it = sel_it_temp;

    do{
        if(sel_it == *sel_end_cp){

            if(sel_it_temp == *sel_end_cp && ((*sel_end_cp)->_start + *sel_end_char > sel_it_temp->_start + sel_char_temp)) {
                *sel_cp = sel_it_temp;
                *sel_char = sel_char_temp;
            }else {
                *sel_cp = *sel_end_cp;
                *sel_char = *sel_end_char;
                *sel_end_cp = sel_it_temp;
                *sel_end_char = sel_char_temp;
            }

            breaked = true;
            break;
        }

    }while(--sel_it != test_cp);

    if(!breaked) {

        bool _found_it = false;
        sel_it = sel_it_temp;

        do {
            if(sel_it == *sel_end_cp) {
                _found_it = true;
                break;
            }
        }while (++sel_it != del_me2.end());


        if(_found_it || *sel_end_cp == del_me2.end()) {
            *sel_cp = sel_it_temp;
            *sel_char = sel_char_temp;
        }else {
            *sel_cp = *sel_end_cp;
            *sel_char = *sel_end_char;
            *sel_end_cp = sel_it_temp;
            *sel_end_char = sel_char_temp;
        }
    }

}

void _Selection_Up(
                const std::list<_span2> &del_me2,
                const std::list<_span2>::iterator test_cp,
                const std::list<_span2>::iterator sel_it_temp,
                std::list<_span2>::iterator *sel_cp,
                std::list<_span2>::iterator *sel_end_cp,
                const size_t sel_char_temp,
                size_t *sel_char,
                size_t *sel_end_char)
{

    bool breaked = false;
    std::list<_span2>::iterator sel_it = sel_it_temp;


    do{
        if(sel_it == *sel_cp){
            if(sel_it != sel_it_temp) {
                *sel_end_cp = sel_it_temp;
                *sel_end_char = sel_char_temp;
            }else {
                if(sel_it_temp == *sel_cp && ((*sel_cp)->_start + *sel_char <= sel_it_temp->_start + sel_char_temp)) {
                    *sel_end_cp = sel_it_temp;
                    *sel_end_char = sel_char_temp;
                }else {
                    *sel_end_cp = *sel_cp;
                    *sel_end_char = *sel_char;
                    *sel_cp = sel_it_temp;
                    *sel_char = sel_char_temp;
                }
            }
            breaked = true;
            break;
        }

    }while(--sel_it != test_cp);

    if(!breaked) {

        bool _found_it = false;
        sel_it = sel_it_temp;

        do {
            if(sel_it == *sel_cp) {
                _found_it = true;
                break;
            }
            if(sel_it == *sel_end_cp)
                break;
        }while (++sel_it != del_me2.end());


        if(_found_it) {
            *sel_end_cp = *sel_cp;
            *sel_end_char = *sel_char;
            *sel_cp = sel_it_temp;
            *sel_char = sel_char_temp;
        }
        else {
            *sel_end_cp = sel_it_temp;
            *sel_end_char = sel_char_temp;
        }
    }

}


void find_next_page(
            std::string &per_buffer,
            std::list<_span2> &del_me2,
            std::list<std::list<_span2>::iterator> &remit_list,
            std::list<char> &undo_type,
            std::list<_span2>::iterator *test_cp,
            std::list<_span2>::iterator *it_cp,
            std::list<_span2>::iterator *sel_cp,
            std::list<_span2>::iterator *sel_end_cp,
            std::list<std::list<_span2>::iterator>::iterator &remit_it,
            std::list<char>::iterator &undo_type_it,
            size_t *char_pos,
            size_t *sel_char,
            size_t *sel_end_char,
            size_t *off_char,
            int _width,
            int _height,
            int s_w,
            int s_h,
            int x_offset,
            int y_offset
                    ) {


    //if(*test_cp != del_me2.begin())
    //    *test_cp = del_me2.erase(*test_cp);


    if(*test_cp != del_me2.begin()) {
        *test_cp = del_me2.erase(*test_cp);

        std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
        bool is_break = false;

        while(t1 != remit_list.begin()) {
            if(*--t1 == *test_cp) {
                //std::cout << "hmm \n";
                is_break = true;
                break;
            }
        }


        if(!is_break) {

            std::list<_span2>::iterator temp_it = *test_cp;

            if((--temp_it)->_start != temp_it->_end && temp_it->_end == (*test_cp)->_start) {

                if(*test_cp == *it_cp) {
                    *char_pos += (temp_it->_end - temp_it->_start);
                    *it_cp = temp_it;
                }

                if(*test_cp == *sel_cp) {
                    *sel_char += (temp_it->_end - temp_it->_start);
                    *sel_cp = temp_it;
                }

                if(*test_cp == *sel_end_cp) {
                    *sel_end_char += (temp_it->_end - temp_it->_start);
                    *sel_end_cp = temp_it;
                }

                temp_it->_end = (*test_cp)->_end;

                del_me2.erase(*test_cp);

            }

        }

    }


    std::list<_span2>::iterator temp_it = *it_cp;
    bool is_break = false, last_run = false;

    size_t t_end = 0;
    __cursor t_x = {x_offset, y_offset};

    while(temp_it->_start != temp_it->_end) {


        t_end = (temp_it == *it_cp) ? (*it_cp)->_start + *char_pos : temp_it->_end;

        while(t_end > temp_it->_start && temp_it->append) {


            while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));


            if(per_buffer[t_end] == new_line) {


                // forward_ cursor_check

find_next:      t_x.x = x_offset; t_x.y = y_offset;

                std::list<_span2>::iterator found_it = temp_it;

                int first_run = 0;

                do{


                    size_t start_pos = !first_run++ ? t_end : found_it->_start;
                    size_t end_pos = (found_it == *it_cp) ? (*it_cp)->_start + *char_pos : found_it->_end;

                    while(start_pos < end_pos && found_it->append) {


                        if(per_buffer[start_pos] == new_line){
                            t_x.x = x_offset;
                            t_x.y += s_h;
                        }else if(per_buffer[start_pos] == '\t'){

                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( t_x.x > _width - s_w ) {
                                t_x.x %= (_width - s_w);// t_x.x + x_offset - (_width - s_w);
                                t_x.x = s_w * (t_x.x / s_w);
                                t_x.y += s_h;
                            }
                        }else {
                            t_x.x += s_w;
                            if( t_x.x > _width - s_w) {
                                t_x.x = x_offset;
                                t_x.y += s_h;
                            }
                        }

                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);


                    }

                }while(found_it++ != *it_cp);


            }

            if(t_x.y >= _height || last_run) {
                is_break = true;
                break;
            }

        }

        if(is_break)
            break;
        temp_it--;
    }

    if(!is_break) {
        ++temp_it;
        last_run = true;
        goto find_next;
    }


    int first_run = 0;

    *off_char = 0;

    t_x.x = x_offset;

    is_break = false;

    while(temp_it != del_me2.end()) {

        size_t start_pos = !first_run++ ? t_end : temp_it->_start;


        while(start_pos < temp_it->_end && temp_it->append) {

            if(t_x.y < _height) {
                is_break = true;
                t_end = start_pos;
               break;
            }

            if(per_buffer[start_pos] == new_line){
                t_x.x = x_offset;
                *off_char = t_x.x;
                t_x.y -= s_h;
            }else if(per_buffer[start_pos] == '\t'){

                t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( t_x.x > _width - s_w ) {
                    t_x.x %= (_width - s_w);// t_x.x + x_offset - (_width - s_w);
                    t_x.x = s_w * (t_x.x / s_w);
                    *off_char = t_x.x;
                    t_x.y -= s_h;
                }
            }else {
                t_x.x += s_w;
                if( t_x.x > _width - s_w) {
                    t_x.x = x_offset;
                    *off_char = t_x.x;
                    t_x.y -= s_h;
                }
            }


            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

        }

        if(is_break)
            break;
        temp_it++;
    }

    if(!is_break)
        t_end = temp_it->_end;



    if(t_end < temp_it->_end) {

        if(t_end != temp_it->_start) {

            std::list<_span2>::iterator to_put = del_me2.begin();

            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
            std::list<char>::iterator tt_undo_it = undo_type.end();
            bool is_break = false;

            while(tt_rem_it-- != remit_list.begin()) {
                --tt_undo_it;
                if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                    //if(tt_rem_it == remit_it) {

                    //    remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));
                    //    to_put = *remit_it;
                    //}else
                        to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));

                /*    if(*tt_undo_it == 1) {

                        if(tt_undo_it == undo_type_it)
                            undo_type_it = undo_type.insert(tt_undo_it, 1);
                        else
                            undo_type.insert(tt_undo_it, 1);

                        is_break = true;
                    }else{ */
                        undo_type.insert(tt_undo_it, 0);
                        is_break = true;
                        break;
                    //}

                }

            }

            if(!is_break)
                to_put = del_me2.insert(temp_it, {temp_it->_start, t_end, true});

            if(temp_it == *it_cp)
                *char_pos -= (t_end - temp_it->_start);

            if(temp_it == *sel_cp){

                if(to_put->_start + *sel_char < to_put->_end)
                    *sel_cp = to_put;
                else
                    *sel_char -= (t_end - temp_it->_start);

            }

            if(temp_it == *sel_end_cp){


                if(to_put->_start + *sel_end_char < to_put->_end)
                    *sel_end_cp = to_put;
                else
                    *sel_end_char -= (t_end - temp_it->_start);

            }

            temp_it->_start = t_end;
            *test_cp = del_me2.insert(temp_it, {0, 0, true});
        }else if (--temp_it != del_me2.begin()) {

            if(!temp_it->append)
                while(!(--temp_it)->append);

            if(temp_it != del_me2.begin()) {
                if(temp_it == *it_cp) {
                    *test_cp = del_me2.insert(++(*it_cp), {0, 0, true});
                    *char_pos = 0;
                }else
                    *test_cp = del_me2.insert(++temp_it, {0, 0, true});
            }else
                *test_cp = del_me2.begin();

        }else
            *test_cp = del_me2.begin();

    }else{

        *test_cp = del_me2.insert(temp_it, {0, 0, true});
        *it_cp = temp_it;
        *char_pos = 0;
    }

}


void find_next_page(
            std::string &per_buffer,
            std::list<_span2> &del_me2,
            std::list<std::list<_span2>::iterator> &remit_list,
            std::list<char> &undo_type,
            std::list<_span2>::iterator *test_cp,
            std::list<_span2>::iterator *it_cp,
            std::list<std::list<_span2>::iterator>::iterator &remit_it,
            std::list<char>::iterator &undo_type_it,
            size_t *char_pos,
            size_t *off_char,
            int _width,
            int _height,
            int s_w,
            int s_h,
            int x_offset,
            int y_offset
                    ) {


    //if(*test_cp != del_me2.begin())
    //    *test_cp = del_me2.erase(*test_cp);


    if(*test_cp != del_me2.begin()) {
        *test_cp = del_me2.erase(*test_cp);

        std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
        bool is_break = false;

        while(t1 != remit_list.begin()) {
            if(*--t1 == *test_cp) {
                //std::cout << "hmm \n";
                is_break = true;
                break;
            }
        }


        if(!is_break) {

            std::list<_span2>::iterator temp_it = *test_cp;

            if((--temp_it)->_start != temp_it->_end && temp_it->_end == (*test_cp)->_start) {

            if(*test_cp == *it_cp) {
                *char_pos += (temp_it->_end - temp_it->_start);
                *it_cp = temp_it;
            }

            temp_it->_end = (*test_cp)->_end;

            del_me2.erase(*test_cp);

            }

        }

    }

    std::list<_span2>::iterator temp_it = *it_cp;
    bool is_break = false, last_run = false;

    size_t t_end = 0;
    __cursor t_x = {x_offset, y_offset};

    while(temp_it->_start != temp_it->_end) {


        t_end = (temp_it == *it_cp) ? (*it_cp)->_start + *char_pos : temp_it->_end;

        while(t_end > temp_it->_start && temp_it->append) {


            while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));


            if(per_buffer[t_end] == new_line) {


                // forward_ cursor_check

find_next:      t_x.x = x_offset; t_x.y = y_offset;

                std::list<_span2>::iterator found_it = temp_it;

                int first_run = 0;

                do{


                    size_t start_pos = !first_run++ ? t_end : found_it->_start;
                    size_t end_pos = (found_it == *it_cp) ? (*it_cp)->_start + *char_pos : found_it->_end;

                    while(start_pos < end_pos && found_it->append) {


                        if(per_buffer[start_pos] == new_line){
                            t_x.x = x_offset;
                            t_x.y += s_h;
                        }else if(per_buffer[start_pos] == '\t'){

                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( t_x.x > _width - s_w ) {
                                t_x.x %= (_width - s_w);// t_x.x + x_offset - (_width - s_w);
                                t_x.x = s_w * (t_x.x / s_w);
                                t_x.y += s_h;
                            }
                        }else {
                            t_x.x += s_w;
                            if( t_x.x > _width - s_w) {
                                t_x.x = x_offset;
                                t_x.y += s_h;
                            }
                        }

                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);


                    }

                }while(found_it++ != *it_cp);


            }

            if(t_x.y >= _height || last_run) {
                is_break = true;
                break;
            }

        }

        if(is_break)
            break;
        temp_it--;
    }

    if(!is_break) {
        ++temp_it;
        last_run = true;
        goto find_next;
    }


    int first_run = 0;

    *off_char = 0;

    t_x.x = x_offset;

    is_break = false;

    while(temp_it != del_me2.end()) {

        size_t start_pos = !first_run++ ? t_end : temp_it->_start;


        while(start_pos < temp_it->_end && temp_it->append) {

            if(t_x.y < _height) {
                is_break = true;
                t_end = start_pos;
               break;
            }

            if(per_buffer[start_pos] == new_line){
                t_x.x = x_offset;
                *off_char = t_x.x;
                t_x.y -= s_h;
            }else if(per_buffer[start_pos] == '\t'){

                t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                if( t_x.x > _width - s_w ) {
                    t_x.x %= (_width - s_w);// t_x.x + x_offset - (_width - s_w);
                    t_x.x = s_w * (t_x.x / s_w);
                    *off_char = t_x.x;
                    t_x.y -= s_h;
                }
            }else {
                t_x.x += s_w;
                if( t_x.x > _width - s_w) {
                    t_x.x = x_offset;
                    *off_char = t_x.x;
                    t_x.y -= s_h;
                }
            }


            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

        }

        if(is_break)
            break;
        temp_it++;
    }

    if(!is_break)
        t_end = temp_it->_end;



    if(t_end < temp_it->_end) {

        if(t_end != temp_it->_start) {

            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
            std::list<char>::iterator tt_undo_it = undo_type.end();
            bool is_break = false;

            while(tt_rem_it-- != remit_list.begin()) {
                --tt_undo_it;
                if(*tt_rem_it == temp_it && *tt_undo_it == 0 ) {

                   // if(tt_rem_it == remit_it)
                   //     remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));
                   // else
                        remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));

              /*      if(*tt_undo_it == 1) {

                        if(tt_undo_it == undo_type_it)
                            undo_type_it = undo_type.insert(tt_undo_it, 1);
                        else
                            undo_type.insert(tt_undo_it, 1);

                        is_break = true;
                    }else{ */
                        undo_type.insert(tt_undo_it, 0);
                        is_break = true;
                        break;
                   // }

                }

            }

            if(!is_break)
                del_me2.insert(temp_it, {temp_it->_start, t_end, true});

            if(temp_it == *it_cp)
                *char_pos -= (t_end - temp_it->_start);

            temp_it->_start = t_end;
            *test_cp = del_me2.insert(temp_it, {0, 0, true});
        }else if (--temp_it != del_me2.begin()){

            if(!temp_it->append)
                while(!(--temp_it)->append);

            if(temp_it != del_me2.begin()) {

                if(temp_it == *it_cp) {
                    *test_cp = del_me2.insert(++(*it_cp), {0, 0, true});
                    *char_pos = 0;
                }else
                    *test_cp = del_me2.insert(++temp_it, {0, 0, true});

            }else
                *test_cp = del_me2.begin();

        }else
            *test_cp = del_me2.begin();


    }else{

        *test_cp = del_me2.insert(temp_it, {0, 0, true});
        *it_cp = temp_it;
        *char_pos = 0;
    }



//     for(auto i : del_me2)
//       std::cout << i._start << " " << i._end << " " << i.append << std::endl;
//     std::cout << "From Next_Page*******\n";


}


static int resizingEventWatcher(void* data, SDL_Event* event) {
  if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
    SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
    if (win == (SDL_Window*) ((P_DATA) data)->_my_win) {

        //*((P_DATA) data)->y_offset = 0;

        int old_width = *((P_DATA) data)->_width;
        int old_height = *((P_DATA) data)->_height;

        *((P_DATA) data)->_width = event->window.data1;
        *((P_DATA) data)->_height = event->window.data2;

        //std::cout << "it_cp " << (*((P_DATA) data)->it_cp)->_start << " " << (*((P_DATA) data)->it_cp)->_end << " " << *((P_DATA) data)->char_pos << std::endl;

    //    for(auto i : ((P_DATA) data)->del_me2)
    //        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
    //    std::cout << "From [Resize First]*******\n";


        /// To find previous new line above test_cp

        bool is_break = false;

        std::list<_span2>::iterator temp_it = *((P_DATA) data)->test_cp;

        if(temp_it != ((P_DATA) data)->del_me2.begin()) {

            size_t t_end = 0;


            while(--temp_it != ((P_DATA) data)->del_me2.begin()) {

                t_end = temp_it->_end;

                while(t_end > temp_it->_start && temp_it->append) {

                    while(((P_DATA) data)->per_buffer[--t_end] == ctrl_line || (((((P_DATA) data)->per_buffer[t_end] & 0x80) != 0) && ((((P_DATA) data)->per_buffer[t_end] & 0xC0) != 0xC0)));
                    if(((P_DATA) data)->per_buffer[t_end] == new_line) {

                        is_break = true;
                        break;
                    }

                }

                if(is_break)
                    break;

            }

            if(temp_it == ((P_DATA) data)->del_me2.begin()) {
                while(++temp_it != ((P_DATA) data)->del_me2.end() && !temp_it->append);
                t_end = temp_it->_start;
            }

            //std::cout << "Prev temp_it " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;

            std::list<_span2>::iterator tt_it = temp_it;
            size_t tt_end = t_end;


            ///forward iterating with old width

            size_t th_count = 0, row_count = 0;
            int32_t t_x = *((P_DATA) data)->x_offset;

            while(temp_it != *((P_DATA) data)->test_cp) {

                t_end = (!th_count++) ? t_end : temp_it->_start;

                while(t_end < temp_it->_end && temp_it->append) {

                    if(((P_DATA) data)->per_buffer[t_end] == new_line) {
                            t_x = *((P_DATA) data)->x_offset;
                            row_count++;
                    }else if(((P_DATA) data)->per_buffer[t_end] == '\t'){

                        t_x = (((t_x/((P_DATA) data)->s_w) / _Tab_Width) + 1) * _Tab_Width * ((P_DATA) data)->s_w;

                        if( t_x > old_width - ((P_DATA) data)->s_w) {
                            t_x %= (old_width - ((P_DATA) data)->s_w);
                            t_x = ((P_DATA) data)->s_w * (t_x / ((P_DATA) data)->s_w);
                            row_count++;
                        }
                    }else {
                        t_x += ((P_DATA) data)->s_w;

                        if(t_x > old_width - ((P_DATA) data)->s_w) {
                            t_x = *((P_DATA) data)->x_offset;
                            row_count++;
                        }
                    }


                    t_end += UTF8_CHAR_LEN(((P_DATA) data)->per_buffer[t_end]);

                }

                temp_it++;

            }


            //std::cout << "Old temp_it " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;



            ///forward iterating with new width

            th_count = 0;
            size_t new_row_count = 0;
            is_break = false; bool _eol = false;
            t_x = *((P_DATA) data)->x_offset;
            bool is_passed = false, is_on = false;

            temp_it = tt_it; t_end = tt_end;



            while(temp_it != ((P_DATA) data)->del_me2.end()) {

                t_end = (!th_count++) ? t_end : temp_it->_start;

                if(temp_it == *((P_DATA) data)->it_cp && !is_passed)
                    is_on = true;
                else if(is_on)
                    is_passed = true;


                while(t_end < temp_it->_end && temp_it->append) {

                    if(((P_DATA) data)->per_buffer[t_end] == new_line) {
                            t_x = *((P_DATA) data)->x_offset;
                            *((P_DATA) data)->off_char = t_x;
                            _eol = true;
                            new_row_count++;
                    }else if(((P_DATA) data)->per_buffer[t_end] == '\t'){

                        t_x = (((t_x/((P_DATA) data)->s_w) / _Tab_Width) + 1) * _Tab_Width * ((P_DATA) data)->s_w;

                        if( t_x > *((P_DATA) data)->_width - ((P_DATA) data)->s_w) {
                            t_x %= (*((P_DATA) data)->_width - ((P_DATA) data)->s_w);
                            t_x = ((P_DATA) data)->s_w * (t_x / ((P_DATA) data)->s_w);
                            *((P_DATA) data)->off_char = t_x;
                            _eol = true;
                            new_row_count++;
                        }else
                            _eol = false;
                    }else {
                        t_x += ((P_DATA) data)->s_w;

                        if(t_x > *((P_DATA) data)->_width - ((P_DATA) data)->s_w) {
                            t_x = *((P_DATA) data)->x_offset;
                            *((P_DATA) data)->off_char = t_x;
                            _eol = true;
                            new_row_count++;
                        }else
                            _eol = false;
                    }



                    if(new_row_count >= row_count && _eol){

                        t_end += UTF8_CHAR_LEN(((P_DATA) data)->per_buffer[t_end]);
                        is_break = true;
                        break;
                    }

                    t_end += UTF8_CHAR_LEN(((P_DATA) data)->per_buffer[t_end]);

                }

                if(is_break)
                    break;

                temp_it++;

            }

            //std::cout << "New temp_it " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;


            /// delete current test_cp


            tt_it = ((P_DATA) data)->del_me2.erase(*((P_DATA) data)->test_cp);


            *((P_DATA) data)->test_cp = ((P_DATA) data)->del_me2.begin();



            if(temp_it != ((P_DATA) data)->del_me2.end()) {

                std::list<std::list<_span2>::iterator>::iterator t1 = ((P_DATA) data)->remit_list.end();
                bool is_break = false;

                while(t1 != (((P_DATA) data)->remit_list).begin()) {
                    if(*--t1 == tt_it) {
                        //std::cout << "[Resize] hmm\n";
                        //*t1 = ttt_it;
                        is_break = true;
                        break;
                    }
                }


                if(!is_break) {

                    std::list<_span2>::iterator ttt_it = tt_it;

                    while(!(--ttt_it)->append);

                    if(ttt_it->_end == tt_it->_start) {
                        ttt_it->_end = tt_it->_end;

                        if(tt_it == *((P_DATA) data)->it_cp) {

                            *((P_DATA) data)->char_pos += tt_it->_start;
                            *((P_DATA) data)->it_cp = ttt_it;

                        }else if(*((P_DATA) data)->char_pos)
                            *((P_DATA) data)->char_pos += (*((P_DATA) data)->it_cp)->_start;

                        if(temp_it == tt_it)
                            temp_it = ttt_it;

                        if(tt_it ==  *((P_DATA) data)->sel_cp) {
                            *((P_DATA) data)->sel_char += tt_it->_start; //(*((P_DATA) data)->it_cp)->_start;
                            *((P_DATA) data)->sel_cp = ttt_it;
                        }else if(*((P_DATA) data)->sel_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_char += (*((P_DATA) data)->sel_cp)->_start;


                        if(tt_it ==  *((P_DATA) data)->sel_end_cp) {
                            *((P_DATA) data)->sel_end_char += tt_it->_start; //(*((P_DATA) data)->it_cp)->_start;
                            *((P_DATA) data)->sel_end_cp = ttt_it;
                        }else if(*((P_DATA) data)->sel_end_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_end_char += (*((P_DATA) data)->sel_end_cp)->_start;


                        ((P_DATA) data)->del_me2.erase(tt_it);

                    }else{
                    if(*((P_DATA) data)->char_pos)
                        *((P_DATA) data)->char_pos += (*((P_DATA) data)->it_cp)->_start;
                    if(*((P_DATA) data)->sel_char)
                        *((P_DATA) data)->sel_char += (*((P_DATA) data)->sel_cp)->_start;
                    if(*((P_DATA) data)->sel_end_char)
                        *((P_DATA) data)->sel_end_char += (*((P_DATA) data)->sel_end_cp)->_start;
                    }

                }else{
                    if(*((P_DATA) data)->char_pos)
                        *((P_DATA) data)->char_pos += (*((P_DATA) data)->it_cp)->_start;
                    if(*((P_DATA) data)->sel_char)
                        *((P_DATA) data)->sel_char += (*((P_DATA) data)->sel_cp)->_start;
                    if(*((P_DATA) data)->sel_end_char)
                        *((P_DATA) data)->sel_end_char += (*((P_DATA) data)->sel_end_cp)->_start;
               }


                // Add New Test_cp


                if(is_passed || is_on && *((P_DATA) data)->char_pos <= t_end) {

                        *((P_DATA) data)->char_pos -= (*((P_DATA) data)->it_cp)->_start;
                        if(*((P_DATA) data)->sel_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_char -= (*((P_DATA) data)->sel_cp)->_start;
                        if(*((P_DATA) data)->sel_end_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_end_char -= (*((P_DATA) data)->sel_end_cp)->_start;

                        find_next_page(((P_DATA) data)->per_buffer, ((P_DATA) data)->del_me2, ((P_DATA) data)->remit_list,
                        ((P_DATA) data)->undo_type,
                        ((P_DATA) data)->test_cp, ((P_DATA) data)->it_cp,
                        ((P_DATA) data)->sel_cp, ((P_DATA) data)->sel_end_cp,
                        ((P_DATA) data)->remit_it,
                        ((P_DATA) data)->undo_type_it,
                        ((P_DATA) data)->char_pos, ((P_DATA) data)->sel_char, ((P_DATA) data)->sel_end_char,
                        ((P_DATA) data)->off_char, *((P_DATA) data)->_width, *((P_DATA) data)->_height,
                        ((P_DATA) data)->s_w, ((P_DATA) data)->s_h + ((P_DATA) data)->font_ascent,
                        *((P_DATA) data)->x_offset, *((P_DATA) data)->y_offset);


                }else if(t_end < temp_it->_end) {


                    std::list<_span2>::iterator to_put = ((P_DATA) data)->del_me2.begin();

                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = ((P_DATA) data)->remit_list.end();
                    std::list<char>::iterator tt_undo_it = ((P_DATA) data)->undo_type.end();
                    bool is_break = false;

                    if(t_end > temp_it->_start) {

                        while(tt_rem_it-- != ((P_DATA) data)->remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == temp_it && *tt_undo_it == 0 ) {

                              //  if(tt_rem_it == ((P_DATA) data)->remit_it) {

                              //  ((P_DATA) data)->remit_it = ((P_DATA) data)->remit_list.insert(tt_rem_it, ((P_DATA) data)->del_me2.insert(temp_it, {temp_it->_start, t_end, true}));
                              //  to_put = *((P_DATA) data)->remit_it;
                              //  }
                               // else
                                    to_put = *((P_DATA) data)->remit_list.insert(tt_rem_it, ((P_DATA) data)->del_me2.insert(temp_it, {temp_it->_start, t_end, true}));

                           /*     if(*tt_undo_it == 1) {

                                    if(tt_undo_it == ((P_DATA) data)->undo_type_it)
                                        ((P_DATA) data)->undo_type_it = ((P_DATA) data)->undo_type.insert(tt_undo_it, 1);
                                    else
                                        ((P_DATA) data)->undo_type.insert(tt_undo_it, 1);

                                    is_break = true;
                                }else{ */
                                    ((P_DATA) data)->undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;
                              //  }


                            }

                        }

                        if(!is_break)
                            to_put = ((P_DATA) data)->del_me2.insert(temp_it, {temp_it->_start, t_end, true});


                    //std::list<_span2>::iterator to_put = ((P_DATA) data)->del_me2.insert(temp_it, {temp_it->_start, t_end, true});

                        temp_it->_start = t_end;

                        if(temp_it == *((P_DATA) data)->sel_cp) {

                           // std::cout << "hmm_1\n";

                            if(*((P_DATA) data)->sel_char < to_put->_end) {
                                *((P_DATA) data)->sel_cp = to_put;
                                *((P_DATA) data)->sel_char -= to_put->_start;
                            }else
                                *((P_DATA) data)->sel_char -= t_end;

                        }else if (*((P_DATA) data)->sel_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_char -= (*((P_DATA) data)->sel_cp)->_start;


                        if(temp_it == *((P_DATA) data)->sel_end_cp) {

                            //std::cout << "hmm_2\n";
                            //std::cout << "sel_end_cp " << *((P_DATA) data)->sel_end_char << std::endl;

                            if(*((P_DATA) data)->sel_end_char < to_put->_end){
                                *((P_DATA) data)->sel_end_cp = to_put;
                                *((P_DATA) data)->sel_end_char -= to_put->_start;
                            }else
                                *((P_DATA) data)->sel_end_char -= t_end;

                        }else if (*((P_DATA) data)->sel_end_cp != ((P_DATA) data)->del_me2.end())
                            *((P_DATA) data)->sel_end_char -= (*((P_DATA) data)->sel_end_cp)->_start;

                        if(temp_it == *((P_DATA) data)->it_cp) {

                            *((P_DATA) data)->char_pos -= t_end;

                        }else if(!*((P_DATA) data)->char_pos && tt_it == ((P_DATA) data)->del_me2.end() ) {
                            while(!(--tt_it)->append);
                            *((P_DATA) data)->it_cp = tt_it;
                             *((P_DATA) data)->char_pos = tt_it->_end - tt_it->_start;
                        }else if(*((P_DATA) data)->char_pos)
                            *((P_DATA) data)->char_pos -= (*((P_DATA) data)->it_cp)->_start;

                        *((P_DATA) data)->test_cp = ((P_DATA) data)->del_me2.insert(temp_it, {0, 0, true});

                    }



                }else if(temp_it->_start != temp_it->_end) {

                    //std::cout << "that's the last resort\n";

                    *((P_DATA) data)->test_cp = ((P_DATA) data)->del_me2.insert(++temp_it, {0, 0, true});

                    while(++temp_it != ((P_DATA) data)->del_me2.end() && !temp_it->append );

                    //while(temp_it != ((P_DATA) data)->del_me2.end()) {

                    if(temp_it == *((P_DATA) data)->it_cp && temp_it == ((P_DATA) data)->del_me2.end()) {
                            while(!(--temp_it)->append);
                            if(temp_it == *((P_DATA) data)->test_cp) {
                                while(++temp_it != ((P_DATA) data)->del_me2.end() && !temp_it->append);
                                *((P_DATA) data)->it_cp = temp_it;
                                *((P_DATA) data)->char_pos = 0;
                            }else {
                                *((P_DATA) data)->it_cp = temp_it;
                                *((P_DATA) data)->char_pos = temp_it->_end;
                            }
                    }


                    //}

                    if(*((P_DATA) data)->char_pos)
                        *((P_DATA) data)->char_pos -= (*((P_DATA) data)->it_cp)->_start;
                    if(*((P_DATA) data)->sel_char)
                        *((P_DATA) data)->sel_char -= (*((P_DATA) data)->sel_cp)->_start;
                    if(*((P_DATA) data)->sel_end_char)
                        *((P_DATA) data)->sel_end_char -= (*((P_DATA) data)->sel_end_cp)->_start;




                }else {
                    *((P_DATA) data)->test_cp = temp_it;

                    if(*((P_DATA) data)->char_pos)
                        *((P_DATA) data)->char_pos -= (*((P_DATA) data)->it_cp)->_start;
                    if(*((P_DATA) data)->sel_char)
                        *((P_DATA) data)->sel_char -= (*((P_DATA) data)->sel_cp)->_start;
                    if(*((P_DATA) data)->sel_end_char)
                        *((P_DATA) data)->sel_end_char -= (*((P_DATA) data)->sel_end_cp)->_start;

                }


            }else {
                *((P_DATA) data)->test_cp = ((P_DATA) data)->del_me2.insert(tt_it, {0, 0, true});
                if(tt_it == ((P_DATA) data)->del_me2.end()) {
                    *((P_DATA) data)->it_cp = tt_it;
                    *((P_DATA) data)->char_pos = 0;
                }

            }


        }


        /// Resizing Procedures
/*
        for(auto i : ((P_DATA) data)->del_me2)
            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
        std::cout << "From [Resize]*******\n";
*/

        SDL_FreeSurface(*((P_DATA) data)->screen);

        SDL_SetWindowSize(win, *((P_DATA) data)->_width, *((P_DATA) data)->_height);

        *((P_DATA) data)->screen = SDL_GetWindowSurface(win);

        adv_cursor_while(((P_DATA) data)->per_buffer, *((P_DATA) data)->test_cp, *((P_DATA) data)->it_cp, ((P_DATA) data)->cr1, *((P_DATA) data)->char_pos, *((P_DATA) data)->off_char, *((P_DATA) data)->y_offset, ((P_DATA) data)->s_w, ((P_DATA) data)->s_h + ((P_DATA) data)->font_ascent, *((P_DATA) data)->_width);

        if((((P_DATA) data)->cr1).y >= *((P_DATA) data)->_height) {

            //if(*((P_DATA) data)->sel_cp == *((P_DATA) data)->sel_end_cp || ((*((P_DATA) data)->sel_cp)->_start + *((P_DATA) data)->sel_char == (*((P_DATA) data)->sel_end_cp)->_start + *((P_DATA) data)->sel_end_char)) {

            find_next_page(((P_DATA) data)->per_buffer, ((P_DATA) data)->del_me2, ((P_DATA) data)->remit_list,
            ((P_DATA) data)->undo_type,
            ((P_DATA) data)->test_cp, ((P_DATA) data)->it_cp,
            ((P_DATA) data)->sel_cp, ((P_DATA) data)->sel_end_cp,
            ((P_DATA) data)->remit_it,
            ((P_DATA) data)->undo_type_it,
            ((P_DATA) data)->char_pos, ((P_DATA) data)->sel_char, ((P_DATA) data)->sel_end_char,
            ((P_DATA) data)->off_char, *((P_DATA) data)->_width, *((P_DATA) data)->_height,
            ((P_DATA) data)->s_w, ((P_DATA) data)->s_h + ((P_DATA) data)->font_ascent,
            *((P_DATA) data)->x_offset, *((P_DATA) data)->y_offset);

            //}


            adv_cursor_while(((P_DATA) data)->per_buffer, *((P_DATA) data)->test_cp, *((P_DATA) data)->it_cp, ((P_DATA) data)->cr1, *((P_DATA) data)->char_pos, *((P_DATA) data)->off_char, *((P_DATA) data)->y_offset, ((P_DATA) data)->s_w, ((P_DATA) data)->s_h + ((P_DATA) data)->font_ascent, *((P_DATA) data)->_width);

            //std::cout << "That's it my baby\n";

        }


        _Render(((P_DATA) data)->per_buffer, ((P_DATA) data)->del_me2, *((P_DATA) data)->test_cp, *((P_DATA) data)->_width, *((P_DATA) data)->_height, ((P_DATA) data)->s_w, ((P_DATA) data)->s_h, *((P_DATA) data)->x_offset, *((P_DATA) data)->y_offset, ((P_DATA) data)->font_ascent, *((P_DATA) data)->off_char, ((P_DATA) data)->font_map, *((P_DATA) data)->screen, ((P_DATA) data)->font_atlas, ((P_DATA) data)->key_color);

        if(*((P_DATA) data)->sel_cp != *((P_DATA) data)->sel_end_cp || ((*((P_DATA) data)->sel_cp)->_start + *((P_DATA) data)->sel_char != (*((P_DATA) data)->sel_end_cp)->_start + *((P_DATA) data)->sel_end_char)) {

            _Sel_Render(((P_DATA) data)->per_buffer, ((P_DATA) data)->del_me2, *((P_DATA) data)->test_cp, *((P_DATA) data)->sel_cp, *((P_DATA) data)->sel_end_cp, *((P_DATA) data)->_width, *((P_DATA) data)->_height, ((P_DATA) data)->s_w, ((P_DATA) data)->s_h + ((P_DATA) data)->font_ascent, ((P_DATA) data)->_cur_height, *((P_DATA) data)->x_offset, *((P_DATA) data)->y_offset, *((P_DATA) data)->off_char, *((P_DATA) data)->sel_char, *((P_DATA) data)->sel_end_char, *((P_DATA) data)->screen);

        }



        SDL_UpdateWindowSurface(((P_DATA) data)->_my_win);
    }
  }
  return 0;
}


int SDL_main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    std::unordered_map<std::string, __cursor> font_map;
    size_t char_pos = 0, clip_sel_char = 0, clip_sel_start = 0;
    int32_t old_x = 0;

    int s_w, s_h, font_ascent, y_offset = 0, _width = 900, _height = 800;   // 117 15
    SDL_Surface *font_atlas = prepare_font_atlas(14, &s_w, &s_h, &font_ascent, font_map);
    int _cur_height = s_h + font_ascent, _cur_width = s_w;
    _height = (_height / (s_h + font_ascent) + 1) * (s_h + font_ascent);
    _width = (_width / s_w + 1) * s_w;
    int x_offset = 0; //(s_w + 5) * _Tab_Width;
    bool shift_pressed = false;
    bool shift_was_pressed = false;
    bool ctrl_pressed = false;
    bool is_copied = false;
    bool clip_pager = false;
    bool is_edit = false;
    bool must_change = false;
    bool insert_toggle = false;
    int ins_offset = 0;


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


    std::cout << "Pixel format = " << SDL_GetPixelFormatName((Cursor->format)->format) << std::endl;
    std::cout << "Size of _span2 " << sizeof(_span2) / sizeof(char) << " bytes." << std::endl;


    Uint32 Cursor_Color = SDL_MapRGBA(Cursor->format, 250, 250, 250, 120);

    //Uint32 *_pixels = (Uint32*) Cursor->pixels;

    SDL_FillRect(Cursor, NULL, Cursor_Color);

    //for(int i = 0; i < _cur_height; i++)
    //    for(int j = 0; j < _cur_width; j++)
    //        _pixels[(j+0) + (0 + i) * Cursor->w] = Cursor_Color;




    Uint32 key_color = SDL_MapRGBA(screen->format, 4, 27, 41, 255);
    //SDL_GetColorKey(screen, &key_color);
    SDL_FillRect(screen, NULL, key_color);



    std::list<_span2> clip_board;
    std::list<_span2> del_me2;
    std::list<_span2> paste_board;
    std::string per_buffer;
    std::string new_buffer;

    std::list<size_t> dec_char;
    std::list<std::list<_span2>::iterator> remit_list;
    std::list<std::list<std::list<_span2>::iterator>::iterator> remit_list_del;
    std::list<std::list<std::list<_span2>::iterator>::iterator> remit_list_undo;
    std::list<char> undo_type;

    std::list<_span2>::iterator it_cp = del_me2.begin();
    std::list<_span2>::iterator copy_it = clip_board.begin();
    std::list<_span2>::iterator clip_it = clip_board.begin();
    std::list<_span2>::iterator clip_test = clip_board.begin();
    std::list<_span2>::iterator test_cp = del_me2.insert(del_me2.begin(), {0, 0, true});

    std::list<size_t>::iterator dec_char_it = dec_char.begin();
    std::list<std::list<_span2>::iterator>::iterator remit_it = remit_list.begin();
    //std::list<std::list<_span2>::iterator>::iterator remit_it_ctrl = remit_list.begin();
    //std::list<std::list<_span2>::iterator>::iterator remit_it_ctrl_undo = remit_list.begin();
    //std::list<std::list<std::list<_span2>::iterator>::iterator>::iterator remit_it_del = remit_list_del.begin();
    std::list<char>::iterator undo_type_it = undo_type.begin();


    if(argc > 1) {
        std::ifstream ofs(argv[1], std::ifstream::binary);
        std::basic_stringstream<char> test_2(std::stringstream::in | std::stringstream::out);

        if(ofs.is_open()) {
            ofs.seekg(0, ofs.end);
            size_t _length = ofs.tellg();
            std::cout << "size of the file is " << _length << " bytes. " << std::endl;
            ofs.seekg(0, ofs.beg);





            char *buffer = new char[16385];

            memset(buffer, 0, 16385);

            size_t total_count = 0;

            /// TO check if there is a CR char in buffer

            ofs.getline(buffer, 16385);


            if(!ofs.eof()) {

                if(buffer[ofs.tellg() - 2] != 0x0d){
                    new_line = '\n';
                    ctrl_line = '\r';
                    step_count = 1;
                    std::cout << "unix file\n";
                }
            }


            memset(buffer, 0, ofs.tellg());
            ofs.seekg(0, ofs.beg);




            if(_length < 16384) {

                memset(buffer, 0, _length + 1);
                ofs.read(buffer, _length);
                per_buffer += buffer;
            }else {

                while(ofs.read(buffer, 16384)) {
                    //std::cout << "hmm is it ok?\n";
                    total_count += ofs.gcount();
                    per_buffer += buffer;
                    if(_length - total_count < 16384) {

                        char *last_buffer = new char[_length - total_count + 1];

                        memset(last_buffer, 0, _length - total_count + 1);

                        ofs.read(last_buffer, _length - total_count);

                        per_buffer += last_buffer;

                        if(last_buffer)
                            delete[] last_buffer;

                        break;
                    }
                }
            }


            //test_2 << ofs.rdbuf();
            //char c;
            //while(ofs.get(c))
            //    per_buffer += c;
            //per_buffer = test_2.str();

            std::cout << "size of the buffer : " << per_buffer.size() << " bytes." << std::endl;
            it_cp = del_me2.insert(it_cp, {0, per_buffer.size(), true});
            delete[] buffer;
            ofs.close();
        }
    }



    //add a trivial char
    per_buffer += ".";

    size_t off_char = 0, page_off_char = 0;
    std::list<_span2>::iterator sel_cp = test_cp;
    std::list<_span2>::iterator sel_end_cp = test_cp;

    //change_board.insert(ud_cp, {del_me2, it_cp, char_pos});


    size_t sel_char = 0, sel_end_char = 0, paste_char = 0, paste_end_char = 0, paste_start = 0, paste_end_start = 0;

   // for(auto i : del_me2)
   //     std::cout << i._start << " " << i._end << " " << i.append << std::endl;
   // std::cout << "*******\n";

    //std::cout << "Maximum Edit size " << del_me2.max_size() << std::endl;


    //inserting the cursor

    __cursor cr1 = {x_offset, y_offset};



    _DATA my_data = {my_test, per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, &sel_cp, &sel_end_cp, remit_it, undo_type_it, &char_pos, &sel_char, &sel_end_char, cr1, &_width, &_height, s_w, s_h, _cur_height, &x_offset, &y_offset, font_ascent, &off_char, font_map, &screen, font_atlas, key_color};
    P_DATA p_my_data = &my_data;

    SDL_AddEventWatch(resizingEventWatcher, p_my_data);

    SDL_Cursor* my_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    SDL_SetCursor(my_cursor);

    //SDL_ShowCursor(SDL_DISABLE);



    SDL_Event e = {};

/*
    SDL_Event del_event = {};

    del_event.type = SDL_KEYDOWN;
    del_event.key.timestamp = 0;
    del_event.key.windowID = 0;
    del_event.key.state = SDL_PRESSED;
    del_event.key.repeat = 0;
    del_event.key.keysym.scancode = SDL_SCANCODE_DELETE;
    del_event.key.keysym.sym = SDLK_DELETE;
    del_event.key.keysym.mod = KMOD_NONE;
*/


    Uint64 last_time = SDL_GetTicks64(), start = 0, lastEvent_Timer= 0, lastEvent_Time = 0;
    int frames = 0;
    bool blink_on = true, lost_focus = false;


    SDL_Rect dest_rect = {cr1.x, cr1.y, _cur_width, _cur_height};
    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
    SDL_UpdateWindowSurface(my_test);

    for(;e.type != SDL_QUIT; SDL_PollEvent(&e)) {

        if((start = SDL_GetTicks64()) > last_time + _Cursor_Delay) {
            last_time = start;
            frames = 0;
            if (blink_on){
/*
                //Inverting the colors of the particular pixels
                Uint32 *_pixels = (Uint32*) screen->pixels;
                for(int i = 0; i < _cur_height; i++)
                    for(int j = 0; j < _cur_width; j++)
                        _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;
*/

                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                SDL_BlitSurface(screen, &temp_rect, Temp_Surface, NULL);
                SDL_BlitSurface(Cursor, NULL, screen, &temp_rect);

                blink_on = false;

                SDL_UpdateWindowSurface(my_test);
            }
            else {
/*
                Uint32 *_pixels = (Uint32*) screen->pixels;
                for(int i = 0; i < _cur_height; i++)
                    for(int j = 0; j < _cur_width; j++)
                        _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w] = (0xFFFFFFFF - _pixels[(j+dest_rect.x) + (dest_rect.y + i)*screen->w])  | 0xFF000000;

*/
                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);

                blink_on = true;

                SDL_UpdateWindowSurface(my_test);
            }
        }

        lastEvent_Time = lost_focus ? lastEvent_Time : start;
        frames++;

        if(e.type == SDL_KEYUP) {

            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                shift_was_pressed = true;
                shift_pressed = false;
            }

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                ctrl_pressed = false;
            }

        }

        if(e.type == SDL_KEYDOWN) {

            if(e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {

                if(!shift_pressed){
                    if( sel_cp == sel_end_cp && (sel_cp->_start + sel_char == sel_end_cp->_start + sel_end_char)) {
                        sel_cp = it_cp;
                        sel_char = char_pos;
                        sel_end_char = char_pos;
                        sel_end_cp = it_cp;

                        if(it_cp->_start + char_pos == it_cp->_end) {
                            while(!(++sel_cp)->append);
                            sel_char = 0;
                            while(!(++sel_end_cp)->append);
                            sel_end_char = 0;
                        }


                        clip_pager = false;

                    }

                    shift_pressed = true;
                    shift_was_pressed = false;

                }

            }

            if(e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                if(!ctrl_pressed)
                    ctrl_pressed = true;

            }


            if(e.key.keysym.sym == SDLK_s) {

                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }


                    std::cout << "Saving...\n";


                    char const *lFilterPatterns[] = { "*.txt", "*.cpp", "*" };

                    char const *selection = tinyfd_saveFileDialog("Save File", "c:\\", 3, lFilterPatterns, NULL);

                    if(selection) {

                        std::ofstream ofs(selection, std::ofstream::binary);

                        if(ofs.is_open()) {

                            //char *buffer = new char[16384];
                            const char *p_buffer = per_buffer.data();

                            for(auto i : del_me2)
                                if(i.append)
                                    ofs.write(p_buffer + i._start, i._end - i._start);
                        }


                        ofs.close();

                        std::cout << "File Saved\n";
                    }





                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;
                }
            }



            if(e.key.keysym.sym == SDLK_o) {

                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    std::cout << "It's about the open file " << std::endl;

                    new_line = '\r'; ctrl_line = '\n'; step_count = 2;

                    del_me2.clear();
                    it_cp = del_me2.begin();
                    test_cp = del_me2.insert(del_me2.begin(), {0, 0, true});
                    char_pos = 0;

                    per_buffer.resize(0);
                    per_buffer.shrink_to_fit();
                    //per_buffer.erase(per_buffer.begin(), per_buffer.end());
                    //per_buffer.clear();

                    char const *lFilterPatterns[] = { "*.txt", "*.cpp", "*" };

                    char const *selection = tinyfd_openFileDialog("Select File", "C:\\", 3, lFilterPatterns, NULL, 0);


                    if(selection) {
                        std::ifstream ofs(selection, std::ifstream::binary);
                        //std::basic_stringstream<char> test_2(std::stringstream::in | std::stringstream::out);

                        if(ofs.is_open()) {
                            ofs.seekg(0, ofs.end);
                            size_t _length = ofs.tellg();
                            std::cout << "size of the file is " << _length << " bytes. " << std::endl;
                            ofs.seekg(0, ofs.beg);

                            char *buffer = new char[16385];

                            memset(buffer, 0, 16385);

                            size_t total_count = 0;

                            /// TO check if there is a CR char in buffer

                            ofs.getline(buffer, 16385);


                            if(!ofs.eof()) {

                                if(buffer[ofs.tellg() - 2] != 0x0d){
                                    new_line = '\n';
                                    ctrl_line = '\r';
                                    step_count = 1;
                                    std::cout << "unix file\n";
                                }
                            }


                            memset(buffer, 0, ofs.tellg());
                            ofs.seekg(0, ofs.beg);


                            if(_length < 16384) {

                                memset(buffer, 0, _length + 1);
                                ofs.read(buffer, _length);
                                per_buffer += buffer;
                            }else {

                                while(ofs.read(buffer, 16384)) {
                                    //std::cout << "hmm is it ok?\n";
                                    total_count += ofs.gcount();
                                    per_buffer += buffer;
                                    if(_length - total_count < 16384) {

                                        char *last_buffer = new char[_length - total_count + 1];

                                        memset(last_buffer, 0, _length - total_count + 1);

                                        ofs.read(last_buffer, _length - total_count);

                                        per_buffer += last_buffer;

                                        if(last_buffer)
                                            delete[] last_buffer;

                                        break;
                                    }
                                }
                            }


                            //test_2 << ofs.rdbuf();
                            //char c;
                            //while(ofs.get(c))
                            //    per_buffer += c;
                            //per_buffer = test_2.str();

                            std::cout << "size of the buffer : " << per_buffer.size() << " bytes." << std::endl;
                            it_cp = del_me2.insert(it_cp, {0, per_buffer.size(), true});
                            delete[] buffer;
                            ofs.close();
                        }
                    }

                    per_buffer += ".";


                    cr1.x = x_offset; cr1.y = y_offset; off_char = x_offset;

                    remit_list.clear();
                    paste_board.clear();
                    undo_type.clear();

                    remit_it = remit_list.begin();
                    undo_type_it = undo_type.begin();

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    shift_was_pressed = false;

                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset,  s_w, s_h + font_ascent, _width);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                }

            }


            if(e.key.keysym.sym == SDLK_INSERT) {

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(insert_toggle)
                    insert_toggle = false;
                else
                    insert_toggle = true;

                if(insert_toggle) {

                    _cur_width = s_w;
                    _cur_height = 2;
                    ins_offset = s_h;
                }else{
                    _cur_width = s_w;
                    _cur_height = s_h + font_ascent;
                    ins_offset = 0;
                }



                SDL_FreeSurface(Cursor);
                SDL_FreeSurface(Temp_Surface);

                Cursor = SDL_CreateRGBSurface(0, _cur_width , _cur_height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
                Temp_Surface = SDL_CreateRGBSurfaceWithFormat(0, Cursor->w , Cursor->h, 32, screen->format->format);


                Cursor_Color = SDL_MapRGBA(Cursor->format, 250, 250, 250, 120);

                SDL_FillRect(Cursor, NULL, Cursor_Color);



                std::cout << "insert is " << insert_toggle << std::endl;

                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_y) {

                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }


                    if(undo_type_it != undo_type.end()) {


                        switch(*undo_type_it) {

                            case 1:

                                //std::cout << "Redo type " << (int) *undo_type_it << std::endl;

                                while(undo_type_it != undo_type.end() && *undo_type_it == 1) {



                                    if(it_cp == del_me2.end()) {
                                        del_me2.erase(test_cp);
                                        test_cp = del_me2.begin();
                                    }

                                    it_cp = *remit_it;
                                    char_pos = it_cp->_end - it_cp->_start;


                                    (*remit_it++)->append = false;

                                    while(!(--it_cp)->append);

                                    if(it_cp->_start == it_cp->_end) {
                                        if(it_cp == del_me2.begin()) {
                                            test_cp = del_me2.begin();
                                            while(++it_cp != del_me2.end() && !it_cp->append);
                                            char_pos = 0;
                                        }else {
                                            test_cp = del_me2.begin();
                                            it_cp = del_me2.erase(it_cp);
                                            while(!(--it_cp)->append);
                                            char_pos = it_cp->_end - it_cp->_start;

                                        }
                                    }else
                                        char_pos = it_cp->_end - it_cp->_start;



                                    undo_type_it++;
                                }

                                //if(*undo_type_it != 0)
                                //    ++undo_type_it;

                                break;

                            case 0:

                                //std::cout << "Redo Type " << (int) *undo_type_it << std::endl;

                                while(undo_type_it != undo_type.end() && *undo_type_it == 0) {

                                  //  if(it_cp == del_me2.end()) {
                                  //      del_me2.erase(test_cp);
                                  //      test_cp = del_me2.begin();
                                  //  }


                                    it_cp = *remit_it;
                                    char_pos = it_cp->_end - it_cp->_start;

                                    (*remit_it++)->append = true;
/*
                                    while(!(--it_cp)->append);

                                    if(it_cp->_start == it_cp->_end) {
                                        if(it_cp == del_me2.begin()) {
                                            //test_cp = del_me2.begin();
                                            while(++it_cp != del_me2.end() && !it_cp->append);
                                            char_pos = 0;
                                        }else {
                                            test_cp = del_me2.begin();
                                            it_cp = del_me2.erase(it_cp);
                                            while(!(--it_cp)->append);
                                            char_pos = it_cp->_end - it_cp->_start;

                                        }
                                    }else
                                        char_pos = it_cp->_end - it_cp->_start;

*/

                                    undo_type_it++;
                                }

                               // if(*undo_type_it != 1)
                               //     ++undo_type_it;


                                break;

                            case 2:

                                break;

                        }
/*
                         for(auto i : del_me2)
                           std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                         std::cout << "From UNDO*******\n";
*/

                        /// Look up

                        int16_t t_x = off_char, t_y = y_offset;
                        bool is_breaked = false;

                        std::list<_span2>::iterator temp_it = test_cp;

                        while(++temp_it != del_me2.end() && t_y < _height) {


                            size_t start_pos = temp_it->_start;

                            while(start_pos < temp_it->_end && t_y < _height && temp_it->append) {


                                if(per_buffer[start_pos] == new_line){
                                    t_x = x_offset;
                                    t_y += (s_h + font_ascent);
                                }else if(per_buffer[start_pos] == '\t'){

                                    t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                    if( t_x > _width - s_w ) {
                                        t_x %= (_width - s_w);// t_x + x_offset - (_width - s_w);
                                        t_x = s_w * (t_x / s_w);
                                        t_y += (s_h + font_ascent);
                                    }
                                }else {
                                    t_x += s_w;
                                    if( t_x > _width - s_w) {
                                        t_x = x_offset;
                                        t_y += (s_h + font_ascent);
                                    }
                                }

                                start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                            }

                            if(temp_it == it_cp && start_pos >= it_cp->_start + char_pos ) {
                                //std::cout << "should be breaked\n";
                                is_breaked = true;
                                break;
                            }

                        }

                       // std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                        if(!is_breaked && it_cp != del_me2.end())
                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);



                        if(cr1.y < 0)
                            y_offset = 0;


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset,  s_w, s_h + font_ascent, _width);

                      //  std::cout << "[REDO] it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;



/*
                         for(auto i : del_me2)
                           std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                         std::cout << "From UNDO*******\n";
*/

                    }



                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    shift_was_pressed = false;


                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                }
            }

            if(e.key.keysym.sym == SDLK_z) {

                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }


                    if(undo_type_it != undo_type.begin()) {


                        switch(*--undo_type_it) {

                            case 0:

                               // std::cout << (int) *undo_type_it << std::endl;

                                do  {

                                    if(it_cp == del_me2.end()) {
                                        del_me2.erase(test_cp);
                                        test_cp = del_me2.begin();
                                    }

                                    it_cp = *--remit_it;
                                    (*remit_it)->append = false;

                                    while(!(--it_cp)->append);

                                    if(it_cp->_start == it_cp->_end) {
                                        if(it_cp == del_me2.begin()) {
                                            //test_cp = del_me2.begin();
                                            while(++it_cp != del_me2.end() && !it_cp->append);
                                            char_pos = 0;
                                        }else {
                                            test_cp = del_me2.begin();
                                            it_cp = del_me2.erase(it_cp);
                                            while(!(--it_cp)->append);
                                            char_pos = it_cp->_end - it_cp->_start;

                                        }
                                    }else
                                        char_pos = it_cp->_end - it_cp->_start;




                                }while(undo_type_it != undo_type.begin() && *--undo_type_it == 0);

                                if(*undo_type_it != 0)
                                    ++undo_type_it;

                                break;

                            case 1:

                             //   std::cout << (int) *undo_type_it << std::endl;

                                do {

                                  //  if(it_cp == del_me2.end()) {
                                  //      del_me2.erase(test_cp);
                                  //      test_cp = del_me2.begin();
                                  //  }


                                    it_cp = *--remit_it;

                                    (*remit_it)->append = true;

                                    while(!(--it_cp)->append);

                                    if(it_cp->_start == it_cp->_end) {
                                        if(it_cp == del_me2.begin()) {
                                            //test_cp = del_me2.begin();
                                            while(++it_cp != del_me2.end() && !it_cp->append);
                                            char_pos = 0;
                                        }else {
                                            test_cp = del_me2.begin();
                                            it_cp = del_me2.erase(it_cp);
                                            while(!(--it_cp)->append);
                                            char_pos = it_cp->_end - it_cp->_start;

                                        }
                                    }else
                                        char_pos = it_cp->_end - it_cp->_start;


                                }while(undo_type_it != undo_type.begin() && *--undo_type_it == 1);

                                if(*undo_type_it != 1)
                                    ++undo_type_it;


                                break;

                            case 2:

                                break;

                        }


                        /// Look up

                        int16_t t_x = off_char, t_y = y_offset;
                        bool is_breaked = false;

                        std::list<_span2>::iterator temp_it = test_cp;

                        while(++temp_it != del_me2.end() && t_y < _height) {


                            size_t start_pos = temp_it->_start;

                            while(start_pos < temp_it->_end && t_y < _height && temp_it->append) {


                                if(per_buffer[start_pos] == new_line){
                                    t_x = x_offset;
                                    t_y += (s_h + font_ascent);
                                }else if(per_buffer[start_pos] == '\t'){

                                    t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                    if( t_x > _width - s_w ) {
                                        t_x %= (_width - s_w);// t_x + x_offset - (_width - s_w);
                                        t_x = s_w * (t_x / s_w);
                                        t_y += (s_h + font_ascent);
                                    }
                                }else {
                                    t_x += s_w;
                                    if( t_x > _width - s_w) {
                                        t_x = x_offset;
                                        t_y += (s_h + font_ascent);
                                    }
                                }

                                start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                            }

                            if(temp_it == it_cp && start_pos >= it_cp->_start + char_pos ) {
                                //std::cout << "should be breaked\n";
                                is_breaked = true;
                                break;
                            }

                        }

                       // std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                        if(!is_breaked && it_cp != del_me2.end())
                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);


                        if(cr1.y < 0)
                            y_offset = 0;

                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                       // std::cout << "[UNDO] it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                        size_t t_total = 0;




                         for(auto i : del_me2) {
                            if(i.append)
                                t_total += (i._end - i._start);
                            //std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                         }
/*
                        for(auto i : del_me2)
                            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "From UNDO*******\n";
*/
                         std::cout << "From UNDO total size = " << t_total << std::endl;


                    }

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    shift_was_pressed = false;


                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                }
            }

            if(e.key.keysym.sym == SDLK_a) {

                if(ctrl_pressed) {

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                    //std::cout << "ALL Selected\n";

                    it_cp = del_me2.end();

                    char_pos = 0;

                    while(!(--it_cp)->append);


                    if(it_cp != del_me2.begin()) {

                        while(++it_cp != del_me2.end() && !it_cp->append);
                        find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                        &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                        x_offset, y_offset);

                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                        while(!(--it_cp)->append);
                        if(it_cp != test_cp)
                            char_pos = it_cp->_end - it_cp->_start;
                        else
                            it_cp = del_me2.end();

                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);


                  //  std::cout << "[ALL] it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;



                    sel_end_cp = del_me2.end();
                    sel_end_char = 0;
                    sel_char = 0;
                    sel_cp = del_me2.begin();
                    while(++sel_cp != del_me2.end() && !sel_cp->append);
                    //sel_cp++;
                    shift_was_pressed = true;
                    //clip_pager = false;


                    if(sel_cp != sel_end_cp) {


                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                        if(cr1.y + _cur_height > _height){

                            //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                            y_offset += _height - (cr1.y + _cur_height);
                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                            cr1.y = _height - _cur_height;

                        }



                        _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                    x_offset, y_offset, off_char, sel_char, sel_end_char, screen);


                    }else {
                        sel_cp = it_cp;
                        sel_char = char_pos;
                        sel_end_char = char_pos;
                        sel_end_cp = it_cp;
                    }


                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;


                }

            }


            if(e.key.keysym.sym == SDLK_c) {
                if(ctrl_pressed) {

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                    //std::cout << "copied\n";

                    ///following is for the paste_board
                    std::list<_span2>::iterator sel_it = sel_cp;
                    paste_board.clear();
                    std::list<_span2>::iterator paste_it = paste_board.begin();

                    if(sel_cp != sel_end_cp) {
                        //std::cout << "not equal\n";
                        paste_board.insert(paste_it, {sel_it->_start + sel_char, sel_it->_end, true});

                        while(++sel_it != sel_end_cp) {

                            if(sel_it->_start != sel_it->_end && sel_it->append) {
                                if(paste_board.back()._end == sel_it->_start)
                                    paste_board.back()._end = sel_it->_end;
                                else
                                    paste_board.insert(paste_it, {sel_it->_start, sel_it->_end, true});
                            }
                        }


                        if(sel_it != del_me2.end() && sel_end_char) {
                            if(paste_board.back()._end == sel_it->_start)
                                paste_board.back()._end = sel_it->_start + sel_end_char;
                            else
                                paste_board.insert(paste_it, {sel_it->_start, sel_it->_start + sel_end_char, true});
                        }



                    }else if(sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)
                        paste_board.insert(paste_it, {sel_it->_start + sel_char, sel_end_cp->_start + sel_end_char, true});


                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                }

            }

            if(e.key.keysym.sym == SDLK_x) {
                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }


                    if(sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)) {


                        if(remit_it != remit_list.end()) {
                            remit_it = remit_list.erase(remit_it, remit_list.end());
                            undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                        }

                        ///following is for the cut & paste & remit section


                        //paste_board.clear();
                        //std::list<_span2>::iterator paste_it = paste_board.begin();
                        std::list<_span2>::iterator paste_it = paste_board.erase(paste_board.begin(), paste_board.end());
                        std::list<_span2>::iterator sel_it = sel_cp;


                        it_cp = sel_cp;

                        //size_t th_count = 0;
                        bool has_test = false;

                        do {


                            if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                                if(sel_it == test_cp) {
                                    has_test = true;
                                    sel_it = del_me2.erase(test_cp);
                                    test_cp = del_me2.begin();
                                    if(sel_it == del_me2.end())
                                        continue;
                                }else
                                    continue;
                            }


                            size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                            size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                            if(t_start > sel_it->_start) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it  && *tt_undo_it == 0 ) {


                                        remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                                sel_it->_start = t_start;

                            }


                            if(t_end < sel_it->_end) {

                                if(t_start < t_end) {

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<_span2>::iterator to_put = del_me2.end();
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == sel_it && *tt_undo_it == 0 ) {


                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;

                                        }

                                    }

                                    if(!is_break)
                                        to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                    paste_board.insert(paste_it, {to_put->_start, to_put->_end, true});
                                    remit_list.insert(remit_it, to_put);
                                    undo_type.insert(undo_type_it, 1);
                                    sel_it->_start = t_end;
                                }


                            }else {

                                sel_it->append = false;
                                paste_board.insert(paste_it, {sel_it->_start, sel_it->_end, true});
                                remit_list.insert(remit_it, sel_it);
                                undo_type.insert(undo_type_it, 1);

                            }



                        }while (sel_it++ != sel_end_cp);


                       while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            char_pos = 0;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;






                        if(has_test && it_cp != del_me2.end()) {
                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);
                        }

/*
                        for(auto i : undo_type)
                            std::cout << (int) i << std::endl;
                        std::cout << "From X undo_type_list*******\n";

                        for(auto i : remit_list)
                            std::cout << (i)->_start << " " << (i)->_end << " " << (i)->append << std::endl;
                        std::cout << "From X remit_list*******\n";

                        for(auto i : paste_board)
                            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "From X PasteBoard*******\n";


                        for(auto i : del_me2)
                            std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "From X*******\n";

*/
                        //std::cout << "[X] it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;



                        sel_cp = it_cp;
                        sel_end_cp = it_cp;
                        sel_char = char_pos;
                        sel_end_char = char_pos;
                        shift_was_pressed = false;


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                        old_x = cr1.x;


                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }


                    }

                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                }

            }


            if(e.key.keysym.sym == SDLK_v) {


                if(ctrl_pressed) {

                    if(!blink_on){
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }


                    if(sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)) {

                        if(remit_it != remit_list.end()) {
                            remit_it = remit_list.erase(remit_it, remit_list.end());
                            undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                        }

                        ///following is for the cut & remit section

                        std::list<_span2>::iterator sel_it = sel_cp;

                        it_cp = sel_cp;

                        //size_t th_count = 0;
                        bool has_test = false;

                        do {

                            if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                                if(sel_it == test_cp) {
                                    has_test = true;
                                    sel_it = del_me2.erase(test_cp);
                                    test_cp = del_me2.begin();
                                    if(sel_it == del_me2.end())
                                        continue;
                                }else
                                    continue;
                            }



                            size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                            size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                            if(t_start > sel_it->_start) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it && *tt_undo_it == 0 ) {


                                        remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;

                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                                sel_it->_start = t_start;

                            }


                            if(t_end < sel_it->_end) {

                                if(t_start < t_end) {

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<_span2>::iterator to_put = del_me2.end();
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == sel_it  &&  *tt_undo_it == 0 ) {


                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;

                                        }

                                    }

                                    if(!is_break)
                                        to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                    remit_list.insert(remit_it, to_put);
                                    undo_type.insert(undo_type_it, 1);
                                    sel_it->_start = t_end;
                                }


                            }else {

                                sel_it->append = false;
                                remit_list.insert(remit_it, sel_it);
                                undo_type.insert(undo_type_it, 1);

                            }



                        }while (sel_it++ != sel_end_cp);



                       while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            char_pos = 0;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;




                        if(has_test && it_cp != del_me2.end()) {
                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);

                        }


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset,  s_w, s_h + font_ascent, _width);

                        if(cr1.y < 0) {
                            y_offset = 0;
                            adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                        }

                    }

                    if(paste_board.size() || SDL_HasClipboardText()) {


                        if(remit_it != remit_list.end()) {
                            remit_it = remit_list.erase(remit_it, remit_list.end());
                            undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                        }

                        if(it_cp->_start + char_pos < it_cp->_end && it_cp != del_me2.end()) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            if(it_cp->_start + char_pos != it_cp->_start) {

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {


                                        remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }
                                }

                                if(!is_break)
                                    del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});

                                it_cp->_start += char_pos;
                            }

                        }else if(it_cp != del_me2.end())
                            ++it_cp;

                        size_t t_start = per_buffer.size();

                        if(paste_board.size()) {
                            for(auto i : paste_board) {

                                for(size_t s_tart = i._start; s_tart != i._end; s_tart += UTF8_CHAR_LEN(per_buffer[s_tart])) {
                                    for(size_t i = 0; i != UTF8_CHAR_LEN(per_buffer[s_tart]); i++)
                                        per_buffer += per_buffer[s_tart + i];
                                }

                            }
                        }else {
                            char *t_del = SDL_GetClipboardText();
                            char *tt_del = t_del;

                            if(step_count == 1) {

                                while(*tt_del){

                                    if(*tt_del != '\r')
                                        per_buffer += *tt_del;

                                    tt_del++;

                                }

                            }else {

                                while(*tt_del){

                                    if(*tt_del == '\n') {
                                        per_buffer += '\r';
                                        per_buffer += *tt_del;
                                    }else
                                        per_buffer += *tt_del;

                                    tt_del++;

                                }


                            }

                            SDL_free(t_del);
                        }


                        remit_list.insert(remit_it, del_me2.insert(it_cp, {t_start, per_buffer.size(), true}));
                        undo_type.insert(undo_type_it, 0);



                        while(!(--it_cp)->append);
                        char_pos = it_cp->_end - it_cp->_start;

                    }


                    if(check_cursor(per_buffer, paste_board, cr1, x_offset, s_w, s_h + font_ascent, _width, _height)) {

                        /// this is where the trick begins...

                        find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                       &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                       x_offset, y_offset);

                    }
/*
                    for(auto i : undo_type)
                        std::cout << (int) i << std::endl;
                    std::cout << "From X undo_type_list*******\n";

                    for(auto i : remit_list)
                        std::cout << (i)->_start << " " << (i)->_end << " " << (i)->append << std::endl;
                    std::cout << "From Paste remit_list*******\n";

                    for(auto i : del_me2)
                        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                    std::cout << "From Paste*******\n";
*/
                   // std::cout << "[Paste] it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    shift_was_pressed = false;

                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;

                    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                    last_time = SDL_GetTicks64() - _Cursor_Delay;
                    blink_on = true;

                    is_copied = false;
                }


            }



            if(e.key.keysym.sym == SDLK_RETURN) {

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(remit_it != remit_list.end()) {
                    remit_it = remit_list.erase(remit_it, remit_list.end());
                    undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                }

                if(sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)) {

                    ///following is for the cut & remit section

                    std::list<_span2>::iterator sel_it = sel_cp;

                    it_cp = sel_cp;

                    //size_t th_count = 0;
                    bool has_test = false;

                    do {

                        if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                            if(sel_it == test_cp) {
                                has_test = true;
                                sel_it = del_me2.erase(test_cp);
                                test_cp = del_me2.begin();
                                if(sel_it == del_me2.end())
                                    continue;
                            }else
                                continue;
                        }



                        size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                        size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                        if(t_start > sel_it->_start) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == sel_it && *tt_undo_it == 0 ) {


                                    remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;

                                }

                            }

                            if(!is_break)
                                del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                            sel_it->_start = t_start;

                        }


                        if(t_end < sel_it->_end) {

                            if(t_start < t_end) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<_span2>::iterator to_put = del_me2.end();
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it && *tt_undo_it == 0) {

                                        to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;

                                    }

                                }

                                if(!is_break)
                                    to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                remit_list.insert(remit_it, to_put);
                                undo_type.insert(undo_type_it, 1);
                                sel_it->_start = t_end;
                            }


                        }else {

                            sel_it->append = false;
                            remit_list.insert(remit_it, sel_it);
                            undo_type.insert(undo_type_it, 1);

                        }



                    }while (sel_it++ != sel_end_cp);


                   while(!(--it_cp)->append);
                    if(it_cp == test_cp) {
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        char_pos = 0;
                    }else
                        char_pos = it_cp->_end - it_cp->_start;


                    if(has_test && it_cp != del_me2.end())
                        find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                        &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                        x_offset, y_offset);


                 //   for(auto i : del_me2)
                 //       std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                 //   std::cout << "From BACK*******\n";


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }

                }

                if(insert_toggle) {

                    if(it_cp != del_me2.end()) {

                        //std::list<_span2>::iterator temp_it = it_cp;
                        size_t start_pos = it_cp->_start + char_pos;


                        if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) < it_cp->_end) { /// within case

                            if(char_pos) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;
                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(it_cp, {it_cp->_start, start_pos, true});
                            }




                            std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {start_pos, start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]), false}));

                            undo_type.insert(undo_type_it, 1);



                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, t1);

                                    undo_type.insert(tt_undo_it, 0);
                                    break;
                                }

                            }


                            it_cp->_start = start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]);


                            if(char_pos){
                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;
                            }



                        }else if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) == it_cp->_end) { /// equal case

                            if(char_pos) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;
                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(it_cp, {it_cp->_start, start_pos, true});

                                it_cp->_start = start_pos;
                            }

                            it_cp->append = false;

                            remit_list.insert(remit_it, it_cp);
                            undo_type.insert(undo_type_it, 1);

                            if(char_pos){
                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;
                            }else
                                while(++it_cp != del_me2.end() && !it_cp->append);


                        }else if(++it_cp != del_me2.end()) { /// end case

                            if(!it_cp->append)
                                while(++it_cp != del_me2.end() && !it_cp->append);

                            if(it_cp != del_me2.end()) {

                                if(it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]) < it_cp->_end) { /// sub within case


                                    std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]), false}));

                                    undo_type.insert(undo_type_it, 1);


                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                            remit_list.insert(tt_rem_it, t1);

                                            undo_type.insert(tt_undo_it, 0);
                                            break;
                                        }

                                    }

                                    it_cp->_start += UTF8_CHAR_LEN(per_buffer[it_cp->_start]);

                                    while(!(--it_cp)->append);
                                    char_pos = it_cp->_end - it_cp->_start;

                                }else {  ///sub end case


                                    it_cp->append = false;

                                    remit_list.insert(remit_it, it_cp);
                                    undo_type.insert(undo_type_it, 1);

                                    while(!(--it_cp)->append);
                                    char_pos = it_cp->_end - it_cp->_start;

                                }
                            }else {
                                while(!(--it_cp)->append);
                                if(it_cp == test_cp) {
                                    while(++it_cp != del_me2.end() && !it_cp->append);
                                    if(it_cp == del_me2.end())
                                        char_pos = 0;
                                    else
                                        char_pos = it_cp->_end - it_cp->_start;
                                }else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }

                        }else {
                            while(!(--it_cp)->append);
                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                if(it_cp == del_me2.end())
                                    char_pos = 0;
                                else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;

                        }
                    }


                }



                if(new_line == '\r')
                    per_buffer += '\r';


                per_buffer += '\n';

                if (it_cp->_start + char_pos == it_cp->_end  && per_buffer.size() - step_count == it_cp->_end ) {

                    it_cp->_end += step_count;
                    char_pos += step_count;

                }else {

                    if(char_pos > 0 && it_cp->_start + char_pos < it_cp->_end) {

                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                        std::list<char>::iterator tt_undo_it = undo_type_it;
                        bool is_break = false;

                        while(tt_rem_it-- != remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == it_cp  && *tt_undo_it == 0) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;

                            }
                        }

                        if(!is_break)
                            del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});

                        //remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));
                        //del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});
                        it_cp->_start = it_cp->_start + char_pos;
                        char_pos = 0;

                    }else if(it_cp->_start + char_pos == it_cp->_end) {

                        //remit_list.insert(remit_it, it_cp);
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        //it_cp++;
                        char_pos = 0;


                    }


                    it_cp = del_me2.insert(it_cp, {per_buffer.size() - step_count, per_buffer.size(), true});

                    //remit_it_ctrl = remit_list.insert(remit_it, it_cp);
                    //remit_list_del.push_back(remit_list.insert(remit_it, it_cp));
                    remit_list.insert(remit_it, it_cp);

                    undo_type.insert(undo_type_it, 0);

                    char_pos += step_count;

                }



                adv_cursor(per_buffer, cr1, it_cp->_start, char_pos - step_count, x_offset, s_w, s_h + font_ascent, _width);
                old_x = x_offset;


                if(cr1.y >= _height){

                    std::list<_span2>::iterator temp_it = test_cp;

                    int t_x = off_char;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while(start_pos + temp_it->_start < temp_it->_end && temp_it->append) {

                            if(per_buffer[temp_it->_start + start_pos] == new_line){
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= (s_h + font_ascent);
                            }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x %= (_width - s_w); // t_x + x_offset - (_width - s_w);
                                    t_x = s_w * (t_x / s_w);
                                    off_char = t_x;
                                    cr1.y -= (s_h + font_ascent);
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    off_char = t_x;
                                    cr1.y -= (s_h + font_ascent);
                                }
                            }

                            start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);

                            if(cr1.y < _height)
                                break;
                        }

                        if(cr1.y < _height) {

                            std::list<_span2>::iterator test_this = test_cp;

                            if(temp_it->_start + start_pos < temp_it->_end) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == temp_it && *tt_undo_it == 0) {

                                        remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true});

                                temp_it->_start += start_pos;


                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, true});
                            }else if(it_cp == temp_it){
                                test_cp = del_me2.insert(++temp_it, {0, 0, true});
                                it_cp = temp_it;
                                char_pos = 0;
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, true});


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);



                           // if(test_this != del_me2.begin())
                           //     del_me2.erase(test_this);

                            if(test_this != del_me2.begin()) {

                                test_this = del_me2.erase(test_this);

                                std::list<std::list<_span2>::iterator>::iterator t1 = remit_it;
                                bool is_break = false;

                                while(t1 != remit_list.begin()) {
                                    if(*--t1 == test_this) {
                                        //std::cout << "hmm \n";
                                        is_break = true;
                                        break;
                                    }
                                }

                                if(!is_break) {

                                    std::list<_span2>::iterator t__t = test_this;

                                    while(!(--t__t)->append);

                                    if(t__t->_end == test_this->_start) {

                                        t__t->_end = test_this->_end;
                                        del_me2.erase(test_this);
                                    }

                                }

                            }

                            break;
                        }
                    }

                }else if(cr1.y + _cur_height > _height){


                    //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                    y_offset += _height - (cr1.y + _cur_height);
                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                    cr1.y = _height - _cur_height;

                }else
                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                sel_cp = it_cp;
                sel_end_cp = it_cp;
                sel_char = char_pos;
                sel_end_char = char_pos;


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_BACKSPACE){

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(remit_it != remit_list.end()) {
                    remit_it = remit_list.erase(remit_it, remit_list.end());
                    undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                }


                if(it_cp != test_cp) {


                    if(sel_cp != sel_end_cp || sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char) {

                        ///following is for the cut & remit section

                        std::list<_span2>::iterator sel_it = sel_cp;

                        it_cp = sel_cp;

                        //size_t th_count = 0;
                        bool has_test = false;

                        do {

                            if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                                if(sel_it == test_cp) {
                                    has_test = true;
                                    sel_it = del_me2.erase(test_cp);
                                    test_cp = del_me2.begin();
                                    if(sel_it == del_me2.end())
                                        continue;
                                }else
                                    continue;
                            }



                            size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                            size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                            if(t_start > sel_it->_start) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it && *tt_undo_it == 0) {


                                        remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;

                                    }

                                }
                                if(!is_break)
                                    del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                                sel_it->_start = t_start;

                            }


                            if(t_end < sel_it->_end) {

                                if(t_start < t_end) {

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<_span2>::iterator to_put = del_me2.end();
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == sel_it &&  *tt_undo_it == 0 ) {


                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;


                                        }

                                    }

                                    if(!is_break)
                                        to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                    remit_list.insert(remit_it, to_put);
                                    undo_type.insert(undo_type_it, 1);
                                    sel_it->_start = t_end;
                                }


                            }else {

                                sel_it->append = false;
                                remit_list.insert(remit_it, sel_it);
                                undo_type.insert(undo_type_it, 1);

                            }



                        }while (sel_it++ != sel_end_cp);


                       while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            char_pos = 0;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;



                        if(has_test && it_cp != del_me2.end()) {
                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);

                        }




                    }else{

                        if(!char_pos) {

                            while(!(--it_cp)->append);
/*
                            for(auto i : del_me2)
                               std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                            std::cout << "From BACK*******\n";

                            std::cout << "should be here too\n";

*/
                            if(it_cp != del_me2.begin()) {

                                it_cp = del_me2.erase(it_cp);

                                while(!(--it_cp)->append);

                                char_pos = it_cp->_end - it_cp->_start;

                                size_t old_char = char_pos;

                                while(per_buffer[it_cp->_start + --char_pos] == ctrl_line || (((per_buffer[it_cp->_start + char_pos] & 0x80) != 0) && ((per_buffer[it_cp->_start + char_pos] & 0xC0) != 0xC0)));


                                if(!char_pos) {

                                  //  std::cout << "this is the zero section \n";

                                    it_cp->append = false;
                                    remit_list.insert(remit_it, it_cp);
                                    undo_type.insert(undo_type_it, 1);

                                 //   std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                                }else{

                                    //std::cout << "Is it here?\n";

                                    std::list<_span2>::iterator t_emp = it_cp;
                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;
/*
                                    for(auto i : del_me2)
                                       std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                                    std::cout << "From BACK*******\n";

                                    std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;
*/

                               //     if(it_cp->_end == (++t_emp)->_start && !t_emp->append) {
                               //         std::cout << "really is it here?\n";
                               //         it_cp->_end = it_cp->_start + char_pos;
                               //         t_emp->_start = it_cp->_start + char_pos;

                               //         it_cp++;
                               //     } else {

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp  && *tt_undo_it == 0 ) {

                                            it_cp->_end = it_cp->_start + char_pos;


                                            remit_list.insert(++tt_rem_it, del_me2.insert(++it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));

                                            undo_type.insert(++tt_undo_it, 0);
                                            is_break = true;
                                            break;


                                        }

                                    }


                                    if(!is_break) {
                                      //  std::cout << "should be here\n";
                                        it_cp->_end = it_cp->_start + char_pos;
                                        remit_list.insert(remit_it, del_me2.insert(++it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));
                                    }else
                                        remit_list.insert(remit_it, *--tt_rem_it);

                                    undo_type.insert(undo_type_it, 1);

                                   // }



                                }

                                while(!(--it_cp)->append);

                                if(it_cp->_start == it_cp->_end) {
                                    if(it_cp == del_me2.begin()) {
                                        test_cp = del_me2.begin();
                                        goto TEST_CP;
                                    }
                                    while(++it_cp != del_me2.end() && !it_cp->append);
                                    //it_cp++;
                                    char_pos = 0;
                                }else
                                    char_pos = it_cp->_end - it_cp->_start;

                               // std::cout << "it_cp second " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                          //      std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                          //      for(auto i : del_me2)
                          //         std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                          //      std::cout << "From BACK*******\n";


                                std::list<_span2>::iterator temp_it = it_cp;

                                bool is_break = false;

                                size_t t_end = 0;

                                while(temp_it->_start != temp_it->_end) {

                                    t_end = temp_it->_end;

                                    while(t_end > temp_it->_start && temp_it->append) {

                                        while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));
                                        if(per_buffer[t_end] == new_line) {
                                            is_break = true;
                                            break;
                                        }

                                    }

                                    if(is_break)
                                        break;
                                    temp_it--;
                                }

                             //   std::cout << "temp_it " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;

                                // forward_ cursor_check

                                __cursor t_x = {x_offset, y_offset};

                                std::list<_span2>::iterator found_it = temp_it;
                                size_t found_end = t_end;

                                int first_run = 0;

                                do{

                                    size_t start_pos = !first_run++ ? t_end : temp_it->_start;
                                    size_t end_pos = (temp_it == it_cp) ? temp_it->_start + char_pos : temp_it->_end;

                            //        std::cout << "temp_it loop " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;
                            //        std::cout << "temp_it start end " << start_pos << " " << end_pos << std::endl;

                                    while(start_pos < end_pos && temp_it->append) {


                                        if(per_buffer[start_pos] == new_line){
                                            t_x.x = x_offset;
                                            off_char = t_x.x;
                                            t_x.y += (s_h + font_ascent);
                                            found_end = start_pos;
                                            found_it = temp_it;
                                        }else if(per_buffer[start_pos] == '\t'){

                                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                            if( t_x.x > _width - s_w ) {
                                                t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                                t_x.x = s_w * (t_x.x / s_w);
                                                off_char = t_x.x;
                                                t_x.y += (s_h + font_ascent);
                                                found_end = start_pos;
                                                found_it = temp_it;
                                            }
                                        }else {
                                            t_x.x += s_w;
                                            if( t_x.x > _width - s_w) {
                                                t_x.x = x_offset;
                                                off_char = t_x.x;
                                                t_x.y += (s_h + font_ascent);
                                                found_end = start_pos;
                                                found_it = temp_it;
                                            }
                                        }

                                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                                    }

                                }while(temp_it++ != it_cp);

                                temp_it = found_it;
                                t_end = found_end;
                         //       std::cout << std::endl;

                          //      std::cout << "temp_it later " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;


                               if(t_end + UTF8_CHAR_LEN(per_buffer[t_end]) < temp_it->_end) {

                           //         std::cout << "[BACK] am I here?\n";

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == temp_it && *tt_undo_it == 0) {


                                            remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end + UTF8_CHAR_LEN(per_buffer[t_end]), true}));

                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;


                                        }

                                    }

                                    if(!is_break)
                                        del_me2.insert(temp_it, {temp_it->_start, t_end + UTF8_CHAR_LEN(per_buffer[t_end]), true});

                                    temp_it->_start = t_end + UTF8_CHAR_LEN(per_buffer[t_end]);

                                    //if(it_cp == temp_it)
                                    //    char_pos

                                    test_cp = del_me2.insert(temp_it, {0, 0, true});

                                    //while(!(--it_cp)->append);
                                    char_pos = it_cp->_end - it_cp->_start;


                                }else if(temp_it->_start != temp_it->_end){

                                    if(temp_it == it_cp) {

                                    //    std::cout << "göööggdf\n";

                                        if(++temp_it != del_me2.end()) {

                                            test_cp = del_me2.insert(temp_it, {0, 0, true});
                                          //  std::cout << "temp_it final " << temp_it->_start << " " << temp_it->_end << " " << t_end << std::endl;

                                            if(!temp_it->append && temp_it != del_me2.end())
                                                while(++temp_it != del_me2.end() && !temp_it->append);
                                            it_cp = temp_it;
                                            char_pos = 0;
                                        }else
                                            test_cp = del_me2.begin();
                                    }else{
                                     //   std::cout << "göökjkjkjöggdf\n";
                                        test_cp = del_me2.insert(++temp_it, {0, 0, true});
                                        //while(!(--it_cp)->append);
                                        //char_pos = it_cp->_end - it_cp->_start;
                                    }

                                }else {
                                    test_cp = temp_it;
                                    //while(!(--it_cp)->append);
                                    //char_pos = it_cp->_end - it_cp->_start;
                                }

                            }else
TEST_CP:                        while(++it_cp != del_me2.end() && !it_cp->append);


                        }else {

                            if(it_cp->_start + char_pos < it_cp->_end) {

                                size_t old_char = char_pos;

                                while(per_buffer[it_cp->_start + --char_pos] == ctrl_line || (((per_buffer[it_cp->_start + char_pos] & 0x80) != 0) && ((per_buffer[it_cp->_start + char_pos] & 0xC0) != 0xC0)));


                                if(char_pos) {

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {


                                            remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));


                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;


                                        }

                                    }

                                    if(!is_break)
                                        del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});

                                    remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));

                                    if(is_break) {
                                        remit_list.insert(tt_rem_it, *--remit_it);
                                        undo_type.insert(tt_undo_it, 0);
                                        ++remit_it;
                                    }

                                    it_cp->_start += old_char;

                                }else {

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {


                                            remit_list.insert(tt_rem_it,  del_me2.insert(it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));

                                            it_cp->_start += old_char;

                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;



                                        }

                                    }

                                    if(is_break) {
                                        remit_list.insert(remit_it, *--tt_rem_it);
                                    }else {
                                        remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));
                                        it_cp->_start += old_char;
                                    }

                                }

                                undo_type.insert(undo_type_it, 1);



                            }else {

                                size_t old_char = char_pos;
                                while(per_buffer[it_cp->_start + --char_pos] == ctrl_line || (((per_buffer[it_cp->_start + char_pos] & 0x80) != 0) && ((per_buffer[it_cp->_start + char_pos] & 0xC0) != 0xC0)));


                                if(char_pos) {

                                    std::list<_span2>::iterator t_emp = it_cp;
                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;
                                    bool is_break = false;

                                    if(it_cp->_end == (++t_emp)->_start && !t_emp->append) {
                                        it_cp->_end = it_cp->_start + char_pos;
                                        t_emp->_start = it_cp->_start + char_pos;

                                        it_cp++;
                                    } else {

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp  && *tt_undo_it == 0 ) {

                                            it_cp->_end = it_cp->_start + char_pos;


                                            remit_list.insert(++tt_rem_it, del_me2.insert(++it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));


                                            undo_type.insert(++tt_undo_it, 0);
                                            is_break = true;
                                            break;

                                        }

                                    }


                                    if(!is_break) {
                                        it_cp->_end = it_cp->_start + char_pos;
                                        remit_list.insert(remit_it, del_me2.insert(++it_cp, {it_cp->_start + char_pos, it_cp->_start + old_char, false}));
                                    }else
                                        remit_list.insert(remit_it, *--tt_rem_it);

                                    undo_type.insert(undo_type_it, 1);

                                    }



                                }else {
                                    it_cp->append = false;
                                    remit_list.insert(remit_it, it_cp);
                                    undo_type.insert(undo_type_it, 1);
                                }

                            }

                            while(!(--it_cp)->append);

                            if(it_cp->_start == it_cp->_end) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                //it_cp++;
                                char_pos = 0;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;


                        }

                    }
/*
                    for(auto i : undo_type)
                        std::cout << (int) i << std::endl;
                    std::cout << "From BACK undo_type_list*******\n";

                    for(auto i : remit_list)
                        std::cout << (i)->_start << " " << (i)->_end << " " << (i)->append << std::endl;
                    std::cout << "From BACK remit_list*******\n";

                    for(auto i : del_me2)
                        std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                    std::cout << "From BACK*******\n";





                    std::cout << "[BACKSPACE] it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;
*/
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    shift_was_pressed = false;

                    if(it_cp == del_me2.end())
                        off_char = x_offset;


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    old_x = cr1.x;


                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }


                }



                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_DELETE){


                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(remit_it != remit_list.end()) {
                    remit_it = remit_list.erase(remit_it, remit_list.end());
                    undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                }


                if(sel_cp != sel_end_cp || sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char) {

                    ///following is for the cut & remit section

                    std::list<_span2>::iterator sel_it = sel_cp;

                    it_cp = sel_cp;

                    //size_t th_count = 0;
                    bool has_test = false;

                    do {

                        if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                            if(sel_it == test_cp) {
                                has_test = true;
                                sel_it = del_me2.erase(test_cp);
                                test_cp = del_me2.begin();
                                if(sel_it == del_me2.end())
                                    continue;
                            }else
                                continue;
                        }



                        size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                        size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                        if(t_start > sel_it->_start) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == sel_it && *tt_undo_it == 0) {


                                    remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;

                                }

                            }
                            if(!is_break)
                                del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                            sel_it->_start = t_start;

                        }


                        if(t_end < sel_it->_end) {

                            if(t_start < t_end) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<_span2>::iterator to_put = del_me2.end();
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it &&  *tt_undo_it == 0 ) {


                                        to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }

                                }

                                if(!is_break)
                                    to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                remit_list.insert(remit_it, to_put);
                                undo_type.insert(undo_type_it, 1);
                                sel_it->_start = t_end;
                            }


                        }else {

                            sel_it->append = false;
                            remit_list.insert(remit_it, sel_it);
                            undo_type.insert(undo_type_it, 1);

                        }



                    }while (sel_it++ != sel_end_cp);


                   while(!(--it_cp)->append);
                    if(it_cp == test_cp) {
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        char_pos = 0;
                    }else
                        char_pos = it_cp->_end - it_cp->_start;



                    if(has_test && it_cp != del_me2.end()) {
                        find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                        &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                        x_offset, y_offset);

                    }



                }else if(it_cp != del_me2.end()) {

                    //std::list<_span2>::iterator temp_it = it_cp;
                    size_t start_pos = it_cp->_start + char_pos;


                    if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) < it_cp->_end) { /// within case

                        if(char_pos) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;
                                }

                            }

                            if(!is_break)
                                del_me2.insert(it_cp, {it_cp->_start, start_pos, true});
                        }




                        std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {start_pos, start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]), false}));

                        undo_type.insert(undo_type_it, 1);



                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                        std::list<char>::iterator tt_undo_it = undo_type_it;

                        while(tt_rem_it-- != remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                remit_list.insert(tt_rem_it, t1);

                                undo_type.insert(tt_undo_it, 0);
                                break;
                            }

                        }


                        it_cp->_start = start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]);


                        if(char_pos){
                            while(!(--it_cp)->append);
                            char_pos = it_cp->_end - it_cp->_start;
                        }



                    }else if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) == it_cp->_end) { /// equal case

                        if(char_pos) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;
                                }

                            }

                            if(!is_break)
                                del_me2.insert(it_cp, {it_cp->_start, start_pos, true});

                            it_cp->_start = start_pos;
                        }

                        it_cp->append = false;

                        remit_list.insert(remit_it, it_cp);
                        undo_type.insert(undo_type_it, 1);

                        if(char_pos){
                            while(!(--it_cp)->append);
                            char_pos = it_cp->_end - it_cp->_start;
                        }else
                            while(++it_cp != del_me2.end() && !it_cp->append);


                    }else if(++it_cp != del_me2.end()) { /// end case

                        if(!it_cp->append)
                            while(++it_cp != del_me2.end() && !it_cp->append);

                        if(it_cp != del_me2.end()) {

                            if(it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]) < it_cp->_end) { /// sub within case


                                std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]), false}));

                                undo_type.insert(undo_type_it, 1);


                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, t1);

                                        undo_type.insert(tt_undo_it, 0);
                                        break;
                                    }

                                }

                                it_cp->_start += UTF8_CHAR_LEN(per_buffer[it_cp->_start]);

                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;

                            }else {  ///sub end case


                                it_cp->append = false;

                                remit_list.insert(remit_it, it_cp);
                                undo_type.insert(undo_type_it, 1);

                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;

                            }
                        }else {
                            while(!(--it_cp)->append);
                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                if(it_cp == del_me2.end())
                                    char_pos = 0;
                                else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;
                        }

                    }else {
                        while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            if(it_cp == del_me2.end())
                                char_pos = 0;
                            else
                                char_pos = it_cp->_end - it_cp->_start;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;

                    }
                }



/*
                std::list<char>::iterator t2 = undo_type.begin();

                for(auto i : remit_list)
                    std::cout << (i)->_start << " " << (i)->_end << " " << (int) *t2++ <<std::endl;
                std::cout << "From DEL remit_list*******\n";


                std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                for(auto i : del_me2)
                    std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                std::cout << "From DEL*******\n";

*/



                adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                sel_cp = it_cp;
                sel_end_cp = it_cp;
                sel_char = char_pos;
                sel_end_char = char_pos;
                shift_was_pressed = false;

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;


            }
            if(e.key.keysym.sym == SDLK_TAB){

                if(!blink_on){
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                if(remit_it != remit_list.end()) {
                    remit_it = remit_list.erase(remit_it, remit_list.end());
                    undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
                }


                if(sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)) {


                    ///following is for the cut & remit section

                    std::list<_span2>::iterator sel_it = sel_cp;

                    it_cp = sel_cp;

                    //size_t th_count = 0;
                    bool has_test = false;

                    do {

                        if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                            if(sel_it == test_cp) {
                                has_test = true;
                                sel_it = del_me2.erase(test_cp);
                                test_cp = del_me2.begin();
                                if(sel_it == del_me2.end())
                                    continue;
                            }else
                                continue;
                        }



                        size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                        size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                        if(t_start > sel_it->_start) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == sel_it && *tt_undo_it == 0 ) {


                                    remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;


                                }

                            }
                            if(!is_break)
                                del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                            sel_it->_start = t_start;

                        }


                        if(t_end < sel_it->_end) {

                            if(t_start < t_end) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<_span2>::iterator to_put = del_me2.end();
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == sel_it && *tt_undo_it == 0) {


                                        to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }

                                }

                                if(!is_break)
                                    to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                                remit_list.insert(remit_it, to_put);
                                undo_type.insert(undo_type_it, 1);
                                sel_it->_start = t_end;
                            }


                        }else {

                            sel_it->append = false;
                            remit_list.insert(remit_it, sel_it);
                            undo_type.insert(undo_type_it, 1);

                        }



                    }while (sel_it++ != sel_end_cp);




                   while(!(--it_cp)->append);
                    if(it_cp == test_cp) {
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        char_pos = 0;
                    }else
                        char_pos = it_cp->_end - it_cp->_start;



                    if(has_test && it_cp != del_me2.end())
                        find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                        &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                        x_offset, y_offset);


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }

                }


                if(insert_toggle) {

                    if(it_cp != del_me2.end()) {

                        //std::list<_span2>::iterator temp_it = it_cp;
                        size_t start_pos = it_cp->_start + char_pos;


                        if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) < it_cp->_end) { /// within case

                            if(char_pos) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;
                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(it_cp, {it_cp->_start, start_pos, true});
                            }




                            std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {start_pos, start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]), false}));

                            undo_type.insert(undo_type_it, 1);



                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, t1);

                                    undo_type.insert(tt_undo_it, 0);
                                    break;
                                }

                            }


                            it_cp->_start = start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]);


                            if(char_pos){
                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;
                            }



                        }else if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) == it_cp->_end) { /// equal case

                            if(char_pos) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;
                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(it_cp, {it_cp->_start, start_pos, true});

                                it_cp->_start = start_pos;
                            }

                            it_cp->append = false;

                            remit_list.insert(remit_it, it_cp);
                            undo_type.insert(undo_type_it, 1);

                            if(char_pos){
                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;
                            }else
                                while(++it_cp != del_me2.end() && !it_cp->append);


                        }else if(++it_cp != del_me2.end()) { /// end case

                            if(!it_cp->append)
                                while(++it_cp != del_me2.end() && !it_cp->append);

                            if(it_cp != del_me2.end()) {

                                if(it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]) < it_cp->_end) { /// sub within case


                                    std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]), false}));

                                    undo_type.insert(undo_type_it, 1);


                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                    std::list<char>::iterator tt_undo_it = undo_type_it;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                            remit_list.insert(tt_rem_it, t1);

                                            undo_type.insert(tt_undo_it, 0);
                                            break;
                                        }

                                    }

                                    it_cp->_start += UTF8_CHAR_LEN(per_buffer[it_cp->_start]);

                                    while(!(--it_cp)->append);
                                    char_pos = it_cp->_end - it_cp->_start;

                                }else {  ///sub end case


                                    it_cp->append = false;

                                    remit_list.insert(remit_it, it_cp);
                                    undo_type.insert(undo_type_it, 1);

                                    while(!(--it_cp)->append);
                                    char_pos = it_cp->_end - it_cp->_start;

                                }
                            }else {
                                while(!(--it_cp)->append);
                                if(it_cp == test_cp) {
                                    while(++it_cp != del_me2.end() && !it_cp->append);
                                    if(it_cp == del_me2.end())
                                        char_pos = 0;
                                    else
                                        char_pos = it_cp->_end - it_cp->_start;
                                }else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }

                        }else {
                            while(!(--it_cp)->append);
                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                if(it_cp == del_me2.end())
                                    char_pos = 0;
                                else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;

                        }
                    }


                }


                per_buffer += '\t';

                if (it_cp->_start + char_pos == it_cp->_end  && per_buffer.size() - 1 == it_cp->_end ) {


                    it_cp->_end += 1;
                    char_pos += 1;

                }else {

                    if(char_pos > 0 && it_cp->_start + char_pos < it_cp->_end) {

                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                        std::list<char>::iterator tt_undo_it = undo_type_it;
                        bool is_break = false;

                        while(tt_rem_it-- != remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == it_cp  && *tt_undo_it == 0 ) {

                                remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));

                                undo_type.insert(tt_undo_it, 0);
                                is_break = true;
                                break;

                            }
                        }

                        if(!is_break)
                            del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});

                        //remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));
                        //del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});
                        it_cp->_start = it_cp->_start + char_pos;
                        char_pos = 0;

                    }else if(it_cp->_start + char_pos == it_cp->_end) {

                        //remit_list.insert(remit_it, it_cp);
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        //it_cp++;
                        char_pos = 0;


                    }


                    it_cp = del_me2.insert(it_cp, {per_buffer.size() - 1, per_buffer.size(), true});

                    //remit_it_ctrl = remit_list.insert(remit_it, it_cp);
                    //remit_list_del.push_back(remit_list.insert(remit_it, it_cp));
                    remit_list.insert(remit_it, it_cp);

                    undo_type.insert(undo_type_it, 0);

                    char_pos += 1;

                }


                adv_cursor(per_buffer, cr1, it_cp->_start, char_pos - 1, x_offset, s_w, s_h + font_ascent, _width);

                old_x = cr1.x;


                if(cr1.y >= _height){

                    std::list<_span2>::iterator temp_it = test_cp;

                    int t_x = off_char;

                    while(++temp_it != del_me2.end()) {

                        size_t start_pos = 0;

                        while(start_pos + temp_it->_start < temp_it->_end && temp_it->append) {

                            if(per_buffer[temp_it->_start + start_pos] == new_line){
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= (s_h + font_ascent);
                            }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                                t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x > _width - s_w) {
                                    t_x %= (_width - s_w); // t_x + x_offset - (_width - s_w);
                                    t_x = s_w * (t_x / s_w);
                                    off_char = t_x;
                                    cr1.y -= (s_h + font_ascent);
                                }
                            }else {
                                t_x += s_w;
                                if(t_x > _width - s_w) {
                                    t_x = x_offset;
                                    off_char = t_x;
                                    cr1.y -= (s_h + font_ascent);
                                }
                            }

                            start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);

                            if(cr1.y < _height)
                                break;
                        }

                        if(cr1.y < _height) {

                            std::list<_span2>::iterator test_this = test_cp;

                            if(temp_it->_start + start_pos < temp_it->_end) {

                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;
                                bool is_break = false;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == temp_it && *tt_undo_it == 0 ) {


                                        remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));

                                        undo_type.insert(tt_undo_it, 0);
                                        is_break = true;
                                        break;


                                    }

                                }

                                if(!is_break)
                                    del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true});

                                temp_it->_start += start_pos;


                                if(temp_it == it_cp)
                                    char_pos -= start_pos;

                                test_cp = del_me2.insert(temp_it, {0, 0, true});
                            }else if(it_cp == temp_it){
                                test_cp = del_me2.insert(++temp_it, {0, 0, true});
                                it_cp = temp_it;
                                char_pos = 0;
                            }else
                                test_cp = del_me2.insert(++temp_it, {0, 0, true});


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);



                           // if(test_this != del_me2.begin())
                           //     del_me2.erase(test_this);

                            if(test_this != del_me2.begin()) {

                                test_this = del_me2.erase(test_this);

                                std::list<std::list<_span2>::iterator>::iterator t1 = remit_it;
                                bool is_break = false;

                                while(t1 != remit_list.begin()) {
                                    if(*--t1 == test_this) {
                                        //std::cout << "hmm \n";
                                        is_break = true;
                                        break;
                                    }
                                }

                                if(!is_break) {

                                    std::list<_span2>::iterator t__t = test_this;

                                    while(!(--t__t)->append);

                                    if(t__t->_end == test_this->_start) {

                                        t__t->_end = test_this->_end;
                                        del_me2.erase(test_this);
                                    }

                                }

                            }

                            break;
                        }
                    }

                }else if(cr1.y + _cur_height > _height){


                    //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                    y_offset += _height - (cr1.y + _cur_height);
                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                    cr1.y = _height - _cur_height;

                }else
                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                sel_cp = it_cp;
                sel_end_cp = it_cp;
                sel_char = char_pos;
                sel_end_char = char_pos;

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_LEFT){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){


                    it_cp = sel_cp;
                    char_pos = sel_char;

                //    std::cout << "it_cp " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_end_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it-- != sel_cp);

                        if(has_test) {
                           // std::cout << "am I here?\n";

                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                        }

                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);



                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;


                    shift_was_pressed = false;


                }else {

                    bool is_selected = true;

                    std::list<_span2>::iterator sel_it_temp = it_cp;
                    size_t sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }

                    if(sel_cp == sel_it_temp && (sel_cp->_start + sel_char == sel_it_temp->_start + sel_char_temp))
                        is_selected = false;


                    if(it_cp != test_cp) {


                        if(!char_pos) {

                            while(!(--it_cp)->append);


                            if(it_cp != test_cp) {

                                char_pos = it_cp->_end - it_cp->_start;

                                while(per_buffer[it_cp->_start + --char_pos] == ctrl_line || (((per_buffer[it_cp->_start + char_pos] & 0x80) != 0) && ((per_buffer[it_cp->_start + char_pos] & 0xC0) != 0xC0)));

                                //if(per_buffer[it_cp->_start + char_pos] == ctrl_line)
                                //    char_pos--;

                            }else if(it_cp != del_me2.begin()) {

                               // std::cout << "am I here?\n";

                                it_cp = del_me2.erase(it_cp);

                                std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                                bool _is_break = false;

                                while(t1 != remit_list.begin()) {
                                    if(*--t1 == it_cp) {
                                        //std::cout << "hmm \n";
                                        _is_break = true;
                                        break;
                                    }
                                }

                                while(!(--it_cp)->append);

                                //char_pos = it_cp->_end;

                                if(!_is_break) {

                                    std::list<_span2>::iterator tt_it = it_cp;
                                    while(!(++tt_it)->append);
                                    if(tt_it != del_me2.end() && tt_it->_start == it_cp->_end) {

                                        char_pos = it_cp->_end;// - it_cp->_start;
                                        it_cp->_end = tt_it->_end;
                                        if(tt_it == sel_end_cp) {
                                            sel_end_char += (char_pos - it_cp->_start);
                                            sel_end_cp = it_cp;
                                        }

                                        del_me2.erase(tt_it);

                                    }else
                                        char_pos = it_cp->_end;

                                }else
                                    char_pos = it_cp->_end;


                                while(per_buffer[--char_pos] == ctrl_line || (((per_buffer[char_pos] & 0x80) != 0) && ((per_buffer[char_pos] & 0xC0) != 0xC0)));


                                std::list<_span2>::iterator temp_it = it_cp;
                                bool is_break = false;

                                size_t t_end = 0;

                                while(temp_it->_start != temp_it->_end) {

                                    t_end = (temp_it == it_cp) ? char_pos : temp_it->_end;

                                    while(t_end > temp_it->_start && temp_it->append) {

                                        while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));

                                        if(per_buffer[t_end] == new_line) {
                                            is_break = true;
                                            break;
                                        }

                                    }


                                    if(is_break)
                                        break;
                                    temp_it--;
                                }

                                // forward_ cursor_check

                                off_char = 0;

                                __cursor t_x = {x_offset, y_offset};

                                std::list<_span2>::iterator found_it = temp_it;
                                size_t found_end = t_end;

                                int first_run = 0;

                                do{


                                    size_t start_pos = !first_run++ ? t_end : temp_it->_start;
                                    size_t end_pos = (temp_it == it_cp) ? char_pos : temp_it->_end;

                                    while(start_pos < end_pos && temp_it->append) {

                                        if(per_buffer[start_pos] == new_line){
                                            t_x.x = x_offset;
                                            off_char = t_x.x;
                                            t_x.y += (s_h + font_ascent);
                                            found_end = start_pos;
                                            found_it = temp_it;
                                        }else if(per_buffer[start_pos] == '\t'){

                                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                            if( t_x.x > _width - s_w ) {
                                                t_x.x %= (_width - s_w);// t_x.x + x_offset - (_width - s_w);
                                                t_x.x = s_w * (t_x.x / s_w);
                                                off_char = t_x.x;
                                                t_x.y += (s_h + font_ascent);
                                                found_end = start_pos;
                                                found_it = temp_it;
                                            }
                                        }else {
                                            t_x.x += s_w;
                                            if( t_x.x > _width - s_w) {
                                                t_x.x = x_offset;
                                                off_char = t_x.x;
                                                t_x.y += (s_h + font_ascent);
                                                found_end = start_pos;
                                                found_it = temp_it;
                                            }
                                        }

                                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                                    }

                                }while(temp_it++ != it_cp);

                                temp_it = found_it;
                                t_end = found_end;


                                if(t_end + UTF8_CHAR_LEN(per_buffer[t_end]) < temp_it->_end) {

                                    std::list<_span2>::iterator to_put = del_me2.begin();

                                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                                    std::list<char>::iterator tt_undo_it = undo_type.end();
                                    bool is_break = false;

                                    while(tt_rem_it-- != remit_list.begin()) {
                                        --tt_undo_it;
                                        if(*tt_rem_it == temp_it && *tt_undo_it == 0) {

                                       /*     if(tt_rem_it == remit_it) {

                                                remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end + UTF8_CHAR_LEN(per_buffer[t_end]), true}));
                                                to_put = *remit_it;
                                            }else */
                                                to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end + UTF8_CHAR_LEN(per_buffer[t_end]), true}));


                                    /*        if(*tt_undo_it == 1) {

                                                if(tt_undo_it == undo_type_it)
                                                    undo_type_it = undo_type.insert(tt_undo_it, 1);
                                                else
                                                    undo_type.insert(tt_undo_it, 1);

                                                is_break = true;
                                            }else{ */
                                                undo_type.insert(tt_undo_it, 0);
                                                is_break = true;
                                                break;
                                          //  }


                                        }

                                    }


                                    if(!is_break)
                                        to_put = del_me2.insert(temp_it, {temp_it->_start, t_end + UTF8_CHAR_LEN(per_buffer[t_end]), true});



                                    temp_it->_start = t_end + UTF8_CHAR_LEN(per_buffer[t_end]);

                                    if(temp_it == sel_cp){

                                        if(to_put->_start + sel_char < to_put->_end)
                                            sel_cp = to_put;
                                        else
                                            sel_char = to_put->_start + sel_char - temp_it->_start;

                                    }

                                    if(temp_it == sel_end_cp){

                                        if(to_put->_start + sel_end_char < to_put->_end)
                                            sel_end_cp = to_put;
                                        else
                                            sel_end_char = to_put->_start + sel_end_char - temp_it->_start;

                                    }


                                    test_cp = del_me2.insert(temp_it, {0, 0, true});
/*
                                    for(auto i : del_me2)
                                       std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                                    std::cout << "From LEFT*******\n";
*/

                                }else if(temp_it->_start != temp_it->_end)
                                    test_cp = del_me2.insert(++temp_it, {0, 0, true});
                                else
                                    test_cp = temp_it;


                                if(!(char_pos - it_cp->_start)) {
                                    while(!(--it_cp)->append);
                                    if(it_cp == test_cp) {
                                        while(++it_cp != del_me2.end() && !it_cp->append);
                                        //it_cp++;
                                        char_pos = 0;
                                    }else
                                        char_pos = it_cp->_end - it_cp->_start;
                                }else
                                    char_pos -= it_cp->_start;




                                _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                            }else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                //it_cp++;
                       }else {

                        //   std::cout << "jjjk\n";


                            while(per_buffer[it_cp->_start + --char_pos] == ctrl_line || (((per_buffer[it_cp->_start + char_pos] & 0x80) != 0) && ((per_buffer[it_cp->_start + char_pos] & 0xC0) != 0xC0)));

                            //if(per_buffer[it_cp->_start + char_pos] == ctrl_line)
                            //    char_pos--;


                            if(!char_pos){
                                while(!(--it_cp)->append);
                                if(it_cp != test_cp)
                                    char_pos = it_cp->_end - it_cp->_start;
                                else
                                    while(++it_cp != del_me2.end() && !it_cp->append);
                            }

                        }


                    }


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }

                    old_x = cr1.x;

                    //std::cout << "[Left]: it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                    sel_it_temp = it_cp;
                    sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }




                    if(is_selected && shift_pressed)
                        _Selection_Up(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);
                    else{

                        sel_cp = sel_it_temp;
                        sel_char = sel_char_temp;
                    }



                    if(shift_pressed) {


                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                        _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                    x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                    }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                    }

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() - _Cursor_Delay;
                blink_on = true;
            }


            if(e.key.keysym.sym == SDLK_RIGHT){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){


                    it_cp = sel_end_cp;
                    char_pos = sel_end_char;


                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it++ != sel_end_cp);

                        if(!has_test) {
                            __cursor selcp_cr1 = {x_offset, y_offset};
                            adv_cursor_while(per_buffer, test_cp, sel_cp, selcp_cr1, sel_char, off_char, y_offset, s_w, s_h + font_ascent, _width);


                            if(check_sel_cursor(per_buffer, del_me2, sel_cp, sel_end_cp, sel_char, sel_end_char, selcp_cr1,
                                             x_offset, s_w, s_h + font_ascent, _width, _height)) {

                                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                                x_offset, y_offset);

                            }

                        }else {

                            del_it = test_cp;
                            has_test = false;

                            do {

                                if(del_it == sel_cp) {
                                    has_test = true;
                                    break;
                                }

                            }while(del_it++ != sel_end_cp);

                            if(has_test)
                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                        }


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;


                    shift_was_pressed = false;


                }else {

                    bool is_selected = true;

                    if(it_cp != del_me2.end()) {

                        std::list<_span2>::iterator sel_it_temp = it_cp;
                        size_t sel_char_temp = char_pos;

                        if(it_cp->_start + char_pos == it_cp->_end) {

                                while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                                sel_char_temp = 0;
                        }

                        if(sel_end_cp == sel_it_temp && (sel_end_cp->_start + sel_end_char == sel_it_temp->_start + sel_char_temp))
                            is_selected = false;


                        if(it_cp->_start + char_pos < it_cp->_end)
                            adv_cursor(per_buffer, cr1, it_cp->_start, char_pos, x_offset, s_w, s_h + font_ascent, _width);


                        char_pos += UTF8_CHAR_LEN(per_buffer[it_cp->_start + char_pos]);


                        if(it_cp->_start + char_pos > it_cp->_end) {

                           // std::cout << "hmm\n";

                            while(++it_cp != del_me2.end() && !it_cp->append);

                            if(it_cp != del_me2.end()) {

                                char_pos = UTF8_CHAR_LEN(per_buffer[it_cp->_start]);

                                adv_cursor(per_buffer, cr1, it_cp->_start, 0, x_offset, s_w, s_h + font_ascent, _width);


                            }else {
                                while(!(--it_cp)->append);

                                if(it_cp == test_cp) {
                                    while(++it_cp != del_me2.end() && !it_cp->append);
                                    char_pos = 0;
                                }else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }

                        }


                        old_x = cr1.x;

                        sel_it_temp = it_cp;
                        sel_char_temp = char_pos;

                        if(it_cp->_start + char_pos == it_cp->_end) {

                                while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                                sel_char_temp = 0;
                        }


                        if(!is_selected) {
                            sel_end_cp = sel_it_temp;
                            sel_end_char = sel_char_temp;
                        }

                        if(cr1.y >= _height){

                            std::list<_span2>::iterator temp_it = test_cp;

                            int t_x = off_char;
                            bool is_break = false;

                            while(++temp_it != del_me2.end()) {

                                size_t start_pos = 0;

                                while(temp_it->_start + start_pos < temp_it->_end && temp_it->append) {

                                    if(per_buffer[temp_it->_start + start_pos] == new_line){
                                        off_char = x_offset;
                                        is_break = true;
                                    }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                                        t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                        if( t_x > _width - s_w) {
                                            t_x %= (_width - s_w);
                                            t_x = s_w * (t_x / s_w);
                                            off_char = t_x;
                                            is_break = true;
                                        }
                                    }else {
                                        t_x += s_w;
                                        if(t_x > _width - s_w) {
                                            off_char = x_offset;
                                            is_break = true;
                                        }
                                    }

                                    start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);
                                    if(is_break)
                                        break;
                                }
                                if(is_break) {

                                    std::list<_span2>::iterator test_this = test_cp;

                                    if(temp_it->_start + start_pos < temp_it->_end) {

                                        size_t old_end_start = sel_end_cp->_start;

                                        std::list<_span2>::iterator to_put = del_me2.begin();

                                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                                        std::list<char>::iterator tt_undo_it = undo_type.end();
                                        bool is_break = false;

                                        while(tt_rem_it-- != remit_list.begin()) {
                                            --tt_undo_it;
                                            if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                                          /*      if(tt_rem_it == remit_it) {
                                                    remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));
                                                    to_put = *remit_it;
                                                }else */
                                                    to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));


                                        /*        if(*tt_undo_it == 1) {

                                                    if(tt_undo_it == undo_type_it)
                                                        undo_type_it = undo_type.insert(tt_undo_it, 1);
                                                    else
                                                        undo_type.insert(tt_undo_it, 1);

                                                    is_break = true;
                                                }else{ */
                                                    undo_type.insert(tt_undo_it, 0);
                                                    is_break = true;
                                                    break;
                                               // }


                                            }

                                        }

                                        if(!is_break)
                                            to_put = del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true});

                                        temp_it->_start += start_pos;

                                        if(temp_it == sel_cp){

                                            if(to_put->_start + sel_char < to_put->_end)
                                                sel_cp = to_put;
                                            else
                                                sel_char -= start_pos;
                                        }

                                        if(temp_it == sel_end_cp){

                                            if(to_put->_start + sel_end_char < to_put->_end)
                                                sel_end_cp = to_put;
                                            else
                                                sel_end_char -= (sel_end_cp->_start - old_end_start);

                                        }

                                        if(temp_it == it_cp)
                                            char_pos -= start_pos;


                                        test_cp = del_me2.insert(temp_it, {0, 0, true});

                                    }else if(it_cp == temp_it){
                                        test_cp = del_me2.insert(++temp_it, {0, 0, true});
                                        it_cp = temp_it;
                                        char_pos = 0;
                                    }else
                                        test_cp = del_me2.insert(++temp_it, {0, 0, true});



                                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                                    //if(test_this != del_me2.begin())
                                    //    del_me2.erase(test_this);

                                    if(test_this != del_me2.begin()) {

                                        test_this = del_me2.erase(test_this);

                                        std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                                        bool is_break = false;

                                        while(t1 != remit_list.begin()) {
                                            if(*--t1 == test_this) {
                                                //std::cout << "hmm \n";
                                                is_break = true;
                                                break;
                                            }
                                        }

                                        if(!is_break) {

                                            std::list<_span2>::iterator t__t = test_this;

                                            while(!(--t__t)->append);

                                            if(t__t->_end == test_this->_start) {
                                                t__t->_end = test_this->_end;
                                                if(test_this == sel_cp){
                                                    size_t tt_add = sel_cp->_start;
                                                    sel_cp = t__t;
                                                    sel_char += (tt_add - sel_cp->_start);
                                                }

                                                if(test_this == sel_end_cp && is_selected) {
                                                    size_t tt_add = sel_end_cp->_start;
                                                    sel_end_cp = t__t;
                                                    sel_end_char += (tt_add - sel_end_cp->_start);
                                                }


                                                del_me2.erase(test_this);
                                            }

                                        }

                                    }


                                    break;

                                }
                            }

                            cr1.y = _height - (s_h + font_ascent);

                        }else if(cr1.y + _cur_height > _height){


                            //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                            y_offset += _height - (cr1.y + _cur_height);
                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                            cr1.y = _height - _cur_height;

                        }

                       // std::cout << "[Right]: it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                        sel_it_temp = it_cp;
                        sel_char_temp = char_pos;

                        if(it_cp->_start + char_pos == it_cp->_end) {

                                while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                                sel_char_temp = 0;
                        }


                        if(is_selected && shift_pressed)
                            _Selection_Down(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);


                        if(shift_pressed) {


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                            _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                        x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                        }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                        }

                    }else {
                        sel_cp = it_cp;
                        sel_end_cp = it_cp;
                        sel_char = char_pos;
                        sel_end_char = char_pos;
                    }
                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;

                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_UP){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){


                    it_cp = sel_cp;
                    char_pos = sel_char;

                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_end_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it-- != sel_cp);

                        if(has_test) {
                           // std::cout << "am I here?\n";

                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                        }



                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        //it_cp++;

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;


                    shift_was_pressed = false;


                }else {


                    bool is_selected = true, is_break = false, line_break = false, line_begin = true, page_passed = false, break_passed = false;
                    size_t th_count = 0, line_count = 0;

                    std::list<_span2>::iterator sel_it_temp = it_cp;
                    size_t sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            //sel_it_temp++;
                            sel_char_temp = 0;
                    }

                    if(sel_cp == sel_it_temp && (sel_cp->_start + sel_char == sel_it_temp->_start + sel_char_temp))
                        is_selected = false;

                    std::list<_span2>::iterator temp_it = it_cp;
                    std::list<_span2>::iterator break_it = test_cp; size_t break_pos = 0;

                    size_t t_end = temp_it != del_me2.end() ? temp_it->_start + char_pos : 0;


                    while(temp_it != del_me2.begin()) {

                        t_end = (!th_count++) ? t_end : temp_it->_end;

                        while(t_end > temp_it->_start && temp_it->append) {

                            while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));
                            if(per_buffer[t_end] == new_line) {

                                if(line_count++) {
                                    is_break = true;
                                    break;
                                }

                            }

                        }

                        if(is_break)
                            break;

                        while(!(--temp_it)->append);

                        if(temp_it == test_cp && temp_it != del_me2.begin()) {

                            temp_it = del_me2.erase(temp_it);


                            std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                            bool is_break = false;

                            while(t1 != remit_list.begin()) {
                                if(*--t1 == temp_it) {
                                    //std::cout << "hmm \n";
                                    is_break = true;
                                    break;
                                }
                            }

                            page_passed = true;

                            while(!(--temp_it)->append);
                            std::list<_span2>::iterator tt_it = temp_it;

                            if(!is_break) {

                                std::list<_span2>::iterator tt_it = temp_it;
                                while(!(++tt_it)->append);
                                if(tt_it != del_me2.end() && tt_it->_start == temp_it->_end) {

                                    if(tt_it == it_cp){
                                        char_pos += (temp_it->_end - temp_it->_start);
                                        it_cp = temp_it;
                                    }

                                    if(tt_it == sel_cp) {
                                        sel_char += (temp_it->_end - temp_it->_start);
                                        sel_cp = temp_it;
                                    }

                                    if(tt_it == sel_end_cp) {
                                        sel_end_char += (temp_it->_end - temp_it->_start);
                                        sel_end_cp = temp_it;
                                    }


                                    t_end = temp_it->_end;
                                    th_count = 0;

                                    temp_it->_end = tt_it->_end;

                                    del_me2.erase(tt_it);
                                    break_it = temp_it; break_pos = t_end;
                                }else {
                                    break_it = tt_it;
                                    break_pos = t_end;
                                }

                            }else {
                                while(!(++tt_it)->append);
                                break_it = tt_it;
                                break_pos = t_end;
                            }

                        }
                    }


                    /// forward_ cursor_check


                    __cursor t_x = {x_offset, y_offset};

                    int temp_x = x_offset;
                    int temp_off_char = 0, t_off_char = 0;

                    th_count = 0;

                    std::list<_span2>::iterator t_last = temp_it;
                    std::list<_span2>::iterator tt_last = temp_it;
                    std::list<_span2>::iterator t_break = temp_it;
                    std::list<_span2>::iterator tt_break = temp_it;
                    size_t char_break = 0 ,t_char_break = 0, tt_end = 0;


                    do{

                        size_t start_pos = !th_count++ ? t_end + UTF8_CHAR_LEN(per_buffer[t_end]) : temp_it->_start;
                        size_t end_pos = (temp_it == it_cp) ? it_cp->_start + char_pos : temp_it->_end;


                        while(start_pos < end_pos && temp_it->append) {

                            if(per_buffer[start_pos] == new_line){
                                if(line_break) {
                                    temp_x = t_x.x;
                                    t_end = tt_end;
                                    t_last = tt_last;

                                    line_break = false;

                                }else{
                                    temp_x = t_x.x;
                                    t_end = start_pos;
                                    t_last = temp_it;

                                }

                                if(line_begin) {
                                    if(!break_passed) {
                                        tt_break = temp_it;
                                        t_char_break = start_pos;
                                        temp_off_char = t_x.x;
                                    }
                                    line_begin = false;
                                }


                                t_break = tt_break;
                                char_break = t_char_break;
                                t_off_char = temp_off_char;
                                //if(!break_passed)
                                //t_off_char = temp_off_char;


                                line_begin = true;

                                t_x.x = x_offset;
                                t_x.y += (s_h + font_ascent);

                            }else if(per_buffer[start_pos] == '\t'){


                                if(((((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w) > old_x && !line_break) {

                                    temp_x = t_x.x;
                                    tt_end = start_pos;
                                    tt_last = temp_it;
                                    line_break = true;

                                }

                                if(line_begin) {

                                    if(!break_passed) {
                                        tt_break = temp_it;
                                        t_char_break = start_pos;
                                        temp_off_char = t_x.x;
                                    }


                                    if(!line_break) {
                                        temp_x = t_x.x;
                                        tt_end = start_pos;
                                        tt_last = temp_it;
                                    }

                                    line_begin = false;
                                }




                                t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( t_x.x > _width - s_w ) {

                                    if(line_break) {
                                        temp_x = t_x.x;
                                        t_end = tt_end;
                                        t_last = tt_last;

                                        line_break = false;

                                    }else {

                                        temp_x = t_x.x;
                                        t_end = start_pos;
                                        t_last = temp_it;

                                    }

                                    t_break = tt_break;
                                    char_break = t_char_break;
                                    t_off_char = temp_off_char;

                                    line_begin = true;


                                    t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                    t_x.x = s_w * (t_x.x / s_w);

                                    t_x.y += (s_h + font_ascent);
                                }
                            }else {

                                if(t_x.x + s_w > old_x && !line_break) {

                                    temp_x = t_x.x;
                                    tt_end = start_pos;
                                    tt_last = temp_it;
                                    line_break = true;

                                }


                                if(line_begin) {

                                    if(!break_passed) {
                                        tt_break = temp_it;
                                        t_char_break = start_pos;
                                        temp_off_char = t_x.x;
                                    }

                                    if(!line_break) {
                                        temp_x = t_x.x;
                                        tt_end = start_pos;
                                        tt_last = temp_it;
                                    }

                                    line_begin = false;

                                }


                                t_x.x += s_w;

                                if( t_x.x > _width - s_w) {


                                    if(line_break) {
                                        temp_x = t_x.x;
                                        t_end = tt_end;
                                        t_last = tt_last;

                                        line_break = false;

                                    }else {

                                        temp_x = t_x.x;
                                        t_end = start_pos;
                                        t_last = temp_it;
                                    }


                                    t_break = tt_break;
                                    char_break = t_char_break;
                                    t_off_char = temp_off_char;

                                    line_begin = true;

                                    t_x.x = x_offset;
                                    t_x.y += (s_h + font_ascent);
                                }
                            }

                            if(break_it == temp_it && break_pos == start_pos)
                                break_passed = true;


                            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                        }


                    }while(temp_it++ != it_cp);

                    if(char_break == t_break->_start) {
                        while(!(--t_break)->append);
                        if(t_break->_start != t_break->_end)
                            ++t_break;
                    }


                    it_cp = t_last;
                    char_pos = t_end;


                    if(page_passed) {

                        if(char_break < t_break->_end) {

                           // std::cout << "Hadiiii Leeennn\n";

                            std::list<_span2>::iterator to_put = del_me2.begin();

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                            std::list<char>::iterator tt_undo_it = undo_type.end();
                            bool is_break = false;

                            if(char_break > t_break->_start) {

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == t_break && *tt_undo_it == 0 ) {

                                  /*      if(tt_rem_it == remit_it) {

                                            remit_it = remit_list.insert(tt_rem_it, del_me2.insert(t_break, {t_break->_start, char_break, true}));
                                            to_put = *remit_it;
                                        }else */
                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(t_break, {t_break->_start, char_break, true}));


                               /*         if(*tt_undo_it == 1) {
                                            if(tt_undo_it == undo_type_it)
                                                undo_type_it = undo_type.insert(tt_undo_it, 1);
                                            else
                                                undo_type.insert(tt_undo_it, 1);

                                            is_break = true;
                                        }else { */
                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;
                                       // }

                                    }

                                }

                                if(!is_break)
                                    to_put = del_me2.insert(t_break, {t_break->_start, char_break, true});

                                t_break->_start = char_break;

                                if(t_break == sel_cp){


                                    if(to_put->_start + sel_char < to_put->_end)
                                        sel_cp = to_put;
                                    else
                                        sel_char = to_put->_start + sel_char - t_break->_start;

                                }

                                if(t_break == sel_end_cp){

                                    if(to_put->_start + sel_end_char < to_put->_end)
                                        sel_end_cp = to_put;
                                    else
                                        sel_end_char = to_put->_start + sel_end_char - t_break->_start;

                                }

                            }


                            test_cp = del_me2.insert(t_break, {0, 0, true});

                        }else if(t_break->_start != t_break->_end) {
                            while(!(++t_break)->append);
                            test_cp = del_me2.insert(t_break, {0, 0, true});
                        }else
                            test_cp = t_break;


                        if(!(char_pos - it_cp->_start)) {
                            while(!(--it_cp)->append);

                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                char_pos = 0;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;
                        }else if(char_pos)
                            char_pos -= it_cp->_start;

                        off_char = t_off_char;
/*
                        for(auto i : remit_list)
                            std::cout << " rem_it " << i->_start << " " << i->_end << std::endl;

                        for(auto i : del_me2)
                           std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "From UP*******\n";
*/

                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }else if(it_cp == del_me2.begin()){
                        while(++it_cp != del_me2.end() && !it_cp->append);
                        char_pos = 0;
                    }else if(!(char_pos - it_cp->_start)) {
                        while(!(--it_cp)->append);
                        if(it_cp != del_me2.begin())
                            char_pos = it_cp->_end - it_cp->_start;
                        else{
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            char_pos = 0;
                        }
                    }else
                        char_pos = t_end - it_cp->_start;



                   // std::cout << "[Up] it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }


                    sel_it_temp = it_cp;
                    sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            //sel_it_temp++;
                            sel_char_temp = 0;
                    }



                    if(is_selected && shift_pressed)
                        _Selection_Up(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);
                    else {
                        sel_cp = sel_it_temp;
                        sel_char = sel_char_temp;
                    }



                    if(shift_pressed) {


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                            _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                        x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                    }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                    }

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }
            if(e.key.keysym.sym == SDLK_DOWN){

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){

                    it_cp = sel_end_cp;
                    char_pos = sel_end_char;


                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it++ != sel_end_cp);

                        if(!has_test) {
                            __cursor selcp_cr1 = {x_offset, y_offset};
                            adv_cursor_while(per_buffer, test_cp, sel_cp, selcp_cr1, sel_char, off_char, y_offset, s_w, s_h + font_ascent, _width);


                            if(check_sel_cursor(per_buffer, del_me2, sel_cp, sel_end_cp, sel_char, sel_end_char, selcp_cr1,
                                             x_offset, s_w, s_h + font_ascent, _width, _height)) {

                                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                                x_offset, y_offset);

                            }

                        }else {

                            del_it = test_cp;
                            has_test = false;

                            do {

                                if(del_it == sel_cp) {
                                    has_test = true;
                                    break;
                                }

                            }while(del_it++ != sel_end_cp);

                            if(has_test)
                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                        }


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;

                    shift_was_pressed = false;

                }else {


                    size_t th_count = 0;
                    bool is_selected = true;


                    std::list<_span2>::iterator sel_it_temp = it_cp;
                    size_t sel_char_temp = char_pos;


                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }

                    if(sel_end_cp == sel_it_temp && (sel_end_cp->_start + sel_end_char == sel_it_temp->_start + sel_char_temp))
                        is_selected = false;


                    int is_break = 0;
                    bool line_break = false;


                    while(it_cp != del_me2.end()) {

                            size_t start_pos = (!th_count++) ? char_pos : 0;

                            while(it_cp->_start + start_pos < it_cp->_end && is_break <= 1 && it_cp->append) {

                                if(per_buffer[it_cp->_start + start_pos] == new_line){

                                    if(is_break++ < 1) {
                                        cr1.x = x_offset;
                                        cr1.y += (s_h + font_ascent);
                                    }else {
                                        char_pos = start_pos;
                                        line_break = true;
                                        break;
                                    }


                                }else if(per_buffer[it_cp->_start + start_pos] == '\t'){

                                    cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                    if( cr1.x > _width - s_w) {
                                        cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                                        cr1.x = s_w * (cr1.x / s_w);
                                        cr1.y += (s_h + font_ascent);
                                        is_break++;
                                    }
                                }else {
                                    cr1.x += s_w;
                                    if(cr1.x > _width - s_w) {
                                        cr1.x = x_offset;
                                        cr1.y += (s_h + font_ascent);
                                        is_break++;
                                    }
                                }


                                start_pos += UTF8_CHAR_LEN(per_buffer[it_cp->_start + start_pos]);

                                if(old_x <= cr1.x && is_break) {
                                    char_pos = start_pos;
                                    line_break = true;
                                    break;
                                }

                            }

                            if(line_break)
                                break;

                            it_cp++;
                        }

                        if(it_cp == del_me2.end()) {
                            while(!(--it_cp)->append);
                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                char_pos = 0;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;
                        }


                        sel_it_temp = it_cp;
                        sel_char_temp = char_pos;

                        if(it_cp->_start + char_pos == it_cp->_end) {

                                while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                                sel_char_temp = 0;
                        }


                        if(!is_selected) {
                            sel_end_cp = sel_it_temp;
                            sel_end_char = sel_char_temp;
                        }




                        if(cr1.y >= _height){

                            std::list<_span2>::iterator temp_it = test_cp;

                            int t_x = off_char;

                            while(++temp_it != del_me2.end()) {

                                size_t start_pos = 0;

                                while(start_pos + temp_it->_start < temp_it->_end && temp_it->append) {

                                    if(per_buffer[temp_it->_start + start_pos] == new_line){
                                        t_x = x_offset;
                                        off_char = t_x;
                                        cr1.y -= (s_h + font_ascent);
                                    }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                                        t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                        if( t_x > _width - s_w) {
                                            t_x %= (_width - s_w);// t_x + x_offset - (_width - s_w);
                                            t_x = s_w * (t_x / s_w);
                                            off_char = t_x;
                                            cr1.y -= (s_h + font_ascent);
                                        }
                                    }else {
                                        t_x += s_w;
                                        if(t_x > _width - s_w) {
                                            t_x = x_offset;
                                            off_char = t_x;
                                            cr1.y -= (s_h + font_ascent);
                                        }
                                    }

                                    start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);

                                    if(cr1.y < _height)
                                        break;
                                }
                                if(cr1.y < _height) {

                                    std::list<_span2>::iterator test_this = test_cp;

                                    if(temp_it->_start + start_pos < temp_it->_end) {

                                        size_t old_end_start = sel_end_cp->_start;
                                        std::list<_span2>::iterator to_put = del_me2.begin();

                                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                                        std::list<char>::iterator tt_undo_it = undo_type.end();
                                        bool is_break = false;

                                        while(tt_rem_it-- != remit_list.begin()) {
                                            --tt_undo_it;
                                            if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                                         /*       if(tt_rem_it == remit_it) {
                                                    remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));
                                                    to_put = *remit_it;
                                                }else */
                                                    to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));


                                       /*         if(*tt_undo_it == 1) {

                                                    if(tt_undo_it == undo_type_it)
                                                        undo_type_it = undo_type.insert(tt_undo_it, 1);
                                                    else
                                                        undo_type.insert(tt_undo_it, 1);

                                                    is_break = true;
                                                }else{ */
                                                    undo_type.insert(tt_undo_it, 0);
                                                    is_break = true;
                                                    break;
                                               // }


                                            }

                                        }

                                        if(!is_break)
                                            to_put = del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true});


                                        temp_it->_start += start_pos;

                                        if(temp_it == sel_cp){

                                            if(to_put->_start + sel_char < to_put->_end)
                                                sel_cp = to_put;
                                            else
                                                sel_char -= start_pos;
                                        }

                                        if(temp_it == sel_end_cp){

                                            if(to_put->_start + sel_end_char < to_put->_end)
                                                sel_end_cp = to_put;
                                            else
                                                sel_end_char -= (sel_end_cp->_start - old_end_start);

                                        }


                                        if(temp_it == it_cp)
                                            char_pos -= start_pos;



                                        test_cp = del_me2.insert(temp_it, {0, 0, true});


                                    }else if(it_cp == temp_it){
                                        while(++temp_it != del_me2.end() && !temp_it->append);
                                        test_cp = del_me2.insert(temp_it, {0, 0, true});
                                        it_cp = temp_it;
                                        char_pos = 0;
                                    }else{
                                        while(++temp_it != del_me2.end() && !temp_it->append);
                                        test_cp = del_me2.insert(temp_it, {0, 0, true});
                                    }



                                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);


                                //    if(test_this != del_me2.begin())
                                //        test_this = del_me2.erase(test_this);

                                    if(test_this != del_me2.begin()) {

                                        test_this = del_me2.erase(test_this);

                                        std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                                        bool is_break = false;

                                        while(t1 != remit_list.begin()) {
                                            if(*--t1 == test_this) {
                                                //std::cout << "hmm \n";
                                                is_break = true;
                                                break;
                                            }
                                        }

                                        if(!is_break) {

                                            std::list<_span2>::iterator t__t = test_this;

                                            while(!(--t__t)->append);

                                            if(t__t->_end == test_this->_start) {
                                                t__t->_end = test_this->_end;
                                                if(test_this == sel_cp){
                                                    size_t tt_add = sel_cp->_start;
                                                    sel_cp = t__t;
                                                    sel_char += (tt_add - sel_cp->_start);
                                                }

                                                if(test_this == sel_end_cp && is_selected) {
                                                    size_t tt_add = sel_end_cp->_start;
                                                    sel_end_cp = t__t;
                                                    sel_end_char += (tt_add - sel_end_cp->_start);
                                                }

                                                del_me2.erase(test_this);
                                            }

                                        }

                                    }
/*
                                    std::list<char>::iterator ttt = undo_type.begin();

                                    for(auto i : remit_list)
                                        std::cout << i->_start << " " << i->_end << " Type " << (int) *ttt++ <<std::endl;
                                    std::cout << "REMIT*******\n";

                                     for(auto i : del_me2)
                                         std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                                     std::cout << "From DOWN*******\n";

*/

                                    break;
                                }
                            }


                        }else if(cr1.y + _cur_height > _height){


                            //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                            y_offset += _height - (cr1.y + _cur_height);
                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                            cr1.y = _height - _cur_height;

                        }


                      //  std::cout << "[Down]: it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;


                        sel_it_temp = it_cp;
                        sel_char_temp = char_pos;

                        if(it_cp->_start + char_pos == it_cp->_end) {

                                while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                                sel_char_temp = 0;
                        }

                        if(is_selected && shift_pressed){

                            _Selection_Down(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);

                        }

                        if(shift_pressed) {


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                            _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                        x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                        }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                        }


                }

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }

            if(e.key.keysym.sym == SDLK_PAGEDOWN) {

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }



                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){

                    it_cp = sel_end_cp;
                    char_pos = sel_end_char;


                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it++ != sel_end_cp);

                        if(!has_test) {
                            __cursor selcp_cr1 = {x_offset, y_offset};
                            adv_cursor_while(per_buffer, test_cp, sel_cp, selcp_cr1, sel_char, off_char, y_offset, s_w, s_h + font_ascent, _width);


                            if(check_sel_cursor(per_buffer, del_me2, sel_cp, sel_end_cp, sel_char, sel_end_char, selcp_cr1,
                                             x_offset, s_w, s_h + font_ascent, _width, _height)) {

                                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                                x_offset, y_offset);

                            }

                        }else {

                            del_it = test_cp;
                            has_test = false;

                            do {

                                if(del_it == sel_cp) {
                                    has_test = true;
                                    break;
                                }

                            }while(del_it++ != sel_end_cp);

                            if(has_test)
                                find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                                &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                                x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);
                        }


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;

                    shift_was_pressed = false;

                }else {


                    size_t th_count = 0;
                    bool is_selected = true;


                    std::list<_span2>::iterator sel_it_temp = it_cp;
                    size_t sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }

                    if(sel_end_cp == sel_it_temp && (sel_end_cp->_start + sel_end_char == sel_it_temp->_start + sel_char_temp))
                        is_selected = false;


                    int is_break = 0;
                    bool line_break = false;

                    while(it_cp != del_me2.end()) {

                        size_t start_pos = (!th_count++) ? char_pos : 0;

                        //std::cout << (int) ((float) _height / (s_h + font_ascent) + 0.5f) << std::endl;



                        while(it_cp->_start + start_pos < it_cp->_end && it_cp->append && is_break <= (int) std::ceil((float) _height / (s_h + font_ascent)) ) {

                            if(per_buffer[it_cp->_start + start_pos] == new_line){

                                if(is_break++ < (int) std::ceil((float) _height / (s_h + font_ascent))) {
                                    cr1.x = x_offset;
                                    cr1.y += (s_h + font_ascent);
                                }else {
                                    char_pos = start_pos;
                                    line_break = true;
                                    break;
                                }


                            }else if(per_buffer[it_cp->_start + start_pos] == '\t'){

                                cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( cr1.x > _width - s_w) {
                                    cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                                    cr1.x = s_w * (cr1.x / s_w);
                                    cr1.y += (s_h + font_ascent);
                                    is_break++;
                                }
                            }else {
                                cr1.x += s_w;
                                if(cr1.x > _width - s_w) {
                                    cr1.x = x_offset;
                                    cr1.y += (s_h + font_ascent);
                                    is_break++;
                                }
                            }


                            start_pos += UTF8_CHAR_LEN(per_buffer[it_cp->_start + start_pos]);

                            if(old_x <= cr1.x && is_break > (int) std::ceil((float) _height / (s_h + font_ascent)) - 1 ) {
                                char_pos = start_pos;
                                line_break = true;
                                break;
                            }

                        }

                        if(line_break)
                            break;

                        it_cp++;
                    }

                    if(it_cp == del_me2.end()) {
                        while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            char_pos = 0;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;
                    }

                    sel_it_temp = it_cp;
                    sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }


                    if(!is_selected) {
                        sel_end_cp = sel_it_temp;
                        sel_end_char = sel_char_temp;
                    }


                    if(cr1.y >= _height) {
                        /// this is where the trick begins...


                        std::list<_span2>::iterator temp_it = it_cp;
                        bool is_break = false, last_run = false;

                        size_t t_end = 0;
                        __cursor t_x = {x_offset, y_offset};

                        while(temp_it != test_cp) {


                            t_end = (temp_it == it_cp) ? it_cp->_start + char_pos : temp_it->_end;

                            while(t_end > temp_it->_start && temp_it->append) {


                                while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));
                                if(per_buffer[t_end] == new_line) {


                                    // forward_ cursor_check

    PLEASE_GO_1:                    t_x.x = last_run ? off_char : x_offset;
                                    t_x.y = y_offset;

                                    std::list<_span2>::iterator found_it = temp_it;

                                    int first_run = 0;

                                    do{


                                        size_t start_pos = !first_run++ ? t_end : found_it->_start;
                                        size_t end_pos = (found_it == it_cp) ? it_cp->_start + char_pos : found_it->_end;

                                        while(start_pos < end_pos && found_it->append) {


                                            if(per_buffer[start_pos] == new_line){
                                                t_x.x = x_offset;
                                                t_x.y += (s_h + font_ascent);
                                            }else if(per_buffer[start_pos] == '\t'){

                                                t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                                if( t_x.x > _width - s_w ) {
                                                    t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                                    t_x.x = s_w * (t_x.x / s_w);
                                                    t_x.y += (s_h + font_ascent);
                                                }
                                            }else {
                                                t_x.x += s_w;
                                                if( t_x.x > _width - s_w) {
                                                    t_x.x = x_offset;
                                                    t_x.y += (s_h + font_ascent);
                                                }
                                            }

                                            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);


                                        }

                                    }while(found_it++ != it_cp);


                                }

                                if(t_x.y > (cr1.y - _height)) {
                                    is_break = true;
                                    break;
                                }

                            }

                            if(is_break)
                                break;
                            temp_it--;
                        }

                        if(!is_break) {
                            ++temp_it;
                            last_run = true;
                            goto PLEASE_GO_1;
                        }


                        int first_run = 0;


                        cr1.x = 0;
                        off_char = 0;

                        is_break = false;


                        while(temp_it != del_me2.end()) {

                            size_t start_pos = !first_run++ ? t_end : temp_it->_start;


                            while(start_pos < temp_it->_end && temp_it->append) {

                                if(t_x.y <= (cr1.y - _height)) {
                                    is_break = true;
                                    t_end = start_pos;
                                   break;
                                }

                                if(per_buffer[start_pos] == new_line){
                                    cr1.x = x_offset;
                                    off_char = cr1.x;
                                    t_x.y -= (s_h + font_ascent);
                                }else if(per_buffer[start_pos] == '\t'){

                                    cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                    if( cr1.x > _width - s_w ) {
                                        cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                                        cr1.x = s_w * (cr1.x / s_w);
                                        off_char = cr1.x;
                                        t_x.y -= (s_h + font_ascent);
                                    }
                                }else {
                                    cr1.x += s_w;
                                    if( cr1.x > _width - s_w) {
                                        cr1.x = x_offset;
                                        off_char = cr1.x;
                                        t_x.y -= (s_h + font_ascent);
                                    }
                                }


                                start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                            }

                            if(is_break)
                                break;
                            temp_it++;
                        }

                        if(!is_break)
                            t_end = temp_it->_end;

                        std::list<_span2>::iterator to_check = test_cp;

                        if(t_end < temp_it->_end) {

                            std::list<_span2>::iterator to_put = del_me2.end();

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                            std::list<char>::iterator tt_undo_it = undo_type.end();
                            bool is_break = false;

                            if(t_end != temp_it->_start) {


                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                                 /*       if(tt_rem_it == remit_it) {
                                            remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));
                                            to_put = *remit_it;
                                        }else */
                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));

                                 /*       if(*tt_undo_it == 1) {

                                            if(tt_undo_it == undo_type_it)
                                                undo_type_it = undo_type.insert(tt_undo_it, 1);
                                            else
                                                undo_type.insert(tt_undo_it, 1);

                                            is_break = true;
                                        }else{ */
                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;
                                     //   }


                                    }


                                }

                                if(!is_break)
                                    to_put = del_me2.insert(temp_it, {temp_it->_start, t_end, true});


                                if(temp_it == it_cp) {
                                    char_pos -= (t_end - temp_it->_start);
                                }

                                if(temp_it == sel_cp) {

                                    if(to_put->_start + sel_char < to_put->_end)
                                        sel_cp = to_put;
                                    else
                                        sel_char -= (t_end - temp_it->_start);
                                }
                                if(temp_it == sel_end_cp) {

                                    if(to_put->_start + sel_end_char < to_put->_end)
                                        sel_end_cp = to_put;
                                    else
                                        sel_end_char -= (t_end - temp_it->_start);
                                }
                                temp_it->_start = t_end;
                                test_cp = del_me2.insert(temp_it, {0, 0, true});

                            }else if (--temp_it != del_me2.begin()){

                                if(!temp_it->append)
                                    while(!(--temp_it)->append);

                                if(temp_it != del_me2.begin())
                                    test_cp = del_me2.insert(++temp_it, {0, 0, true});
                            }else
                                test_cp = del_me2.begin();


                        }else{
                            test_cp = del_me2.insert(temp_it, {0, 0, true});
                            it_cp = temp_it;
                            char_pos = 0;
                        }


                       // if(to_check != del_me2.begin())
                       //    del_me2.erase(to_check);

                        if(to_check != del_me2.begin()) {

                            to_check = del_me2.erase(to_check);

                            std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                            bool is_break = false;

                            while(t1 != remit_list.begin()) {
                                if(*--t1 == to_check) {
                                    //std::cout << "hmm \n";
                                    is_break = true;
                                    break;
                                }
                            }

                            if(!is_break) {

                                std::list<_span2>::iterator temp_it = to_check;

                                while(!(--temp_it)->append);

                                if(temp_it->_start != temp_it->_end && temp_it->_end == to_check->_start) {

                                    if(to_check == it_cp) {
                                        char_pos += (temp_it->_end - temp_it->_start);
                                        it_cp = temp_it;
                                    }

                                    if(to_check == sel_cp) {
                                        sel_char += (temp_it->_end - temp_it->_start);
                                        sel_cp = temp_it;
                                    }

                                    if(to_check == sel_end_cp) {
                                        sel_end_char += (temp_it->_end - temp_it->_start);
                                        sel_end_cp = temp_it;
                                    }


                                    temp_it->_end = to_check->_end;

                                    del_me2.erase(to_check);

                                }

                            }

                        }


                        ///????????
                        while(++it_cp != del_me2.end() && !it_cp->append);

                        if(it_cp != del_me2.end() && it_cp == test_cp){
                            it_cp = temp_it;
                            char_pos = 0;
                        }else
                            while(!(--it_cp)->append);
/*
                        for(auto i : remit_list)
                            std::cout << " rem_it " << i->_start << " " << i->_end << std::endl;


                        for(auto i : del_me2)
                           std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                        std::cout << "From PAGEDOWN*******\n";
*/

                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }else if(cr1.y + _cur_height > _height){


                            //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                            y_offset += _height - (cr1.y + _cur_height);
                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                            cr1.y = _height - _cur_height;

                        }

                  //  std::cout << "[PAGEDOWN] it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                    sel_it_temp = it_cp;
                    sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }

                    if(is_selected && shift_pressed)
                        _Selection_Down(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);


                    if(shift_pressed) {


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h,
                                    x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                            _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                        x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                    }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                    }

                }


                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;

            }
            if(e.key.keysym.sym == SDLK_PAGEUP) {

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }


                if(shift_was_pressed && (sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char))){


                    it_cp = sel_cp;
                    char_pos = sel_char;

                    if(it_cp != del_me2.begin()) {

                        std::list<_span2>::iterator del_it = sel_end_cp;
                        bool has_test = false;

                        do {

                            if(del_it == test_cp) {
                                has_test = true;
                                break;
                            }

                        }while(del_it-- != sel_cp);

                        if(has_test) {
                           // std::cout << "am I here?\n";

                            find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                            &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                            x_offset, y_offset);
                        }


                        if(!char_pos) {
                            while(!(--it_cp)->append);
                            if(it_cp != test_cp)
                                char_pos = it_cp->_end - it_cp->_start;
                            else
                                while(++it_cp != del_me2.end() && !it_cp->append);

                        }


                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    }else
                        while(++it_cp != del_me2.end() && !it_cp->append);

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(cr1.y + _cur_height > _height){

                        //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                        y_offset += _height - (cr1.y + _cur_height);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                        cr1.y = _height - _cur_height;

                    }

                    old_x = cr1.x;
                    sel_cp = it_cp;
                    sel_end_cp = it_cp;
                    sel_char = char_pos;
                    sel_end_char = char_pos;
                    clip_pager = false;


                    shift_was_pressed = false;

                }else {

                    size_t th_count = 0;
                    bool is_selected = true;


                    std::list<_span2>::iterator sel_it_temp = it_cp;
                    size_t sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }

                    if(sel_cp == sel_it_temp && (sel_cp->_start + sel_char == sel_it_temp->_start + sel_char_temp))
                        is_selected = false;


                    std::list<_span2>::iterator temp_it = it_cp;
                    bool is_break = false, last_run = false;


                    size_t t_end = 0;
                    __cursor t_x = {x_offset, y_offset};

                    while(temp_it != del_me2.begin()) {


                        t_end = (temp_it == it_cp) ? it_cp->_start + char_pos : temp_it->_end;

                        while(t_end > temp_it->_start && temp_it->append) {


                            while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));

                            if(per_buffer[t_end] == new_line) {


                                // forward_cursor_check

    PLEASE_GO_3:                t_x.x = x_offset; t_x.y = y_offset;

                                std::list<_span2>::iterator found_it = temp_it;

                                int first_run = 0;

                                do{


                                    size_t start_pos = !first_run++ ? t_end : found_it->_start;
                                    size_t end_pos = (found_it == it_cp) ? it_cp->_start + char_pos : found_it->_end;

                                    while(start_pos < end_pos && found_it->append) {


                                        if(per_buffer[start_pos] == new_line){
                                            t_x.x = x_offset;
                                            t_x.y += (s_h + font_ascent);
                                        }else if(per_buffer[start_pos] == '\t'){

                                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                            if( t_x.x > _width - s_w ) {
                                                t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                                t_x.x = s_w * (t_x.x / s_w);
                                                t_x.y += (s_h + font_ascent);
                                            }
                                        }else {
                                            t_x.x += s_w;
                                            if( t_x.x > _width - s_w) {
                                                t_x.x = x_offset;
                                                t_x.y += (s_h + font_ascent);
                                            }

                                        }

                                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);


                                    }

                                }while(found_it++ != it_cp);


                            }

                            //(_height / (s_h + font_ascent) + 1) * (s_h + font_ascent)

                            if(t_x.y >= (int) std::ceil((float) _height / (s_h + font_ascent)) * (s_h + font_ascent)  || last_run) {
                                is_break = true;
                                break;
                            }

                        }

                        if(is_break)
                            break;
                        temp_it--;
                    }


                    if(!is_break) {
                        temp_it++;
                        last_run = true;
                        goto PLEASE_GO_3;
                    }


                    int first_run = 0;

                    cr1.x = x_offset;


                    bool at_the_beginning = !t_x.y ? true : false;

                    is_break = false;


                    while(temp_it != del_me2.end()) {

                        size_t start_pos = !first_run++ ? t_end : temp_it->_start;


                        while(start_pos < temp_it->_end && temp_it->append) {

                            if(t_x.y <= (int) std::ceil((float) _height / (s_h + font_ascent)) * (s_h + font_ascent) ) {
                                is_break = true;
                                t_end = start_pos;
                                it_cp = temp_it;
                                char_pos = start_pos - temp_it->_start;
                                break;
                            }

                            if(per_buffer[start_pos] == new_line){
                                cr1.x = x_offset;
                                t_x.y -= (s_h + font_ascent);
                            }else if(per_buffer[start_pos] == '\t'){

                                cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( cr1.x > _width - s_w ) {
                                    cr1.x %= (_width - s_w); // cr1.x + x_offset - (_width - s_w);
                                    cr1.x = s_w * (cr1.x / s_w);
                                    t_x.y -= (s_h + font_ascent);
                                }
                            }else {
                                cr1.x += s_w;
                                if( cr1.x > _width - s_w) {
                                    cr1.x = x_offset;
                                    t_x.y -= (s_h + font_ascent);
                                }
                            }


                            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                        }

                        if(is_break)
                            break;
                        temp_it++;
                    }

                    /// SECOND RUN TO FIND TO FIND TEST_CP ///


                    temp_it = it_cp;
                    is_break = false;
                    last_run = false;
                    //_line_count = 0;


                    t_end = 0;
                    t_x.x = x_offset; t_x.y = y_offset;

                    while(temp_it != del_me2.begin()) {


                        t_end = (temp_it == it_cp) ? it_cp->_start + char_pos : temp_it->_end;

                        while(t_end > temp_it->_start && temp_it->append) {


                            while(per_buffer[--t_end] == ctrl_line || (((per_buffer[t_end] & 0x80) != 0) && ((per_buffer[t_end] & 0xC0) != 0xC0)));

                            if(per_buffer[t_end] == new_line) {


                                // forward_cursor_check

    PLEASE_GO_2:                t_x.x = x_offset; t_x.y = y_offset;

                                std::list<_span2>::iterator found_it = temp_it;

                                int first_run = 0;

                                do{


                                    size_t start_pos = !first_run++ ? t_end : found_it->_start;
                                    size_t end_pos = (found_it == it_cp) ? it_cp->_start + char_pos : found_it->_end;

                                    while(start_pos < end_pos && found_it->append) {


                                        if(per_buffer[start_pos] == new_line){
                                            t_x.x = x_offset;
                                            t_x.y += (s_h + font_ascent);
                                            //_line_count++;
                                        }else if(per_buffer[start_pos] == '\t'){

                                            t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                            if( t_x.x > _width - s_w ) {
                                                t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                                t_x.x = s_w * (t_x.x / s_w);
                                                t_x.y += (s_h + font_ascent);
                                                //_line_count++;
                                            }
                                        }else {
                                            t_x.x += s_w;
                                            if( t_x.x > _width - s_w) {
                                                t_x.x = x_offset;
                                                t_x.y += (s_h + font_ascent);
                                                //_line_count++;
                                            }

                                        }

                                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);


                                    }

                                }while(found_it++ != it_cp);


                            }

                            if(t_x.y > cr1.y || last_run) {
                                is_break = true;
                                break;
                            }

                        }

                        if(is_break)
                            break;
                        temp_it--;
                    }

                    if(!is_break) {
                        temp_it++;
                        last_run = true;
                        goto PLEASE_GO_2;

                    }



                    first_run = 0;
                    off_char = 0;
                    cr1.x = x_offset;

                    is_break = false;


                    while(temp_it != del_me2.end()) {

                        size_t start_pos = !first_run++ ? t_end : temp_it->_start;


                        while(start_pos < temp_it->_end && temp_it->append) {


                            if(t_x.y <= cr1.y) {
                                is_break = true;
                                t_end = start_pos;
                                break;
                            }

                            if(per_buffer[start_pos] == new_line){
                                cr1.x = x_offset;
                                off_char = cr1.x;
                                t_x.y -= (s_h + font_ascent);
                            }else if(per_buffer[start_pos] == '\t'){

                                cr1.x = (((cr1.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                if( cr1.x > _width - s_w ) {
                                    cr1.x %= (_width - s_w);  // cr1.x + x_offset - (_width - s_w);
                                    cr1.x = s_w * (cr1.x / s_w);
                                    off_char = cr1.x;
                                    t_x.y -= (s_h + font_ascent);
                                }
                            }else {
                                cr1.x += s_w;
                                if( cr1.x > _width - s_w) {
                                    cr1.x = x_offset;
                                    off_char = cr1.x;
                                    t_x.y -= (s_h + font_ascent);
                                }
                            }


                            start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                        }

                        if(is_break)
                            break;
                        temp_it++;
                    }

                    std::list<_span2>::iterator to_put = del_me2.end();

                    std::list<_span2>::iterator to_check = test_cp;

                    if(t_end < temp_it->_end) {

                       // std::cout << "hmm234\n";

                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_list.end();
                        std::list<char>::iterator tt_undo_it = undo_type.end();
                        bool is_break = false;

                        if(t_end != temp_it->_start) {

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                                    /* if(tt_rem_it == remit_it) {
                                            remit_it = remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));
                                            to_put = *remit_it;
                                        }else */
                                            to_put = *remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, t_end, true}));

                             /*           if(*tt_undo_it == 1) {

                                            if(tt_undo_it == undo_type_it)
                                                undo_type_it = undo_type.insert(tt_undo_it, 1);
                                            else
                                                undo_type.insert(tt_undo_it, 1);

                                            is_break = true;
                                        }else{ */
                                            undo_type.insert(tt_undo_it, 0);
                                            is_break = true;
                                            break;
                                    //   }


                                    }


                                }


                                if(!is_break)
                                    to_put = del_me2.insert(temp_it, {temp_it->_start, t_end, true});

                                if(temp_it == it_cp) {
                                    char_pos -= (t_end - temp_it->_start);
                                }


                                if(temp_it == sel_cp) {

                                    if(to_put->_start + sel_char < to_put->_end)
                                        sel_cp = to_put;
                                    else
                                        sel_char -= (t_end - temp_it->_start);

                                }

                                if(temp_it == sel_end_cp) {

                                    if(to_put->_start + sel_end_char < to_put->_end)
                                        sel_end_cp = to_put;
                                    else
                                        sel_end_char -= (t_end - temp_it->_start);
                                }

                                temp_it->_start = t_end;
                                test_cp = del_me2.insert(temp_it, {0, 0, true});

                        }else if (--temp_it != del_me2.begin()){

                            if(!temp_it->append)
                                while(!(--temp_it)->append);

                            if(temp_it != del_me2.begin())
                                test_cp = del_me2.insert(++temp_it, {0, 0, true});
                            else
                                test_cp = del_me2.begin();
                        }else
                            test_cp = del_me2.begin();

                    }


                   // if(to_check != del_me2.begin())
                   //     to_check = del_me2.erase(to_check);

                    if(to_check != del_me2.begin()) {

                        to_check = del_me2.erase(to_check);

                        std::list<std::list<_span2>::iterator>::iterator t1 = remit_list.end();
                        bool is_break = false;

                        while(t1 != remit_list.begin()) {
                            if(*--t1 == to_check) {
                                //std::cout << "hmm \n";
                                is_break = true;
                                break;
                            }
                        }

                        if(!is_break) {

                            std::list<_span2>::iterator temp_it = to_check;

                            while(!(--temp_it)->append);

                            if(temp_it->_start != temp_it->_end && temp_it->_end == to_check->_start) {

                                if(to_check == it_cp) {
                                    char_pos += (temp_it->_end - temp_it->_start);
                                    it_cp = temp_it;
                                }

                                if(to_check == sel_cp) {
                                    sel_char += (temp_it->_end - temp_it->_start);
                                    sel_cp = temp_it;
                                }

                                if(to_check == sel_end_cp) {
                                    sel_end_char += (temp_it->_end - temp_it->_start);
                                    sel_end_cp = temp_it;
                                }


                                temp_it->_end = to_check->_end;

                                del_me2.erase(to_check);

                            }

                        }


                    }



                    page_off_char = off_char;


                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);


                    /// to find cr1.x location

                    if(!at_the_beginning) {

                        temp_it = test_cp;
                        t_x.x = off_char; t_x.y = y_offset;
                        is_break = false;

                        while(temp_it != del_me2.end()) {

                            size_t start_pos = 0;
                            size_t end_pos = temp_it->_end - temp_it->_start;

                            while(start_pos < end_pos && temp_it->append) {

                                if(t_x.y >= cr1.y && t_x.x >= old_x) {
                                    is_break = true;
                                    if(!start_pos) {
                                        while(!(--temp_it)->append);
                                        if(temp_it != test_cp) {
                                            it_cp = temp_it;
                                            char_pos = temp_it->_end - temp_it->_start;
                                        }else {
                                            while(++temp_it != del_me2.end() && !temp_it->append);
                                            it_cp == temp_it;
                                            char_pos = 0;

                                        }
                                    }else {
                                        it_cp = temp_it;
                                        char_pos = start_pos;
                                    }

                                    cr1.x = t_x.x;

                                    break;

                                }


                                if(per_buffer[temp_it->_start + start_pos] == new_line){
                                    if(t_x.y >= cr1.y) {
                                        is_break = true;
                                        cr1.x = t_x.x;
                                        it_cp = temp_it;
                                        char_pos = start_pos;
                                        break;
                                    }
                                    t_x.x = x_offset;
                                    t_x.y += (s_h + font_ascent);
                                }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                                    t_x.x = (((t_x.x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                                    if( t_x.x > _width - s_w ) {
                                        t_x.x %= (_width - s_w); // t_x.x + x_offset - (_width - s_w);
                                        t_x.x = s_w * (t_x.x / s_w);
                                        t_x.y += (s_h + font_ascent);
                                    }
                                }else {

                                    t_x.x += s_w;

                                    if( t_x.x > _width - s_w) {
                                        t_x.x = x_offset;
                                        t_x.y += (s_h + font_ascent);
                                    }
                                }



                                start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);
                            }

                            if(is_break)
                                break;

                            temp_it++;

                        }

                    }

                    sel_it_temp = it_cp;
                    sel_char_temp = char_pos;

                    if(it_cp->_start + char_pos == it_cp->_end) {

                            while(++sel_it_temp != del_me2.end() && !sel_it_temp->append);
                            sel_char_temp = 0;
                    }
/*
                    for(auto i : del_me2)
                       std::cout << i._start << " " << i._end << " " << i.append << std::endl;
                    std::cout << "From PAGEUP*******\n";
*/

                    if(cr1.y < 0) {
                        y_offset = 0;
                        adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    }

                   // std::cout << "[PAGE UP] it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                    if(is_selected && shift_pressed)
                        _Selection_Up(del_me2, test_cp, sel_it_temp, &sel_cp, &sel_end_cp, sel_char_temp, &sel_char, &sel_end_char);
                    else {

                        sel_cp = sel_it_temp;
                        sel_char = sel_char_temp;

                    }

                    if(shift_pressed) {


                            _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h,
                                    x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                            _Sel_Render(per_buffer, del_me2, test_cp, sel_cp, sel_end_cp, _width, _height, s_w, s_h + font_ascent, _cur_height,
                                        x_offset, y_offset, off_char, sel_char, sel_end_char, screen);
                    }else {
                            sel_cp = it_cp;
                            sel_end_cp = it_cp;
                            sel_char = char_pos;
                            sel_end_char = char_pos;
                    }

                }



                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;
            }

        }


        if(e.type == SDL_TEXTINPUT) {

            if(!blink_on) {
                SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
            }


            if(remit_it != remit_list.end()) {
                remit_it = remit_list.erase(remit_it, remit_list.end());
                undo_type_it = undo_type.erase(undo_type_it, undo_type.end());
            }




            if(sel_cp != sel_end_cp || (sel_cp->_start + sel_char != sel_end_cp->_start + sel_end_char)) {

                ///following is for the cut & remit section

                std::list<_span2>::iterator sel_it = sel_cp;

                it_cp = sel_cp;

                //size_t th_count = 0;
                bool has_test = false;

                do {

                    if(sel_it == test_cp || !sel_it->append || sel_it == del_me2.end()) {

                        if(sel_it == test_cp) {
                            has_test = true;
                            sel_it = del_me2.erase(test_cp);
                            test_cp = del_me2.begin();
                            if(sel_it == del_me2.end())
                                continue;
                        }else
                            continue;
                    }



                    size_t t_start = (sel_it == sel_cp) ? sel_cp->_start + sel_char : sel_it->_start;
                    size_t t_end = (sel_it == sel_end_cp) ? sel_end_cp->_start + sel_end_char : sel_it->_end;


                    if(t_start > sel_it->_start) {

                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                        std::list<char>::iterator tt_undo_it = undo_type_it;
                        bool is_break = false;

                        while(tt_rem_it-- != remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == sel_it && *tt_undo_it == 0 ) {

                                remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {sel_it->_start, t_start, true}));

                                undo_type.insert(tt_undo_it, 0);
                                is_break = true;
                                break;

                            }

                        }

                        if(!is_break)
                            del_me2.insert(sel_it, {sel_it->_start, t_start, true});

                        sel_it->_start = t_start;

                    }


                    if(t_end < sel_it->_end) {

                        if(t_start < t_end) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<_span2>::iterator to_put = del_me2.end();
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == sel_it && *tt_undo_it == 0) {


                                    to_put = *remit_list.insert(tt_rem_it, del_me2.insert(sel_it, {t_start, t_end, false}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;

                                }

                            }

                            if(!is_break)
                                to_put = del_me2.insert(sel_it, {t_start, t_end, false});

                            remit_list.insert(remit_it, to_put);
                            undo_type.insert(undo_type_it, 1);
                            sel_it->_start = t_end;
                        }


                    }else {

                        sel_it->append = false;
                        remit_list.insert(remit_it, sel_it);
                        undo_type.insert(undo_type_it, 1);

                    }



                }while (sel_it++ != sel_end_cp);


               while(!(--it_cp)->append);
                if(it_cp == test_cp) {
                    while(++it_cp != del_me2.end() && !it_cp->append);
                    char_pos = 0;
                }else
                    char_pos = it_cp->_end - it_cp->_start;



                if(has_test && it_cp != del_me2.end()) {
                    find_next_page(per_buffer, del_me2, remit_list, undo_type, &test_cp, &it_cp, remit_it, undo_type_it,
                                    &char_pos, &off_char, _width, _height, s_w, s_h + font_ascent,
                                    x_offset, y_offset);

                }


                adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);

                if(cr1.y < 0) {
                    y_offset = 0;
                    adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);
                    _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);

                }

            }

            if(insert_toggle) {

                if(it_cp != del_me2.end()) {

                    //std::list<_span2>::iterator temp_it = it_cp;
                    size_t start_pos = it_cp->_start + char_pos;


                    if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) < it_cp->_end) { /// within case

                        if(char_pos) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;
                                }

                            }

                            if(!is_break)
                                del_me2.insert(it_cp, {it_cp->_start, start_pos, true});
                        }




                        std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {start_pos, start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]), false}));

                        undo_type.insert(undo_type_it, 1);



                        std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                        std::list<char>::iterator tt_undo_it = undo_type_it;

                        while(tt_rem_it-- != remit_list.begin()) {
                            --tt_undo_it;
                            if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                remit_list.insert(tt_rem_it, t1);

                                undo_type.insert(tt_undo_it, 0);
                                break;
                            }

                        }


                        it_cp->_start = start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]);


                        if(char_pos){
                            while(!(--it_cp)->append);
                            char_pos = it_cp->_end - it_cp->_start;
                        }



                    }else if(start_pos + UTF8_CHAR_LEN(per_buffer[start_pos]) == it_cp->_end) { /// equal case

                        if(char_pos) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, start_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;
                                }

                            }

                            if(!is_break)
                                del_me2.insert(it_cp, {it_cp->_start, start_pos, true});

                            it_cp->_start = start_pos;
                        }

                        it_cp->append = false;

                        remit_list.insert(remit_it, it_cp);
                        undo_type.insert(undo_type_it, 1);

                        if(char_pos){
                            while(!(--it_cp)->append);
                            char_pos = it_cp->_end - it_cp->_start;
                        }else
                            while(++it_cp != del_me2.end() && !it_cp->append);


                    }else if(++it_cp != del_me2.end()) { /// end case

                        if(!it_cp->append)
                            while(++it_cp != del_me2.end() && !it_cp->append);

                        if(it_cp != del_me2.end()) {

                            if(it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]) < it_cp->_end) { /// sub within case


                                std::list<_span2>::iterator t1 = *remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + UTF8_CHAR_LEN(per_buffer[it_cp->_start]), false}));

                                undo_type.insert(undo_type_it, 1);


                                std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                                std::list<char>::iterator tt_undo_it = undo_type_it;

                                while(tt_rem_it-- != remit_list.begin()) {
                                    --tt_undo_it;
                                    if(*tt_rem_it == it_cp && *tt_undo_it == 0 ) {

                                        remit_list.insert(tt_rem_it, t1);

                                        undo_type.insert(tt_undo_it, 0);
                                        break;
                                    }

                                }

                                it_cp->_start += UTF8_CHAR_LEN(per_buffer[it_cp->_start]);

                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;

                            }else {  ///sub end case


                                it_cp->append = false;

                                remit_list.insert(remit_it, it_cp);
                                undo_type.insert(undo_type_it, 1);

                                while(!(--it_cp)->append);
                                char_pos = it_cp->_end - it_cp->_start;

                            }
                        }else {
                            while(!(--it_cp)->append);
                            if(it_cp == test_cp) {
                                while(++it_cp != del_me2.end() && !it_cp->append);
                                if(it_cp == del_me2.end())
                                    char_pos = 0;
                                else
                                    char_pos = it_cp->_end - it_cp->_start;
                            }else
                                char_pos = it_cp->_end - it_cp->_start;
                        }

                    }else {
                        while(!(--it_cp)->append);
                        if(it_cp == test_cp) {
                            while(++it_cp != del_me2.end() && !it_cp->append);
                            if(it_cp == del_me2.end())
                                char_pos = 0;
                            else
                                char_pos = it_cp->_end - it_cp->_start;
                        }else
                            char_pos = it_cp->_end - it_cp->_start;

                    }
                }


            }






            //std::cout << "[Text]: it_cp = " << it_cp->_start << " " << it_cp->_end << " " << char_pos << std::endl;

            std::string t1 = e.text.text;

            per_buffer += t1;


            if (it_cp->_start + char_pos == it_cp->_end  && per_buffer.size() - t1.size() == it_cp->_end ) {


                it_cp->_end += t1.size();
                char_pos += t1.size();

            }else {

                if(char_pos > 0 && it_cp->_start + char_pos < it_cp->_end) {

                    std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                    std::list<char>::iterator tt_undo_it = undo_type_it;
                    bool is_break = false;

                    while(tt_rem_it-- != remit_list.begin()) {
                        --tt_undo_it;
                        if(*tt_rem_it == it_cp  && *tt_undo_it == 0 ) {

                            remit_list.insert(tt_rem_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));

                            undo_type.insert(tt_undo_it, 0);
                            is_break = true;
                            break;

                        }
                    }

                    if(!is_break)
                        del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});

                    //remit_list.insert(remit_it, del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true}));
                    //del_me2.insert(it_cp, {it_cp->_start, it_cp->_start + char_pos, true});
                    it_cp->_start = it_cp->_start + char_pos;
                    char_pos = 0;

                }else if(it_cp->_start + char_pos == it_cp->_end) {

                    //remit_list.insert(remit_it, it_cp);
                    while(++it_cp != del_me2.end() && !it_cp->append);
                    //it_cp++;
                    char_pos = 0;


                }


                it_cp = del_me2.insert(it_cp, {per_buffer.size() - t1.size(), per_buffer.size(), true});

                //remit_it_ctrl = remit_list.insert(remit_it, it_cp);
                //remit_list_del.push_back(remit_list.insert(remit_it, it_cp));
                remit_list.insert(remit_it, it_cp);

                undo_type.insert(undo_type_it, 0);

                char_pos += t1.size();

            }
/*
            std::list<char>::iterator t2 = undo_type.begin();

            for(auto i : remit_list)
                std::cout << (i)->_start << " " << (i)->_end << " " << (int) *t2++ <<std::endl;
            std::cout << "From TEXT remit_list*******\n";

            for(auto i : del_me2)
                std::cout << i._start << " " << i._end << " " << i.append << std::endl;
            std::cout << "From Text*******\n";

*/

            adv_cursor(per_buffer, cr1, it_cp->_start, char_pos - t1.size(), x_offset, s_w, s_h + font_ascent, _width);

            old_x = cr1.x;


            if(cr1.y >= _height){

                std::list<_span2>::iterator temp_it = test_cp;

                int t_x = off_char;

                while(++temp_it != del_me2.end()) {

                    size_t start_pos = 0;

                    while(start_pos + temp_it->_start < temp_it->_end && temp_it->append) {

                        if(per_buffer[temp_it->_start + start_pos] == new_line){
                            t_x = x_offset;
                            off_char = t_x;
                            cr1.y -= (s_h + font_ascent);
                        }else if(per_buffer[temp_it->_start + start_pos] == '\t'){

                            t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( t_x > _width - s_w) {
                                t_x %= (_width - s_w); // t_x + x_offset - (_width - s_w);
                                t_x = s_w * (t_x / s_w);
                                off_char = t_x;
                                cr1.y -= (s_h + font_ascent);
                            }
                        }else {
                            t_x += s_w;
                            if(t_x > _width - s_w) {
                                t_x = x_offset;
                                off_char = t_x;
                                cr1.y -= (s_h + font_ascent);
                            }
                        }

                        start_pos += UTF8_CHAR_LEN(per_buffer[temp_it->_start + start_pos]);

                        if(cr1.y < _height)
                            break;
                    }
                    if(cr1.y < _height) {

                        std::list<_span2>::iterator test_this = test_cp;

                        if(temp_it->_start + start_pos < temp_it->_end) {

                            std::list<std::list<_span2>::iterator>::iterator tt_rem_it = remit_it;
                            std::list<char>::iterator tt_undo_it = undo_type_it;
                            bool is_break = false;

                            while(tt_rem_it-- != remit_list.begin()) {
                                --tt_undo_it;
                                if(*tt_rem_it == temp_it  && *tt_undo_it == 0 ) {

                                    remit_list.insert(tt_rem_it, del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true}));

                                    undo_type.insert(tt_undo_it, 0);
                                    is_break = true;
                                    break;

                                }

                            }

                            if(!is_break)
                                del_me2.insert(temp_it, {temp_it->_start, temp_it->_start + start_pos, true});

                            temp_it->_start += start_pos;


                            if(temp_it == it_cp)
                                char_pos -= start_pos;

                            test_cp = del_me2.insert(temp_it, {0, 0, true});
                        }else if(it_cp == temp_it){
                            test_cp = del_me2.insert(++temp_it, {0, 0, true});
                            it_cp = temp_it;
                            char_pos = 0;
                        }else
                            test_cp = del_me2.insert(++temp_it, {0, 0, true});


                        _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);



                       // if(test_this != del_me2.begin())
                       //     del_me2.erase(test_this);

                        if(test_this != del_me2.begin()) {

                            test_this = del_me2.erase(test_this);

                            std::list<std::list<_span2>::iterator>::iterator t1 = remit_it;
                            bool is_break = false;

                            while(t1 != remit_list.begin()) {
                                if(*--t1 == test_this) {
                                    //std::cout << "hmm \n";
                                    is_break = true;
                                    break;
                                }
                            }

                            if(!is_break) {

                                std::list<_span2>::iterator t__t = test_this;

                                while(!(--t__t)->append);

                                if(t__t->_end == test_this->_start) {

                                    t__t->_end = test_this->_end;
                                    del_me2.erase(test_this);
                                }

                            }

                        }

                        break;
                    }
                }

            }else if(cr1.y + _cur_height > _height){


                //std::cout << "need to be offsetted by " << _height - (cr1.y + _cur_height) << std::endl;
                y_offset += _height - (cr1.y + _cur_height);
                _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);
                cr1.y = _height - _cur_height;

            }else
                _Render(per_buffer, del_me2, test_cp, _width, _height, s_w, s_h, x_offset, y_offset, font_ascent, off_char, font_map, screen, font_atlas, key_color);



            //std::cout << "list bucket size " << del_me2.size() << std::endl;


            sel_cp = it_cp;
            sel_end_cp = it_cp;
            sel_char = char_pos;
            sel_end_char = char_pos;
            shift_was_pressed = false;

            dest_rect.x = cr1.x; dest_rect.y = cr1.y;
            last_time = SDL_GetTicks64() -_Cursor_Delay;
            blink_on = true;
        }

        if(e.type == SDL_MOUSEMOTION) {

        }

        if(e.type == SDL_MOUSEBUTTONDOWN) {
            if(e.button.button == SDL_BUTTON_LEFT) {

                if(!blink_on) {
                    SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                    SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                }

                int m_x = 0, m_y = 0;
                SDL_GetMouseState(&m_x, &m_y);


                std::cout << "Left Mouse button pressed " << m_x / s_w  << " " << m_y / (s_h + font_ascent) << std::endl;

                int t_x = off_char, t_y = y_offset;
                std::list<_span2>::iterator temp_it = test_cp;

                std::list<_span2>::iterator last_it = test_cp;
                size_t last_pos = 0;
                bool is_break = false;

                while(++temp_it != del_me2.end()&& !is_break) {

                    size_t start_pos = temp_it->_start;

                    while(start_pos < temp_it->_end && t_y < _height && temp_it->append) {


                        if(t_y / (s_h + font_ascent) > m_y / (s_h + font_ascent)) {
                            std::cout << "found last " << last_it->_start <<  " " << last_pos << std::endl;
                            it_cp = last_it;
                            char_pos = last_pos - temp_it->_start;
                            is_break = true;
                            break;
                        }

                        if(t_x / s_w >= m_x / s_w) {

                            if(t_y / (s_h + font_ascent) == m_y / (s_h + font_ascent)) {
                                std::cout << "found " << temp_it->_start <<  " " << start_pos << std::endl;
                                it_cp = temp_it;
                                char_pos = start_pos - temp_it->_start;
                                is_break = true;
                                break;
                            }
                        }


                        if(per_buffer[start_pos] == new_line){
                            t_x = x_offset;
                            t_y += (s_h + font_ascent);
                        }else if(per_buffer[start_pos] == '\t'){

                            t_x = (((t_x/s_w) / _Tab_Width) + 1) * _Tab_Width * s_w;

                            if( t_x > _width - s_w ) {
                                t_x %= (_width - s_w);// t_x + x_offset - (_width - s_w);
                                t_x = s_w * (t_x / s_w);
                                t_y += (s_h + font_ascent);
                            }
                        }else {
                            t_x += s_w;
                            if( t_x > _width - s_w) {
                                t_x = x_offset;
                                t_y += (s_h + font_ascent);
                            }
                        }

                        last_pos = start_pos;
                        last_it = temp_it;
                        start_pos += UTF8_CHAR_LEN(per_buffer[start_pos]);

                    }
                }

                adv_cursor_while(per_buffer, test_cp, it_cp, cr1, char_pos, off_char, y_offset, s_w, s_h + font_ascent, _width);



                old_x = cr1.x;
                sel_cp = it_cp;
                sel_end_cp = it_cp;
                sel_char = char_pos;
                sel_end_char = char_pos;
                shift_was_pressed = false;

                dest_rect.x = cr1.x; dest_rect.y = cr1.y;
                last_time = SDL_GetTicks64() -_Cursor_Delay;
                blink_on = true;

            }
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
                        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y + ins_offset, _cur_width, _cur_height};
                        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
                    }

                    old_x = cr1.x;
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
    TTF_Quit();
    SDL_Quit();


    return EXIT_SUCCESS;
}

