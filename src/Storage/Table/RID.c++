#include<iostream>
#include"Storage/Table/RID.h"
using namespace std;


RID::RID(int page_id, int slot_num){
    this->page_id = page_id;
    this->slot_num = slot_num;
    this->actual_pair = pair(page_id,slot_num);
}

void RID::setRID(int page_id, int slot_num){
    this->page_id = page_id;
    this->slot_num = slot_num;
}

//actual rids are needed when updating a tuple with into a new place
void RID::updateActualPair(RID rid){

    this->actual_pair = pair(rid.getPageId(),rid.getSlotNum());
}

RID RID::getActualPair(){
    return RID(actual_pair.first, actual_pair.second);
}

int RID::getPageId(){return this->page_id;}
int RID::getSlotNum(){return this->slot_num;}



void RID::serialize(char *data){
    int offset{0};
    // serialize page_id
    memcpy(data+offset, &page_id, sizeof(int));
    offset+=sizeof(int);

    // serialize slot_num
    memcpy(data+offset, &slot_num, sizeof(int));
    offset+=sizeof(int);

    // serialize actual RID:
    memcpy(data+offset, &actual_pair.first, sizeof(int));
    offset+=sizeof(int);

    // serialize slot_num
    memcpy(data+offset, &actual_pair.second, sizeof(int));
    offset+=sizeof(int);
}


void RID::deserialize(char *data){
    int offset{0};

    // deserialize page_id
    memcpy(&this->page_id,data+offset, sizeof(int));
    offset+=sizeof(int);

    // deserialize slot_num
    memcpy(&this->slot_num,data+offset, sizeof(int));
    offset+=sizeof(int);

    // deserialize actual RID:
    memcpy(&this->actual_pair.first,data+offset, sizeof(int));
    offset+=sizeof(int);

    // deserialize slot_num
    memcpy(&this->actual_pair.second,data+offset, sizeof(int));
    offset+=sizeof(int);
}

int RID::getSerializedSize(){
    return 4*(sizeof(int));
}

void RID::print(){
    cout<<"RID : ("<<this->actual_pair.first<<", "<<this->actual_pair.second<<")"<<endl;
}

/*
int 
main(){
    cout<<"first rid"<<endl;
    RID rid1 = RID(1,1);
    //cout<<rid1.getPageId()<<endl;
    //cout<<rid1.getSlotNum()<<endl;
    rid1.print();
    // test serialization and deserialization
    char* data;
    rid1.serialize(data);
    RID des_rid(-1,-1);
    des_rid.deserialize(data);

    cout<<"deserialized rid"<<endl;
    
    //cout<<des_rid.getPageId()<<endl;
    //cout<<des_rid.getSlotNum()<<endl;
    des_rid.print();
    cout<<"des sized: "<<des_rid.getSerializedSize();
    assert(des_rid.getPageId() == rid1.getPageId() && des_rid.getSlotNum() == rid1.getSlotNum());
}
*/