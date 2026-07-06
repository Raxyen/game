#include "FileManager.h"

void FileManager::saveScoreToFile(Player& player) {
    out_file.open("scores.txt", std::ios_base::app);
    out_file << player.getName() << " " << player.getScore() << std::endl;
    out_file.close();
}

std::vector <std::pair<std::wstring, unsigned int>> FileManager::readScoreListFromFile() {
    in_file.open("scores.txt");
    std::vector <std::pair<std::wstring, unsigned int>> scorelist;
    std::string temp_name;
    unsigned int temp_score;
    while (in_file >> temp_name >> temp_score) {
        // convert narrow string to wide string before storing
        std::wstring wname(temp_name.begin(), temp_name.end());
        scorelist.emplace_back(make_pair(wname, temp_score));
    }
    in_file.close();
    return scorelist;
}