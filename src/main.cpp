#include "urd.hpp"

int main(int argc, char* argv[]){

    ::unlink(urd::SOCKET_FILE);

    urd().run();
}
