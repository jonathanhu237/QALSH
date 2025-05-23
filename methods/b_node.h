#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

#include "b_tree.h"
#include "block_file.h"
#include "def.h"

namespace nns {

class BTree;

// -----------------------------------------------------------------------------
//  BNode: basic structure of node in b-tree
// -----------------------------------------------------------------------------
class BNode {
   public:
    BNode();           // constructor
    virtual ~BNode();  // destructor

    // -------------------------------------------------------------------------
    virtual void init(  // init a new node, which not exist
        int level,      // level (depth) in b-tree
        BTree *btree);  // b-tree of this node

    // -------------------------------------------------------------------------
    virtual void init_restore(  // load an exist node from disk to init
        BTree *btree,           // b-tree of this node
        int block);             // address of file of this node

    // -------------------------------------------------------------------------
    virtual void read_from_buffer(const char *buf) {}

    // -------------------------------------------------------------------------
    virtual void write_to_buffer(char *buf) {}

    // -------------------------------------------------------------------------
    virtual inline int get_entry_size() { return 0; }

    // -------------------------------------------------------------------------
    virtual int find_position_by_key(float key) { return -1; }

    // -------------------------------------------------------------------------
    virtual inline float get_key(int index) { return key_[index]; }

    // -------------------------------------------------------------------------
    virtual BNode *get_left_sibling();  // get left sibling node

    virtual BNode *get_right_sibling();  // get right sibling node

    // -------------------------------------------------------------------------
    inline int get_block() { return block_; }

    // -------------------------------------------------------------------------
    inline int get_num_entries() { return num_entries_; }

    // -------------------------------------------------------------------------
    inline int get_level() { return level_; }

    // -------------------------------------------------------------------------
    //  level_: sizeof(char)
    //  num_entries_, left_sibling_, and right_sibling_: sizeof(int)
    //  get header size in b-node
    // -------------------------------------------------------------------------
    inline int get_header_size() { return sizeof(char) + sizeof(int) * 3; }

    // -------------------------------------------------------------------------
    inline float get_key_of_node() { return key_[0]; }

    // -------------------------------------------------------------------------
    inline bool isFull() {
        if (num_entries_ >= capacity_)
            return true;
        else
            return false;
    }

    // -------------------------------------------------------------------------
    inline void set_left_sibling(int left_sibling) { left_sibling_ = left_sibling; }

    // -------------------------------------------------------------------------
    inline void set_right_sibling(int right_sibling) { right_sibling_ = right_sibling; }

   protected:
    char level_;         // level of b-tree (level > 0)
    int num_entries_;    // number of entries in this node
    int left_sibling_;   // address in disk for left  sibling
    int right_sibling_;  // address in disk for right sibling
    float *key_;         // keys

    bool dirty_;    // if dirty, write back to file
    int block_;     // addr in disk for this node
    int capacity_;  // max num of entries can be stored
    BTree *btree_;  // b-tree of this node
};

// -----------------------------------------------------------------------------
//  BIndexNode: structure of index node in b-tree
// -----------------------------------------------------------------------------
class BIndexNode : public BNode {
   public:
    BIndexNode();           // constructor
    virtual ~BIndexNode();  // destructor

    // -------------------------------------------------------------------------
    virtual void init(  // init a new node, which not exist
        int level,      // level (depth) in b-tree
        BTree *btree);  // b-tree of this node

    virtual void init_restore(  // load an exist node from disk to init
        BTree *btree,           // b-tree of this node
        int block);             // address of file of this node

    // -------------------------------------------------------------------------
    virtual void read_from_buffer(  // read a b-node from buffer
        const char *buf);           // store info of a b-node

    virtual void write_to_buffer(  // write a b-node into buffer
        char *buf);                // store info of a b-node (return)

    // -------------------------------------------------------------------------
    //  entry: key_: sizeof(float) and son_: sizeof(int)
    // -------------------------------------------------------------------------
    virtual inline int get_entry_size() { return sizeof(float) + sizeof(int); }

    // -------------------------------------------------------------------------
    virtual int find_position_by_key(  // find pos just less than input key
        float key);                    // input key

    // -------------------------------------------------------------------------
    virtual inline float get_key(int index) {
        assert(index >= 0 && index < num_entries_);
        return key_[index];
    }

    // -------------------------------------------------------------------------
    virtual BIndexNode *get_left_sibling();  // get left sibling node

    virtual BIndexNode *get_right_sibling();  // get right sibling node

    // -------------------------------------------------------------------------
    inline int get_son(int index) {  // get son by index
        assert(index >= 0 && index < num_entries_);
        return son_[index];
    }

    // -------------------------------------------------------------------------
    void add_new_child(  // add new child by its child node
        float key,       // input key
        int son);        // input son

   protected:
    int *son_;  // address of son node
};

// -----------------------------------------------------------------------------
//  BLeafNode: structure of leaf node in b-tree
// -----------------------------------------------------------------------------
class BLeafNode : public BNode {
   public:
    BLeafNode();           // constructor
    virtual ~BLeafNode();  // destructor

    // -------------------------------------------------------------------------
    virtual void init(  // init a new node, which not exist
        int level,      // level (depth) in b-tree
        BTree *btree);  // b-tree of this node

    virtual void init_restore(  // load an exist node from disk to init
        BTree *btree,           // b-tree of this node
        int block);             // address of file of this node

    // -------------------------------------------------------------------------
    virtual void read_from_buffer(  // read a b-node from buffer
        const char *buf);           // store info of a b-node

    virtual void write_to_buffer(  // write a b-node into buffer
        char *buf);                // store info of a b-node (return)

    // -------------------------------------------------------------------------
    virtual inline int get_entry_size() { return sizeof(int); }

    // -------------------------------------------------------------------------
    virtual int find_position_by_key(  // find pos just less than input key
        float key);                    // input key

    // -------------------------------------------------------------------------
    virtual inline float get_key(int index) {
        assert(index >= 0 && index < num_keys_);
        return key_[index];
    }

    // -------------------------------------------------------------------------
    virtual BLeafNode *get_left_sibling();  // get left sibling node

    virtual BLeafNode *get_right_sibling();  // get right sibling node

    // -------------------------------------------------------------------------
    //  array of key_ with number capacity_keys_ + number_keys_
    // -------------------------------------------------------------------------
    inline int get_key_size(int block_length) {  // block length
        capacity_keys_ = (int)ceil((float)block_length / BTREE_LEAF_SIZE);
        return capacity_keys_ * sizeof(float) + sizeof(int);
    }

    // -------------------------------------------------------------------------
    inline int get_increment() { return BTREE_LEAF_SIZE / get_entry_size(); }

    // -------------------------------------------------------------------------
    inline int get_num_keys() { return num_keys_; }

    // -------------------------------------------------------------------------
    inline int get_entry_id(int index) {
        assert(index >= 0 && index < num_entries_);
        return id_[index];
    }

    // -------------------------------------------------------------------------
    void add_new_child(  // add new child by input id and key
        int id,          // input object id
        float key);      // input key

   protected:
    int num_keys_;  // number of keys
    int *id_;       // object id

    int capacity_keys_;  // max num of keys can be stored
};

}  // end namespace nns
