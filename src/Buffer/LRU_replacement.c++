#include<iostream>
#include"Buffer/LRU_replacement.h"



Frame::Frame(int key= -1, char* value =nullptr){
        this->key = key;
        this->value = value;
        next = nullptr;
        prev = nullptr;
}

LRU::LRU(int capacity){
    this->capacity = capacity;
    D_head = new Frame();
    D_tail = new Frame();

    D_head->next = D_tail;
    D_tail->prev = D_head;
}

void LRU::put_frame(int key, char* value){

            //cout<<"key : "<<nodes_map.size()<<" "<<capacity<<endl;

        if(frames_map.find(key) != frames_map.end()){
            Frame* curr_node = frames_map[key];
            curr_node->value = value;

            curr_node->prev->next = curr_node->next;
            curr_node->next->prev = curr_node->prev;

            Frame* head_next = D_head->next;

            head_next->prev = curr_node;
            curr_node->next = head_next;

            D_head->next = curr_node;
            curr_node->prev = D_head;
            return;
        }
        if(frames_map.size()==capacity){

            //cout<<D_tail->prev->key<<endl;
            Frame* deleted_node = D_tail->prev;
            frames_map.erase(deleted_node->key);

            //Node* prev_prev_tail = deleted_node->prev;
            deleted_node->prev->next = D_tail;
            D_tail->prev = deleted_node->prev;
            delete deleted_node;
             
        }

        Frame* new_node = new Frame(key,value);
        frames_map[key] = new_node;

        Frame* head_next = D_head->next;
        head_next->prev = new_node;
        new_node->next = head_next;

        D_head->next = new_node;
        new_node->prev = D_head;
}

char* LRU::get_frame(int key){

        if(frames_map.find(key) != frames_map.end()){
            Frame* target_node = frames_map[key];
            
            target_node->prev->next = target_node->next;
            target_node->next->prev = target_node->prev;

            Frame* next_node = D_head->next;

            next_node->prev = target_node;
            //next_node = target_node;
            target_node->prev = D_head;
            D_head->next = target_node;
            target_node->next = next_node;

            return frames_map[key]->value;
        }

        return nullptr;
}


int LRU::evict_frame(){
    Frame* evicted_frame = D_tail->prev;
    int frame_id = evicted_frame->key;
    frames_map.erase(frame_id);

    evicted_frame->prev->next = D_tail;
    D_tail->prev = evicted_frame->prev;

    delete evicted_frame;
    return frame_id;
}

void LRU::remove_frame(int key){
    Frame* evicted_frame = frames_map[key];

    int frame_id = evicted_frame->key;
    frames_map.erase(frame_id);

    evicted_frame->prev->next = evicted_frame->next;
    evicted_frame->next->prev = evicted_frame->prev;

    delete evicted_frame;
}

/*
int main(){
    LRU* lru = new LRU(5);

    char* frame1 = new char[5];
    memcpy(frame1,&"nader",5);

    lru->put_frame(1,frame1);
    char* get_frame1 = lru->get_frame(1);

    for(int i=0;i<5;i++)cout<<get_frame1[i];
    cout<<endl;

    //lru->evict_frame();
    lru->remove_frame(1);

    if(lru->frames_map.find(1) == lru->frames_map.end()){
        cout<<"frame 1 evicted successfully"<<endl;
    }
}
*/