#include "Buffer.hpp"

Buffer::Buffer(const char* BufferName, int _width, int _height) {

    if((strlen(BufferName) + 1) <= 250)
        memcpy(this->BufferName, BufferName, strlen(BufferName) + 1);

    this->_width = _width;
    this->_height = _height;

    screen = SDL_CreateRGBSurface(0, _width , _height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    prepare_font_atlas();

    _cur_height = s_h + font_ascent;
    _cur_width = 1;
    t_height = (_height / _cur_height) * _cur_height;

    line_offset = 7 * s_w;
    cr1.x = line_offset;
    cr1.y = 0;
    count_spaces = 0;
    current_head_line = 0;
    max_line = 0;
    copy_line_len = 0;

    x_offset = 0;
    y_offset = -_cur_height;

    ctrl_pressed = false; shift_pressed = false; shift_was_pressed = false;

    blink_on = true;
    dest_rect = {cr1.x, cr1.y, _cur_width, _cur_height};

    line_map_size = 10000;

    line_map = (int*) malloc(line_map_size * sizeof(int));
    memset(line_map, -1, line_map_size * sizeof(int));
    line_map[0] = 0;


    Cursor = SDL_CreateRGBSurface(0, _cur_width , _cur_height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    Temp_Surface = SDL_CreateRGBSurfaceWithFormat(0, Cursor->w , Cursor->h, 32, screen->format->format);


    SDL_FillRect(Cursor, NULL, SDL_MapRGBA(Cursor->format, 29, 242, 10, 180));
    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));

}

void Buffer::shift_control() {

    if(shift_was_pressed && !shift_pressed) {
        my_table.batch_start = {my_table.it, my_table.Pos};
        my_table.batch_end = {my_table.it, my_table.Pos};
        shift_was_pressed = false;
    }

}

void Buffer::set_shift(bool pressed) {

        shift_pressed = pressed;

        if(pressed == false) {

            if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second))
                shift_was_pressed = true;

        }else {

            if(!shift_was_pressed) {
                my_table.batch_start = {my_table.it, my_table.Pos};
                my_table.batch_end = {my_table.it, my_table.Pos};
            }

        }
}

int Buffer::find_line(int offset) {

    int left = current_head_line;
    int right = current_head_line;

    if(line_map[current_head_line] == offset)
        return current_head_line;

    while(left || right < line_map_size) {

        if(left > 0)
            if(line_map[--left] == offset)
                return left;

        if(right < line_map_size)
            if(line_map[++right] == offset)
                return right;

    }

    return -1;
}


int Buffer::find_head_line(int offset) {

    int left = current_head_line;
    int right = current_head_line;

    if(line_map[current_head_line] == offset)
        return current_head_line;

    while(left || right < line_map_size) {

        if(left > 0)
            if(line_map[--left] == offset) {
                current_head_line = left;
                return left;
            }

        if(right < line_map_size)
            if(line_map[++right] == offset) {
                current_head_line = right;
                return right;
            }
    }

    return -1;
}


void Buffer::remove_line(int len, int offset) {

    int line_count = current_head_line;


    for(auto it = my_table.head; it != my_table.it; ++it) {
        if(!my_table.buffer.substr(it->offset, UTF8_CHAR_LEN(my_table.buffer[it->offset])).compare("\r\n"))
            line_count++;
    }

    memmove(line_map + line_count, line_map + line_count + len, sizeof(int) * max_line);

    max_line--;

    for(int i = 0; i < 10; i++)
        std::cout << "line " << i << " offset " << line_map[i] << "\n";

    std::cout << "Max Line " << max_line << "\n";

}

void Buffer::insert_line(int len, int offset) {

    int line_count = current_head_line;


    for(auto it = my_table.head; it != my_table.it; ++it) {
        if(!my_table.buffer.substr(it->offset, UTF8_CHAR_LEN(my_table.buffer[it->offset])).compare("\r\n"))
            line_count++;
    }


    bool needs_update = false;

    if(++max_line == line_map_size)
        needs_update = true;

    if(needs_update) {
        std::cout << "needs update\n";
    }


    memmove(line_map + line_count + len, line_map + line_count, sizeof(int) * max_line);


    line_map[line_count] = offset;

    for(int i = 0; i < 10; i++)
        std::cout << "line " << i << " offset " << line_map[i] << "\n";

    std::cout << "Max Line " << max_line << "\n";


}

void Buffer::update_line_numbers() {

    size_t line_count = my_table.line_map[my_table.head->offset];

    auto it = my_table.head;

    do {
        if(!my_table.buffer.substr(it->offset, UTF8_CHAR_LEN(my_table.buffer[it->offset])).compare("\r\n"))
            my_table.line_map[it->offset] = line_count++;
    }while(it++ != my_table.it);

}

void Buffer::prepare_font_atlas() {

    float dpi_v, dpi_h;
    SDL_GetDisplayDPI(0, NULL, &dpi_h, &dpi_v);
    TTF_Font *my_font = TTF_OpenFontDPI(".\\fonts\\UbuntuMono-Regular.ttf", 13, dpi_h, dpi_v);
    //TTF_Font *my_font = TTF_OpenFont(".\\fonts\\consola.ttf", font_size);

    std::string my_string = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    my_string += "ÇĞİÖŞÜçğıöşü"; /// Turkish

    font_atlas = TTF_RenderUTF8_Blended(my_font, my_string.c_str(), {29, 242, 10});
    font_atlas_reverse = TTF_RenderUTF8_Blended(my_font, my_string.c_str(), {0, 0, 0});

    //SDL_SaveBMP(source, ".\\fonts\\test.bmp");
    //SDL_Surface *source = SDL_LoadBMP(".\\fonts\\test.bmp");

    TTF_SizeUTF8(my_font, my_string.c_str(), &s_w, &s_h);

    uint32_t count = 0;
    int32_t main_cnt = 0;

    while(count < my_string.size()) {
        count += UTF8_CHAR_LEN(my_string[count]);
        main_cnt++;
    }

    s_w /= main_cnt;

    font_ascent = TTF_FontLineSkip(my_font) - s_h;

    main_cnt = 0; count = 0;

    while(count < my_string.size()) {
        int new_count = UTF8_CHAR_LEN(my_string[count]);
        font_map[my_string.substr(count, new_count)] = {s_w * main_cnt++, 0};
        count += new_count;
    }

    TTF_CloseFont(my_font);

}

void Buffer::Cursor_Blink() {

    if (blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(screen, &temp_rect, Temp_Surface, NULL);
        SDL_BlitSurface(Cursor, NULL, screen, &temp_rect);

        blink_on = false;
    }
    else {
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);

        blink_on = true;

    }

}



void Buffer::Render() {

    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));


    int t_x = line_offset + x_offset, t_y = y_offset;
    bool render_batch = false;

    SDL_Surface *Cursor = SDL_CreateRGBSurface(0, s_w , _cur_height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_FillRect(Cursor, NULL, SDL_MapRGBA(Cursor->format, 29, 242, 10, 180));

    //SDL_Rect line_rect = {0, 0, line_offset_rect, _height};

    //SDL_FillRect(screen, &line_rect, SDL_MapRGBA(screen->format, 30, 32, 36, 255));

    //std::cout << "Offset = " << my_table.head->offset << " Head Line " << find_head_line((int) my_table.head->offset) << "\n";

    //std::cout << "Head offset " << my_table.head->offset << "\n";

    //size_t _line_count = my_table.line_map[my_table.head->offset];
    size_t _line_count = find_head_line((int) my_table.head->offset);

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

                //my_table.line_map[it->offset] = _line_count;
                _linec = std::to_string(++_line_count);

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
        SDL_Rect temp_rect = {line_offset, t_y, s_w, _cur_height};
        SDL_BlitSurface(font_atlas, &temp_font, screen, &temp_rect);
    }

    SDL_FreeSurface(Cursor);

}

void Buffer::find_page_head() {

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

__cursor Buffer::find_cursor() {

    __cursor t_cr1 = {line_offset + x_offset, y_offset};

    if(my_table.piece_map.size()) {

        auto it = my_table.head;
        int _count_spaces = 0;
        bool first_encounter = false;

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
                    count_spaces = _count_spaces;
                    _count_spaces = 0;
                    first_encounter = false;
                }else{
                    t_cr1.x += s_w;
                    if(!first_encounter) {
                        if(!my_table.buffer.substr(Pos, UTF8_CHAR_LEN(my_table.buffer[Pos])).compare(" "))
                            _count_spaces++;
                        else
                            first_encounter = true;
                    }

                }
            }

        }while(it++ != my_table.it && t_cr1.y <= _height + _cur_height);

    }


    return t_cr1;

}

void Buffer::find_x(__cursor *to_find) {


    if(to_find->x > (_width - s_w)) {
        while(to_find->x >= (_width >> 1)){
            to_find->x -= s_w;
            x_offset -= s_w;
        }
    }else if(to_find->x < line_offset ) {
        while(x_offset < 0 && to_find->x <= (_width >> 1)) {
            to_find->x += s_w;
            x_offset += s_w;
        }
    }
}

int Buffer::page_upwards() {

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

void Buffer::page_downwards() {

    auto t = my_table.head;

    while(++t != my_table.piece_map.end()) {

        if(!my_table.buffer.substr(t->offset, 2).compare("\r\n")) {
            my_table.head = t;
            break;
        }
    }

}

void Buffer::find_page() {

    __cursor to_find = find_cursor();

    if(to_find.y > (t_height - _cur_height)) {

       find_page_head();

       to_find.y = -_cur_height;

       while(to_find.y < _height>>1 && my_table.head != my_table.piece_map.begin()) {
            to_find.y = page_upwards();
            to_find = find_cursor();
       }

       to_find = find_cursor();
    }

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

}


bool Buffer::batch_delete(bool is_cut) {

    if(is_cut)
        my_table.batch_buffer.clear();

    if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {

        my_table.it = my_table.batch_end.first;
        my_table.Pos = my_table.batch_end.second;

        auto it_start = my_table.batch_start.first;

        while(it_start != my_table.piece_map.end()){
            if(!my_table.buffer.substr(it_start->offset, 2).compare("\r\n"))
                break;
            it_start++;
        }

        int start_line = find_line(it_start->offset) + 1;
        int c_lines = 0;

        auto it = my_table.batch_start.first;
        size_t batch_length = 0;

        do {
            size_t length = (it == my_table.batch_end.first) ? my_table.batch_end.second : it->length;
            size_t s_pos = (it == my_table.batch_start.first) ? my_table.batch_start.second : 0;
            for(size_t t_pos = s_pos; t_pos < length; t_pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + t_pos])) {
                if(!my_table.buffer.substr(it->offset, 2).compare("\r\n"))
                    c_lines++;
                batch_length++;
                if(is_cut)
                    my_table.batch_buffer += my_table.buffer.substr(it->offset + t_pos, UTF8_CHAR_LEN(my_table.buffer[it->offset + t_pos]));
            }
        }while(it++ != my_table.batch_end.first);

        if(c_lines) {
            memmove(line_map + start_line, line_map + start_line + c_lines, sizeof(int) * max_line);
            max_line -= c_lines;
        }

        for(int i = 0; i < 100; i++)
            std::cout << "line " << i << " offset " << line_map[i] << "\n";

        std::cout << "Max Line " << max_line << "\n";

        while(batch_length--)
            my_table.delete_text(false);

        find_page();


        my_table.batch_start.first = my_table.it;
        my_table.batch_start.second = my_table.Pos;
        my_table.batch_end.first = my_table.it;
        my_table.batch_end.second = my_table.Pos;
        shift_was_pressed = false;

        return true;
    }
    return false;

}

void Buffer::Undo() {
    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

    if(my_table.undo_list.size()) {
            if(!my_table.buffer.substr(my_table.undo_list.back().piece.offset, 2).compare("\r\n")) {
                if(my_table.undo_list.back().type == my_table.INSERT) {

                    std::cout << "The line is " <<find_line(my_table.undo_list.back().piece.offset) << "\n";

                    int start_line = find_line(my_table.undo_list.back().piece.offset);

                    memmove(line_map + start_line, line_map + start_line + 1, sizeof(int) * max_line);
                    max_line--;

                    for(int i = 0; i < 10; i++)
                        std::cout << "line " << i << " offset " << line_map[i] << "\n";

                    std::cout << "Max Line " << max_line << "\n";

                }
            }

    }

    my_table.undo();
    find_page();


    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Redo() {
    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

    my_table.redo();
    find_page();

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Cut() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    batch_delete(true);
    shift_control();

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}


void Buffer::Copy() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    my_table.batch_buffer.clear();
    copy_line_len = 0;

    if(my_table.batch_start.first != my_table.batch_end.first || (my_table.batch_start.first == my_table.batch_end.first && my_table.batch_start.second != my_table.batch_end.second)) {

        auto it = my_table.batch_start.first;

        do {
            size_t length = (it == my_table.batch_end.first) ? my_table.batch_end.second : it->length;
            size_t s_pos = (it == my_table.batch_start.first) ? my_table.batch_start.second : 0;
            for(size_t t_pos = s_pos; t_pos < length; t_pos += UTF8_CHAR_LEN(my_table.buffer[it->offset + t_pos])) {
                my_table.batch_buffer += my_table.buffer.substr(it->offset + t_pos, UTF8_CHAR_LEN(my_table.buffer[it->offset + t_pos]));
                if(!my_table.buffer.substr(it->offset, 2).compare("\r\n"))
                    copy_line_len++;
            }

        }while(it++ != my_table.batch_end.first);

        my_table.batch_start.first = my_table.it;
        my_table.batch_start.second = my_table.Pos;
        my_table.batch_end.first = my_table.it;
        my_table.batch_end.second = my_table.Pos;
        shift_was_pressed = false;

    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Paste() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    batch_delete(false);
    shift_control();

    if(!my_table.batch_buffer.empty()) {

        auto it_start = my_table.it;

        while(it_start != my_table.piece_map.end()){
            if(!my_table.buffer.substr(it_start->offset, 2).compare("\r\n"))
                break;
            it_start++;
        }

        int start_line = find_line(it_start->offset) + 1;

        max_line += copy_line_len;

        memmove(line_map + start_line + copy_line_len, line_map + start_line, sizeof(int) * max_line);

        int line_count = start_line;


        for(size_t pos = 0; pos < my_table.batch_buffer.size(); pos += UTF8_CHAR_LEN(my_table.batch_buffer[pos])) {
            my_table.insert_text(my_table.batch_buffer.substr(pos, UTF8_CHAR_LEN(my_table.batch_buffer[pos])));
            if(!my_table.buffer.substr(my_table.it->offset, 2).compare("\r\n"))
                 line_map[line_count++] = my_table.it->offset;

        }


        for(int i = 0; i < 10; i++)
            std::cout << "line " << i << " offset " << line_map[i] << "\n";

        std::cout << "Max Line " << max_line << "\n";

        //update_line_numbers();
        find_page();

    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::Tab() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    batch_delete(false);
    shift_control();

    for(int i = 0; i < tab_width - (cr1.x - line_offset) % tab_width; i++)
        my_table.insert_text(" ");

    __cursor to_find = find_cursor();

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::Return() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    batch_delete(false);
    shift_control();

    my_table.insert_text("\r\n");

    insert_line(1, (int) my_table.it->offset);

    find_cursor();

    for(int i = 0; i < count_spaces; i++)
        my_table.insert_text(" ");

    __cursor to_find = find_cursor();

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

    if(cr1.y > t_height - _cur_height) {
        page_downwards();
        cr1.y = t_height - _cur_height;
    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::Back_Space() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    if (!batch_delete(false)) {

        shift_control();

        if(!cr1.y && !x_offset && cr1.x == line_offset)
            cr1.y = page_upwards();

        if(my_table.it != my_table.piece_map.begin())
            if(!my_table.buffer.substr(my_table.it->offset, 2).compare("\r\n"))
                remove_line(1,0);

        if(my_table.delete_text(false)) {

            __cursor to_find = find_cursor();
            find_x(&to_find);
            cr1.x = to_find.x;
            cr1.y = to_find.y;
        }
    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Left() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

    bool is_continue = false;

    if(my_table.it == my_table.batch_start.first && my_table.Pos == my_table.batch_start.second)
        is_continue = true;

    if(!cr1.y && !x_offset && cr1.x == line_offset)
        cr1.y = page_upwards();

    if(my_table.left()){

        if(shift_pressed) {
            if(is_continue)
                my_table.batch_start = {my_table.it, my_table.Pos};
            else
                my_table.batch_end = {my_table.it, my_table.Pos};
        }

        __cursor to_find = find_cursor();

        find_x(&to_find);

        cr1.x = to_find.x;
        cr1.y = to_find.y;

    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::Right() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

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

        __cursor to_find = find_cursor();

        find_x(&to_find);

        cr1.x = to_find.x;
        cr1.y = to_find.y;

        if(cr1.y > t_height - _cur_height) {
            page_downwards();
            cr1.y = t_height - _cur_height;
        }
    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Down() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

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

        if(find_cursor().y - old_cr.y > _cur_height) {
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

        cr1 = find_cursor();

        if(cr1.x == old_cr.x)
            break;
    }

    find_x(&cr1);

    if(cr1.y > t_height - _cur_height) {
        page_downwards();
        cr1.y = t_height - _cur_height;
    }

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::PageDown() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

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

        cr1 = find_cursor();


        if(cr1.y > t_height - _cur_height) {

            do {
                page_downwards();
            }while(find_cursor().y >= _cur_height);

            page_passed = true;
            cr1 = find_cursor();
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

            cr1 = find_cursor();

            break;
        }

        if(page_passed && cr1.x == old_cr.x && cr1.y == old_cr.y)
           break;

    }

    find_x(&cr1);

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}

void Buffer::Up() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

    bool is_continue = false;

    __cursor to_find = {line_offset, 0};


    if(!cr1.y)
        cr1.y = page_upwards();


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

        to_find = find_cursor();

        if(to_find.y < cr1.y && to_find.x <= cr1.x)
            break;
    }

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::PageUp() {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    shift_control();

    bool is_continue = false;
    bool page_passed = false;

    __cursor to_find = {cr1.x, cr1.y};


    while(my_table.left()) {

        my_table.right();
        if(my_table.it == my_table.batch_start.first && my_table.Pos == my_table.batch_start.second)
            is_continue = true;
        my_table.left();

        if(!to_find.y && to_find.x == line_offset + x_offset) {

            do {
                if(!page_upwards())
                    break;

            }while( find_cursor().y < t_height - _cur_height);


            page_passed = true;
        }

        if(shift_pressed) {
            if(is_continue)
                my_table.batch_start = {my_table.it, my_table.Pos};
            else
                my_table.batch_end = {my_table.it, my_table.Pos};
        }

        to_find = find_cursor();


        if(page_passed && to_find.y == cr1.y && to_find.x <= cr1.x)
            break;
    }

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::Text_Input(std::string t1) {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    batch_delete(false);
    shift_control();

    size_t _count = 0;

    while(_count < t1.size()) {

        my_table.insert_text(t1.substr(_count, UTF8_CHAR_LEN(t1[_count])));
        _count += UTF8_CHAR_LEN(t1[_count]);

    }

    __cursor to_find = find_cursor();

    find_x(&to_find);

    cr1.x = to_find.x;
    cr1.y = to_find.y;

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;

}

void Buffer::On_Resize(int w, int h) {

    _width = w;
    _height = h;

    SDL_FreeSurface(screen);

    screen = SDL_CreateRGBSurface(0, _width , _height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));

}

void Buffer::Resize_Buffer(int w, int h) {

    if(!blink_on){
        SDL_Rect temp_rect = {dest_rect.x, dest_rect.y, _cur_width, _cur_height};
        SDL_BlitSurface(Temp_Surface, NULL, screen, &temp_rect);
    }

    _width = w;
    _height = h;
    t_height = (_height / _cur_height) * _cur_height;

    SDL_FreeSurface(screen);

    screen = SDL_CreateRGBSurface(0, _width , _height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));

    find_page();

    dest_rect.x = cr1.x; dest_rect.y = cr1.y;
    blink_on = true;
}
