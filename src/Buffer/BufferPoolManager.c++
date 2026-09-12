#include<iostream>
#include"Buffer/BufferPoolManager.h"
#define evict_using_lru true;

using namespace std;


BufferPoolManager::BufferPoolManager(DiskManager* dm){
    this->disk_manager = dm;
    this->lru = new LRU(BUFFER_SIZE);

    for(int i{};i<BUFFER_SIZE;i++){
        this->frames[i] = new char[PAGE_SIZE];

        this->is_dirty[i] = false;
        this->pages_ids[i]=-1;
        pin_count[i] = 0;
        this->free_frames_ids.push_back(i);
    }

}


char* BufferPoolManager::fetchPage(int page_id){
    
    if(page_id ==-1){
        return nullptr;
    }
    //cout<<"fetching page "<<page_id<<endl;
    int frame_id = -1;

    if(page_table.find(page_id) != page_table.end()){
        frame_id = page_table[page_id];

        //just to update the cache
        char* dummy = lru->get_frame(frame_id);
        
        return frames[frame_id];

    }

    if(free_frames_ids.empty()){

        
        // using LRU :
        #ifdef evict_using_lru
            frame_id  = lru->evict_frame();

        #else 
            // just evict the last frame
            frame_id = BUFFER_SIZE-1;
        #endif

        int old_page_id = pages_ids[frame_id];
        
        if(is_dirty[frame_id]){
            disk_manager->writePage(old_page_id, frames[frame_id]);
        }

        page_table.erase(old_page_id);
    }
    else{
    
        frame_id = free_frames_ids.back();
        free_frames_ids.pop_back();
    
    }

    char* buffer = frames[frame_id];

    disk_manager->readPage(page_id, buffer);

    PageHeader* header1 = reinterpret_cast<PageHeader*>(buffer);
    //cout<<"header 1 : "<<header1->page_id<<endl;
    //cout<<"header 1 : "<<header1->num_tuples<<endl;

    pages_ids[frame_id] = page_id;
    is_dirty[frame_id] = false;
    page_table[page_id] = frame_id;
    //pin_count[frame_id]++;
    lru->put_frame(frame_id, frames[frame_id]);
    return frames[frame_id];
}


int BufferPoolManager::newPage(){
    
    int frame_id = -1;

    if(free_frames_ids.empty()){
        
        // using LRU :
        #ifdef evict_using_lru
            frame_id  = lru->evict_frame();

        #else 
            // just evict the last frame
            frame_id = BUFFER_SIZE-1;
        #endif

        int old_page_id = pages_ids[frame_id];
        if(is_dirty[frame_id]){
            disk_manager->writePage(old_page_id, frames[frame_id]);
        }

        page_table.erase(old_page_id);
    }
    else{
    
        frame_id = free_frames_ids.back();
        free_frames_ids.pop_back();
    
    }

    //char* buffer = frames[frame_id];

    int new_page_id = (disk_manager->allocatePage())/PAGE_SIZE;
    disk_manager->readPage(new_page_id,frames[frame_id]);

    //cout<<"new page id :"<<new_page_id<<endl;


    pages_ids[frame_id] = new_page_id;
    is_dirty[frame_id] = false;
    page_table[new_page_id] = frame_id;
    //pin_count[frame_id]++;
    lru->put_frame(frame_id, frames[frame_id]);


    return new_page_id;
}









void BufferPoolManager::deletePage(int page_id){

    if(page_table.find(page_id) != page_table.end()){
        
        int frame_id = page_table[page_id];
        
        if(is_dirty[frame_id]){
            disk_manager->writePage(page_id, frames[frame_id]);
        }

        page_table.erase(page_id);
        pages_ids[frame_id] = -1;
        is_dirty[frame_id] = false;
        free_frames_ids.push_back(frame_id);

        lru->remove_frame(frame_id);

    }
}


void BufferPoolManager::markAsDirty(int page_id){
    int frame_id = page_table[page_id];
    is_dirty[frame_id] = true;
}


void BufferPoolManager::unpinPage(int page_id, bool is_dirty_flag) {
    if (page_table.find(page_id) == page_table.end()) return;

    int frame_id = page_table[page_id];
    if (pin_count[frame_id] > 0) {
        pin_count[frame_id]--;
    }
    
    if (is_dirty_flag) {
        is_dirty[frame_id] = true;
    }
}



BufferPoolManager::~BufferPoolManager(){
    cout<<"deleting BPM"<<endl;
    for(int i{};i<BUFFER_SIZE;i++){

        int page_id = pages_ids[i];

        if(pin_count[i] > 0) {
            cerr <<"Warning: Page"<<pages_ids[i] 
                      <<" still has "<<pin_count[i]<<" pins during shutdown."<<endl;
        }

        if(is_dirty[i] && page_id != -1)
            disk_manager->writePage(page_id, frames[i]);

        if(frames[i]!=nullptr)
            delete[] frames[i];
    }
    cout<<"BPM is deleted"<<endl;

}

////////////////////////////////////////////////


void testPersistence1(BufferPoolManager BPM) {
    char* buffer2 =  BPM.fetchPage(1);
    PageHeader* header1 = reinterpret_cast<PageHeader*>(buffer2);
    cout<<"header 2 : "<<header1->page_id<<endl;
    cout<<"header 2 : "<<header1->num_tuples<<endl;

    char* buffer3 =  BPM.fetchPage(2);
     header1 = reinterpret_cast<PageHeader*>(buffer2);
    cout<<"header 3 : "<<header1->page_id<<endl;
    cout<<"header 3 : "<<header1->num_tuples<<endl;


    char* buffer4 =  BPM.fetchPage(3);
     header1 = reinterpret_cast<PageHeader*>(buffer2);
    cout<<"header 4 : "<<header1->page_id<<endl;
    cout<<"header 4 : "<<header1->num_tuples<<endl;


    char* buffer5 =  BPM.fetchPage(4);
     header1 = reinterpret_cast<PageHeader*>(buffer2);
    cout<<"header 4 : "<<header1->page_id<<endl;
    cout<<"header 4 : "<<header1->num_tuples<<endl;


    char* buffer6 =  BPM.fetchPage(5);
     header1 = reinterpret_cast<PageHeader*>(buffer2);
    cout<<"header 5 : "<<header1->page_id<<endl;
    cout<<"header 5 : "<<header1->num_tuples<<endl;
}



void createTenPages(DiskManager* dm) {
    Page p1(1);
    Field f1(TYPE_STRING,"nader");
    Tuple t1({f1,f1});
    p1.insertTuple(t1);
    p1.insertTuple(t1);
    cout<<"t1 inserted"<<endl;

    const char* buffer1 = p1.getData();
    dm->writePage(1,buffer1);
    dm->writePage(2,buffer1);
    dm->writePage(3,buffer1);
    dm->writePage(4,buffer1);
    dm->writePage(5,buffer1);
    dm->writePage(6,buffer1);
    dm->writePage(7,buffer1);
    dm->writePage(8,buffer1);
    dm->writePage(9,buffer1);
    dm->writePage(10,buffer1);

    cout<<"done "<<endl;
}

/*
int main(){

    DiskManager* dm = new DiskManager("elfeel");
    BufferPoolManager BPM(dm);

    
    //createTenPages(dm);
    
    
    //testPersistence1(BPM);

    int new_page_id =  BPM.newPage();
    cout<<"new page id "<<new_page_id<<endl;

    new_page_id =  BPM.newPage();
    cout<<"new page id "<<new_page_id<<endl;
    dm->~DiskManager();
  


}
*/