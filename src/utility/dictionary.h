#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <algorithm>
#include <unordered_map>
#include <vector>

/**
 * @brief A dictionary class that maps keys to values.
 * The dictionary is implemented using an unordered map and a vector of keys.
 * Unlike std::map, this class preserves the order of insertion.
 *
 * @tparam K The key type. Must be hashable.
 * @tparam V The value type.
 */
template <typename K, typename V>
class Dictionary {
    std::unordered_map<K, V> map;
    std::vector<K> keys;

public:
    class iterator {
        typename std::vector<K>::iterator it;
        std::unordered_map<K, V>& map;

    public:
        iterator(typename std::vector<K>::iterator it, std::unordered_map<K, V>& map) : it(it), map(map) {}

        std::pair<const K, V>& operator*() {
            return *map.find(*it);
        }

        iterator& operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return it != other.it;
        }
    };

    class const_iterator {
        typename std::vector<K>::const_iterator it;
        const std::unordered_map<K, V>& map;

    public:
        const_iterator(typename std::vector<K>::const_iterator it, const std::unordered_map<K, V>& map) : it(it), map(map) {}

        const std::pair<const K, V>& operator*() const {
            return *map.find(*it);
        }

        const_iterator& operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const const_iterator& other) const {
            return it != other.it;
        }
    };

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
        if (!contains(key)) {
            keys.push_back(key);
        }
        map[key] = value;
    }

    /**
     * @brief Access the value associated with a key.
     * The value type must have a default constructor, even if this operator is used to set the value.
     *
     * @param key The key.
     * @return V& A reference to the value.
     */
    V& operator[](K key) {
        insert(key, V());
        return map[key];
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return const V& A const reference to the value.
     */
    const V& operator[](K key) const {
        return map.at(key);
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return V& A reference to the value.
     * @throw std::out_of_range If the key is not in the dictionary.
     */
    V& at(K key) {
        return map.at(key);
    }

    /**
     * @brief Access the value associated with a key.
     *
     * @param key The key.
     * @return const V& A const reference to the value.
     * @throw std::out_of_range If the key is not in the dictionary.
     */
    const V& at(K key) const {
        return map.at(key);
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

    iterator begin() {
        return iterator(keys.begin(), map);
    }

    iterator end() {
        return iterator(keys.end(), map);
    }

    const_iterator begin() const {
        return const_iterator(keys.begin(), map);
    }

    const_iterator end() const {
        return const_iterator(keys.end(), map);
    }

    /**
     * @brief Finds a key in the dictionary.
     *
     * @param key The key to find.
     * @return iterator An iterator containing a pair of the key and value if it is found, or end() if it is not found.
     */
    iterator find(K key) {
        auto it = std::find(keys.begin(), keys.end(), key);
        return iterator(it, map);
    }

    /**
     * @brief Finds a key in the dictionary.
     *
     * @param key The key to find.
     * @return const_iterator An iterator containing a pair of the key and value if it is found, or end() if it is not found.
     */
    const_iterator find(K key) const {
        auto it = std::find(keys.begin(), keys.end(), key);
        return const_iterator(it, map);
    }
};

#endif // DICTIONARY_H
