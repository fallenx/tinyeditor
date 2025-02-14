#ifndef BUFFER_HPP
#define BUFFER_HPP
#include "Model.hpp"

typedef struct {
    int s_w;
    int s_h;
    int font_ascent;
    int _width;
    int _height;
    int x_offset;
    int y_offset;
    int _cur_height;
    int t_height;
    int line_offset;
}dimensions;

class Buffer {

    int s_w, s_h, font_ascent, _width, _height, x_offset, y_offset, _cur_height, t_height, line_offset, count_spaces, line_map_size;
    int _cur_width;
    int current_head_line;
    bool ctrl_pressed, shift_pressed, shift_was_pressed;
    SDL_Surface *screen;
    SDL_Surface *font_atlas;
    SDL_Surface *font_atlas_reverse;
    SDL_Surface *Cursor;
    SDL_Surface *Temp_Surface;
    bool blink_on;
    SDL_Rect dest_rect;
    std::unordered_map<std::string, __cursor> font_map;
    int *line_map;

    char BufferName[256];
    Model my_table;

    __cursor cr1;

    __cursor find_cursor();
    void find_page();
    void find_page_head();
    int page_upwards();
    void page_downwards();
    void find_x(__cursor *);
    bool batch_delete(bool);
    void prepare_font_atlas();
    void update_line_numbers();
    void shift_control();
    void insert_line(int, int);
    void delete_line(int, int);
    int find_head_line(int);

public:

    Buffer(const char*,int, int);
    ~Buffer() { SDL_FreeSurface(screen); SDL_FreeSurface(font_atlas); SDL_FreeSurface(Cursor); SDL_FreeSurface(Temp_Surface); SDL_FreeSurface(font_atlas_reverse); free(line_map); }
    void Render();
    void Cursor_Blink();
    void Undo();
    void Redo();
    void Tab();
    void Return();
    void Back_Space();
    void Left();
    void Right();
    void Up();
    void Down();
    void PageUp();
    void PageDown();
    void Cut();
    void Paste();
    void Copy();
    void Text_Input(std::string);
    void Resize_Buffer(int, int);
    void On_Resize(int, int);

    void print_buffername() {
        std::cout << BufferName;
    }

    void set_ctrl(bool pressed) {ctrl_pressed = pressed;}
    void set_shift(bool);
    bool get_ctrl() {return ctrl_pressed;}
    bool get_shift() {return shift_pressed;}
    __cursor Get_Cursor() { return cr1;}

    dimensions Get_Dimensions() { return {s_w, s_h, font_ascent, _width, _height, x_offset, y_offset, _cur_height, t_height, line_offset}; }
    SDL_Surface* Get_Screen() { return screen;};
    bool Get_Blink() { return blink_on;}


};


#endif // BUFFER
