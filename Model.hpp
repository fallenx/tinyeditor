#ifndef MODEL_HPP
#define MODEL_HPP

#include "Helper.hpp"


class Model {

    public:
    enum {INSERT, DELETE, BATCH_INSERT, BATCH_DELETE};
    struct Piece {size_t offset; size_t length;};
    struct Undo_Piece {size_t type; Piece piece;};
    struct Redo_Piece {size_t type; size_t Pos; size_t offset; Piece piece;};
    std::map<size_t, std::list<Piece>::iterator> deleted_list;
    std::unordered_map<size_t, size_t> line_map;
    std::list<Piece> piece_map;
    std::vector<Redo_Piece> redo_list;
    std::vector<Undo_Piece> undo_list;
    std::list<Piece>::iterator it;
    std::list<Piece>::iterator head;
    std::pair<std::list<Piece>::iterator, size_t> batch_start;
    std::pair<std::list<Piece>::iterator, size_t> batch_end;
    std::string buffer;
    std::string batch_buffer;
    size_t Pos;

    Model();
    inline size_t next();
    inline size_t prev();

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
