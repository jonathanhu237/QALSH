#pragma once

#include <algorithm>
#include <iostream>

#include "def.h"

namespace nns {

// -----------------------------------------------------------------------------
struct Result {  // basic structure to store results
    float key_;  // distance / random projection value
    int id_;     // object id
};

// -----------------------------------------------------------------------------
inline int cmp(  // cmp func for lower_bound (ascending)
    Result a,    // 1st element
    Result b)    // 2nd element
{
    return a.key_ < b.key_;
}

// -----------------------------------------------------------------------------
int ResultComp(       // compare function for qsort (ascending)
    const void *e1,   // 1st element
    const void *e2);  // 2nd element

// -----------------------------------------------------------------------------
int ResultCompDesc(   // compare function for qsort (descending)
    const void *e1,   // 1st element
    const void *e2);  // 2nd element

// -----------------------------------------------------------------------------
//  MinK_List maintains the smallest k values (float) and the k object ids (int)
// -----------------------------------------------------------------------------
class MinK_List {
   public:
    MinK_List(int max);  // constructor (given max size)
    ~MinK_List();        // destructor

    // -------------------------------------------------------------------------
    inline void reset() { num_ = 0; }

    // -------------------------------------------------------------------------
    inline float min_key() { return (num_ > 0 ? list_[0].key_ : MAXREAL); }

    // -------------------------------------------------------------------------
    inline float max_key() { return (num_ >= k_ ? list_[k_ - 1].key_ : MAXREAL); }

    // -------------------------------------------------------------------------
    inline float ith_key(int i) { return (i < num_ ? list_[i].key_ : MAXREAL); }

    // -------------------------------------------------------------------------
    inline int ith_id(int i) { return (i < num_ ? list_[i].id_ : MININT); }

    // -------------------------------------------------------------------------
    inline int size() { return num_; }

    // -------------------------------------------------------------------------
    inline bool isFull() {
        if (num_ >= k_)
            return true;
        else
            return false;
    }

    // -------------------------------------------------------------------------
    float insert(   // insert item
        float key,  // key of item
        int id);    // id of item

   protected:
    int k_;         // max number of keys
    int num_;       // number of key current active
    Result *list_;  // the list itself
};

}  // end namespace nns
