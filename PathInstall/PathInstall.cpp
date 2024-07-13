#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <Windows.h>
#include "./json.hpp"
static std::filesystem::path PathInstall_Home = "C:\\PathInstall";

std::vector<std::string> argParser(int argc, char* argv[]) {
    std::vector<std::string> returnString;
    for (int i = 0; i < argc; ++i) {
        returnString.push_back(argv[i]);

    }
    return returnString;
}
void doPrerequisites() {
    std::cout << "Checking/doing prerequisites" << std::endl;
    if (!std::filesystem::is_directory(PathInstall_Home)) {
        std::filesystem::create_directory(PathInstall_Home);
    }
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key. Error code: " << result << std::endl;
        return;
    }
    DWORD bufferSize = 32000;
    auto buffer = std::make_unique<char[]>(bufferSize);
    if (RegQueryValueExA(hKey, "Path", NULL, NULL, reinterpret_cast<LPBYTE>(buffer.get()), &bufferSize) != ERROR_SUCCESS) {
        std::cerr << "Failed to read PATH environment variable from registry." << std::endl;
        RegCloseKey(hKey);
        return;
    }
    std::string path(buffer.get());
    if (!strstr(path.c_str(), PathInstall_Home.string().c_str())) {
        path += ";";
        path += PathInstall_Home.string();
        if (RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, reinterpret_cast<const BYTE*>(path.c_str()), path.size() + 1) != ERROR_SUCCESS) {
            std::cerr << "Failed to set PATH environment variable in the registry." << std::endl;
            RegCloseKey(hKey);
            return;
        }
        
        
    }
    RegCloseKey(hKey);
    DWORD_PTR result2;
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>("Environment"), SMTO_ABORTIFHUNG, 5000, &result2);
    
    return;
}

int main(int argc, char* argv[]) {
    auto args = argParser(argc, argv);
    doPrerequisites();
    
    if (args.size() < 2) {
        std::cout << "Usage: PathInstall.exe install/remove <file>\nor PathInstall.exe list" << std::endl;
    }
    if (args[1] == "install") {
        if (args.size() < 3) {
            std::cout << "Usage: PathInstall.exe install/remove <file>\nor PathInstall.exe list" << std::endl;
        }
        std::filesystem::path originalFile = std::filesystem::absolute(args[2]);
        std::filesystem::path copyPath = std::filesystem::absolute(PathInstall_Home.string().c_str() + args[2]);
        
        if (!std::filesystem::exists(originalFile)) {
            std::cout << "File to install does not exist at given Path" << std::endl;
            return -1;
        }
        if (!std::filesystem::exists(copyPath)) {
            std::cout << "File to install does not exist at given Path" << std::endl;
            return -1;
        }
        std::cout << "Copying to :" << copyPath.string() << " from: " << originalFile.string() << std::endl;
        std::filesystem::copy_file(originalFile, copyPath);
    }
    else if (args[1] == "remove") {
        if (args.size() < 3) {
            std::cout << "Usage: PathInstall.exe install/remove <file>\nor PathInstall.exe list" << std::endl;
        }
        std::filesystem::path originalFile = std::filesystem::absolute(args[2]);
        for (auto& dir_entry : std::filesystem::directory_iterator(PathInstall_Home)) {
            if (dir_entry.is_regular_file() && dir_entry.path().filename() == originalFile.filename()) {
                std::filesystem::remove(originalFile);
            }
        }
    }
    else if (args[1] == "list") {
        if (args.size() > 2) {
            std::cout << "Usage: PathInstall.exe install/remove <file>\nor PathInstall.exe list" << std::endl;
        }
        for (auto& dir_entry : std::filesystem::directory_iterator(PathInstall_Home)) {
            std::cout << dir_entry.path().filename() << std::endl;
        }
    }
    else {
        std::cout << "Usage: PathInstall.exe install/remove <file>\nor PathInstall.exe list" << std::endl;
    }
}

