#include<iostream>
#include"TableHeap.h"
using namespace std;


// upcomming updates :
/*
 1. split initiating new Table heap into a different function
 2. check first if the table exists before creating one
 3. loadMeta implementation 
*/
TableHeap::TableHeap(BufferPoolManager* BPM,int first_page_id=-1, int last_page_id=-1){
    this->BPM = BPM;
    // let first page for meta data
    
    
    if(first_page_id==-1 && last_page_id==-1){

        cout<<"new table"<<endl;
        this->first_page_id = BPM->newPage();
        this->last_page_id = BPM->newPage();

        // i added this to force the first page_id wich is required to contain table meta data, 
        //to point at it's next pag_id which contains actual tuples
        char* page_buffer = BPM->fetchPage(this->first_page_id);
        PageHeader* pageHeaer = reinterpret_cast<PageHeader*>(page_buffer);
        pageHeaer->next_page_id = this->last_page_id;
        BPM->markAsDirty(this->first_page_id);

        cout<<"page id "<<this->first_page_id<<" next page_id : "<<pageHeaer->next_page_id<<endl;
        cout<<this->last_page_id;

    }else{

        cout<<"existing table"<<endl;
        this->first_page_id = first_page_id;
        loadMetaData();
        /*
        this->first_page_id = first_page_id;
        char* page_buffer = BPM->fetchPage(this->first_page_id);
        PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
        cout<<"page id "<<first_page_id<<" next page_id : "<<pageHeader->next_page_id<<endl;
        cout<<this->last_page_id;
        */
    }
    

};


// insert a tuple into the table heap
RID TableHeap::insertTuple(Tuple tuple){
    

    //cout<<"last page id :"<<this->last_page_id<<endl;
    // seleect last page in the table
    char* page_buffer = BPM->fetchPage(this->last_page_id);
    PageHeader* pageHeaer = reinterpret_cast<PageHeader*>(page_buffer);

    Page* page = reinterpret_cast<Page*>(page_buffer);
    //Page* page = new (page_buffer)Page(last_page_id);
    
    // this returns -1 if there is no enough space
    int slot_num  = page->insertTuple(tuple);

    //cout<<pageHeaer->next_page_id<< " || " <<pageHeaer->page_id<<" || "<<pageHeaer->num_tuples<<endl;

    // we need to allocate new page
    if(slot_num == -1){
        //cout<<"allocating new page "<<endl;
        
        // create it
        int new_page_id = BPM->newPage();
        //update last page next pointer to point to the new page 
        pageHeaer->next_page_id = new_page_id;
        // marking as dirty to be written on disk latter
        BPM->markAsDirty(last_page_id);

        //cout<<pageHeaer->next_page_id<< " || " <<pageHeaer->page_id<<" || "<<pageHeaer->num_tuples<<endl;

        // fetch the new page
        char* new_page_buffer = BPM->fetchPage(new_page_id);
        
        PageHeader* new_pageHeader = reinterpret_cast<PageHeader*>(new_page_buffer);
        Page* new_page = reinterpret_cast<Page*>(new_page_buffer);

        //cout<<new_pageHeader->next_page_id<< " || " <<new_pageHeader->page_id<<" || "<<new_pageHeader->num_tuples<<endl;

        this->last_page_id = new_page_id;
        //cout<<"last vs new page ids : "<<this->last_page_id<<"  ;; "<<new_page_id<<endl;
        // insert the tuple .
        int new_slot_num  = new_page->insertTuple(tuple);
        BPM->markAsDirty(new_page_id);

        //cout<<"tuple is inserted successfuly , "<< new_page_id<<" "<<new_slot_num<<endl;
        //return RID(new_page_id, new_slot_num);

    }
    // else means there is enough space in this page
    BPM->markAsDirty(last_page_id); // just for the buffer pool to flush the page into the file 
    
    //cout<<"tuple is inserted successfuly ,,,, "<< last_page_id<<" "<<slot_num<<endl;
    num_of_tuples++;

    if(starting_rid.getActualPair().getPageId()==-1){
        starting_rid = RID(last_page_id, slot_num);
    }
    stopping_rid = RID(last_page_id, slot_num);
    
    //push to table rids
    this->table_rids.push_back(stopping_rid);
   
    return stopping_rid;
}


// return a tuple with this rid
Tuple* TableHeap::getTuple(RID rid){
    // fetch the page
    char* page_buffer = BPM->fetchPage(rid.getActualPair().getPageId());
    Page* page = reinterpret_cast<Page*>(page_buffer);

    // select this tuple from the page
    Tuple* tuple = new Tuple({});
    bool res = page->getTuple(rid.getActualPair().getSlotNum(),*tuple);

    //if the tuple is deleted
    if(res == false){
        return nullptr;
    }
    
    return tuple;
}


// we need the old rid, and the new tuple
RID TableHeap::updateTuple(RID rid, Tuple tuple){

    // select the page
    char* page_buffer = BPM->fetchPage(rid.getPageId());
    Page* page = reinterpret_cast<Page*>(page_buffer);

    // get the old tuple
    Tuple* old_tuple = this->getTuple(rid);

    // if new tuple size > old one size
    // we need to delete the old one, insert the new one in another place with another RID
    if(old_tuple->getTupleSize() < tuple.getTupleSize()){
      
        page->deleteTuple(rid.getSlotNum());
        RID new_rid = insertTuple(tuple);
        rid.updateActualPair(new_rid);
        
    }
    else{
        page->updateTuple(rid.getSlotNum(), tuple);
    }


    return rid;

}

//this function takes a list of rids instead of just one and updates the whole list with out extra function calls
vector<RID> TableHeap::updateTuple(vector<RID> rids, Tuple tuple){

    vector<RID>updated_rids;

    for(auto&rid: rids){
        // select the page
        char* page_buffer = BPM->fetchPage(rid.getPageId());
        Page* page = reinterpret_cast<Page*>(page_buffer);

        // get the old tuple
        Tuple* old_tuple = this->getTuple(rid);

        // if new tuple size > old one size
        // we need to delete the old one, insert the new one in another place with another RID
        if(old_tuple->getTupleSize() < tuple.getTupleSize()){
        
            page->deleteTuple(rid.getSlotNum());
            RID new_rid = insertTuple(tuple);
            rid.updateActualPair(new_rid);

        }
        else{
            page->updateTuple(rid.getSlotNum(), tuple);
        }

        updated_rids.push_back(rid);
    }

    return updated_rids;

}


void TableHeap::deleteTuple(RID rid){

    std::cout << "BPM = " << BPM << std::endl;
    std::cout << "page_id = " << rid.getPageId() << std::endl;
    std::cout << "slot_id = " << rid.getSlotNum() << std::endl;

    // select the targted page from the buffer
    char* page_buffer = BPM->fetchPage(rid.getPageId());
    if (page_buffer == nullptr) {
        cout << "ERROR: fetchPage returned nullptr for page "<< rid.getPageId() << std::endl;
        return;
    }
    Page* page = reinterpret_cast<Page*>(page_buffer);

    // just delete the selected slot num
    bool res = page->deleteTuple(rid.getActualPair().getSlotNum());
    num_of_tuples--;
}

bool TableHeap::deleteTupleBool(RID rid){

    // select the targted page from the buffer
    char* page_buffer = BPM->fetchPage(rid.getPageId());
    Page* page = reinterpret_cast<Page*>(page_buffer);

    // just delete the selected slot num
    bool res = page->deleteTuple(rid.getActualPair().getSlotNum());
    num_of_tuples--;
    return res;
}


void TableHeap::displayTablePages() {
    int next = this->first_page_id;
    //return;
    std::cout << "\n--- Table Heap Structure ---\n";
    
    while (next != -1) {
        char* page_buffer = BPM->fetchPage(next);
        PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
        
        // Print detailed info for each link in the chain
        std::cout << "[Page " << next << " | Tuples: " << header->num_tuples 
                  << " | Next: " << header->next_page_id << "]" << std::endl;
        
        //break;
        next = header->next_page_id;
    }
    std::cout << "--- End of Table ---\n";
}


//saves table meta data at first_page of the table 
void TableHeap::saveMetaData(){

    cout<<"saving table meta"<<endl;
    char* buffer = BPM->fetchPage(first_page_id);
    PageHeader* header = reinterpret_cast<PageHeader*>(buffer);

    int offset =sizeof(PageHeader);
    
    //1. save table name size
    int table_name_size = table_name.length();
    memcpy(buffer + offset , &table_name_size, sizeof(table_name_size));
    offset += sizeof(table_name_size);


    // save table name itself
    memcpy(buffer + offset, this->table_name.c_str(), table_name_size);
    offset += table_name_size;

    // save number of tuples
    memcpy(buffer+offset, &num_of_tuples, sizeof(int));
    offset+=sizeof(int);


    // save first , last page ids
    memcpy(buffer+offset , &this->first_page_id, sizeof(first_page_id));
    offset += sizeof(this->first_page_id);

    memcpy(buffer+offset , &this->last_page_id, sizeof(last_page_id));
    offset += sizeof(this->last_page_id);

    
    // save starting and stopping rids
    starting_rid.serialize(buffer+offset);
    offset+=starting_rid.getSerializedSize();

    stopping_rid.serialize(buffer+offset);
    offset+=stopping_rid.getSerializedSize();

    // save cols size
    int num_cols = this->cols.size();
    memcpy(buffer+offset , &num_cols, sizeof(num_cols));
    offset += sizeof(num_cols);


    // save each col
    for(auto& c : this->cols){
        c.printCol();
        switch (c.getColType()){
        case TYPE_INT:
            {buffer[offset] = 'I'; break;}
        case TYPE_STRING:
            {buffer[offset] = 'S'; break;}
        case TYPE_FLOAT:
            {buffer[offset] = 'F'; break;}
        case TYPE_BOOL:
            {buffer[offset] = 'B'; break;}
        default:
            cerr<<"incorrect field type"<<endl;
            break;
        }
        offset+=1;
        cout<<"from save table meta"<<endl;
        c.serializeCol(buffer+ offset);
        offset += c.getColSize();
    }

    // to do : save table indexes:
    // 1. save indexes_map :  map<string, vector<Index*> > indexes_map;
    // i will just save index_index, col_index, first_page_id

    //map< pair<col_name, index_t>, first_page_id>
    //map< pair<string,indexes_t>, Index_pages_struct*> indexes_pages_ids;

    int indexes_size = indexes_pages_ids.size();
    memcpy(buffer+offset, &indexes_size,sizeof(int));
    offset+=sizeof(int);

    for(const auto&[Pair, i]:indexes_pages_ids){

        cout<<"pair : "<<Pair.first<< " "<<Pair.second<<endl;
        int col_index{0};
        for(auto&col:this->cols){
            if(col.getColName() == Pair.first){
                break;
            }
            col_index++;
        }
        // save col_index
        memcpy(buffer+offset,&col_index,sizeof(int));
        offset+=sizeof(int);

        int index_type = static_cast<int>(Pair.second);
        //save index_index
        memcpy(buffer+offset,&index_type,sizeof(int));
        offset+=sizeof(int);

        //save index first pageid
        memcpy(buffer+offset,&i->index_first_page_id,sizeof(int));
        offset+=sizeof(int);

        //save index last pageid
        memcpy(buffer+offset,&i->index_last_page_id,sizeof(int));
        offset+=sizeof(int);
    }


    // fetch this page and write this data into it , then mark as dirty to be written on disk later
    BPM->markAsDirty(first_page_id);

    cout<<"table meta is saved"<<endl;

}


// to do(done)
void TableHeap::loadMetaData(){
    cout<<"loading table meta"<<endl;
    char* page_buffer = BPM->fetchPage(first_page_id);
    int offset{sizeof(PageHeader)};
    
    int table_name_size;
    memcpy(&table_name_size, page_buffer + offset, sizeof(table_name_size));
    offset += sizeof(table_name_size);

    // load table name itself
    this->table_name.assign(page_buffer + offset, table_name_size);
    offset += table_name_size;

    // load number of tuples
    memcpy(&this->num_of_tuples,page_buffer+offset, sizeof(int));
    offset+=sizeof(int);

    // load first , last page ids
    memcpy(&this->first_page_id, page_buffer+offset, sizeof(first_page_id));
    offset += sizeof(this->first_page_id);

    memcpy(&this->last_page_id, page_buffer+offset, sizeof(last_page_id));
    offset += sizeof(this->last_page_id);    
    cout<<"last page from load "<<this->last_page_id<<"=============================================================================="<<endl;


    // save starting and stopping rids
    starting_rid.deserialize(page_buffer+offset);
    offset+=starting_rid.getSerializedSize();

    stopping_rid.deserialize(page_buffer+offset);
    offset+=stopping_rid.getSerializedSize();

    // load cols size
    int num_cols{0};
    memcpy(&num_cols,page_buffer+offset, sizeof(num_cols));
    offset += sizeof(num_cols);

    //this->cols.resize(num_cols);
    // load each col

    for(int i=0;i<num_cols;i++){
        char type = page_buffer[offset];
        cout<<"Type : "<<type<<endl;
        FieldType field_type;
        switch (type)
        {
        case 'I':
            {field_type =  TYPE_INT; break;}
        case 'S':
            {field_type =  TYPE_STRING; break;}
        case 'F':
            {field_type =  TYPE_FLOAT; break;}
        case 'B':
            {field_type =  TYPE_BOOL; break;}
        default:
            cerr<<"invalid field type"<<endl;
            break;
        }
        offset+=1;
        Column c(field_type,"",1);
        c.deSerializeCol(page_buffer+ offset);
        //c.getField()->print();
        c.printCol();
        this->cols.push_back(c);
        offset += c.getColSize();
    }


        //map< pair<col_name, index_t>, first_page_id>
    //map< pair<string,indexes_t>, Index_pages_struct*> indexes_pages_ids;


    int indexes_size{0};
    memcpy(&indexes_size,page_buffer+offset, sizeof(int));
    offset+=sizeof(int);

    this->indexes_map.clear(); 

    for(int i=0;i<indexes_size;i++){
        cout<< i<<" index"<<endl;
        int col_index{0};
        // load col_index
        memcpy(&col_index,page_buffer+offset, sizeof(int));
        offset+=sizeof(int);

        Column col = this->cols[col_index];
        int index_type;

        //load index_index
        memcpy(&index_type,page_buffer+offset, sizeof(int));
        offset+=sizeof(int);

        int index_first_page_id{-1};
        //save index first pageid
        memcpy(&index_first_page_id,page_buffer+offset, sizeof(int));
        offset+=sizeof(int);

        int index_last_page_id{-1};
        //save index first pageid
        memcpy(&index_last_page_id,page_buffer+offset, sizeof(int));
        offset+=sizeof(int);

        Index_pages_struct* index_struct = new Index_pages_struct(index_first_page_id, index_last_page_id);
        indexes_pages_ids[pair(col.getColName(),static_cast<indexes_t>(index_type))] = index_struct;

        Index* index{nullptr};
        switch(index_type){
            case STATIC_HASH_INDEX:{
                hashIndex* h_index = new hashIndex(this->BPM,col.getColType(),1,1,1);
                h_index->set_first_page_id(index_first_page_id);
                h_index->loadIndexMeta();
                h_index->load_hash_table(this->first_page_id);
                cout<<"h_index capacity : "<< h_index->capacity<<endl;
                index = new  StaticHashIndexWrapper(BPM,h_index, col.getColName(), col.getColType());
                break;
            }
            case BPLUS_TREE_INDEX:{
                BPlusTree* B_tree_index = new BPlusTree(7,index_first_page_id,this->BPM);
                B_tree_index->loadBPlusTree();
                index = new BPlusTreeIndexWrapper(B_tree_index, col.getColName(), col.getColType());
                break;
            }
            default:
                cerr<<"index type is not identified"<<endl;
        }
        if(index != nullptr)
            this->indexes_map[col.getColName()].push_back(index);


    }
    
    this->table_rids.clear();
    this->setAllRIDs();


}


vector<Column> TableHeap::getCols(){
    return this->cols;
}


void TableHeap::setCols(vector<Column> cols){
    this->cols = std::move(cols);
}

int TableHeap::getColIndex(string col_name){
    int index{0};
    for(auto&col:this->cols){
        if(col.getColName()==col_name){
            return index;
        }
        index++;
    }
    return index;
}

string TableHeap::getTableName(){
    cout<<this->table_name<<endl;
    return this->table_name;
}


void TableHeap::setTableName(string table_name){
    this->table_name = table_name;
}

/*
void TableHeap::printColumns(){
    
    cout<<"nuumber of cols : "<< cols.size()<<endl;
    for(auto& col : cols){
        col.printCol();
        cout<<"----"<<endl;
    }
}
*/
void TableHeap::printColumns() {
    cout << "Columns Tuple (" << cols.size() << ") : [ ";
    
    for (size_t i = 0; i < cols.size(); ++i) {
        cols[i].printCol();
        
        // Add a comma separator between columns, but not after the last one
        if (i < cols.size() - 1) {
            cout << ", ";
        }
    }
    
    cout << " ]" << endl;
}
//indexes :
void TableHeap::createIndex(indexes_t index_type, string col_name,int index_size){

    int first_page_id =  indexes_pages_ids[make_pair(col_name, index_type)] ==nullptr?-1:indexes_pages_ids[make_pair(col_name, index_type)]->index_first_page_id;
    cout<<"creating index"<<endl;
    Index* index;
    // switch case for all kinds of indexes (for now it's just static hash index)
    int col_index{0};
    switch (index_type)
    {
        // first index
    case STATIC_HASH_INDEX:{
        //get col field type
        FieldType field_tye;
        for(auto& col: this->cols){
            if(col_name == col.getColName()){
                field_tye = col.getColType();
                break;
            }
            col_index++;
        }

        if(this->getIndex(col_name,index_type) == nullptr){
            
            // create a static hash index based on this field type, with needed size
            hashIndex* h_index = new hashIndex(this->BPM,field_tye,col_index,index_size,first_page_id);
            Index* index = new StaticHashIndexWrapper(this->BPM,h_index , col_name,field_tye); 
            indexes_map[col_name].push_back(index);

            Index_pages_struct* pages_struct = new Index_pages_struct(h_index->get_first_pageid(),h_index->get_last_pageid());
            indexes_pages_ids[pair(col_name, STATIC_HASH_INDEX)] = pages_struct;
        
        }

        break;
    }
    case BPLUS_TREE_INDEX:{

        FieldType fieldype;
        for(auto& col:this->cols){
            if(col.getColName() == col_name){
                fieldype = col.getColType();
            }
        }

        if(this->getIndex(col_name,index_type) == nullptr){
            int root_page_id = BPM->newPage();
            BPlusTree* b_index = new BPlusTree(index_size,root_page_id, this->BPM);
            index = new BPlusTreeIndexWrapper(b_index, col_name, fieldype);
            this->indexes_map[col_name].push_back(index);

            Index_pages_struct* pages_struct = new Index_pages_struct(root_page_id,-1);
            indexes_pages_ids[pair(col_name, BPLUS_TREE_INDEX)] = pages_struct;
        }
        
        break;
    }

    
    default:

        break;
    }
}


Index* TableHeap::getIndex(string col_name, indexes_t index_type) {
    if (this->indexes_map.find(col_name) == indexes_map.end()) {
        return nullptr;
    }

    vector<Index*>& indexes_vector = indexes_map[col_name];
    for (auto& index : indexes_vector) {
        
        if (index_type == STATIC_HASH_INDEX) {
            if (dynamic_cast<StaticHashIndexWrapper*>(index) != nullptr) {
                return index;
            }
        } 
        else if (index_type == BPLUS_TREE_INDEX) {
            if (dynamic_cast<BPlusTreeIndexWrapper*>(index) != nullptr) {
                return index;
            }
        }
    }

    return nullptr;
}


Tuple TableHeap::getTupleFromRID(RID rid){

    int page_id = rid.getActualPair().getPageId();
    int slot_num = rid.getActualPair().getSlotNum();

    char* buffer = BPM->fetchPage(page_id);
    Page* page = reinterpret_cast<Page*>(buffer);

    Tuple tmp_tuple = Tuple({});
    bool res = page->getTuple(slot_num,tmp_tuple);
    
    if(res==false)tmp_tuple=Tuple({});
    
    return tmp_tuple;
}

void TableHeap::deleteTableHeap(){
    //to delete the table:
    //1. walk through every page starting with first_page_id till last page_id
    int next_page_id = first_page_id;
    while(next_page_id!=-1){
        // fetch the page
        char* buffer = BPM->fetchPage(next_page_id);
        if(buffer == nullptr)return;

        PageHeader* header = reinterpret_cast<PageHeader*>(buffer);

        int curr_id = next_page_id;
        //move to the next one
        next_page_id = header->next_page_id;

        //delete it 
        BPM->deletePage(curr_id);
    }

    first_page_id =-1;
    last_page_id =-1;
}



vector<RID> TableHeap::setAllRIDs(){

    //start with the first page_id and loop throug each slot at it till finishes
    // then start the next page 
    int page_id = this->first_page_id;
    
    while(page_id != -1){

        char* page_buffer = BPM->fetchPage(page_id);
        Page* page = reinterpret_cast<Page*>(page_buffer);
        PageHeader* page_header = reinterpret_cast<PageHeader*>(page_buffer);

        if(page == nullptr) throw runtime_error("failed to fetch page");

        int num_slots = page_header->num_tuples;

        for(int slot_num = 0; slot_num < num_slots; slot_num++){
            //if the tuple is deleted, skip
            if(!page->is_deleted(slot_num)){
                this->table_rids.emplace_back(RID(page_id, slot_num));
            }
        }
        //fetch the next page 
        page_id = page_header->next_page_id;

    }
    for(auto&rid:this->table_rids)rid.print();

    return this->table_rids;
}


TableHeap::~TableHeap(){
    saveMetaData();
}


/*
int
main(){
    
    DiskManager* dm = new DiskManager("tableHeapDB");
    BufferPoolManager* BPM = new BufferPoolManager(dm);
    
    TableHeap table_heap = TableHeap(BPM);

    Field f1(TYPE_STRING,"nadermohamedelfeelisthebestevernadermohamedelfeelisthebestever");
    Tuple t1({f1,f1});
    
    for (int i = 1; i <= 100; ++i) {
        RID rid = table_heap.insertTuple(t1).getActualPair();
        if (rid.getPageId() != -1) {
            std::cout << "rid" << i << " : " 
                  << rid.getPageId() << " , " 
                  << rid.getSlotNum() << std::endl;
        } else {
        std::cerr << "Failed to insert tuple " << i << std::endl;
        }
    }
    
    table_heap.displayTablePages();

    Tuple* t2 = table_heap.getTuple(RID(1,1));
    t2->print();
    Tuple t3({f1,f1,f1});
    RID rid3 =  table_heap.updateTuple(RID(1,1),t3);
    t2 = table_heap.getTuple(rid3.getActualPair());
    t2->print();

    table_heap.deleteTuple(rid3.getActualPair());
    t2 = table_heap.getTuple(rid3.getActualPair());
    t2->print();
    
    dm->~DiskManager();
    BPM->~BufferPoolManager();   

}
*/