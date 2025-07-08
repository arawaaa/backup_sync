#include "directorystate.h"

DirectoryState::DirectoryState(bool copySymlink, string base, string foreign)
    : base_path(base), external_device(foreign), copy_symlinks(copySymlink)
{
    if (!exists(base_path) || !exists(external_device))
        throw std::runtime_error("Invalid Paths!");

    if (!is_directory(base_path) || !is_directory(external_device))
        throw std::runtime_error("Enter directories, not files!");

    if (!filesystem::path(base_path).is_absolute() || !filesystem::path(foreign).is_absolute())
        throw std::runtime_error("Absolute paths required");
}

bool DirectoryState::startCopy()
{
    stack<pair<filesystem::directory_entry, filesystem::path>> dfs;
    dfs.emplace(base_path, "");
    seen.emplace(base_path);

    while (!dfs.empty()) {
        auto dir = dfs.top();
        dfs.pop();

        if (copy_symlinks && dir.first.is_symlink()) {
            error_code ec;
            filesystem::path points_to = read_symlink(dir.first.path(), ec);
            if (ec.value())
                std::cerr << "Filesystem Error: " << ec.message() << std::endl;
            else {
                filesystem::path go_up;
                string within = dir.second.string();
                for_each(within.begin(), within.end(), [&go_up](char &a) {
                    if (a == '/' && go_up.empty()) {
                        go_up = "..";
                    } else if (a == '/')
                        go_up /= "..";
                });
                if (points_to.is_absolute() && points_to.string().starts_with(base_path.string())) {
                    string cleaved_path = points_to.string().substr(base_path.string().length() + 1, points_to.string().length() - (base_path.string().length() + 1));
                    std::error_code ec;
                    filesystem::remove(external_device / dir.second, ec);

                    filesystem::create_symlink(go_up.empty() ? filesystem::path(cleaved_path) : go_up / filesystem::path(cleaved_path),
                        external_device / dir.second);
                } else if (points_to.is_relative() && filesystem::exists(dir.first.path().parent_path() / points_to) &&
                           filesystem::canonical(dir.first.path().parent_path() / points_to).string().starts_with(base_path.string())) {
                    string canonical_path = filesystem::canonical(dir.first.path().parent_path() / points_to).string();
                    string cleaved_path = canonical_path.substr(base_path.string().length() + 1, canonical_path.length() - (base_path.string().length() + 1));
                    std::error_code ec;
                    filesystem::remove(external_device / dir.second, ec);

                    filesystem::create_symlink(go_up.empty() ? filesystem::path(cleaved_path) : go_up / filesystem::path(cleaved_path),
                        external_device / dir.second);
                }
            }
        } else if (dir.first.is_regular_file()) {
            try {
                filesystem::file_status replace_status = filesystem::status(base_path / dir.second);
                std::error_code ec;
                filesystem::permissions(external_device / dir.second, filesystem::perms::all, ec);
                copy(base_path / dir.second, external_device / dir.second, filesystem::copy_options::update_existing);
                filesystem::permissions(external_device / dir.second, replace_status.permissions());
            } catch (std::filesystem::filesystem_error e) {
                std::cerr << "Unable to copy " << e.what() << std::endl;
            }
        } else if (dir.first.is_directory() && !dir.first.is_symlink()) {
            if (dir.second != "" && !exists(external_device / dir.second))
                filesystem::create_directory(external_device / dir.second);

            for (const filesystem::directory_entry& entry : filesystem::directory_iterator(dir.first)) {
                if (!seen.contains(entry)) {
                    dfs.emplace(entry, dir.second / entry.path().filename());
                    seen.emplace(entry);
                }
            }
        }
    }
    return true;
}

bool DirectoryState::declutter()
{
    stack<pair<filesystem::directory_entry, filesystem::path>> dfs;
    dfs.emplace(external_device, "");

    while (!dfs.empty()) {
        auto dirent = dfs.top();
        dfs.pop();

        if (dirent.first.is_directory() && !dirent.first.is_symlink()) {
            for (const filesystem::directory_entry& entry : filesystem::directory_iterator(dirent.first)) {
                if (!exists(base_path / dirent.second / entry.path().filename())) {
                    filesystem::remove_all(entry);
                    continue;
                }

                if (!seen.contains(entry)) {
                    dfs.emplace(entry, dirent.second / entry.path().filename());
                    seen.emplace(entry);
                }
            }
        }
    }
    seen.clear();
    return true;
}

