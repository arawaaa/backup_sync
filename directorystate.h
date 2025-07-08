#pragma once

#include <filesystem>
#include <unordered_set>
#include <stack>
#include <string>
#include <exception>
#include <utility>
#include <iostream>
#include <algorithm>

using namespace std;

class DirectoryState {
    filesystem::path base_path;
    filesystem::path external_device;
    unordered_set<filesystem::path> seen;

    bool copy_symlinks;
public:
    DirectoryState(bool copySymlink, string base, string foreign);

    bool startCopy();

    bool declutter();
};
