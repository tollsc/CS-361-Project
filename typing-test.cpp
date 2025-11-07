#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <thread>
#include <chrono>
// #include <mutex> NOTE: Enable for timer

// TODO: Implement timer later if time allows
// NOTE: Research mutex behavior for print statements (I believe only during tests):
    // mtx.lock();
    // // Print/delete behavior
    // std::cout.flush();
    // mtx.unlock();

// std::mutex mtx; NOTE: Enable for timer

// void countdown(int timer) {
//     for (int i = timer; i > 0; i--) {
//         mtx.lock();
//         std::cout << "\033[" << 9 << "H";
//         std::cout << "\033[0m" << "\r" << i << " ";
//         std::cout.flush();
//         mtx.unlock();
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     }
// }

// TODO: Get other screens working!

// TODO: See if possible to fix the typing prompt’s starting row and track only the horizontal cursor (col),
// using substr redraws for fast backspace updates. Might fix delay but idk might be bs


void backspace_pressed_on_extra(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& row, int& extras, int& prompt_length) {
    prompt.erase(prompt.begin() + prompt.find('\0'));
    std::cout << "\b ";
    std::cout << "\033[90m" << prompt.substr(index);
    std::cout << "\033[" << row + 1 << ";" << prompt_length + extras - 1 << "H";
    std::cout << "\033[K";
    std::cout.flush();
    extras--;
}

void backspace_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col, int& row, int& extras, int& prompt_length) {
    if (index < 1) return; // First char typed is backspace
    if (typed[index - 1] == ' ') return; // Tries to backspace a space

    typed.pop_back(); // Pop back latest ch

    if (index < 2) { // Deletes first char
        std::cout << "\b \b";
    }
    else { // Deletes any char after
        if (prompt[index - 1] == '\0') { // Deletes an extra char
            backspace_pressed_on_extra(typed, ch, prompt, index, row, extras, prompt_length);
        } else { // Deletes normal char
            std::cout << "\033[90m" << prompt.substr(index);
        }
    }

    index--;
    col--;
    std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H";
    std::cout.flush();
}

void space_key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col) {
    while(prompt[index] != ' ' && prompt[index] != '\n') {
        typed.push_back(ch);
        std::cout << prompt[index];
        index++;
        col++;
    }
}

void extra_key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col, int& row, int& extras) {
    typed.push_back(ch);
    std::cout << "\033[38;5;88m" << typed[index];
    prompt.insert(prompt.begin() + index, '\0');
    std::cout << "\033[90m" << prompt.substr(index);
    index++;
    col++;
    extras++;
    std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H";
    return;
}

void key_pressed_end_line(int& index, int& col, int& row) {
    std::cout << std::endl;
    index++;
    col = 0;
    row++;
    return;
}

void key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col, int& row, int& extras) {
    std::cout << "\033[0m"; // Default text

    // TODO: consider return values/if they need to cotinue (keyword) while loop
    if ((ch == ' ' && index < 1) || (ch == ' ' && typed[index - 1] == ' ')) return; // Space key pressed when it shouldn't have been
    if (ch == ' ') { // Space key pressed
        space_key_pressed(typed, ch, prompt, index, col);
        // return; // TODO: Figure out why this works only without return if have time
    }
    if (ch != ' ' && (prompt[index] == ' ' || prompt[index] == '\n')) { // Extra key pressed (should've spaced)
        extra_key_pressed(typed, ch, prompt, index, col, row, extras);
        return;
    }

    typed.push_back(ch); // Push ch to typed

    if (typed[index] == ' ' && prompt[index] == '\n') { // Space key pressed end of line
        key_pressed_end_line(index, col, row);
        return;
    } else if (typed[index] == prompt[index]) std::cout << "\u001b[0m" << prompt[index]; // Correct key pressed
    else std::cout << "\u001b[31m" << prompt[index]; // Incorrect key pressed

    index++;
    col++;
}

void homeView(std::string prompt) {
    std::cout << "\033[2J\033[H\n";
    std::cout << "Welcome to " << "\033[38;5;202m" << "chartype!\n\n\n";
    std::cout << "\033[0m" << "Press any key to start typing the prompt below:\n";
    std::cout << "(Or press [1] to open the menu)\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[10H\033[0m";
}

void testView(std::string prompt, int timer, int prompt_length) {
    std::cout << "\033[2J\033[H\n\n\n\n\n\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[" << 16 << "H" << "\033[90m";
    std::cout << "Press [tab] for next test\n"
                 "Press [enter] to retry test\n"
                 "Press [/] to hide tip\n";
    // std::thread t(countdown, timer); NOTE: timer
    std::cout << "\033[10H\033[0m";

    std::vector<char> typed;
    char ch;
    int index = 0;
    int col = 0;
    int row = 9;
    int extras = 0;

    std::cout << "\033[" << row + 1 << "H";

    while (true) {
        if (_kbhit()) {
            ch = _getch();
            if (ch == 27) break; // ESC to quit // TODO adjust
            else if (ch == '\b') { // backspace
                backspace_pressed(typed, ch, prompt, index, col, row, extras, prompt_length);
            } else { // normal key
                key_pressed(typed, ch, prompt, index, col, row, extras); // TODO check if all lines up
            }
        }
    }

    // t.join(); // NOTE: timer
}

int main() {
    //std::cout << "\033[2J\033[H";
    // TODO: function to get random prompt
    std::string prompt = "the quick brown fox jumps over the lazy dog\n"
                         "the quick brown fox jumps over the lazy dog\n"
                         "the quick brown fox jumps over the lazy dog\n";
    std::string og_prompt = prompt;
    std::string promptl1 = "the quick brown fox jumps over the lazy dog\n";
    int prompt_length = promptl1.length();
    int timer = 15;
    homeView(prompt);
    std::string test;
    std::cin >> test;
    testView(prompt, timer, prompt_length);
}