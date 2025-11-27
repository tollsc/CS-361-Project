#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <thread>
#include <chrono>
// #include <mutex> NOTE: Enable for timer

// TODO: Get other screens working!

void backspace_pressed_on_extra(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col, int& row, int& extras, int& prompt_length) {
    prompt.erase(prompt.begin() + (index - 1));
    std::cout << "\b ";
    std::cout << "\033[90m" << prompt.substr(index);
    // TODO debug: I believe problem lies in prompt_length only working for first row.
    std::cout << "\033[" << row + 1 << ";" << prompt_length + extras - 1 << "H";
    std::cout << "\033[K";
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
            backspace_pressed_on_extra(typed, ch, prompt, index, col, row, extras, prompt_length);
        } else { // Deletes normal char
            std::cout << "\033[90m" << prompt.substr(index);
        }
    }

    index--;
    col--;
    std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H";
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

int homeView(std::string prompt, char& first_ch) {
    std::cout << "\033[2J\033[H\n";
    std::cout << "Welcome to " << "\033[38;5;202m" << "chartype!\n\n\n";
    std::cout << "\033[0m" << "Press any key to start typing the prompt below:\n";
    std::cout << "(Or press [q] to open the menu)\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[10H\033[0m";
    char home_ch;
    while(true) {
        if (_kbhit()) {
            home_ch = _getch();
            first_ch = home_ch;
            if (home_ch == 'q') return 3;
            else return 1;
        }
    }
}

int testView(std::string prompt, int prompt_length, char& first_ch) {
    std::cout << "\033[2J\033[H\n\n\n\n\n\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[" << 16 << "H" << "\033[90m";
    std::cout << "Press [tab] for next test\n"
                 "Press [enter] to retry test\n"
                 "Press [/] to hide tip\n";
    std::cout << "\033[10H\033[0m";

    std::vector<char> typed;
    char ch;
    int index = 0;
    int col = 0;
    int row = 9;
    int extras = 0;

    std::cout << "\033[" << row + 1 << "H";

    while (true) {  // TODO: separate this while loop into two+ different functions
        if (first_ch != '\0') {
                ch = first_ch;
                if (ch == 27) {
                    return 2; // ESC to quit // TODO adjust
                }
                else if (ch == '\b') { // backspace
                    backspace_pressed(typed, ch, prompt, index, col, row, extras, prompt_length);
                } else { // normal key
                    key_pressed(typed, ch, prompt, index, col, row, extras); // TODO check if all lines up
                }
                first_ch = '\0';
        } else {
            if (_kbhit()) {
                ch = _getch();
                if (ch == 27) {
                    return 2; // ESC to quit // TODO adjust
                }
                else if (ch == '\b') { // backspace
                    backspace_pressed(typed, ch, prompt, index, col, row, extras, prompt_length);
                } else { // normal key
                    key_pressed(typed, ch, prompt, index, col, row, extras); // TODO check if all lines up
                }
            }
        }
    }
}

int resultsView() {
    std::cout << "\033[2J\033[H\n";
    std::cout << "\n\n\n\n\n\n\n\n";
    std::cout << "\033[0m" << "wpm: " << "\033[38;5;202m" << "80\n";
    std::cout << "\033[0m" << "acc: " << "\033[38;5;202m" << "95%\n";
    std::cout << "\033[0m" << "test time: " << "\033[38;5;202m" << "30s\n";
    std::cout << "\033[0m" << "difficulty: " << "\033[38;5;202m" << "classic\n\n\n";
    std::cout << "\033[0m" << "Press [tab] for next test\n"
                              "Press [q] to retry test\n\n";
    char results_ch;
    while(true) {
        if (_kbhit()) {
            results_ch = _getch();
        }
        if (results_ch == '\t') return 0;
        if (results_ch == 'q') return 1;
    }
}

int menuView() {
    std::cout << "\033[2J\033[H\n\n\n\n\n";
    std::cout << "Press [1] to close the menu\n\n";
    std::cout << "more\n";
    std::cout << "[w] about\n\n";
    std::cout << "settings\n";
    std::cout << "Press [key] for setting --> type new value --> [enter] to apply changes\n";
    std::cout << "[3] time -- " << "30s : " << "\033[38;5;202m" << "15" << "\033[0m\n";
    std::cout << "[4] difficulty -- " << "c : " << "\033[38;5;202m" << "e" << "\033[0m\n";
    std::cout << "\t(c: classic experience. h: hard, fails test if you enter an\n"
                 "\tincorrect word. e: expert, fails test if you press an incorrect key)\n\n";
    std::cout << "danger zone\n";
    std::cout << "[5] reset settings\n";
    // TODO: Implement reset settings screen/message
    char menu_ch;
    while(true) {
        if (_kbhit()) {
            menu_ch = _getch();
        }
        if (menu_ch == '1') return 0;
        if (menu_ch == 'w') return 4;
    }
}

int aboutView() {
    std::cout << "\033[2J\033[H\n\n\n\n\n";
    std::cout << "Press [1] to close the menu\n\n\n";
    std::cout << "Press [2] to go back\n\n";
    std::cout << "\033[38;5;202mChartype\033[0m"
                 " is a CLI typing test inspired by Monkeytype. It provides a\n"
                 "minimalist typing experience with simplicity and flow. "
                 "\033[38;5;202mChartype\033[0m offers\n"
                 "real-time feedback per key pressed, results after each test, and\n"
                 "customization options for user preference.\n";
    char about_ch;
    while(true) {
        if (_kbhit()) {
            about_ch = _getch();
        }
        if (about_ch == '1') return 0;
        if (about_ch == '2') return 3;
    }
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
    int menu = 0;
    char first_ch = '\0';

    while (true) {
        if (menu == 0) menu = homeView(og_prompt, first_ch);
        if (menu == 1) menu = testView(prompt, prompt_length, first_ch);
        if (menu == 2) menu = resultsView();
        if (menu == 3) menu = menuView();
        if (menu == 4) menu = aboutView();
    }
}