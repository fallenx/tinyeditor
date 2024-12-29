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


    enum {INSERT, LEFT, RIGHT, DELETE, UNDO, REDO, NEWLINE};


    srand(time(NULL));

    for(;;) {

        Model my_model;
        //std::cout << "----------Start-------------------\n";

        for(int test_count = 0; test_count < 10000000; test_count++) {


            //if(!(test_count % 1000))
            //    std::cout << "Table Size " << my_model.piece_map.size() << "\n";

            switch(rand() % 7) {

                case NEWLINE:
                    {

                        for(int i= 0; i < rand() % 10 + 1;i++) {
                            //std::cout << "NewLine\n";
                            my_model.insert_text("\r\n");
                            //my_model.print_at();
                            //my_model.printbuffer();
                        }

                    }

                    break;

                case INSERT:
                    {


                        for(int i= 0; i < rand() % 10 + 1;i++) {
                            //std::cout << "Insert\n";
                            std::string to_insert;
                            to_insert = my_map[rand() % 25 + 67];//my_map.size()];
                            my_model.insert_text(to_insert);
                            //my_model.print_at();
                            //my_model.printbuffer();
                        }

                    }

                    break;

                case LEFT:

                    for(int i= 0; i < rand() % 10 + 1;i++) {
                        //std::cout << "Left\n";
                        my_model.left();
                        //my_model.print_at();
                        //my_model.printbuffer();
                    }
                    break;

                case RIGHT:

                    for(int i= 0; i < rand() % 10 + 1; i++) {
                        //std::cout << "Right\n";
                        my_model.right();
                        //my_model.print_at();
                        //my_model.printbuffer();
                    }
                    break;

                case UNDO:
                    for(int i= 0; i < rand() % 10 + 1; i++) {
                        //std::cout << "[Undo] \n";
                        my_model.undo();
                        //my_model.print_at();
                        //my_model.printbuffer();
                    }
                    break;

                case REDO:
                    for(int i= 0; i < rand() % 10 + 1; i++) {
                        //std::cout << "Redo\n";
                        my_model.redo();
                        //my_model.print_at();
                        //my_model.printbuffer();
                    }
                    break;

                case DELETE:
                    for(int i= 0; i < rand() % 10 + 1; i++) {
                        //std::cout << "Delete\n";
                        my_model.delete_text(false);
                        //my_model.print_at();
                        //my_model.printbuffer();
                    }
                    break;

            }

        }

        std::cout << "Table Size " << my_model.piece_map.size() << "\n";
        std::cout << "Buffer Size " << my_model.buffer.size() << "\n";

    }

    //my_model.printbuffer();


    //my_model.printbuffer();



    return 0;
}

