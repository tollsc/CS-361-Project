#include <iostream>
#include <vector>
#include <conio.h>
#include <string>

void homeView(std::string prompt) {
    std::cout << "\033[2J\033[H\n";
    std::cout << "Welcome to " << "\033[38;5;202m" << "chartype!\n\n\n";
    std::cout << "\033[0m" << "Press any key to start typing the prompt below:\n";
    std::cout << "(Or press [1] to open the menu)\n\n\n\n";
    std::cout << "\033[38;5;244m" << prompt;
    std::cout << "\033[10H\033[0m";
    std::string test;
    std::cin >> test;
}

int main() {
    //std::cout << "\033[2J\033[H";
    // TODO: function to get random prompt
    std::string prompt = "the quick brown fox jumps over the lazy dog\n"
                         "the quick brown fox jumps over the lazy dog\n"
                         "the quick brown fox jumps over the lazy dog\n";
    homeView(prompt);
}