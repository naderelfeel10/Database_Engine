#ifndef BPlusIndex_H
#define BPlusIndex_H

#include <algorithm>
#include <iostream>
#include <vector>
#include"Storage/Page/Field.h"
#include"Storage/Table/RID.h"
#include"Buffer/BufferPoolManager.h"

using namespace std;

struct Node;


struct Entry{
    Field key;
    RID value; // for leaf nodes only
    Node* next; // for inner nodes , this is a pointer to the (virtecal) node

    // to do
    void serialize(char* data);
    void deserialize(char* data);
    Entry();
    Entry(Field key, RID rid);

};

struct Node {

    bool isLeaf;
    std::vector<Entry> entries;// vector of entries used in leaf nodes
    vector<Field> keys;
    vector<Node*> children;
    vector<int> children_pages_ids;
    Node* next; // the sibling node (horizontal)
    // i need those for serialization, also for storage on the actual disk
    int curr_page_id;
    int next_page_id;

    // to do
    void serialize(char* data);
    void deserialize(char* data);

    Node(bool leaf);

};


class BPlusTree {
public:

    Node* root;
    // minimum degree
    int t;
    int meta_page_id{-1};
    BufferPoolManager* bpm;

    void saveBPlusTree();
    //save each node recursivly
    void saveNode(Node* node);

    void loadBPlusTree();
    Node* loadNode(int page_id);

    //function to split a child node
    void splitChild(Node* parent, int index, Node* child);

    //function to insert a key in a non-full node
    void insertNonFull(Node* node, Field key);
    
    void insertNonFull(Node* node, Field key, RID value);

    //function to remove a key from a node
    void remove(Node* node, Field key);

    //function to borrow a key from the previous sibling
    void borrowFromPrev(Node* node, int index);

    //function to borrow a key from the next sibling
    void borrowFromNext(Node* node, int index);

    // Function to merge two nodes
    void merge(Node* node, int index);

    // function To print the tree
    void printTree(Node* node, int level);


public:

    BPlusTree(int degree, int meta_page_id, BufferPoolManager* bpm) 
        :t(degree), root(nullptr), bpm(bpm), meta_page_id(meta_page_id) {}

    void insert(Field key);
    void insert(Field key, RID value);
    
    // to do : i need to change to return an Entry(done in findValue)
    bool search(Field key);
    vector<RID> findValue(Field key);

    void remove(Field key);
    vector<RID> rangeQuery(Field lower, Field upper);

    //to be implemented
    void deleteRangeQuery(Field lower, Field upper);
    
    void printTree(Node* node, int level, string indent, bool isLast);
    void printTree();
    void clear(Node* node);
    
    // to do
    void serialize(char* data);
    void deserialize(char* data);

};


#endif