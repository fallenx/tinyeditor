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
                reinsert(temp.piece, true);

                break;

            case DELETE:

                it = deleted_list[temp.offset];
                Pos = temp.Pos;

                delete_text(true);

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

                it = deleted_list[temp.piece.offset];
                Pos = temp.piece.length;

                std::cout << "it " << it->offset << " " << Pos << "\n";

                print_at();
                printbuffer();

                delete_text(false);

/*
                if(it != piece_map.begin()) {

                    std::pair<size_t,size_t> old_ones (it->offset, it->length);
                    if(it != piece_map.end() && (--it)->offset + it->length == old_ones.first) {
                        Pos = it->length;
                        it->length += old_ones.second;
                        it = piece_map.erase(++it);
                        --it;
                    }else
                        Pos = it == piece_map.end() ? (--it)->length : it->length;

                }else
                    Pos = 0;

                */

                redo_list.push_back({INSERT, Pos, it->offset, temp.piece});

                //deleted_list[it->offset] = it;

                break;

            case DELETE:

                it = deleted_list[temp.piece.offset];
                Pos = temp.piece.length;

                reinsert(undo_list.back().piece, false);
                undo_list.pop_back();

                redo_list.push_back({DELETE, Pos, it->offset, temp.piece});

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

bool Model::delete_text(bool from_redo=false) {

    if(Pos) {

        if(!from_redo)
            redo_list.resize(0);

        if(Pos < it->length) {
            std::pair<size_t, size_t> old_piece(it->offset, Pos);
            prev();

            undo_list.push_back({DELETE, {it->offset + Pos, next()}});

            it->offset += old_piece.second; it->length -= old_piece.second;
            deleted_list[it->offset] = it;

            if(Pos) {
                it = piece_map.insert(it, {old_piece.first, Pos});
                Pos = it->length;
            }else {
                if(it != piece_map.begin())
                    Pos = (--it)->length;
            }

        }else if(!prev()) {

            undo_list.push_back({DELETE, {it->offset + Pos, next()}});

            it = piece_map.erase(it);

            if(it != piece_map.begin()) {

                std::pair<size_t, size_t> old_ones (it->offset, it->length);

                if(it != piece_map.end() && (--it)->offset + it->length == old_ones.first) {
                    Pos = it->length;
                    it->length += old_ones.second;
                    it = piece_map.erase(++it);
                    --it;
                }else
                    Pos = it == piece_map.end() ? (--it)->length : it->length;
            }

        }else{
            undo_list.push_back({DELETE, {it->offset + Pos, next()}});
            it->length = Pos;
        }

        undo_list.push_back({DELETE, {it->offset, Pos}});
        deleted_list[it->offset] = it;


        return true;
    }

    return false;

}

void Model::insert_text(std::string text) {

    redo_list.resize(0);

    size_t old_size = buffer.size();
    buffer += text;

    if(Pos && it->offset + Pos == old_size) { // Consecutive Modification
        it->length += text.size();
        undo_list.push_back({INSERT, {it->offset, it->length}});
        //deleted_list[it->offset] = it;
        //undo_list.back().piece.length = it->length;
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

    print_at();
    printbuffer();

}

void Model::reinsert(Piece piece, bool from_redo) {

    if(Pos && it->offset + Pos == piece.offset) { // Consecutive Modification
        it->length += next();
        std::pair<size_t, size_t> old_piece(it->offset, it->length);

        if(++it != piece_map.end() && it->offset == old_piece.first + old_piece.second) {
            Pos = old_piece.second;
            it->offset -= Pos;
            it->length += Pos;
            it = piece_map.erase(--it);
            deleted_list[it->offset] = it;
        }else {
            Pos = (--it)->length;
        }

    }else {

        if(Pos && Pos < it->length) { /// Middle Insertion

            std::pair<size_t, size_t> old_piece(it->offset, it->length);
            it->length = Pos;   /// Split-Left Modification
            it = piece_map.insert(++it, {old_piece.first + Pos, old_piece.second - Pos}); /// Split-Right Insertion
            deleted_list[it->offset] = it;

        }else if (Pos)
            ++it;

        if(it != piece_map.end() && piece.offset + piece.length == it->offset){
            it->length += piece.length;
            it->offset -= piece.length;
            Pos = piece.length;
        }else{
            it = piece_map.insert(it, {piece.offset, piece.length});  // New Text Insertion
            Pos = it->length;
        }

        if(from_redo)
            undo_list.push_back({INSERT, {it->offset, it->length}});

        deleted_list[it->offset] = it;

    }

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





