#include "CacheManager.hpp"
#include <iostream>

CacheManager::CacheManager(size_t max_size_bytes, size_t max_element_size_bytes)
    : max_size(max_size_bytes), max_element_size(max_element_size_bytes), current_size(0) {}

std::string CacheManager::find(const std::string& url) {
    // std::lock_guard automatically locks the mutex and unlocks it when it goes out of scope
    std::lock_guard<std::mutex> lock(cache_mutex);

    auto it = cache_map.find(url);
    if (it == cache_map.end()) {
        return ""; // Cache miss
    }

    // Move the found item to the front of the LRU list to mark it as recently used
    lru_list.splice(lru_list.begin(), lru_list, it->second);
    std::cout << "Cache HIT for URL: " << url << std::endl;
    return it->second->response_data;
}

void CacheManager::add(const std::string& url, const std::string& data) {
    size_t element_size = data.length();
    if (element_size > max_element_size) {
        std::cout << "Element too large to cache: " << url << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(cache_mutex);

    // Evict old items until there is enough space for the new one
    while (current_size  > max_size-element_size && !cache_map.empty()) { //write like c>m-e to prevent overflow, actually c+e<m
        evict();
    }

    if (current_size <= max_size-element_size) {
        // If the item already exists, we update it by removing the old one first
        auto it = cache_map.find(url);
        if (it != cache_map.end()) {
            current_size -= it->second->response_data.length();
            lru_list.erase(it->second);
            cache_map.erase(it);
        }
        
        // Add the new item to the front of the list
        lru_list.emplace_front(CacheEntry{url, data, time(nullptr)});
        cache_map[url] = lru_list.begin();
        current_size += element_size;
        std::cout << "Cached URL: " << url << std::endl;
    }
}

void CacheManager::evict() {
    // This function assumes the mutex is already locked
    if (lru_list.empty()) return;

    // The least recently used item is at the back of the list
    const CacheEntry& lru_entry = lru_list.back();
    std::cout << "Evicting from cache: " << lru_entry.url << std::endl;

    current_size -= lru_entry.response_data.size();
    cache_map.erase(lru_entry.url);
    lru_list.pop_back();
}