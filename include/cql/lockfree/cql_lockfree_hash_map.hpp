/* 
 * File:   lockfree_hash_map.hpp
 * Author: mc
 *
 * Created on September 18, 2013, 1:20 PM
 */

#ifndef CQL_LOCKFREE_HASH_MAP_HPP_
#define	CQL_LOCKFREE_HASH_MAP_HPP_

//
// REQUIRES NEW C++11 STANDARD
// 
#include <functional>
#include <utility>
#include <cassert>
#include <iterator>

#include <cds/opt/hash.h>
#include <cds/container/michael_list_hp.h>
#include <cds/container/split_list_map.h>

namespace cql {
    template<typename TKey, typename TValue>
    struct cql_lockfree_hash_map_traits_t
        : public cds::container::split_list::type_traits 
    {

        typedef cds::container::michael_list_tag  ordered_list    ;   // what type of ordered list we want to use
        typedef std::hash<TKey>                   hash            ;   // hash functor for the key stored in split-list map

        // Type traits for our MichaelList class
        struct ordered_list_traits: public cds::container::michael_list::type_traits {
            typedef std::less<TKey> less;   // comparer that specifies order of list nodes
        };
    };
    
    template<typename TKey, typename TValue>
    class cql_lockfree_hash_map_t { 
    public:
        typedef
            cds::container::SplitListMap<cds::gc::HP, TKey, TValue, cql_lockfree_hash_map_traits_t<TKey, TValue> > 
            base_hash_map;
        
        typedef
            typename base_hash_map::iterator
            iterator;
        
        typedef
            typename base_hash_map::const_iterator
            const_iterator;
        
        private:
            
        // Functor copies value associated with given key 
        // in hash_map to storage pointed by param storage.
        struct copy_value_functor_t {
            inline
            copy_value_functor_t(TValue* storage)
                    : _storage(storage) 
            {
                assert(storage != NULL);
            };
            
            inline void
            operator ()(std::pair<TKey const, TValue> const& pair) {
                *_storage = pair.second;
            }
            
            inline void
            operator ()(bool is_new, std::pair<TKey const, TValue> const& pair) {
                // is_new is true when this function is called for newly
                // inserted item
                *_storage = pair.second;
            }
            
        private:
            TValue* const _storage;
        };
        
    public:
        
        cql_lockfree_hash_map_t() 
            : _map() { }
    
        cql_lockfree_hash_map_t(size_t expected_items_count, size_t load_factor = 1)
            : _map(expected_items_count, load_factor) { }  
        
        inline virtual
        ~cql_lockfree_hash_map_t() { }
        
        inline size_t
        size() const {
            return _map.size();
        }
        
        inline void
        clear() {
            _map.clear();
        }
    
        inline bool 
        try_add(TKey const& key, TValue const& value) {
            return _map.insert(key, value);
        }
        
        inline bool
        try_erase(TKey const& key, TValue* const value) {
            assert(value != NULL);
            copy_value_functor_t f(value);
            return _map.erase(key, f);
        }

		inline bool
		try_erase(TKey const& key) {
			return _map.erase(key);
		}
        
        inline bool
        try_get(TKey const& key, TValue* const value) {
            assert(value != NULL);
            copy_value_functor_t f(value);
            return _map.find(key, f);
        }
        
        // Inserts hash map key to container using back inserter.
        // How to use:
        // std::vector<TKey> keys;
        // map.unsafe_get_keys(back_inserter(keys));
        // 
        // Warning: This is called unsafe because it may not return all keys
        // contained in map.
        template<typename TOutputIterator>
        inline void
        get_keys(TOutputIterator b_inserter) {
            typename base_hash_map::iterator it = _map.begin();
            for(; it != _map.end(); ++it) {
                *b_inserter++ = it->first; // copy key
            }
        }
        
        template<typename TOutputIterator>
        inline void
        get_values(TOutputIterator b_inserter) {
            typename base_hash_map::iterator it = _map.begin();
            for(; it != _map.end(); ++it) {
                *b_inserter++ = it->second; // copy key
            }
        }
        
        // Retruns map iterator.
        // This may not traverse all hash map elements
        // due to concurrency issues.
        inline iterator
        begin() {
            return _map.begin();
        }
        
        inline iterator
        end() {
            return _map.end();
        }
        
        inline const_iterator
        cbegin() const {
            return _map.cbegin(); 
        }
        
        inline const_iterator
        cend() const {
            return _map.cend();
        }
        
    public:
        base_hash_map   _map;
    };
}

#endif	/* CQL_LOCKFREE_HASH_MAP_HPP_ */

