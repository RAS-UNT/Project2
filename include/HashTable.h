// Purpose: Defines a custom facility ID hash table with separate chaining.
// Author: Rami Badr
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "Facility.h"
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

class HashTable {
public:
    static constexpr std::size_t DEFAULT_CAPACITY = 17;

    enum class InsertResult {
        Inserted,
        DuplicateId,
        InvalidId
    };

    // Creates a table with a positive bucket count; chains grow as needed.
    explicit HashTable(std::size_t capacity = DEFAULT_CAPACITY);

    // Copies records and their bucket indices together without sharing storage.
    HashTable(const HashTable&) = default;
    HashTable& operator=(const HashTable&) = default;

    // Normalizes the ID and rejects invalid or duplicate keys without changing data.
    InsertResult insert(const Facility& facility);

    // Searches only the hashed bucket. Returned pointers may expire after insertion.
    const Facility* find(const std::string& id) const;

    // Returns the number of buckets, not a maximum number of facilities.
    std::size_t capacity() const;

    // Returns the number of accepted records.
    std::size_t size() const;

    // Counts accepted insertions into previously nonempty buckets.
    std::size_t collisions() const;

    // Exposes read-only records in accepted insertion order for later displays.
    const std::vector<Facility>& records() const;

    // Writes diagnostic fields without a newline for use in a load summary.
    void displayDiagnostics(std::ostream& output) const;

private:
    std::vector<std::vector<std::size_t>> buckets;
    std::vector<Facility> entries;
    std::size_t collisionCount;

    // Applies a polynomial string hash to an already normalized, valid ID.
    std::size_t hashNormalizedId(const std::string& id) const;
};

#endif
