/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
    struct Node {
        value_type data;
        Node* hash_next;
        Node* order_prev;
        Node* order_next;
        
        Node(const value_type& val) : data(val), hash_next(nullptr), order_prev(nullptr), order_next(nullptr) {}
        Node(value_type&& val) : data(std::move(val)), hash_next(nullptr), order_prev(nullptr), order_next(nullptr) {}
    };

    // Hash table implementation
    Node** buckets;
    size_t bucket_count;
    size_t element_count;
    
    // Linked list for insertion order
    Node* head;
    Node* tail;
    
    // Hash function and equality comparator
    Hash hash_func;
    Equal equal_func;
    
    // Constants
    static const size_t INITIAL_BUCKET_COUNT = 16;
    static const double LOAD_FACTOR_THRESHOLD;
    
    // Helper functions
    size_t get_bucket_index(const Key& key) const {
        return hash_func(key) % bucket_count;
    }
    
    void rehash(size_t new_bucket_count) {
        if (new_bucket_count == 0) new_bucket_count = INITIAL_BUCKET_COUNT;
        
        // Create new buckets
        Node** new_buckets = new Node*[new_bucket_count]();
        
        // Rehash all elements
        for (size_t i = 0; i < bucket_count; ++i) {
            Node* node = buckets[i];
            while (node) {
                Node* next = node->hash_next;
                size_t new_index = hash_func(node->data.first) % new_bucket_count;
                node->hash_next = new_buckets[new_index];
                new_buckets[new_index] = node;
                node = next;
            }
        }
        
        // Clean up old buckets
        delete[] buckets;
        buckets = new_buckets;
        bucket_count = new_bucket_count;
    }
    
    void ensure_capacity() {
        if (element_count >= bucket_count * LOAD_FACTOR_THRESHOLD) {
            rehash(bucket_count * 2);
        }
    }
    
public:
 
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		Node* node_ptr;
        const linked_hashmap* container;
        
        iterator(Node* node, const linked_hashmap* cont) : node_ptr(node), container(cont) {}
        
        friend class linked_hashmap;
        friend class const_iterator;
		
	public:
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the 
		// iterator points to. 
		// STL algorithms and containers may use these type_traits (e.g. the following 
		// typedef) to work properly. 
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;


		iterator() : node_ptr(nullptr), container(nullptr) {}
		iterator(const iterator &other) : node_ptr(other.node_ptr), container(other.container) {}
        
        iterator& operator=(const iterator &other) {
            if (this != &other) {
                node_ptr = other.node_ptr;
                container = other.container;
            }
            return *this;
        }
        
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
            iterator temp = *this;
            if (node_ptr) {
                node_ptr = node_ptr->order_next;
            } else {
                throw invalid_iterator();
            }
            return temp;
        }
        
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
            if (node_ptr) {
                node_ptr = node_ptr->order_next;
            } else {
                throw invalid_iterator();
            }
            return *this;
        }
        
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
            iterator temp = *this;
            if (node_ptr) {
                if (node_ptr == container->head) {
                    // Can't decrement past beginning
                    throw invalid_iterator();
                }
                node_ptr = node_ptr->order_prev;
            } else {
                // end() iterator, go to last element
                node_ptr = container->tail;
            }
            return temp;
        }
        
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
            if (node_ptr) {
                if (node_ptr == container->head) {
                    // Can't decrement past beginning
                    throw invalid_iterator();
                }
                node_ptr = node_ptr->order_prev;
            } else {
                // end() iterator, go to last element
                node_ptr = container->tail;
            }
            return *this;
        }
        
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return node_ptr->data;
        }
        
		bool operator==(const iterator &rhs) const {
            return node_ptr == rhs.node_ptr && container == rhs.container;
        }
        
		bool operator==(const const_iterator &rhs) const {
            return node_ptr == rhs.node_ptr && container == rhs.container;
        }
        
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
		bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return &(node_ptr->data);
        }
	};
 
	class const_iterator {
	private:
        const Node* node_ptr;
        const linked_hashmap* container;
        
        const_iterator(const Node* node, const linked_hashmap* cont) : node_ptr(node), container(cont) {}
        
        friend class linked_hashmap;
        friend class iterator;
		
	public:
		const_iterator() : node_ptr(nullptr), container(nullptr) {}
		const_iterator(const const_iterator &other) : node_ptr(other.node_ptr), container(other.container) {}
		const_iterator(const iterator &other) : node_ptr(other.node_ptr), container(other.container) {}
        
        const_iterator& operator=(const const_iterator &other) {
            if (this != &other) {
                node_ptr = other.node_ptr;
                container = other.container;
            }
            return *this;
        }
        
        // Implement similar methods as iterator
        const_iterator operator++(int) {
            const_iterator temp = *this;
            if (node_ptr) {
                node_ptr = node_ptr->order_next;
            } else {
                throw invalid_iterator();
            }
            return temp;
        }
        
        const_iterator& operator++() {
            if (node_ptr) {
                node_ptr = node_ptr->order_next;
            } else {
                throw invalid_iterator();
            }
            return *this;
        }
        
        const_iterator operator--(int) {
            const_iterator temp = *this;
            if (node_ptr) {
                if (node_ptr == container->head) {
                    // Can't decrement past beginning
                    throw invalid_iterator();
                }
                node_ptr = node_ptr->order_prev;
            } else {
                // end() iterator, go to last element
                node_ptr = container->tail;
            }
            return temp;
        }
        
        const_iterator& operator--() {
            if (node_ptr) {
                if (node_ptr == container->head) {
                    // Can't decrement past beginning
                    throw invalid_iterator();
                }
                node_ptr = node_ptr->order_prev;
            } else {
                // end() iterator, go to last element
                node_ptr = container->tail;
            }
            return *this;
        }
        
        const value_type& operator*() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return node_ptr->data;
        }
        
        const value_type* operator->() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return &(node_ptr->data);
        }
        
        bool operator==(const iterator &rhs) const {
            return node_ptr == rhs.node_ptr && container == rhs.container;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return node_ptr == rhs.node_ptr && container == rhs.container;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
	};
 
	/**
	 * TODO two constructors
	 */
	linked_hashmap() : bucket_count(INITIAL_BUCKET_COUNT), element_count(0), head(nullptr), tail(nullptr) {
        buckets = new Node*[bucket_count]();
    }
    
	linked_hashmap(const linked_hashmap &other) : bucket_count(other.bucket_count), element_count(0), head(nullptr), tail(nullptr) {
        buckets = new Node*[bucket_count]();
        try {
            for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
                insert(*it);
            }
        } catch (...) {
            clear();
            delete[] buckets;
            throw;
        }
    }
 
	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
        if (this != &other) {
            linked_hashmap temp(other);
            swap(temp);
        }
        return *this;
    }
 
    void swap(linked_hashmap& other) {
        std::swap(buckets, other.buckets);
        std::swap(bucket_count, other.bucket_count);
        std::swap(element_count, other.element_count);
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(hash_func, other.hash_func);
        std::swap(equal_func, other.equal_func);
    }
 
	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
        clear();
        delete[] buckets;
    }
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
        size_t index = get_bucket_index(key);
        Node* node = buckets[index];
        while (node) {
            if (equal_func(node->data.first, key)) {
                return node->data.second;
            }
            node = node->hash_next;
        }
        throw index_out_of_bound();
    }
    
	const T & at(const Key &key) const {
        size_t index = get_bucket_index(key);
        Node* node = buckets[index];
        while (node) {
            if (equal_func(node->data.first, key)) {
                return node->data.second;
            }
            node = node->hash_next;
        }
        throw index_out_of_bound();
    }
 
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
        iterator it = find(key);
        if (it != end()) {
            return it->second;
        }
        // Insert default constructed value
        T default_value = T();
        pair<iterator, bool> result = insert(value_type(key, default_value));
        return result.first->second;
    }
 
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
        return at(key);
    }
 
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
        return iterator(head, this);
    }
    
	const_iterator cbegin() const {
        return const_iterator(head, this);
    }
 
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
        return iterator(nullptr, this);
    }
    
	const_iterator cend() const {
        return const_iterator(nullptr, this);
    }
 
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
        return element_count == 0;
    }
 
	/**
	 * returns the number of elements.
	 */
	size_t size() const {
        return element_count;
    }
 
	/**
	 * clears the contents
	 */
	void clear() {
        Node* node = head;
        while (node) {
            Node* next = node->order_next;
            delete node;
            node = next;
        }
        head = tail = nullptr;
        element_count = 0;
        
        // Clear buckets
        for (size_t i = 0; i < bucket_count; ++i) {
            buckets[i] = nullptr;
        }
    }
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
        ensure_capacity();
        
        size_t index = get_bucket_index(value.first);
        
        // Check if key already exists
        Node* node = buckets[index];
        while (node) {
            if (equal_func(node->data.first, value.first)) {
                return pair<iterator, bool>(iterator(node, this), false);
            }
            node = node->hash_next;
        }
        
        // Create new node
        Node* new_node = new Node(value);
        
        // Add to hash chain
        new_node->hash_next = buckets[index];
        buckets[index] = new_node;
        
        // Add to order list
        if (!head) {
            head = tail = new_node;
        } else {
            tail->order_next = new_node;
            new_node->order_prev = tail;
            tail = new_node;
        }
        
        element_count++;
        return pair<iterator, bool>(iterator(new_node, this), true);
    }
 
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
        if (pos.container != this || pos.node_ptr == nullptr) {
            throw invalid_iterator();
        }
        
        Node* node = pos.node_ptr;
        
        // Remove from hash chain
        size_t index = get_bucket_index(node->data.first);
        Node* curr = buckets[index];
        Node* prev = nullptr;
        
        while (curr) {
            if (curr == node) {
                if (prev) {
                    prev->hash_next = curr->hash_next;
                } else {
                    buckets[index] = curr->hash_next;
                }
                break;
            }
            prev = curr;
            curr = curr->hash_next;
        }
        
        // Remove from order list
        if (node->order_prev) {
            node->order_prev->order_next = node->order_next;
        } else {
            head = node->order_next;
        }
        
        if (node->order_next) {
            node->order_next->order_prev = node->order_prev;
        } else {
            tail = node->order_prev;
        }
        
        delete node;
        element_count--;
    }
 
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
        return find(key) != cend() ? 1 : 0;
    }
 
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
        size_t index = get_bucket_index(key);
        Node* node = buckets[index];
        while (node) {
            if (equal_func(node->data.first, key)) {
                return iterator(node, this);
            }
            node = node->hash_next;
        }
        return end();
    }
    
	const_iterator find(const Key &key) const {
        size_t index = get_bucket_index(key);
        Node* node = buckets[index];
        while (node) {
            if (equal_func(node->data.first, key)) {
                return const_iterator(node, this);
            }
            node = node->hash_next;
        }
        return cend();
    }
};

// Define the static constant
template<class Key, class T, class Hash, class Equal>
const double linked_hashmap<Key, T, Hash, Equal>::LOAD_FACTOR_THRESHOLD = 0.75;

} // namespace sjtu

#endif
