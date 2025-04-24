#include "b_tree.h"

namespace nns {

// -----------------------------------------------------------------------------
//  BTree: b-tree to index hash values produced by qalsh
// -----------------------------------------------------------------------------
BTree::BTree()  // default constructor
{
    root_ = -1;
    file_ = NULL;
    root_ptr_ = NULL;
}

// -----------------------------------------------------------------------------
BTree::~BTree()  // destructor
{
    char *header = new char[file_->get_blocklength()];
    write_header(header);       // write root_ to header
    file_->set_header(header);  // write back to disk
    delete[] header;

    if (root_ptr_ != NULL) {
        delete root_ptr_;
        root_ptr_ = NULL;
    }
    if (file_ != NULL) {
        delete file_;
        file_ = NULL;
    }
}

// -----------------------------------------------------------------------------
void BTree::init(       // init a new tree
    int b_length,       // block length
    const char *fname)  // file name
{
    file_ = new BlockFile(b_length, fname);  // b-tree stores here

    // -------------------------------------------------------------------------
    //  init the first node to store
    //  (1) block length (page size of a node),
    //  (2) number of nodes (including both index node and leaf node),
    //  (3) root_ (address of root node)
    // -------------------------------------------------------------------------
    root_ptr_ = new BIndexNode();
    root_ptr_->init(0, this);
    root_ = root_ptr_->get_block();
    delete_root();
}

// -----------------------------------------------------------------------------
void BTree::init_restore(  // load the tree from a tree file
    const char *fname)     // file name
{
    FILE *fp = fopen(fname, "r");  // check whether the file exists
    if (!fp) {
        printf("tree file %s does not exist\n", fname);
        exit(1);
    }
    fclose(fp);

    // -------------------------------------------------------------------------
    //  it doesn't matter to initialize block length to 0. after reading file,
    //  the block length will be reinitialized by file.
    // -------------------------------------------------------------------------
    file_ = new BlockFile(0, fname);
    root_ptr_ = NULL;

    // -------------------------------------------------------------------------
    //  read the content after first 8 bytes of first block into header
    // -------------------------------------------------------------------------
    char *header = new char[file_->get_blocklength()];
    file_->read_header(header);  // read remain bytes from header
    read_header(header);         // init root_ from header
    delete[] header;
}

// -----------------------------------------------------------------------------
int BTree::bulkload(      // bulkload a tree from memory
    int n,                // number of entries
    const Result *table)  // hash table
{
    BIndexNode *index_child = NULL;
    BIndexNode *index_prev_nd = NULL;
    BIndexNode *index_act_nd = NULL;
    BLeafNode *leaf_child = NULL;
    BLeafNode *leaf_prev_nd = NULL;
    BLeafNode *leaf_act_nd = NULL;

    int id = -1;
    int block = -1;
    float key = MINREAL;

    // -------------------------------------------------------------------------
    //  build leaf node from hash table (level = 0)
    // -------------------------------------------------------------------------
    bool first_node = true;  // determine relationship of sibling
    int start_block = 0;     // position of first node
    int end_block = 0;       // position of last node

    for (int i = 0; i < n; ++i) {
        id = table[i].id_;
        key = table[i].key_;

        if (!leaf_act_nd) {
            leaf_act_nd = new BLeafNode();
            leaf_act_nd->init(0, this);

            if (first_node) {
                // init start_block
                first_node = false;
                start_block = leaf_act_nd->get_block();
            } else {
                // label sibling
                leaf_act_nd->set_left_sibling(leaf_prev_nd->get_block());
                leaf_prev_nd->set_right_sibling(leaf_act_nd->get_block());

                delete leaf_prev_nd;
                leaf_prev_nd = NULL;
            }
            end_block = leaf_act_nd->get_block();
        }
        leaf_act_nd->add_new_child(id, key);  // add new entry

        // if this node has been full, change next node to store entries
        if (leaf_act_nd->isFull()) {
            leaf_prev_nd = leaf_act_nd;
            leaf_act_nd = NULL;
        }
    }
    if (leaf_prev_nd != NULL) {
        delete leaf_prev_nd;
        leaf_prev_nd = NULL;
    }
    if (leaf_act_nd != NULL) {
        delete leaf_act_nd;
        leaf_act_nd = NULL;
    }

    // -------------------------------------------------------------------------
    //  build b-tree level by level
    //  stop condition: lastEndBlock == lastStartBlock (only one node, as root)
    // -------------------------------------------------------------------------
    int cur_level = 1;  // current level (leaf level is 0)
    int last_start_block = start_block;
    int last_end_block = end_block;

    while (last_end_block > last_start_block) {
        first_node = true;
        for (int i = last_start_block; i <= last_end_block; ++i) {
            block = i;
            if (cur_level == 1) {
                leaf_child = new BLeafNode();
                leaf_child->init_restore(this, block);
                key = leaf_child->get_key_of_node();
                delete leaf_child;
                leaf_child = NULL;
            } else {
                index_child = new BIndexNode();
                index_child->init_restore(this, block);
                key = index_child->get_key_of_node();
                delete index_child;
                index_child = NULL;
            }

            if (!index_act_nd) {
                index_act_nd = new BIndexNode();
                index_act_nd->init(cur_level, this);

                if (first_node) {
                    first_node = false;
                    start_block = index_act_nd->get_block();
                } else {
                    index_act_nd->set_left_sibling(index_prev_nd->get_block());
                    index_prev_nd->set_right_sibling(index_act_nd->get_block());
                    delete index_prev_nd;
                    index_prev_nd = NULL;
                }
                end_block = index_act_nd->get_block();
            }
            index_act_nd->add_new_child(key, block);  // add new entry

            // if this node has been full, change next node to store entries
            if (index_act_nd->isFull()) {
                index_prev_nd = index_act_nd;
                index_act_nd = NULL;
            }
        }
        // release the space
        if (index_prev_nd != NULL) {
            delete index_prev_nd;
            index_prev_nd = NULL;
        }
        if (index_act_nd != NULL) {
            delete index_act_nd;
            index_act_nd = NULL;
        }

        last_start_block = start_block;  // update info
        last_end_block = end_block;      // build b-tree of higher level
        ++cur_level;
    }
    root_ = last_start_block;  // update the root_

    if (index_prev_nd != NULL) delete index_prev_nd;
    if (index_act_nd != NULL) delete index_act_nd;
    if (index_child != NULL) delete index_child;
    if (leaf_prev_nd != NULL) delete leaf_prev_nd;
    if (leaf_act_nd != NULL) delete leaf_act_nd;
    if (leaf_child != NULL) delete leaf_child;

    return 0;
}

// -----------------------------------------------------------------------------
void BTree::load_root()  // load root of b-tree
{
    if (root_ptr_ == NULL) {
        root_ptr_ = new BIndexNode();
        root_ptr_->init_restore(this, root_);
    }
}

// -----------------------------------------------------------------------------
void BTree::delete_root()  // delete root of b-tree
{
    if (root_ptr_ != NULL) {
        delete root_ptr_;
        root_ptr_ = NULL;
    }
}

}  // end namespace nns
