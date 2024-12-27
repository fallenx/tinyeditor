#ifndef MODEL_HPP
#define MODEL_HPP

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <string>

class Model {

    public:
    enum {INSERT, DELETE};
    struct Piece {size_t offset; size_t length;};
    struct Undo_Piece {size_t type; Piece piece;};
    struct Redo_Piece {size_t type; size_t Pos; size_t offset; Piece piece;};
    std::map<size_t, std::list<Piece>::iterator> deleted_list;
    std::list<Piece> piece_map;
    std::vector<Redo_Piece> redo_list;
    std::vector<Undo_Piece> undo_list;
    std::list<Piece>::iterator it;
    size_t page_head;
    std::string buffer;
    size_t Pos;

    Model();
    inline size_t next();
    inline size_t prev();
    inline int UTF8_CHAR_LEN(char);

    void undo();
    void redo();
    bool left();
    bool right();
    bool delete_text(bool);
    void insert_text(std::string);
    void reinsert(Piece, bool);
    void printbuffer();
    void print_at();
};

#endif // MODEL_HPP
