#include <ctime>
#include <cstdlib>
#include "Model.hpp"

inline int UTF8_CHAR_LEN(char byte) {return byte == 0x0d ? 2 : (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1; }

int main(int argc, char *argv[]) {

    std::string my_string = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    my_string += "ÇĞİÖŞÜçğıöşü"; /// Turkish

    std::map<size_t, std::string> my_map;

    size_t _cnt = 0;

    while(_cnt < my_string.size()) {
            my_map[_cnt] = my_string.substr(_cnt, UTF8_CHAR_LEN(my_string[_cnt]));
            _cnt += UTF8_CHAR_LEN(my_string[_cnt]);
    }

    enum {INSERT, LEFT, RIGHT, UNDO, REDO};

    Model my_model;

    srand(time(NULL));

    for(int test_count = 0; test_count < 10000000; test_count++) {

        if(!(test_count % 10000))
            std::cout << "Table Size " << my_model.piece_map.size() << "\n";

        switch(rand() % 5) {

            case INSERT:
                {

                    //std::cout << "Insert\n";

                    int _length = rand() % 6 + 1;
                    std::string to_insert;

                    for(int i = 0; i < _length; i++) {

                        size_t _rnd = rand() % my_string.size() + 1;

                        //while(((my_string[--_rnd] & 0x80) != 0) && ((my_string[_rnd] & 0xC0) != 0xC0));

                        //auto it = my_map.lower_bound(_rnd);

                        to_insert += (my_map.lower_bound(_rnd))->second;

                    }

                    my_model.insert_text(to_insert);
                }

                break;

            case LEFT:
                //std::cout << "Left\n";
                my_model.left();
                break;

            case RIGHT:
                //std::cout << "Right\n";
                my_model.right();
                break;

            case UNDO:
                //std::cout << "[Undo] ";
                //my_model.print_at();
                my_model.undo();
                break;

            case REDO:
                //std::cout << "Redo\n";
                my_model.redo();
                break;

        }

    }

    std::cout << "Table Size " << my_model.piece_map.size() << "\n";
    std::cout << "Buffer Size " << my_model.buffer.size() << "\n";


    //my_model.printbuffer();



    return 0;
}

