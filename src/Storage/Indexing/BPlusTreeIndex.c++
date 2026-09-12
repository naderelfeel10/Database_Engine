#include<iostream>
#include"Storage/Indexing/BPlusTreeIndex.h"
using namespace std;



void Entry::serialize(char* data){
        int offset{0};

        this->key.serialize(data+offset);
        offset+=this->key.getSerializedSize();

        this->value.serialize(data+offset);
        offset+=this->value.getSerializedSize();

}

void Entry::deserialize(char* data){
        int offset{0};

        this->key.deserialize(data+offset);
        offset+=this->key.getSerializedSize();

        this->value.deserialize(data+offset);
        offset+=this->value.getSerializedSize();

        this->next = nullptr;
}

Entry::Entry():value(RID(-1,-1)) ,next(nullptr){

}

Entry::Entry(Field key, RID rid):key(key),value(rid),next(nullptr){

}




void Node::serialize(char* data) {
        int offset = sizeof(PageHeader);

        // save some meta data
        memcpy(data + offset, &this->isLeaf, sizeof(bool));
        offset += sizeof(bool);

        // if leaf save entries else if internal save keys
        int num_elements = this->isLeaf ? this->entries.size() : this->keys.size();
        memcpy(data + offset, &num_elements, sizeof(int));
        offset += sizeof(int);

        // Write the right horizontal sibling page id
        memcpy(data + offset, &this->next_page_id, sizeof(int));
        offset += sizeof(int);

        // saving the content 
        if (this->isLeaf) {
            // if leaf so we have to save all entries 
            for (auto& entry : this->entries) {
                entry.serialize(data + offset);
                offset += (entry.key.getSerializedSize() + entry.value.getSerializedSize()); 
            }
        } else {
            // inner : serialize the parallel Key and Children Page ID arrays
            //inner node has N keys and N+1 children pointers.
            for (size_t i = 0; i < this->keys.size(); ++i) {
                this->keys[i].serialize(data + offset);
                offset += this->keys[i].getSerializedSize();
            }

            // write out the raw integer child page ids
            for (size_t i = 0; i < this->children.size(); ++i) {

                int child_pid = this->children[i]->curr_page_id; 
                memcpy(data + offset, &child_pid, sizeof(int));
                offset += sizeof(int);
            }
        }
}


void Node::deserialize(char* data) {
        int offset = sizeof(PageHeader);
        
        // load meta 
        memcpy(&this->isLeaf, data + offset, sizeof(bool));
        offset += sizeof(bool);
        
        int num_elements = 0;
        memcpy(&num_elements, data + offset, sizeof(int));
        offset += sizeof(int);
        
        memcpy(&this->next_page_id, data + offset, sizeof(int));
        offset += sizeof(int);
        
        this->entries.clear();
        this->keys.clear();
        this->children.clear();
        this->next = nullptr;
        
        // load body
        if (this->isLeaf) {
            for (int i = 0; i < num_elements; ++i) {
                Entry entry;
                entry.deserialize(data + offset);
                offset += (entry.key.getSerializedSize() + entry.value.getSerializedSize()); 
                this->entries.push_back(entry);
            }

        }else{

            // if not leaf so we need keys
            for (int i = 0; i < num_elements; ++i) {
                Field key;
                key.deserialize(data + offset);
                offset += key.getSerializedSize();
                this->keys.push_back(key);
            }

            // we also need children page ids
            if (num_elements > 0) {
                for (int i = 0; i <= num_elements; ++i) {
                    int child_pid;
                    memcpy(&child_pid, data + offset, sizeof(int));
                    offset += sizeof(int);
                    this->children_pages_ids.push_back(child_pid);
                }
            }
        }
}

Node::Node(bool leaf = false): isLeaf(leaf), next(nullptr){

}



/*
// Implementation of splitChild function
void BPlusTree::splitChild(Node* parent, int index, Node* child) {
    Node* newChild = new Node(child->isLeaf);
    newChild->curr_page_id = bpm->newPage();

    // If it's a leaf, the "split point" is different
    if (child->isLeaf) {
        // copy entries to new leaf
        newChild->entries.assign(child->entries.begin() + t - 1, child->entries.end());
        child->entries.resize(t - 1);
        
        // use the first key of the NEW child as the separator in the parent
        parent->keys.insert(parent->keys.begin() + index, newChild->entries[0].key);
        
        // maintain leaf linked list
        newChild->next = child->next;
        child->next = newChild;
    } 
    else {

        parent->keys.insert(parent->keys.begin() + index, child->keys[t - 1]);
        newChild->keys.assign(child->keys.begin() + t, child->keys.end());
        child->keys.resize(t - 1);

        newChild->children.assign(child->children.begin() + t, child->children.end());
        child->children.resize(t);
    }
    
    parent->children.insert(parent->children.begin() + index + 1, newChild);
}
*/
/*
void BPlusTree::splitChild(Node* parent, int index, Node* child) {
    Node* newChild = new Node(child->isLeaf);
    newChild->curr_page_id = bpm->newPage();

    if (child->isLeaf) {
        // Copy entries to new leaf
        newChild->entries.assign(child->entries.begin() + t - 1, child->entries.end());
        child->entries.resize(t - 1);
        
        // CRUCIAL FIX: Keep the keys vectors synchronized!
        child->keys.clear();
        for (const auto& e : child->entries) child->keys.push_back(e.key);
        
        newChild->keys.clear();
        for (const auto& e : newChild->entries) newChild->keys.push_back(e.key);
        
        // Use the first key of the NEW child as the separator in the parent
        parent->keys.insert(parent->keys.begin() + index, newChild->entries[0].key);
        
        // Maintain leaf linked list
        newChild->next = child->next;
        child->next = newChild;
    } 
    else {
        parent->keys.insert(parent->keys.begin() + index, child->keys[t - 1]);
        newChild->keys.assign(child->keys.begin() + t, child->keys.end());
        child->keys.resize(t - 1);

        newChild->children.assign(child->children.begin() + t, child->children.end());
        child->children.resize(t);
        
        // Keep page IDs in sync if using them
        if (!child->children_pages_ids.empty()) {
            newChild->children_pages_ids.assign(child->children_pages_ids.begin() + t, child->children_pages_ids.end());
            child->children_pages_ids.resize(t);
        }
    }
    
    parent->children.insert(parent->children.begin() + index + 1, newChild);
    if (!parent->children_pages_ids.empty() || index <= parent->children_pages_ids.size()) {
        parent->children_pages_ids.insert(parent->children_pages_ids.begin() + index + 1, newChild->curr_page_id);
    }
}
*/
void BPlusTree::splitChild(Node* parent, int index, Node* child) {
    Node* newChild = new Node(child->isLeaf);
    newChild->curr_page_id = bpm->newPage();

    if (child->isLeaf) {
        newChild->entries.assign(child->entries.begin() + t - 1, child->entries.end());
        child->entries.resize(t - 1);
        
        child->keys.clear();
        for (const auto& e : child->entries) child->keys.push_back(e.key);
        
        newChild->keys.clear();
        for (const auto& e : newChild->entries) newChild->keys.push_back(e.key);
        
        parent->keys.insert(parent->keys.begin() + index, newChild->entries[0].key);
        
        newChild->next_page_id = child->next_page_id;
        newChild->next = child->next;
        child->next_page_id = newChild->curr_page_id;
        child->next = newChild;
    } 
    else {
        parent->keys.insert(parent->keys.begin() + index, child->keys[t - 1]);
        
        newChild->keys.assign(child->keys.begin() + t, child->keys.end());
        child->keys.resize(t - 1);

        newChild->children_pages_ids.assign(child->children_pages_ids.begin() + t, child->children_pages_ids.end());
        child->children_pages_ids.resize(t);

        newChild->children.assign(child->children.begin() + t, child->children.end());
        child->children.resize(t);
    }
    
    parent->children.insert(parent->children.begin() + index + 1, newChild);
    parent->children_pages_ids.insert(parent->children_pages_ids.begin() + index + 1, newChild->curr_page_id);

    // Persist modified structural variations back down to disk page instances
    char* child_buf = bpm->fetchPage(child->curr_page_id);
    child->serialize(child_buf);
    bpm->markAsDirty(child->curr_page_id);

    char* new_child_buf = bpm->fetchPage(newChild->curr_page_id);
    newChild->serialize(new_child_buf);
    bpm->markAsDirty(newChild->curr_page_id);
}


// Implementation of insertNonFull function

// to do : operator overloading >, < in Field Class (done)
void BPlusTree::insertNonFull(Node* node, Field key)
{
    if (node->isLeaf) {
        node->keys.insert(upper_bound(node->keys.begin(),
                                      node->keys.end(),
                                      key),
                          key);
    }
    else {
        int i = node->keys.size() - 1;
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insertNonFull(node->children[i], key);
    }
}



void BPlusTree::insertNonFull(Node* node, Field key, RID value) {
    if (node->isLeaf) {
        auto it = lower_bound(node->entries.begin(), node->entries.end(), key, 
            [](const Entry& e, const Field& k) { return e.key < k; });
        
        node->entries.insert(it, Entry(key, value));
        
        node->keys.clear();
        for (const auto& entry : node->entries) {
            node->keys.push_back(entry.key);
        }
        
        char* page_buffer = bpm->fetchPage(node->curr_page_id);
        node->serialize(page_buffer);
        bpm->markAsDirty(node->curr_page_id);
    } else {
        int i = node->keys.size() - 1;
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++; 

        if (node->children.size() <= i) {
            node->children.resize(node->children_pages_ids.size(), nullptr);
        }
        if (node->children[i] == nullptr) {
            node->children[i] = loadNode(node->children_pages_ids[i]);
        }
        
        Node* child = node->children[i];
        size_t child_size = child->isLeaf ? child->entries.size() : child->keys.size();

        if (child_size == 2 * t - 1) { 
            splitChild(node, i, child);
            if (key > node->keys[i]) {
                i++;
            }
            child = node->children[i];
        }
        
        insertNonFull(child, key, value);
        
        char* page_buffer = bpm->fetchPage(node->curr_page_id);
        node->serialize(page_buffer);
        bpm->markAsDirty(node->curr_page_id);
    }
}

/*
// Implementation of remove function
void BPlusTree::remove(Node* node, Field key)
{
    //if node is a leaf
    if (node->isLeaf) {
        // find the entry in the entries vector
        auto it = find_if(node->entries.begin(), node->entries.end(),
                               [&](const Entry& e) { return e.key == key; });

        // remove from entries
        if (it != node->entries.end()) {
            node->entries.erase(it);

            // 2. Also keep the 'keys' vector in sync if you use it in leaves
            auto kit = find(node->keys.begin(), node->keys.end(), key);
            if (kit != node->keys.end()) {
                node->keys.erase(kit);
            }
        }
    }
    else {
        int idx = lower_bound(node->keys.begin(),node->keys.end(), key) - node->keys.begin();

        if (idx < node->keys.size()&& node->keys[idx] == key) {

            if (node->children[idx]->keys.size() >= t) {
                Node* predNode = node->children[idx];
                while (!predNode->isLeaf) {
                    predNode = predNode->children.back();
                }
                Field pred = predNode->keys.back();
                node->keys[idx] = pred;
                remove(node->children[idx], pred);
            }

            else if (node->children[idx + 1]->keys.size()
                     >= t) {
                Node* succNode = node->children[idx + 1];
                while (!succNode->isLeaf) {
                    succNode = succNode->children.front();
                }
                Field succ = succNode->keys.front();
                node->keys[idx] = succ;
                remove(node->children[idx + 1], succ);
            }
            else {
                merge(node, idx);
                remove(node->children[idx], key);
            }
        }
        else {
            if (node->children[idx]->keys.size() < t) {
                if (idx > 0
                    && node->children[idx - 1]->keys.size()
                           >= t) {
                    borrowFromPrev(node, idx);
                }
                else if (idx < node->children.size() - 1
                         && node->children[idx + 1]
                                    ->keys.size()
                                >= t) {
                    borrowFromNext(node, idx);
                }
                else {
                    if (idx < node->children.size() - 1) {
                        merge(node, idx);
                    }
                    else {
                        merge(node, idx - 1);
                    }
                }
            }
            remove(node->children[idx], key);
        }
    }
}
*/

void BPlusTree::remove(Node* node, Field key)
{
    if (node->isLeaf) {
        auto it = find_if(node->entries.begin(), node->entries.end(),
                          [&](const Entry& e) { return e.key == key; });

        if (it != node->entries.end()) {
            node->entries.erase(it);

            node->keys.clear();
            for (const auto& e : node->entries) {
                node->keys.push_back(e.key);
            }
            
            // Mark leaf changes dirty
            char* page_buffer = this->bpm->fetchPage(node->curr_page_id);
            node->serialize(page_buffer);
            this->bpm->markAsDirty(node->curr_page_id);
        }
    }
    else {
        int idx = lower_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin();

        if (idx < node->children_pages_ids.size() && (node->children.size() <= idx || node->children[idx] == nullptr)) {

            if (node->children.size() <= idx) node->children.resize(node->children_pages_ids.size(), nullptr);
            node->children[idx] = loadNode(node->children_pages_ids[idx]);
        }


        if (idx < node->keys.size() && node->keys[idx] == key) {

            if ((idx + 1) < node->children_pages_ids.size() && (node->children.size() <= (idx + 1) || node->children[idx + 1] == nullptr)) {
                if (node->children.size() <= (idx + 1)) node->children.resize(node->children_pages_ids.size(), nullptr);
                node->children[idx + 1] = loadNode(node->children_pages_ids[idx + 1]);
            }

            if (node->children[idx]->keys.size() >= t) {

                Node* predNode = node->children[idx];

                while (!predNode->isLeaf) {

                    if (predNode->children.empty() || predNode->children.back() == nullptr) {
                        predNode->children.resize(predNode->children_pages_ids.size(), nullptr);
                        predNode->children[predNode->children_pages_ids.size() - 1] = loadNode(predNode->children_pages_ids.back());
                    }
                    predNode = predNode->children.back();
                }

                Field pred = predNode->keys.back();
                node->keys[idx] = pred;
                remove(node->children[idx], pred);
            }

            else if ((idx + 1) < node->children.size() && node->children[idx + 1]->keys.size() >= t) {

                Node* succNode = node->children[idx + 1];

                while (!succNode->isLeaf) {

                    if (succNode->children.empty() || succNode->children.front() == nullptr) {
                        succNode->children.resize(succNode->children_pages_ids.size(), nullptr);
                        succNode->children[0] = loadNode(succNode->children_pages_ids.front());
                    }

                    succNode = succNode->children.front();
                }

                Field succ = succNode->keys.front();
                node->keys[idx] = succ;
                remove(node->children[idx + 1], succ);
            }
            else {
                merge(node, idx);
                remove(node->children[idx], key);
            }
        }

        else {

            if (idx > 0 && (node->children.size() <= (idx - 1) || node->children[idx - 1] == nullptr)) {
                node->children[idx - 1] = loadNode(node->children_pages_ids[idx - 1]);
            }

            if ((idx + 1) < node->children_pages_ids.size() && (node->children.size() <= (idx + 1) || node->children[idx + 1] == nullptr)) {
                node->children[idx + 1] = loadNode(node->children_pages_ids[idx + 1]);
            }

            if (node->children[idx]->keys.size() < t) {
                if (idx > 0 && node->children[idx - 1]->keys.size() >= t) {
                    borrowFromPrev(node, idx);
                }

                else if (idx < (int)node->children.size() - 1 && node->children[idx + 1]->keys.size() >= t) {
                    borrowFromNext(node, idx);
                }

                else {
                    if (idx < (int)node->children.size() - 1) {
                        merge(node, idx);
                    }
                    else {
                        merge(node, idx - 1);
                        idx--;
                    }
                }
            }
            remove(node->children[idx], key);
        }

        // Save changed path tracking states down to memory block
        char* page_buffer = this->bpm->fetchPage(node->curr_page_id);
        node->serialize(page_buffer);
        this->bpm->markAsDirty(node->curr_page_id);
    }
}
/*
// Implementation of borrowFromPrev function
void BPlusTree::borrowFromPrev(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index - 1];

    child->keys.insert(child->keys.begin(),
                       node->keys[index - 1]);
    node->keys[index - 1] = sibling->keys.back();
    sibling->keys.pop_back();

    if (!child->isLeaf) {
        child->children.insert(child->children.begin(),
                               sibling->children.back());
        sibling->children.pop_back();
    }
}
*/

void BPlusTree::borrowFromPrev(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index - 1];

    if (child->isLeaf) {
        // Leaves share data via entries vector
        child->entries.insert(child->entries.begin(), sibling->entries.back());
        sibling->entries.pop_back();

        // Keep local key vector in sync with entries
        child->keys.clear();
        for (const auto& e : child->entries) child->keys.push_back(e.key);
        sibling->keys.clear();
        for (const auto& e : sibling->entries) sibling->keys.push_back(e.key);

        // Update parent splitting boundary key
        node->keys[index - 1] = child->entries.front().key;
    } 
    else {
        // Internal Nodes shift pure routing keys
        child->keys.insert(child->keys.begin(), node->keys[index - 1]);
        node->keys[index - 1] = sibling->keys.back();
        sibling->keys.pop_back();

        // Shift memory pointers
        child->children.insert(child->children.begin(), sibling->children.back());
        sibling->children.pop_back();

        if (!sibling->children_pages_ids.empty()) {
            child->children_pages_ids.insert(child->children_pages_ids.begin(), sibling->children_pages_ids.back());
            sibling->children_pages_ids.pop_back();
        }
    }

    char* child_buf = this->bpm->fetchPage(child->curr_page_id);
    child->serialize(child_buf);
    this->bpm->markAsDirty(child->curr_page_id);

    char* sib_buf = this->bpm->fetchPage(sibling->curr_page_id);
    sibling->serialize(sib_buf);
    this->bpm->markAsDirty(sibling->curr_page_id);
}



void BPlusTree::borrowFromNext(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index + 1];

    if (child->isLeaf) {
        child->entries.push_back(sibling->entries.front());
        sibling->entries.erase(sibling->entries.begin());

        // Keep local key vectors in sync
        child->keys.clear();
        for (const auto& e : child->entries) child->keys.push_back(e.key);
        sibling->keys.clear();
        for (const auto& e : sibling->entries) sibling->keys.push_back(e.key);

        // Update parent key to match the new configuration
        node->keys[index] = sibling->entries.front().key;
    } 
    else {
        child->keys.push_back(node->keys[index]);
        node->keys[index] = sibling->keys.front();
        sibling->keys.erase(sibling->keys.begin());

        child->children.push_back(sibling->children.front());
        sibling->children.erase(sibling->children.begin());

        if (!sibling->children_pages_ids.empty()) {
            child->children_pages_ids.push_back(sibling->children_pages_ids.front());
            sibling->children_pages_ids.erase(sibling->children_pages_ids.begin());
        }
    }

    // Mark changes to disk buffers
    char* child_buf = this->bpm->fetchPage(child->curr_page_id);
    child->serialize(child_buf);
    this->bpm->markAsDirty(child->curr_page_id);

    char* sib_buf = this->bpm->fetchPage(sibling->curr_page_id);
    sibling->serialize(sib_buf);
    this->bpm->markAsDirty(sibling->curr_page_id);
}



// Implementation of merge function
void BPlusTree::merge(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index + 1];

    if (child->isLeaf) {
        //leeaves collapse entries directly
        child->entries.insert(child->entries.end(), sibling->entries.begin(), sibling->entries.end());
        
        child->keys.clear();
        for (const auto& e : child->entries) child->keys.push_back(e.key);

        //maintain leaf linked list structure
        child->next = sibling->next;
    } 
    else {
        // Internal nodes pull down the separating key from the parent
        child->keys.push_back(node->keys[index]);
        child->keys.insert(child->keys.end(), sibling->keys.begin(), sibling->keys.end());

        child->children.insert(child->children.end(), sibling->children.begin(), sibling->children.end());
        
        if (!sibling->children_pages_ids.empty()) {
            child->children_pages_ids.insert(child->children_pages_ids.end(), 
                                             sibling->children_pages_ids.begin(), 
                                             sibling->children_pages_ids.end());
        }
    }

    //remove references from the parent
    node->keys.erase(node->keys.begin() + index);
    node->children.erase(node->children.begin() + index + 1);

    if (!node->children_pages_ids.empty()) {
        node->children_pages_ids.erase(node->children_pages_ids.begin() + index + 1);
    }

    //update child block structure on disk
    char* child_buf = this->bpm->fetchPage(child->curr_page_id);
    child->serialize(child_buf);
    this->bpm->markAsDirty(child->curr_page_id);

    delete sibling;
}


// Implementation of printTree function
void BPlusTree::printTree(Node* node, int level, std::string indent, bool isLast) {
    if (node == nullptr) return;

    std::cout << indent;
    if (level > 0) {
        cout << (isLast ? "└── " : "├── ");
        indent += (isLast ? "    " : "│   ");
    }

    if (node->isLeaf) {
        std::cout << " [LEAF - Page: " << node->curr_page_id << "] Data: [";
        for (size_t i = 0; i < node->entries.size(); ++i) {
            std::cout << node->entries[i].key.getFieldValueInt();
            if (i < node->entries.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << " [INTERNAL - Page: " << node->curr_page_id << "] Keys: ";
        for (Field& key : node->keys) {
            std::cout << "(" << key.getFieldValueInt() << ") ";
        }
        std::cout << std::endl;
    }

    if (!node->isLeaf) {
        for (size_t i = 0; i < node->children.size(); ++i) {
            bool last_child = (i == node->children.size() - 1);
            printTree(node->children[i], level + 1, indent, last_child);
        }
    }
}


void BPlusTree::printTree() {
    if (!root) {
        cout <<"Empty Tree"<<endl;
        return;
    }
    printTree(root, 0, "", true);
}



// to do : operator overloading for comparision in Field (done)
bool BPlusTree::search(Field key) {
    if (!root) return false;
    
    Node* current = root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i]) i++;
        
        if (current->children.size() <= i) current->children.resize(current->children_pages_ids.size(), nullptr);
        if (current->children[i] == nullptr) {
            current->children[i] = loadNode(current->children_pages_ids[i]);
        }
        current = current->children[i];
    }
    
    for (const auto& entry : current->entries) {
        if (entry.key == key) return true;
        if (entry.key > key) break;
    }
    return false;
}


vector<RID> BPlusTree::findValue(Field key) {
    vector<RID> results;
    if (root == nullptr) return results;

    Node* current = root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i]) i++;
        
        if (current->children.size() <= i) current->children.resize(current->children_pages_ids.size(), nullptr);
        if (current->children[i] == nullptr) {
            current->children[i] = loadNode(current->children_pages_ids[i]);
        }
        current = current->children[i];
    }

    bool keepScanning = true;
    while (current !=nullptr&&keepScanning){
        for (const auto& entry : current->entries) {
            if (entry.key == key) {
                results.push_back(entry.value);
            } else if (entry.key > key) {
                keepScanning = false;
                break;
            }
        }
        if (keepScanning) {
            int next_pid = current->next_page_id;
            if (next_pid <= -1) break;
            current = loadNode(next_pid); 
        }
    }
    return results;
}


// to do : change the return values into vector of Entries not keys (done)
vector<RID> BPlusTree::rangeQuery(Field lower, Field upper) {
    vector<RID> result;
    if (root == nullptr) return result;

    Node* current = root;
    //keep searching till you find the leaf nodes
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size() && lower > current->keys[i]) i++;
        
        if (current->children.size() <= i) current->children.resize(current->children_pages_ids.size(), nullptr);

        if (current->children[i] == nullptr) {
            current->children[i] = loadNode(current->children_pages_ids[i]);
        }
        current = current->children[i];
    }

    // walk through the leaf nodes and store matched keys in your range
    while (current != nullptr) {
        for (Entry& e : current->entries) {
            if (e.key >= lower && e.key <= upper) {
                result.push_back(e.value);
            }
            if (e.key > upper) return result;
        }
        int next_pid = current->next_page_id;
        if (next_pid <= -1) break;
        current = loadNode(next_pid);
    }
    return result;
}


void BPlusTree::insert(Field key)
{
    if (root == nullptr) {
        root = new Node(true);
        root->keys.push_back(key);
    }
    else {
        if (root->keys.size() == 2 * t - 1) {
            Node* newRoot = new Node();
            newRoot->children.push_back(root);
            splitChild(newRoot, 0, root);
            root = newRoot;
        }
        insertNonFull(root, key);
    }
}
//overloaded insert function
void BPlusTree::insert(Field key, RID value) {

    // if the root is null, means the tree is empty so i will create the root node
    if (root == nullptr) {

        root = new Node(true);
        root->curr_page_id = bpm->newPage();
        root->entries.push_back(Entry(key, value));
        root->keys.push_back(key);
        
        // manually fetch the page
        char* page_buffer = bpm->fetchPage(root->curr_page_id);
        root->serialize(page_buffer);
        bpm->markAsDirty(root->curr_page_id);

    } else {
        size_t root_size = root->isLeaf ? root->entries.size() : root->keys.size();
        if (root_size == 2 * t - 1) { 
            Node* newRoot = new Node(false);
            newRoot->curr_page_id = bpm->newPage();
            
            newRoot->children.push_back(root);
            newRoot->children_pages_ids.push_back(root->curr_page_id);
            
            splitChild(newRoot, 0, root);
            root = newRoot;
        }
        insertNonFull(root, key, value);
    }
}

/*
// insert overload funtion , for key,value insertion
void BPlusTree::insert(Field key, RID value) {
    if (root == nullptr) {
        root = new Node(true);
        root->curr_page_id = bpm->newPage();
        root->entries.push_back(Entry(key, value));
    } else {
        size_t root_size = root->isLeaf ? root->entries.size() : root->keys.size();
        if (root_size == 2 * t - 1) { 
            Node* newRoot = new Node(false);
            newRoot->curr_page_id = bpm->newPage();
            newRoot->children.push_back(root);
            splitChild(newRoot, 0, root);
            root = newRoot;
        }
        insertNonFull(root, key, value);
    }
}
*/

void BPlusTree::remove(Field key)
{
    if (root == nullptr) {
        return;
    }

    remove(root, key);

    if (root->keys.empty() && !root->isLeaf) {
        Node* tmp = root;
        root = root->children[0];
        delete tmp;
    }

}



void BPlusTree::clear(Node* node) {

    if (!node) return;
    if (!node->isLeaf) {
        for (Node* child : node->children) clear(child);
    }
    delete node;
}

/*
void BPlusTree::saveBPlusTree() {
    char* page_buffer = this->bpm->fetchPage(this->root->curr_page_id);
    int offset{sizeof(PageHeader)};

    memcpy(page_buffer + offset, &this->t, sizeof(int));
    offset += sizeof(int);

    // if the tree isn't empty, recursively save all internal nodes and leaves
    if (this->root != nullptr) {
        saveNode(this->root);
    }

}*/

void BPlusTree::saveBPlusTree() {
    //int offset = sizeof(PageHeader);
    int offset = 0;
    char* meta_buffer = this->bpm->fetchPage(this->meta_page_id);

    memcpy(meta_buffer + offset, &this->t, sizeof(int));
    offset += sizeof(int);

    int root_pid = (this->root != nullptr) ? this->root->curr_page_id : -1;
    memcpy(meta_buffer + offset, &root_pid, sizeof(int));
    offset += sizeof(int);

    this->bpm->markAsDirty(this->meta_page_id);

    if (this->root != nullptr) {
        saveNode(this->root);
    }
}



//save each node recursivly
void BPlusTree::saveNode(Node* node) {

    if (node == nullptr) return;
    // children page ids to serialze easily
    node->children_pages_ids.clear();

    if (!node->isLeaf) {
        for (Node* child : node->children) {
            if (child != nullptr) {
                node->children_pages_ids.push_back(child->curr_page_id);
            }
        }
    }

    char* page_buffer = this->bpm->fetchPage(node->curr_page_id);
    node->serialize(page_buffer);
    this->bpm->markAsDirty(node->curr_page_id);

    // i need to recursivly save all children nodes
    if (!node->isLeaf) {
        for (Node* child : node->children) {
            saveNode(child);
        }
    }
}


/*
void BPlusTree::loadBPlusTree() {
    char* page_buffer = this->bpm->fetchPage(this->root->curr_page_id);
    int offset{sizeof(PageHeader)};

    memcpy(&this->t, page_buffer + offset, sizeof(int));
    offset += sizeof(int);

    if (this->root != nullptr) {
        clear(this->root);
        this->root = nullptr;
    }
    
    // reconstruct all nodes
    if(this->root)
    if (this->root->curr_page_id != -1) {
        this->root = loadNode(this->root->curr_page_id);
    }
}
*/
/*
void BPlusTree::loadBPlusTree() {
    if (this->root == nullptr) return;

    int root_page_id = this->root->curr_page_id;

    char* page_buffer = this->bpm->fetchPage(root_page_id);
    int offset{sizeof(PageHeader)};

    memcpy(&this->t, page_buffer + offset, sizeof(int));
    offset += sizeof(int);

    //clear(this->root);
    this->root = nullptr;

    if (root_page_id != -1) {
        this->root = loadNode(root_page_id);
    }

}
*/

void BPlusTree::loadBPlusTree() {
    char* meta_buffer = this->bpm->fetchPage(this->meta_page_id);
    int offset{0};

    memcpy(&this->t, meta_buffer + offset, sizeof(int));
    offset += sizeof(int);

    int root_pid{-1};
    memcpy(&root_pid, meta_buffer + offset, sizeof(int));
    offset += sizeof(int);


    if (this->root != nullptr) {
        clear(this->root);
        this->root = nullptr;
    }

    //reconstruct full index 
    if (root_pid != -1) {
        this->root = loadNode(root_pid);
    }
}


Node* BPlusTree::loadNode(int page_id) {

    if (page_id <= -1){
        return nullptr;
    }
    char* page_buffer = this->bpm->fetchPage(page_id);

    // dummy node
    Node* node = new Node();
    node->curr_page_id = page_id;
    node->deserialize(page_buffer);

    if (!node->isLeaf) {
        for (int child_pid : node->children_pages_ids) {
            Node* child_node = loadNode(child_pid);
            if (child_node != nullptr) {
                node->children.push_back(child_node);
            }
        }
    }

    return node;
}





/*

int main(){

    DiskManager* dm = new DiskManager("tesBTreeDB");
    BufferPoolManager* bpm = new BufferPoolManager(dm);
    BPlusTree tree(3,bpm);

    // Insert elements
    Field f1(TYPE_INT, 10);
    Field f2(TYPE_INT, 20);
    Field f3(TYPE_INT, 6);
    Field f4(TYPE_INT, 7);
    Field f5(TYPE_INT, 15);
    Field f6(TYPE_INT, 25);
    Field f7(TYPE_INT, 30);
    Field f8(TYPE_INT, 10);
    Field f9(TYPE_INT, 60);
    Field f10(TYPE_INT, 100);

    RID rid1(1,1);
    RID rid2(1,2);
    RID rid3(1,3);
    RID rid4(1,4);
    RID rid5(1,5);
    RID rid6(1,6);
    RID rid7(1,7);
    RID rid8(1,8);
    RID rid9(1,9);
    RID rid10(1,10);

    tree.insert(f1, rid1);
    tree.insert(f2, rid2);
    tree.insert(f3, rid3);
    tree.insert(f4, rid4);
    tree.insert(f5, rid5);
    tree.insert(f6, rid6);
    tree.insert(f7, rid7);
    tree.insert(f8, rid8);
    tree.insert(f9, rid9);
    tree.insert(f10, rid10);


    cout << "B+ Tree after insertions:" << endl;
    tree.printTree();

    // Search for a key
    Field searchKey = f1;
    cout << "\nSearching for key " << searchKey.getFieldValueInt() << ": "
         << (tree.search(searchKey) ? "Found" : "Not Found")
         << endl;


    // Perform a range query
    Field lower = f3, upper = f10;
    vector<RID> rangeResult = tree.rangeQuery(lower, upper);
    cout << "\nRange query [" << lower.getFieldValueInt() << ", " << upper.getFieldValueInt()
         << "]: "<<endl;
    for (RID e : rangeResult) {
        //cout << key << " ";
        e.print();
    }
    cout << endl;

    // Remove a key
    Field removeKey = f4;
    tree.remove(removeKey);
    cout << "\nB+ Tree after removing " << removeKey.getFieldValueInt() << ":"
         << endl;
    tree.printTree();

    RID searchRID1 = tree.findValue(f6)[0];
    cout<<"RID : "<<"( "<<searchRID1.getActualPair().getPageId()<<", "<<searchRID1.getActualPair().getSlotNum()<<" )"<<endl;

    //RID searchRID2 = tree.findValue(f4)[0];
    //cout<<"RID : "<<"( "<<searchRID2.getActualPair().getPageId()<<", "<<searchRID2.getActualPair().getSlotNum()<<" )"<<endl;

    
    return 0;
}


*/


// testing for old load and store (not working for curr version)
/*
int main() {
    DiskManager* dm = new DiskManager("tesBTreeDB");
    BufferPoolManager* bpm = new BufferPoolManager(dm);
    BPlusTree tree(3,bpm); 
    
    // Insert elements
    Field f1(TYPE_INT, 10);
    Field f2(TYPE_INT, 20);
    Field f3(TYPE_INT, 6);
    Field f4(TYPE_INT, 7);
    Field f5(TYPE_INT, 15);
    Field f6(TYPE_INT, 25);
    Field f7(TYPE_INT, 30);
    Field f8(TYPE_INT, 10);
    Field f9(TYPE_INT, 60);
    Field f10(TYPE_INT, 100);

    RID rid1(1,1);
    RID rid2(1,2);
    RID rid3(1,3);
    RID rid4(1,4);
    RID rid5(1,5);
    RID rid6(1,6);
    RID rid7(1,7);
    RID rid8(1,8);
    RID rid9(1,9);
    RID rid10(1,10);

    tree.insert(f1, rid1);
    tree.insert(f2, rid2);
    tree.insert(f3, rid3);
    tree.insert(f4, rid4);
    tree.insert(f5, rid5);
    tree.insert(f6, rid6);
    tree.insert(f7, rid7);
    tree.insert(f8, rid8);
    tree.insert(f9, rid9);
    tree.insert(f10, rid10);


    cout << "B+ Tree after insertions:" << endl;
    tree.printTree();

    // Search for a key
    Field searchKey = f1;
    cout << "\nSearching for key " << searchKey.getFieldValueInt() << ": "
         << (tree.search(searchKey) ? "Found" : "Not Found")
         << endl;

    
    // test save and load
    cout << "\nExecuting Save Operations..." << endl;
    char metadata_buffer[512] = {0};
    tree.saveBPlusTree(metadata_buffer, bpm);

    cout <<"Wiping local runtime volatile allocations & flash testing RAM frames..."<<endl;

    tree.clear(tree.root);
    tree.root = nullptr;
    //bpm->flashVolatileRAMPool(); 

    cout << "Executing Restore Operation from Raw Blocks..." << endl;
    tree.loadBPlusTree(metadata_buffer, bpm);

    cout << "=== B+TREE AFTER SUCCESSFUL RELOAD ===" << endl;
    tree.printTree();

    cout << "running Data Assertions on reloaded tree structure" << endl;

    bool found_item = tree.search(Field(TYPE_INT, 15));
    cout << "Searching for Key (15) on Reloaded Index Tree: " << (found_item ? "SUCCESS" : "FAILED") << endl;
    assert(found_item == true);

    vector<RID> match = tree.findValue(Field(TYPE_INT, 60));
    if(!match.empty()){

        cout<<"verified Key (60)restored target value mapping -> page: "<<match[0].getPageId()<<", slot: "<<match[0].getSlotNum()<<endl;
        assert(match[0].getSlotNum() == 9);
    }
    cout<<"done"<<endl;

    delete bpm;
    return 0;
}
*/