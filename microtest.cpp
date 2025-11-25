#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback to collect response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
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

    std::cout << "Raw POST response: " << response << "\n";

    try {
        return json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "POST parse error: " << e.what() << "\n";
        return json{};
    }
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

    std::cout << "Raw GET response: " << response << "\n";

    try {
        return json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "GET parse error: " << e.what() << "\n";
        return json{};
    }
}

int main() {
    std::cout << "Program started\n"; 
    try {
        // --- Test text-cleaning microservice (POST) ---
        std::string minimalUrl = "http://localhost:3008/minimal";
        std::string postBody = R"({"text":"Hello, WORLD!!!"})";

        json minimalResp = postRequest(minimalUrl, postBody);
        if (minimalResp.contains("error")) {
            std::cout << "Minimal error: " << minimalResp["error"] << "\n";
        } else if (minimalResp.contains("cleanedText")) {
            std::cout << "Minimal cleaned: " << minimalResp["cleanedText"] << "\n";
        }

        // --- Test percentage microservice (GET) ---
        std::string percentUrl = "http://localhost:3002/calculate-percentage?divisor=10&dividend=5";

        json percentResp = getRequest(percentUrl);
        if (percentResp.contains("error")) {
            std::cout << "Percentage error: " << percentResp["error"] << "\n";
        } else if (percentResp.contains("percentage")) {
            std::cout << "Percentage: " << percentResp["percentage"] << "%\n";
        }

        // --- Test sanitize microservice (POST) ---
        std::string sanitizeUrl = "http://localhost:3008/sanitize";
        std::string sanitizeBody = R"({"text":"Café\t\t au  🌍 lait\n\n"})";

        json sanitizeResp = postRequest(sanitizeUrl, sanitizeBody);
        if (sanitizeResp.contains("error")) {
            std::cout << "Sanitize error: " << sanitizeResp["error"] << "\n";
        } else if (sanitizeResp.contains("cleanedText")) {
            std::cout << "Sanitized text: " << sanitizeResp["cleanedText"] << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
    }
    return 0;
}
