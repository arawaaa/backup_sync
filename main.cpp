#include <iostream>
#include <filesystem>

#include <string.h>

#include "directorystate.h"
// DFS recursion on directory tree
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "binary_name_required") << " [-d -t num] remote local" << std::endl;
        return 2;
    }
    bool enable_declutter = false;
    for (int i = 0; i < argc; i++) {
        if (!strcmp("-d", argv[i])) {
            enable_declutter = true;
            break;
        }
    }
    DirectoryState d(true, argv[argc - 1], argv[argc - 2]);
    std::cout << "\033[31mWarning: Ensure that the remote tree shares no nodes with the local tree\033[37m" << std::endl;
    std::cout << "Copying filesystem sub-tree rooted at " << argv[argc - 1] << " into " << argv[argc - 2] << std::endl;
    std::cout << "Do you want to proceed? [Y/n] ";
    char in;
    std::cin >> in;

    if (in == 'Y') {
        if (enable_declutter)
            d.declutter();
        return !d.startCopy();
    } else return 255;
}
