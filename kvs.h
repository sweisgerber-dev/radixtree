/*
 *  (c) Copyright 2016-2017 Hewlett Packard Enterprise Development Company LP.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the
 *  Application containing code generated by the Library and added to the
 *  Application during this compilation process under terms of your choice,
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

#ifndef KVS_H
#define KVS_H

#include <cstddef> // size_t

#include "common.h"          // TagGptr
#include "nvmm/global_ptr.h" // GlobalPtr

namespace famradixtree {

using Gptr = nvmm::GlobalPtr;

// TODO: use std::string instead of buf + size?
class KeyValueStore {
  public:
    static nvmm::PoolId const default_heap_id_ = 2;

    virtual ~KeyValueStore() {};

    static void Start(std::string base = "", std::string user = "");
    static void Reset(std::string base = "", std::string user = "");
    static void Restart(std::string base = "", std::string user = "");

    enum IndexType {
        INVALID = 0,
        RADIX_TREE,
        HASH_TABLE,
        RADIX_TREE_TINY,
    };

    enum ErrorCode {
        KEY_DOES_NOT_EXIST = -2,
        ERROR = -1,
        NO_ERROR = 0
    };

    static KeyValueStore *MakeKVS(IndexType type, nvmm::GlobalPtr location,
                                  std::string base = "", std::string user = "",
                                  size_t heap_size = default_heap_size_,
                                  nvmm::PoolId heap_id = default_heap_id_);

    static KeyValueStore *MakeKVS(std::string type, nvmm::GlobalPtr location,
                                  std::string base = "", std::string user = "",
                                  size_t heap_size = default_heap_size_,
                                  nvmm::PoolId heap_id = default_heap_id_);

    /*
      public KVS APIs
    */
    // doing some maintenance work (e.g., free up memory that was delayed freed
    virtual void Maintenance() = 0;

    // return the root global pointer of the kvs
    virtual nvmm::GlobalPtr Location() = 0;

    // return max key len
    virtual size_t MaxKeyLen() = 0;

    // return max value len
    virtual size_t MaxValLen() = 0;

    // insert if key does not exist; update if key exists
    // return 0 (no error); -1 (error)
    virtual int Put(char const *key, size_t const key_len, char const *val,
                    size_t const val_len) = 0;

    // return 0 (key exists); -1 (error); -2(key does not exist)
    // also return actual val_len
    virtual int Get(char const *key, size_t const key_len, char *val,
                    size_t &val_len) = 0;

    // Inserts if not found, if found returns the same.
    // return 0 (inserted) -3 (Found) ; -1 (error)
    virtual int FindOrCreate(char const *key, size_t const key_len,
                             char const *val, size_t const val_len,
                             char *ret_val, size_t &ret_len) = 0;

    // return 0 (key exists); -1 (error); -2 (key does not exist)
    virtual int Del(char const *key, size_t const key_len) = 0;

    // scan APIs (radixtree only)
    // return 0 (key exists); -1 (error); -2 (no key in range)
    virtual int Scan(int &iter_handle, char *key, size_t &key_len, char *val,
                     size_t &val_len, char const *begin_key,
                     size_t const begin_key_len, bool const begin_key_inclusive,
                     char const *end_key, size_t const end_key_len,
                     bool const end_key_inclusive) {
        return -1;
    };

    // return 0 (next key exists); -1 (error); -2 (no next key)
    virtual int GetNext(int iter_handle, char *key, size_t &key_len, char *val,
                        size_t &val_len) {
        return -1;
    };

    static constexpr const char *OPEN_BOUNDARY_KEY = "\0";
    static constexpr const size_t OPEN_BOUNDARY_KEY_SIZE = 1;

    /*
      internal APIs for consistent DRAM caching

      TODO: these should be internal APIs

      NOTE:
      - now we only concern about key nodes: once a key node exists, the key
      exists
        - when a key is deleted, its key node is still there but its value
      pointer is null
      - for non-cached operations, they return both a key pointer and the
      up-to-date value pointer
        - if the key pointer is null, the key node does not exist
        - if the key pointer is valid, the key node exists, but the value
      pointer could be null
      - these functions essentially expose the internal state of the underlying
      index data
      structures to the caching layer
        - the caller is now responsible for checking if the returned key pointer
      and value pointer
      are valid or null
    */

    // for non-cached put
    // return 0 with key_ptr and new val_ptr (no error)
    // return -1 (error)
    virtual int Put(char const *key, size_t const key_len, char const *val,
                    size_t const val_len, Gptr &key_ptr, TagGptr &val_ptr) {
        return -1;
    };

    // for cached put using key ptr
    // return 0 with new val_ptr (no error)
    // return -1 (error)
    virtual int Put(Gptr const key_ptr, TagGptr &val_ptr, char const *val,
                    size_t const val_len) {
        return -1;
    };

    // for non-cached Get
    // return 0 with val, key_ptr and val_ptr
    // - if key_ptr is null, key node is not found and there is no val_ptr
    // - if key_ptr is not null, key node is found (val_ptr could be null)
    // return -1 (error)
    // NOTE: caller is responsible for checking if the returned val_ptr is null
    virtual int Get(char const *key, size_t const key_len, char *val,
                    size_t &val_len, Gptr &key_ptr, TagGptr &val_ptr) {
        return -1;
    };

    // for cached Get using key ptr and val ptr
    // return 0 with up-to-date val_ptr and val
    // - if the given val_ptr is already up-to-date, then there is nothing in
    // val (not needed)
    // - if the given val_ptr is stale, then val contains the new value
    // - exception: if get_value is true, the new value is always fetched from
    // FAM (for shortcut caching)
    // return -1 (error)
    // NOTE: caller is responsible for checking if the returned val_ptr is null
    // NOTE: caller is responsible for checking if the given val_ptr matches the
    // returned val_ptr
    virtual int Get(Gptr const key_ptr, TagGptr &val_ptr, char *val,
                    size_t &val_len, bool get_value = false) {
        return -1;
    };

    // for non-cached Del
    // return 0 with key_ptr and val_ptr
    // - if key_ptr is null, key node is not found and there is no val_ptr
    // - if key_ptr is not null, key node is found, and its value pointer will
    // be set to null (even
    // if it was already null)
    // return -1 (error)
    virtual int Del(char const *key, size_t const key_len, Gptr &key_ptr,
                    TagGptr &val_ptr) {
        return -1;
    };

    // for cached Del using key ptr
    // return 0 with val_ptr (key node found and val_ptr is set to null)
    // the value pointer of the key node will be set to null (even if it wae
    // already null)
    virtual int Del(Gptr const key_ptr, TagGptr &val_ptr) {
        return -1;
    };

    virtual void ReportMetrics() {
        return;
    };

  protected:
    static size_t const default_heap_size_ = 1024 * 1024 * 1024; // 1024MB
};

} // namespace famradixtree

#endif
