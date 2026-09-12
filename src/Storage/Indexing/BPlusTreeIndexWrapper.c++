#include<iostream>
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
using namespace std;


void BPlusTreeIndexWrapper::Insert(Field&field,string col_name, vector<Column> tuple_cols, RID rid){

    if(field.getFieldType() != this->field_type){
        cerr<<"field types don't match"<<endl;
        return;
    }

    this->BPlusTreeIndex->insert(field,rid);
}



void BPlusTreeIndexWrapper::Delete(Field&field){
    if(field.getFieldType() != this->field_type){
        cerr<<"field types don't match"<<endl;
        return;
    }
    this->BPlusTreeIndex->remove(field);

}


vector<RID>  BPlusTreeIndexWrapper::Search(Field&field)const{

    if(field.getFieldType() != this->field_type){
        cerr<<"field types don't match"<<endl;
        //return RID(-1,-1);
    }
    return (this->BPlusTreeIndex->findValue(field));

}

vector<RID> BPlusTreeIndexWrapper::searchRange(Field lower, Field upper){

    if(lower.getFieldType() != this->field_type || upper.getFieldType() != this->field_type ){
        cerr<<"field types don't match"<<endl;
        return {RID(-1,-1)};
    }
    return (this->BPlusTreeIndex->rangeQuery(lower, upper));

}

void BPlusTreeIndexWrapper::displayIndexPages(){
    this->BPlusTreeIndex->printTree();
};

string BPlusTreeIndexWrapper::get_index_col_name(){
    return this->col_name;
}

BPlusTreeIndexWrapper::~BPlusTreeIndexWrapper() {

    this->BPlusTreeIndex->saveBPlusTree();
    cout<<"saving B+ tree"<<endl;
    
    if(this->BPlusTreeIndex->root != nullptr) {

        BPlusTreeIndex->clear(this->BPlusTreeIndex->root);
        this->BPlusTreeIndex->root = nullptr;
        
    }
}