// Purpose: Implements consistent text handling for campus input and searches.
// Author: Rami Badr
#include "TextUtils.h"
#include <cctype>

// Handles spaces, tabs, and CRLF whitespace without altering internal text.
std::string TextUtils::trim(const std::string& value) {
    std::size_t first = 0;
    std::size_t last = value.size();
    while (first < last && std::isspace(static_cast<unsigned char>(value[first]))) ++first;
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) --last;
    return value.substr(first, last - first);
}

// Normalizes comparison keys only; display spelling remains in the original record.
std::string TextUtils::normalizeKey(const std::string& value) {
    std::string key = trim(value);
    for (char& character : key) {
        if (character >= 'a' && character <= 'z') {
            character = static_cast<char>(character - 'a' + 'A');
        }
    }
    return key;
}

// Keeps empty fields so later loaders can reject incomplete records correctly.
std::vector<std::string> TextUtils::splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        std::size_t separator = line.find('|', start);
        if (separator == std::string::npos) {
            fields.push_back(trim(line.substr(start)));
            return fields;
        }
        fields.push_back(trim(line.substr(start, separator - start)));
        start = separator + 1;
    }
}

// A # within a name is ordinary data, not the beginning of a comment.
bool TextUtils::isBlankOrComment(const std::string& line) {
    std::string trimmed = trim(line);
    return trimmed.empty() || trimmed.front() == '#';
}
