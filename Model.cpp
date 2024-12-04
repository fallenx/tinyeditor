#include "Model.hpp"


Model::Model() : it(piece_map.begin()), Pos(0) {}

inline size_t Model::next() { return buffer[it->offset + Pos] == 0x0d ? 2 : (( 0xE5000000 >> (( buffer[it->offset + Pos] >> 3 ) & 0x1e )) & 3 ) + 1; }
inline size_t Model::prev() { while(((buffer[it->offset + --Pos] & 0x80) != 0) && ((buffer[it->offset + Pos] & 0xC0) != 0xC0)); return Pos; }

void Model::redo() {

    if(redo_list.size()) {

        Redo_Piece temp = redo_list.back();
        redo_list.pop_back();

        switch(temp.type) {

            case INSERT:

                if(temp.Pos)
                    it = deleted_list[temp.offset];
                else
                    it = piece_map.begin();

                Pos = temp.Pos;
                reinsert(temp.piece);

                break;
        }
    }
}

void Model::undo() {

    if(undo_list.size()) {

        Undo_Piece temp = undo_list.back();
        undo_list.pop_back();

        switch(temp.type) {

            case INSERT:

                it = piece_map.erase(deleted_list[temp.piece.offset]);

                if(it != piece_map.begin()) {

                    std::pair<size_t,size_t> old_ones (it->offset, it->length);
                    if((--it)->offset + it->length == old_ones.first) {
                        Pos = it->length;
                        it->length += old_ones.second;
                        it = piece_map.erase(++it);
                        --it;
                    }else
                        Pos = it->length;

                }else
                    Pos = 0;

                redo_list.push_back({INSERT, Pos, it->offset, temp.piece});
                deleted_list[it->offset] = it;

                break;

            case DELETE:

                if(temp.piece.length){

                    it = piece_map.erase(deleted_list[temp.piece.offset]);
                }else
                    it = deleted_list[temp.piece.offset];

                temp = undo_list.back();
                undo_list.pop_back();

                it->offset -= temp.piece.length;
                it->length += temp.piece.length;
                Pos = temp.piece.length;

                deleted_list[it->offset] = it;

                std::cout << "Delete " << it->offset << " " << Pos << "\n";

                break;
        }


    }
}

bool Model::left() {

    if(Pos) {
        if(!prev()) {
            if(it != piece_map.begin())
                Pos = (--it)->length;
        }
        return true;
    }
    return false;

}

bool Model::right() {

    if(piece_map.size()) {
        if(Pos < it->length) {
            Pos += next();
            return true;
        }else if(++it != piece_map.end()){
            Pos = 0;
            Pos = next();
            return true;
        }else {
            --it;
            Pos = it->length;
            return false;
        }
    }else
        return false;
}

bool Model::delete_text() {

    if(Pos) {

        redo_list.resize(0);
        undo_list.push_back({DELETE, {it->offset, Pos}});

        if(Pos < it->length) {
            std::pair<size_t, size_t> old_piece(it->offset, Pos);
            prev();

            it->offset += old_piece.second; it->length -= old_piece.second;

            if(Pos) {
                it = piece_map.insert(it, {old_piece.first, Pos});
                Pos = it->length;
            }else {
                if(it != piece_map.begin())
                    Pos = (--it)->length;
            }

            undo_list.push_back({DELETE, {it->offset, Pos}});
            deleted_list[it->offset] = it;

        }else if(!prev()) {

            it = piece_map.erase(it);

            if(it != piece_map.begin()) {

                std::pair<size_t,size_t> old_ones (it->offset, it->length);
                if((--it)->offset + it->length == old_ones.first) {
                    Pos = it->length;
                    it->length += old_ones.second;
                    it = piece_map.erase(++it);
                    --it;
                }else
                    Pos = it->length;

            }else
                Pos = 0;

            undo_list.push_back({DELETE, {it->offset, Pos}});
            deleted_list[it->offset] = it;

        }else
            it->length = Pos;

        return true;
    }

    return false;

}

void Model::insert_text(std::string text) {

    redo_list.resize(0);

    size_t old_size = buffer.size();
    buffer += text;

    if(Pos && it->offset + Pos  == old_size) { // Consecutive Modification
        it->length += text.size();
        undo_list.back().piece.length = it->length;
    }else {
        if(Pos && Pos < it->length) { /// Middle Insertion

            std::pair<size_t, size_t> old_piece(it->offset, it->length);
            it->length = Pos;   /// Split-Left Modification
            it = piece_map.insert(++it, {old_piece.first + Pos, old_piece.second - Pos}); /// Split-Right Insertion
            deleted_list[it->offset] = it;

        }else if (Pos)
            ++it;

        it = piece_map.insert(it, {old_size, text.size()});  // New Text Insertion
        undo_list.push_back({INSERT, {it->offset, it->length}});
        deleted_list[it->offset] = it;

    }

    Pos = it->length;
}

void Model::reinsert(Piece piece) {

    if(Pos && Pos < it->length) { /// Middle Insertion

        std::pair<size_t, size_t> old_piece(it->offset, it->length);
        it->length = Pos;   /// Split-Left Modification
        it = piece_map.insert(++it, {old_piece.first + Pos, old_piece.second - Pos}); /// Split-Right Insertion
        deleted_list[it->offset] = it;

    }else if (Pos)
        ++it;

    it = piece_map.insert(it, {piece.offset, piece.length});  // New Text Insertion
    undo_list.push_back({INSERT, {it->offset, it->length}});
    deleted_list[it->offset] = it;

    Pos = it->length;
}


void Model::printbuffer() {

    for(auto i : piece_map) {
        std::cout << i.offset << " " << i.length << "\n";
        std::cout << buffer.substr(i.offset, i.length) << "\n";
    }

}

void Model::print_at() {

    if(piece_map.size())
        std::cout << "[" << buffer.substr(it->offset, it->length) << "] Pos: " << Pos << "\n";
}





