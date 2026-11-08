// Purpose: Implements case-insensitive facility ID normalization and validation.
// Author: Rami Badr
#include "Facility.h"
#include "TextUtils.h"

// Uses the same case and whitespace rules for file IDs and user-entered IDs.
std::string normalizeFacilityId(const std::string& id) {
    return TextUtils::normalizeKey(id);
}

// Checks the entire normalized ID, rejecting suffixes, internal spaces, and bad digits.
bool isValidFacilityId(const std::string& id) {
    const std::string key = normalizeFacilityId(id);
    if (key.size() != 4 || key.front() != 'F') return false;
    for (std::size_t index = 1; index < key.size(); ++index) {
        if (key[index] < '0' || key[index] > '9') return false;
    }
    return true;
}
