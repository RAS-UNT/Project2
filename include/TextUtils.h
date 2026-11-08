// Purpose: Shared whitespace, matching, and field parsing rules for campus files.
// Author: Rami Badr
#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>
#include <vector>

namespace TextUtils {

// Removes outer whitespace without changing spaces inside a field.
std::string trim(const std::string& value);

// Produces a trimmed, ASCII-uppercase key for case-insensitive matching.
std::string normalizeKey(const std::string& value);

// Splits pipe-delimited records while retaining empty and trailing fields.
std::vector<std::string> splitFields(const std::string& line);

// Recognizes only blank lines and lines whose first non-whitespace character is #.
bool isBlankOrComment(const std::string& line);

}

#endif
