// Purpose: Implements custom string hashing, chained insertion, and facility lookup.
// Author: Rami Badr
#include "HashTable.h"
#include <ostream>
#include <stdexcept>

// Rejects zero capacity so hashing can always take a valid remainder.
HashTable::HashTable(std::size_t capacity) : buckets(capacity), collisionCount(0) {
    if (capacity == 0) {
        throw std::invalid_argument("Hash table capacity must be positive.");
    }
}

// Hashes every ID character using h = (31 * h + character) modulo capacity.
std::size_t HashTable::hashNormalizedId(const std::string& id) const {
    std::size_t hash = 0;
    for (unsigned char character : id) {
        hash = (31 * hash + character) % buckets.size();
    }
    return hash;
}

// Keeps the first record for each ID and appends successful records in input order.
HashTable::InsertResult HashTable::insert(const Facility& facility) {
    const std::string key = normalizeFacilityId(facility.id);
    if (!isValidFacilityId(key)) return InsertResult::InvalidId;

    std::vector<std::size_t>& chain = buckets[hashNormalizedId(key)];
    for (std::size_t recordIndex : chain) {
        if (entries[recordIndex].id == key) return InsertResult::DuplicateId;
    }

    Facility normalized = facility;
    normalized.id = key;
    entries.push_back(normalized);

    // Roll back the record if allocating its chain entry fails.
    try {
        chain.push_back(entries.size() - 1);
    } catch (...) {
        entries.pop_back();
        throw;
    }

    if (chain.size() > 1) ++collisionCount;
    return InsertResult::Inserted;
}

// Uses the normalized ID's bucket rather than scanning all accepted records.
const Facility* HashTable::find(const std::string& id) const {
    const std::string key = normalizeFacilityId(id);
    if (!isValidFacilityId(key)) return nullptr;

    for (std::size_t recordIndex : buckets[hashNormalizedId(key)]) {
        if (entries[recordIndex].id == key) return &entries[recordIndex];
    }
    return nullptr;
}

// The bucket count stays fixed; separate chains have no fixed record limit.
std::size_t HashTable::capacity() const {
    return buckets.size();
}

// Rejected insertions never append to the record list.
std::size_t HashTable::size() const {
    return entries.size();
}

// Lookups and rejected insertions do not contribute to this total.
std::size_t HashTable::collisions() const {
    return collisionCount;
}

// Bucket storage order does not affect the accepted record order.
const std::vector<Facility>& HashTable::records() const {
    return entries;
}

// Allows a file loader to include the table's current state in its summary.
void HashTable::displayDiagnostics(std::ostream& output) const {
    output << "capacity=" << capacity() << " entries=" << size()
           << " collisions=" << collisions();
}
