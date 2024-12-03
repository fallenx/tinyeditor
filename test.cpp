#include <ctime>
#include <cstdlib>
#include "Model.hpp"

inline int UTF8_CHAR_LEN(char byte) {return byte == 0x0d ? 2 : (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1; }

int main(int argc, char *argv[]) {

    std::string my_string = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    my_string += "ÇĞİÖŞÜçğıöşü"; /// Turkish

    std::map<size_t, std::string> my_map;

    size_t mn_cnt = 0;

    for(size_t _cnt = 0; _cnt < my_string.size(); _cnt += UTF8_CHAR_LEN(my_string[_cnt]))
        my_map[mn_cnt++] = my_string.substr(_cnt, UTF8_CHAR_LEN(my_string[_cnt]));

    //for(auto i : my_map)
    //    std::cout << i.second << " ";


    enum {INSERT, LEFT, RIGHT, UNDO, REDO};

    Model my_model;

    srand(time(NULL));

    for(int test_count = 0; test_count < 1000000; test_count++) {

        if(!(test_count % 1000))
            std::cout << "Table Size " << my_model.piece_map.size() << "\n";

        switch(rand() % 5) {

            case INSERT:
                {

                    //std::cout << "Insert\n";

                    int _length = rand() % 10 + 1;
                    std::string to_insert;

                    for(int i = 0; i < _length; i++)
                        to_insert += my_map[rand() % my_map.size()];

                    my_model.insert_text(to_insert);
                }

                break;

            case LEFT:
                //std::cout << "Left\n";
                for(int i= 0; i < rand() % 20 + 1;i++)
                    my_model.left();
                break;

            case RIGHT:
                //std::cout << "Right\n";
                for(int i= 0; i < rand() % 30 + 1; i++)
                    my_model.right();
                break;

            case UNDO:
                //std::cout << "[Undo] ";
                //my_model.print_at();
                for(int i= 0; i < rand() % 5 + 1; i++)
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


    //my_model.printbuffer();



    return 0;
}

