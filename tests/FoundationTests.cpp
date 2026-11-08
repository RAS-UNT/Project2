// Purpose: Tests text parsing and facility ID rules before module integration.
// Author: Rami Badr
#include "Facility.h"
#include "TextUtils.h"
#include <iostream>
#include <string>
#include <vector>

// Reports failures even when standard assertions are disabled in a Release build.
void expect(bool condition, const std::string& description, int& checks, int& failures) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

// Exercises normalization without replacing or modifying the instructor's data files.
int main() {
    int checks = 0;
    int failures = 0;
    expect(TextUtils::trim(" \tLibrary\r\n") == "Library", "trim outer whitespace", checks, failures);
    expect(TextUtils::trim(" \t\r\n") == "", "trim whitespace-only text", checks, failures);
    expect(TextUtils::trim("").empty(), "trim empty text", checks, failures);
    expect(TextUtils::trim(" Student  Union ") == "Student  Union", "preserve internal spaces", checks, failures);
    expect(TextUtils::normalizeKey(" student union\r") == "STUDENT UNION", "case-insensitive names", checks, failures);
    expect(TextUtils::normalizeKey("Student  Union") != TextUtils::normalizeKey("Student Union"),
           "internal spaces matter", checks, failures);
    expect(TextUtils::isBlankOrComment("  # comment\r"), "leading-whitespace comment", checks, failures);
    expect(TextUtils::isBlankOrComment(" \t\r"), "blank line", checks, failures);
    expect(TextUtils::isBlankOrComment(""), "empty line", checks, failures);
    expect(!TextUtils::isBlankOrComment(" Room #5 "), "embedded hash is data", checks, failures);

    std::vector<std::string> fields = TextUtils::splitFields(" f004 | Room #5 | Science Building | Computer Lab\r");
    expect(fields.size() == 4, "four pipe-delimited fields", checks, failures);
    expect(fields[0] == "f004" && fields[1] == "Room #5" && fields[2] == "Science Building" &&
           fields[3] == "Computer Lab", "trim fields and preserve spelling", checks, failures);
    fields = TextUtils::splitFields("F001|Name|Building|");
    expect(fields.size() == 4 && fields.back().empty(), "trailing empty field retained", checks, failures);
    fields = TextUtils::splitFields("|Name||Category");
    expect(fields.size() == 4 && fields[0].empty() && fields[2].empty(), "leading and middle empty fields", checks, failures);
    expect(TextUtils::splitFields("F001|Name|Building|Category|Extra").size() == 5,
           "extra fields retained for rejection", checks, failures);
    expect(TextUtils::splitFields("").size() == 1, "empty record remains detectable", checks, failures);

    expect(normalizeFacilityId(" \tf004\r\n") == "F004", "normalize lowercase ID", checks, failures);
    for (const std::string& id : std::vector<std::string>{"F000", "F001", "F999", "f004", " F004\r"}) {
        expect(isValidFacilityId(id), "valid ID: " + id, checks, failures);
    }
    for (const std::string& id : std::vector<std::string>{"", " ", "F01", "F0001", "A001", "F0A1", "F-01",
                                                         "F 01", "F001suffix", "F001|", "F00\n1"}) {
        expect(!isValidFacilityId(id), "invalid ID: " + id, checks, failures);
    }

    Facility facility{normalizeFacilityId("f004"), "Room #5", "Science Building", "Computer Lab"};
    expect(facility.id == "F004", "record stores normalized ID", checks, failures);
    expect(facility.name == "Room #5" && facility.building == "Science Building" && facility.category == "Computer Lab",
           "record retains all fields", checks, failures);

    if (failures != 0) return 1;
    std::cout << "All " << checks << " foundation checks passed.\n";
    return 0;
}
