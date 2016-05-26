// Copyright (C) 2014 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EMUGL_COMMON_ID_TO_OBJECT_MAP_H
#define EMUGL_COMMON_ID_TO_OBJECT_MAP_H

#include <stddef.h>

namespace emugl {

// Base implementation class for IdToObjectMap template.
// Used to reduce template-instanciated code generation.
class IdToObjectMapBase {
public:
    // The type of keys in this map.
    typedef unsigned KeyType;

    // Values higher than kMaxId cannot be used as map keys.
    enum {
        kMaxId = 0xfffffffdU,
    };

    static inline bool isValidKey(KeyType key) {
        return key <= kMaxId;
    }

protected:
    IdToObjectMapBase();

    ~IdToObjectMapBase();

    void clear();

    // Return size
    inline size_t size() const { return mCount; }

    inline size_t capacity() const { return 1U << mShift; }

    // Return true iff the map contains a given key.
    bool contains(KeyType key) const;

    // Find a value associated with a given |key| in the map.
    // On success, return true and sets |*value| to the value/pointer,
    // which is _still_ owned by the map.
    // On failure, return false and sets |*value| to NULL.
    bool find(KeyType key, void** value) const;

    // Associate a value with a given |key| in the map.
    // Return the old value for the key, if any. Caller is responsible
    // for freeing it.
    void* set(KeyType key, void* value);

    // Remove the value associated with a given |key|.
    // Return the old value, if any. Caller is responsible for
    // freeing it.
    void* remove(KeyType key);

    size_t mCount;
    size_t mShift;
    KeyType* mKeys;
    void** mValues;

private:
    // Resize the map if needed to ensure it can hold at least |newSize|
    // entries.
    void resize(size_t newSize);
};

// A templated data container that acts as a dictionary mapping unsigned
// integer keys to heap-allocated objects of type T. The dictionary
// owns the objects associated with its keys, and automatically destroys
// them when it is destroyed, or during replacement or removal.
template <class T>
class IdToObjectMap : public IdToObjectMapBase {
public:
    // Initialize an empty instance.
    IdToObjectMap() : IdToObjectMapBase() {}

    // Destroy this instance.
    ~IdToObjectMap() {
        clear();
    }

    // Return the number of items in this map.
    inline size_t size() const { return IdToObjectMapBase::size(); }

    // Return true iff the map is empty.
    inline bool empty() const { return !IdToObjectMapBase::size(); }

    // Remove all items from the map.
    void clear();

    // Returns true iff the dictionary contains a value for |key|.
    inline bool contains(KeyType key) const {
        return IdToObjectMapBase::contains(key);
    }

    // Find the value corresponding to |key| in this map.
    // On success, return true, and sets |*value| to point to the
    // value (still owned by the instance). On failure, return false.
    inline bool find(KeyType key, T** value) const {
        return IdToObjectMapBase::find(key, reinterpret_cast<void**>(value));
    }

    // Return the value associated with a given |key|, or NULL if it is
    // not in the map. Result is still owned by the map.
    inline T* get(KeyType key) const {
        T* result = NULL;
        this->find(key, &result);
        return result;
    }

    // Associate |value| with a given |key|. Returns true if a previous
    // value was replaced, and false if this is the first time a value
    // was associated with the given key. IMPORTANT: This transfers
    // ownership of |value| to the map instance. In case of replacement,
    // the old value is automatically destroyed. Using NULL as the value
    // is equivalent to calling remove().
    bool set(KeyType key, T* value);

    // Remove any value associated with |key|.
    // Return true iff a value was associated with the key and destroyed
    // by this function, false if there was no value associated with the
    // key (or if it was NULL).
    bool remove(KeyType key);
};

template <class T>
void IdToObjectMap<T>::clear() {
    size_t n = capacity();
    while (n > 0) {
        --n;
        if (!isValidKey(mKeys[n]))
            continue;

        delete static_cast<T*>(mValues[n]);
        mValues[n] = NULL;
        mKeys[n] = kMaxId + 1U;
    }
    mCount = 0;
}

template <class T>
bool IdToObjectMap<T>::set(KeyType key, T* value) {
    T* oldValue = static_cast<T*>(IdToObjectMapBase::set(key, value));
    if (!oldValue) {
        return false;
    }
    delete oldValue;
    return true;
}

template <class T>
bool IdToObjectMap<T>::remove(KeyType key) {
    T* oldValue = static_cast<T*>(IdToObjectMapBase::remove(key));
    if (!oldValue)
        return false;
    delete oldValue;
    return true;
}

}  // namespace emugl


#endif  // EMUGL_COMMON_ID_TO_OBJECT_MAP_H
