#pragma once

#include <fstream>

#include "Player.h"

class FileManager {
private:
    std::ifstream in_file;
    std::ofstream out_file;
public:
    void saveScoreToFile(Player& player);
    std::vector <std::pair<std::wstring, unsigned int>> readScoreListFromFile();
};