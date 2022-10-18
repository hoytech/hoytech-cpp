#include <iostream>

#include "hoytech/file_change_monitor.h"

int main() {
    hoytech::file_change_monitor mon("./file.txt");

    mon.setDebounce(50);

    mon.run([&](){
        std::cout << "CHANGED" << std::endl;
    });

    while(1) {
        ::sleep(1000);
    }
}
