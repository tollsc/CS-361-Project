#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
// #include <mutex> NOTE: Enable for timer

using json = nlohmann::json;

// Callback to collect response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Helper for GET request
json getRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "GET request failed: " << curl_easy_strerror(res) << "\n";
        }

        curl_easy_cleanup(curl);
    }

    // std::cout << "Raw GET response: " << response << "\n";

    try {
        return json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "GET parse error: " << e.what() << "\n";
        return json{};
    }
}

// Helper for POST request
json postRequest(const std::string& url, const std::string& body) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "POST request failed: " << curl_easy_strerror(res) << "\n";
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    // std::cout << "Raw POST response: " << response << "\n";

    try {
        return json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "POST parse error: " << e.what() << "\n";
        return json{};
    }
}

void backspace_pressed_on_extra(std::vector<char>& typed, char& ch, std::string& prompt, int& index,
        int& col, int& row, int& extras) {
    prompt.erase(prompt.begin() + (index - 1));
    std::cout << "\b \b ";
    size_t newline_pos = prompt.find('\n', index);
    std::cout << "\033[90m" << prompt.substr(index, newline_pos - index);
    std::cout << "\033[K";
    index--;
    col--;
    extras--;
    std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H\033[0m";
}

void backspace_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index,
        int& col, int& row, int& extras) {
    if (index < 1) return; // First char typed is backspace
    if (typed[index - 1] == ' ') return; // Tries to backspace a space

    typed.pop_back(); // Pop back latest ch

    if (index < 2) { // Deletes first char
        std::cout << "\b \b";
        index--;
        col--;
        std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H\033[0m";
    }
    else { // Deletes any char after
        if (prompt[index - 1] == '_') { // Deletes an extra char
            backspace_pressed_on_extra(typed, ch, prompt, index, col, row, extras);
        } else { // Deletes normal char
            std::cout << "\b \b ";
            size_t newline_pos = prompt.find('\n', index);
            std::cout << "\033[90m" << prompt.substr(index, newline_pos - index);
            std::cout << "\033[K";
            index--;
            col--;
            std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H\033[0m";
        }
    }
}

void space_key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index, int& col) {
    int start = index;
    while (prompt[index] != ' ' && prompt[index] != '\n') {
        typed.push_back(ch);
        index++;
        col++;
    }
    std::cout << "\033[90m" << prompt.substr(start, index - start) << "\033[0m";
}

void extra_key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index,
        int& col, int& row, int& extras) {
    typed.push_back(ch);
    prompt.insert(prompt.begin() + index, '_');
    size_t newline_pos = prompt.find('\n', index + 1); // find end of current line
    std::cout << "\033[38;5;88m" << typed[index]       // show extra char in red
              << "\033[90m" << prompt.substr(index + 1, newline_pos - (index + 1)) // redraw remainder of line
              << "\033[K"; // clear to end of line
    index++;
    col++;
    extras++;
    std::cout << "\033[" << row + 1 << ";" << (col + 1) << "H\033[0m";
}

void key_pressed_end_line(int& index, int& col, int& row) {
    std::cout << std::endl;
    index++;
    col = 0;
    row++;
    return;
}

void key_pressed(std::vector<char>& typed, char& ch, std::string& prompt, int& index,
        int& col, int& row, int& extras, int& typed_word_count) {
    std::cout << "\033[0m"; // Default text

    // TODO: consider return values/if they need to cotinue (keyword) while loop
    if ((ch == ' ' && index < 1) || (ch == ' ' && typed[index - 1] == ' ')) return; // Space key pressed when it shouldn't have been
    if (ch == ' ') { // Space key pressed
        space_key_pressed(typed, ch, prompt, index, col);
        typed_word_count++;
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
    std::cout << "(Or press [1] to open the menu)\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[10H\033[0m";
    char home_ch = '\0';
    while(true) {
        if (_kbhit()) {
            home_ch = _getch();
            first_ch = home_ch;
            if (home_ch == '1') return 3;
            else return 1;
        }
    }
}

int testView(std::vector<char>& typed, std::string& prompt,
        char& first_ch, int prompt_word_count, int& typed_word_count, double& time_passed) {
    std::cout << "\033[2J\033[H\n\n\n\n\n\n\n\n\n";
    std::cout << "\033[90m" << prompt;
    std::cout << "\033[" << 16 << "H" << "\033[90m";
    std::cout << "Press [tab] for next test\n"
                 "Press [enter] to retry test\n"
                 "Press [/] to hide tip\n";
    std::cout << "\033[10H\033[0m";

    char ch;
    int index = 0;
    int col = 0;
    int row = 9;
    int extras = 0;

    int total_keys = 0;
    int correct_keys = 0;

    std::cout << "\033[" << row + 1 << "H";

    auto start = std::chrono::steady_clock::now();

    while (true) {  // TODO: separate this while loop into two+ different functions
        if (typed_word_count >= prompt_word_count) {
            auto end = std::chrono::steady_clock::now();
            time_passed = std::chrono::duration<double>(end - start).count();
            return 2;
            // TODO left off debug here
        }
        if (first_ch != '\0') {
                ch = first_ch;
                if (ch == 27) {
                    auto end = std::chrono::steady_clock::now();
                    time_passed = std::chrono::duration<double>(end - start).count();
                    return 2; // ESC to quit // TODO adjust
                }
                else if (ch == '\b') { // backspace
                    backspace_pressed(typed, ch, prompt, index, col, row, extras);
                } else { // normal key
                    key_pressed(typed, ch, prompt, index, col, row, extras, typed_word_count); // TODO check if all lines up
                }
                first_ch = '\0';
        } else {
            if (index >= prompt.length()) {
                auto end = std::chrono::steady_clock::now();
                time_passed = std::chrono::duration<double>(end - start).count();
                return 2;
            }
            if (_kbhit()) {
                ch = _getch();
                if (ch == 27) {
                    auto end = std::chrono::steady_clock::now();
                    time_passed = std::chrono::duration<double>(end - start).count();
                    return 2; // ESC to quit // TODO adjust
                }
                else if (ch == '\b') { // backspace
                    backspace_pressed(typed, ch, prompt, index, col, row, extras);
                } else { // normal key
                    key_pressed(typed, ch, prompt, index, col, row, extras, typed_word_count); // TODO check if all lines up
                }
            }
        }
    }
}

int resultsView(std::vector<char>& typed, std::string& prompt, int prompt_word_count, int& typed_word_count, double& time_passed) {
       // Accuracy code and MS #2 Implementation ------------------------------------
    int correct = 0;
    int total = (std::min)(typed.size(), prompt.size());
    for (size_t i = 0; i < total; ++i) {
        if (typed[i] == prompt[i]) {
            correct++;
        }
    }
    std::string percentUrl = "http://localhost:3002/calculate-percentage?divisor=" 
                         + std::to_string(total) + "&dividend=" 
                         + std::to_string(correct);
    json percentResp = getRequest(percentUrl);
    double accuracy = 0.0;
    if (percentResp.contains("percentage")) {
        accuracy = percentResp["percentage"];
    }
    // double accuracy = (double)correct / total * 100.0; // Uncomment for local
    // -----------------------------------------------------------------------------

    double time_min = time_passed / 60.0;
    double wpm_exact = (double)correct / 5.0 / time_min;
    double raw_exact = (double)total / 5.0 / time_min;

    // Cast to int for display
    int wpm = static_cast<int>(std::round(wpm_exact));
    int raw = static_cast<int>(std::round(raw_exact));

    std::cout << "\033[2J\033[H\n";
    std::cout << "\n\n\n\n\n\n\n\n";
    std::cout << "\033[0m" << "wpm: " << "\033[38;5;202m" << wpm << "\n";
    std::cout << "\033[0m" << "acc: " << "\033[38;5;202m" << accuracy << "%\n";
    std::cout << "\033[0m" << "raw: " << "\033[38;5;202m" << raw << "\n";
    std::cout << "\033[0m" << "time: " << "\033[38;5;202m" << std::fixed << std::setprecision(1) << time_passed << "s\n";
    std::cout << "\033[0m" << "difficulty: " << "\033[38;5;202m" << "_\n\n\n";
    std::cout << "\033[0m" << "Press [tab] for next test\n"
                              "Press [enter] to retry test\n\n";

    int results_ch = '\0';
    while(true) {
        if (_kbhit()) {
            results_ch = _getch();
        }
        if (results_ch == '\t') return 0;
        if (results_ch == '\r') return 1;
    }
}

int menuView() {
    std::cout << "\033[2J\033[H\n\n\n\n\n";
    std::cout << "Press [1] to close the menu\n\n";
    std::cout << "more\n";
    std::cout << "[2] about\n\n";
    std::cout << "settings\n";
    std::cout << "Press [key] for setting --> type new value --> [enter] to apply changes\n";
    std::cout << "[3] time -- " << "30s : " << "\033[38;5;202m" << "15" << "\033[0m\n";
    std::cout << "[4] difficulty -- " << "c : " << "\033[38;5;202m" << "e" << "\033[0m\n";
    std::cout << "\t(c: classic experience. h: hard, fails test if you enter an\n"
                 "\tincorrect word. e: expert, fails test if you press an incorrect key)\n\n";
    std::cout << "danger zone\n";
    std::cout << "[5] reset settings\n";
    // TODO: Implement reset settings screen/message
    char menu_ch = '\0';
    while(true) {
        if (_kbhit()) {
            menu_ch = _getch();
        }
        if (menu_ch == '1') return 0;
        if (menu_ch == '2') return 4;
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
    char about_ch = '\0';
    while(true) {
        if (_kbhit()) {
            about_ch = _getch();
        }
        if (about_ch == '1') return 0;
        if (about_ch == '2') return 3;
    }
}

int get_word_count(std::string prompt) {
    int count = 0;
    size_t start = 0;
    while (start < prompt.size()) {
        size_t end = prompt.find_first_of(" \n", start);
        if (end == std::string::npos) {  
            // std::string word = prompt.substr(start);
            // TODO use word
            break; // no more delimiters, exit loop
        } else {
            // std::string word = prompt.substr(start, end - start);
            // TODO use word
            count++;
            start = end + 1; // advance past space/newline
        }
    }
    return count;
}

int main() {
    //std::cout << "\033[2J\033[H";
    // TODO: function to get random prompt
    std::vector<std::string> prompts = {
        "The quick brown fox jumps over the lazy dog.\n"
        "The quick brown fox jumps over the lazy dog.\n"
        "The quick brown fox jumps over the lazy dog.\n",
        "Across between early still and matter turn;\n"
        "clear place why leave forward, right under no\n"
        "small part, before public note center, up beyond.\n",
        "Waves crash gently against the silent shore;\n"
        "salt air drifts across the fading horizon,\n"
        "and tides whisper secrets to the moon.\n"
    };
    
    // Pick a prompt
    std::string raw_prompt = prompts.at(2);

     // MS #8 implementation: minimal mode ------------------
    json body;
    body["text"] = raw_prompt;
    std::string postBody = body.dump();
    std::string minimalUrl = "http://localhost:3008/minimal";
    json minimalResp = postRequest(minimalUrl, postBody);
    std::string prompt;
    if (minimalResp.contains("cleanedText")) {
        prompt = minimalResp["cleanedText"];
    } else {
        prompt = raw_prompt; // fallback if service fails
    }
    // -------------------------------------------------------
    
    int prompt_word_count = get_word_count(prompt);
    int typed_word_count = 0;
    int menu = 0;
    char first_ch = '\0';
    double time_passed = 0;

    std::vector<char> typed;

    while (true) {
        if (menu == 0) menu = homeView(prompt, first_ch);
        if (menu == 1) menu = testView(typed, prompt, first_ch, prompt_word_count, typed_word_count, time_passed);
        if (menu == 2) menu = resultsView(typed, prompt, prompt_word_count, typed_word_count, time_passed);
        if (menu == 3) menu = menuView();
        if (menu == 4) menu = aboutView();
    }
}