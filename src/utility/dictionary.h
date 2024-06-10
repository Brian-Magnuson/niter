#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief A dictionary class that maps keys to values.
 * The dictionary is implemented using an unordered map and a vector of keys.
 * Unlike std::map, this class preserves the order of insertion.
 *
 * @tparam K The key type. Must be hashable.
 * @tparam V The value type.
 * @tparam Hash The hash function for the keys. Defaults to std::hash<K>.
 */
template <typename K, typename V, typename Hash = std::hash<K>>
class Dictionary {
    std::unordered_map<K, unsigned, Hash> map;
    std::vector<std::pair<K, V>> keys;

public:
    Dictionary() = default;

    Dictionary(const Dictionary& other) = default;

    /**
     * @brief Insert a key-value pair into the dictionary.
     * If the key does not exist, it is also added to the end of the keys vector.
     * If the key already exists, the value is updated.
     *
     * @param key The key.
     * @param value The value.
     */
    void insert(K key, V value) {
        auto it = map.find(key);
        if (it == map.end()) {
            map[key] = keys.size();
            keys.push_back({key, value});
        } else {
            keys.at(it->second).second = value;
        }
    }

    /**
     * @brief Access the value associated with a key.
     * The value type must have a default constructor, even if this operator is used to set the value.
     *
     * @param key The key.
     * @return V& A reference to the value.
     */
    V& operator[](K key) {
        auto it = map.find(key);
        if (it == map.end()) {
            map[key] = keys.size();
            keys.push_back({key, V()});
            return keys.back().second;
        } else {
            return keys.at(it->second).second;
        }
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return const V& A const reference to the value.
     */
    const V& operator[](K key) const {
        return keys.at(map.at(key)).second;
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return V& A reference to the value.
     * @throw std::out_of_range If the key is not in the dictionary.
     */
    V& at(K key) {
        return keys.at(map.at(key)).second;
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return const V& A const reference to the value.
     * @throw std::out_of_range If the key is not in the dictionary.
     */
    const V& at(K key) const {
        return keys.at(map.at(key)).second;
    }

    /**
     * @brief Get the index of a key in the dictionary.
     * If the key is not in the dictionary, returns -1.
     *
     * @param key The key.
     * @return int The index of the key. -1 if the key is not in the dictionary.
     */
    int get_index(K key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return -1;
        } else {
            return (int)(it->second);
        }
    }

    /**
     * @brief Check if the dictionary contains a key.
     *
     * @param key The key to check.
     * @return true If the key is in the dictionary.
     * @return false If the key is not in the dictionary.
     */
    bool contains(K key) const {
        return map.find(key) != map.end();
    }

    /**
     * @brief Gets the size of the dictionary.
     *
     * @return size_t The number of keys in the dictionary.
     */
    size_t size() const {
        return map.size();
    }

    /**
     * @brief Clear the dictionary.
     *
     */
    void clear() {
        map.clear();
        keys.clear();
    }

    typename std::vector<std::pair<K, V>>::iterator begin() {
        return keys.begin();
    }

    typename std::vector<std::pair<K, V>>::iterator end() {
        return keys.end();
    }

    typename std::vector<std::pair<K, V>>::const_iterator begin() const {
        return keys.begin();
    }

    typename std::vector<std::pair<K, V>>::const_iterator end() const {
        return keys.end();
    }

    typename std::vector<std::pair<K, V>>::iterator find(K key) {
        auto iter = map.find(key);
        if (iter == map.end()) {
            return keys.end();
        }
        return keys.begin() + iter->second;
    }

    typename std::vector<std::pair<K, V>>::const_iterator find(K key) const {
        auto iter = map.find(key);
        if (iter == map.end()) {
            return keys.end();
        }
        return keys.begin() + iter->second;
    }
};

#endif // DICTIONARY_H
