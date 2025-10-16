#ifndef CACHE_MANAGER_HPP
#define CACHE_MANAGER_HPP

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>
#include <memory>

// Represents a single item stored in the cache
struct CacheEntry {
    std::string url;
    std::string response_data;
    time_t last_accessed;
};

class CacheManager {
public:
    CacheManager(size_t max_size_bytes, size_t max_element_size_bytes);

    // Tries to find an item in the cache. Returns an empty string if not found.
    std::string find(const std::string& url);

    // Adds a new item to the cache, evicting old items if necessary.
    void add(const std::string& url, const std::string& data);

private:
    // Internal function to remove the least recently used item
    void evict();

    const size_t max_size;
    const size_t max_element_size;
    size_t current_size;

    // A list to track the usage order (LRU)
    std::list<CacheEntry> lru_list;
    // A hash map for fast lookups of items in the list
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> cache_map;
    
    // A mutex to ensure that only one thread can modify the cache at a time
    std::mutex cache_mutex;
};

#endif // CACHE_MANAGER_HPP