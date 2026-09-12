#include<iostream>
#include"Q_Execution/ExternalMergeSortExecuter.h"
using namespace std;


ExternalMergeSort::ExternalMergeSort(BufferPoolManager* BPM,AbstractExecuter* child_executer,Column sort_key,sorting_methods sorting_method):BPM(BPM),child_executer(child_executer)
                                                                                            ,sort_key(sort_key),sorting_method(sorting_method){
    this->open();
};

vector<Column> ExternalMergeSort::get_output_schema(){
    /*for(auto& col: this->child_executer->get_output_schema()){
        col.printCol();
    }*/
    return this->output_schema;
}

TableHeap* ExternalMergeSort::getTableHeap(){
    return this->child_executer->getTableHeap();
}

void ExternalMergeSort::open(){
    //most of sorting logic will be here:
    /*
    *1. open the child executer
    * 2. fetch the whole table from the child
    * 3. insert into pages
    * 4.sort them in memory
    *5. flush onto disk
     */
    this->child_executer->open();
    //set output schema
    this->output_schema = this->child_executer->get_output_schema();
    /*for(auto&col:this->child_executer->get_output_schema()){
        string col_name = this->child_executer->getTableHeap()->getTableName()+'.'+col.getColName();
        col.setColName(col_name);
        //col.printCol();
        this->output_schema.push_back(col);
    }*/
    Tuple tuple({});
    // we have to create a page to store upcomming tuples at it then locally sort it then write it on disk
    this->tmp_page_id = this->BPM->newPage();
    this->tmp_page_buffer = this->BPM->fetchPage(tmp_page_id);
    Page* page = reinterpret_cast<Page*>(tmp_page_buffer);
    
    int col_index = this->getTableHeap()->getColIndex(this->sort_key.getColName());
    //cout<<col_index<<", "<<this->sort_key.getColName();
    // fetching the every tuple from the child till the table is exausted
    while(this->child_executer->getNext(&tuple)){
        // insert the tuple into the page
        int slot_num = page->insertTuple(tuple);
        // check if the page is full
        if(slot_num == -1){
            // if full then : 
            write_run_buffer_on_disk(col_index, tuple);

            this->tmp_page_buffer = this->BPM->fetchPage(tmp_page_id);
            page = reinterpret_cast<Page*>(tmp_page_buffer);
                // clear the map
            run_buffer.clear();

            //retinsert faild tuple
            int slot_num = page->insertTuple(tuple);
            run_buffer.push_back(tuple);
            BPM->markAsDirty(tmp_page_id);
        }
        // if the page is not full:
        else{
            this->run_buffer.push_back(tuple);
        }
    }
    
    // after finishing, we have to check if run_buffer is empty
    // if not empty we have to write data into page
    if(!run_buffer.empty()){
        write_run_buffer_on_disk(col_index, tuple);
        run_buffer.clear(); 
    }

    // now each page is sorted locally
    // we need to merge each B-1 pages into one run


    // we have a Q of pages ids to merge
    // steps :
    // 1. pop first 2 pages
    // w. extract targted field to sort on
    //3. store each page fields into an array
    // 4.merge these 2 pages into a one sorted run 

    // i will stop when i have just one page left

    cout<<"pages_ids size : "<<pages_ids.size()<<endl;
    while(pages_ids.size()>1){

        //get pages ods from the Q
        int first_page_id = pages_ids.front();
        pages_ids.pop();
        int second_page_id = pages_ids.front();
        pages_ids.pop();

        //cout<<first_page_id<<" || "<<second_page_id<<" || "<<pages_ids.size()<<endl;


        // fetch pages
        char* left_page_buffer = this->BPM->fetchPage(first_page_id);
        PageHeader* left_page_header = reinterpret_cast<PageHeader*>(left_page_buffer);
        Page* left_page = reinterpret_cast<Page*>(left_page_buffer);

        ///
            int next_page_id1 = left_page_header->next_page_id;
            vector<int>left_run_pages_ids = {first_page_id};

            //cout<<first_page_id<<" | ";
            while(next_page_id1 != -1){
                //cout<<next_page_id1<<" | ";
                left_run_pages_ids.push_back(next_page_id1);
                char* page_buffer = this->BPM->fetchPage(next_page_id1);
                PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
                next_page_id1 = header->next_page_id;
            }
           // cout<<endl;
        ////
       // cout<<"left_size :"<<left_run_pages_ids.size()<<endl;

        char* right_page_buffer = this->BPM->fetchPage(second_page_id);
        PageHeader* right_page_header = reinterpret_cast<PageHeader*>(right_page_buffer);
        Page* right_page = reinterpret_cast<Page*>(right_page_buffer);
        vector<int>right_run_pages_ids = {second_page_id};

        ///
            next_page_id1 = right_page_header->next_page_id;

            //cout<<second_page_id<<" | ";
            while(next_page_id1 != -1){
                //cout<<next_page_id1<<" | ";
                right_run_pages_ids.push_back(next_page_id1);
                char* page_buffer = this->BPM->fetchPage(next_page_id1);
                PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
                next_page_id1 = header->next_page_id;
            }
            //cout<<endl;
        ////
        //cout<<"right_size :"<<right_run_pages_ids.size()<<endl;

        /*
        // i need to get all left&right run pages ids into an array
        vector<int>left_run_pages_ids = {first_page_id};
        int next_left = left_page_header->next_page_id;
        while(next_left!=-1){
            left_run_pages_ids.push_back(next_left);

            char* next_buffer = this->BPM->fetchPage(next_left);
            PageHeader* next_page_header = reinterpret_cast<PageHeader*>(next_buffer);

            next_left = next_page_header->next_page_id;
        }
        cout<<"left_size :"<<left_run_pages_ids.size()<<endl;

        vector<int>right_run_pages_ids = {second_page_id};
        int next_right = right_page_header->next_page_id;
        while(next_right!=-1){
            right_run_pages_ids.push_back(next_right);

            char* next_buffer = this->BPM->fetchPage(next_right);
            PageHeader* next_page_header = reinterpret_cast<PageHeader*>(next_buffer);

            next_right = next_page_header->next_page_id;
        }
        cout<<"right_size :"<<right_run_pages_ids.size()<<endl;

        */
        // prepare a vector of targted field to sort on
        // a  2d vector, each row contains fields of one page
        // rid of a tuple is (row_num, col_num)

        vector<vector<Field>> left_page_fields;
        for(int id : left_run_pages_ids){
            char* buf = this->BPM->fetchPage(id);
            Page* pg = reinterpret_cast<Page*>(buf);
            left_page_fields.push_back(pg->get_field_from_all_tuples(col_index));
        }

        vector<vector<Field>> right_page_fields;
        for(int id : right_run_pages_ids){
            char* buf = this->BPM->fetchPage(id);
            Page* pg = reinterpret_cast<Page*>(buf);
            right_page_fields.push_back(pg->get_field_from_all_tuples(col_index));
        }

        // i will merge those 2 arrays and store actual tuples in new pages
        
        //prepare the new run to insert data in
        int new_page_id = this->BPM->newPage();
        char* new_page_buffer = this->BPM->fetchPage(new_page_id);
        PageHeader* new_page_header = reinterpret_cast<PageHeader*>(new_page_buffer);
        Page* new_page = reinterpret_cast<Page*>(new_page_buffer);

        // i have to push it into pages_ids to be considered in upcomming merges
        this->pages_ids.push(new_page_id);
        //cout<<"pushing :"<<new_page_id<<endl;

        //merge
        int left{0},right{0};

        int left_size{0};
        int right_size{0};

        for(auto& row:left_page_fields){
            left_size+=row.size();
        }
        for(auto& row:right_page_fields){
            right_size+=row.size();
        }

        this->BPM->markAsDirty(new_page_id);

        int left_page_id{0};
        int left_slot_num{0};
        
        int right_page_id{0};
        int right_slot_num{0};
        
        while(left<left_size && right<right_size){

            // sorting asc :
            if(sorting_method == ASC){
                // prepare the tuple to insert
                //left_page_id = left/left_page_fields.size();
                //left_slot_num = left%left_page_fields[left_page_id].size();
                //if(left_slot_num ==0 && left_page_id!=0){
                //    left_page_id++;
                //}
                
                //left_page_id = left/left_page_fields.size();
                //right_slot_num = right%right_page_fields[right_page_id].size();
                //    right_page_id++;
                //}


                if(left_page_fields[left_page_id][left_slot_num] < right_page_fields[right_page_id][right_slot_num]){

                    int dummy_right{-1};
                    insert_tuple(new_page_id,new_page,new_page_header, left_run_pages_ids[left_page_id], left_slot_num, left, dummy_right);
                    
                    left_slot_num++;
                    if(left_slot_num >= left_page_fields[left_page_id].size()){
                        left_slot_num = 0;
                        left_page_id++;
                    }

                }else{

                    int dummy_left{-1};
                    insert_tuple(new_page_id,new_page,new_page_header, right_run_pages_ids[right_page_id], right_slot_num, dummy_left, right);
                    
                    right_slot_num++;
                    if(right_slot_num >= right_page_fields[right_page_id].size()){
                        right_slot_num = 0;
                        right_page_id++;
                    }

                }


            //////
            }else if(sorting_method == DESC){
                // prepare the tuple to insert

                if(left_page_fields[left_page_id][left_slot_num] > right_page_fields[right_page_id][right_slot_num]){

                    int dummy_right{-1};
                    insert_tuple(new_page_id,new_page,new_page_header, left_run_pages_ids[left_page_id], left_slot_num, left, dummy_right);
                    
                    left_slot_num++;
                    if(left_slot_num >= left_page_fields[left_page_id].size()){
                        left_slot_num = 0;
                        left_page_id++;
                    }
                }else{

                    int dummy_left{-1};
                    insert_tuple(new_page_id,new_page,new_page_header, right_run_pages_ids[right_page_id], right_slot_num, dummy_left, right);
                    
                    right_slot_num++;
                    if(right_slot_num >= right_page_fields[right_page_id].size()){
                        right_slot_num = 0;
                        right_page_id++;
                    }                
                }

            }



        }

        // copy the rest
        while(left<left_size){
            int dummy_right{-1};
            insert_tuple(new_page_id,new_page,new_page_header, left_run_pages_ids[left_page_id], left_slot_num, left, dummy_right);
            
            left_slot_num++;
            if(left_slot_num >= left_page_fields[left_page_id].size()){
                left_slot_num = 0;
                left_page_id++;
            }
            
        }

        while(right<right_size){            
            int dummy_left{-1};
            insert_tuple(new_page_id,new_page,new_page_header, right_run_pages_ids[right_page_id], right_slot_num, dummy_left, right);
            
            right_slot_num++;
            if(right_slot_num >= right_page_fields[right_page_id].size()){
                right_slot_num = 0;
                right_page_id++;
            }  
        }

    
        // after merging the 2 pages, no need for them so i will just delete them
        
        /*while(first_page_id !=-1){
            char* page_buffer = BPM->fetchPage(first_page_id);
            PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
            this->BPM->deletePage(first_page_id);
            first_page_id = header->next_page_id;
        }

        while(second_page_id !=-1){
            this->BPM->deletePage(second_page_id);
            second_page_id = right_page_header->next_page_id;
        }*/

    }


    // prepare pages RIDs for getNext function
    int page_id = this->pages_ids.front();
    curr_rid_pointer = RID(page_id, 0);
    char* page_buffer = this->BPM->fetchPage(page_id);
    PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
    int next_page_id = header->next_page_id;

    //cout<<page_id<<" | ";
    while(next_page_id != -1){
        page_id = next_page_id;
        //cout<<page_id<<" | ";
        char* page_buffer = this->BPM->fetchPage(next_page_id);
        PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
        next_page_id = header->next_page_id;
    }
    //cout<<endl;

}


void ExternalMergeSort::insert_tuple(int &new_page_id,Page*& new_page,PageHeader*& new_page_header, int page_id, int slot_num, int &left, int &right){
        // prepare the tuple to insert
        
        char* page_buffer = this->BPM->fetchPage(page_id);
        Page* page = reinterpret_cast<Page*>(page_buffer);
        
        Tuple tuple({});
        page->getTuple(slot_num, tuple);

        char* new_buf = this->BPM->fetchPage(new_page_id);
        new_page = reinterpret_cast<Page*>(new_buf);
        new_page_header = reinterpret_cast<PageHeader*>(new_buf);

        int new_slot_num = new_page->insertTuple(tuple);
        this->BPM->markAsDirty(new_page_id);

        // check if the page is full 
        if(new_slot_num == -1){
            int next_page_id = this->BPM->newPage();

            //refetch current output page again before writing next_page_id into header
            new_buf = this->BPM->fetchPage(new_page_id);
            new_page_header = reinterpret_cast<PageHeader*>(new_buf);
            new_page_header->next_page_id = next_page_id;
            this->BPM->markAsDirty(new_page_id);

            char* next_buf = this->BPM->fetchPage(next_page_id);
            new_page_id = next_page_id;
            new_page = reinterpret_cast<Page*>(next_buf);
            new_page_header = reinterpret_cast<PageHeader*>(next_buf);

            new_page->insertTuple(tuple);
            this->BPM->markAsDirty(new_page_id);
        }
        // advace pointer
        if(left == -1){
            right++;
        }else{
            left++;
        }
}


void ExternalMergeSort::write_run_buffer_on_disk(int col_index, Tuple& tuple){
    

    // create new page
    int page_id2 = this->BPM->newPage();
    char* page_buffer2 = this->BPM->fetchPage(page_id2);
    Page* page2 = reinterpret_cast<Page*>(page_buffer2);

    this->tmp_page_buffer = this->BPM->fetchPage(tmp_page_id);
    Page* page = reinterpret_cast<Page*>(tmp_page_buffer);

    // sort the vector based on sorting key:
    // if asc
    if(sorting_method == ASC){
        sort(run_buffer.begin(), run_buffer.end(), [col_index](const Tuple &a , const Tuple &b){
            return a.fields[col_index] < b.fields[col_index];
        });
    // else if desc
    }else if (sorting_method == DESC){
        sort(run_buffer.begin(), run_buffer.end(), [col_index](const Tuple &a , const Tuple &b){
            return a.fields[col_index] > b.fields[col_index];
        });
    }
    // insert sorted tuples from map
    // insert into the actual page that is gonna be flushed on disk for upcomming merging
    for(auto&v : run_buffer){
        int slot_num = page2->insertTuple(v);
        if(slot_num == -1){
            cerr<<"stooooooop"<<endl;
            
        }
    }

    // flush the page on disk
    BPM->markAsDirty(page_id2);
    // i need to keep track of pages i used
    pages_ids.push(page_id2);

    //RID rid = RID(page_id2, 0);
    //pages_rids.push_back(rid);


    //clear the tmp page to be ready for the next batch
    PageHeader* header = reinterpret_cast<PageHeader*>(tmp_page_buffer);
    header->num_tuples = 0;
    header->free_space_pointer = PAGE_SIZE;
    memset(tmp_page_buffer + sizeof(PageHeader), 0, PAGE_SIZE - sizeof(PageHeader));

}



// after preprocessing and sorting the table, just return the next tuple in thr normal way
bool ExternalMergeSort::getNext(Tuple*tuple){


    int curr_page_id = this->curr_rid_pointer.getPageId();
    int curr_slot_num = this->curr_rid_pointer.getSlotNum();

    //int lst_page_id = this->last_rid_pointer.getPageId();

    // fetch the page 
    char* page_buffer = this->BPM->fetchPage(curr_page_id);
    if (page_buffer == nullptr) {
        return false; 
    }
    PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    int num_of_tuples = pageHeader->num_tuples;

    int next_page_id = pageHeader->next_page_id;
    int next_slot_num = curr_rid_pointer.getSlotNum()+1;

    // if this is the last slot in the page
    // then increment page_id
    if(curr_slot_num >= num_of_tuples ){
        //this->curr_rid_pointer = RID(next_page_id,0);
        curr_page_id=next_page_id;
        //if next page is beyond the last page in the table
        // then return EOF 
        if(curr_page_id==-1){
            tuple = nullptr;
            return false;
        }else{
            // if it's not the last page in the table, then get the first slot in it
            this->curr_rid_pointer = RID(next_page_id,0);
            
            this->getTuple(this->curr_rid_pointer, *tuple);
            //tuple->print();
            this->curr_rid_pointer = RID(next_page_id,1);
            return true;
        }

    }
    this->getTuple(this->curr_rid_pointer, *tuple);
    //tuple->print();
    this->curr_rid_pointer = RID(curr_page_id,next_slot_num);
    return true;

}


void ExternalMergeSort::getTuple(RID rid, Tuple& tuple) {

    char* page_buffer = this->BPM->fetchPage(rid.getPageId());
    Page* page = reinterpret_cast<Page*>(page_buffer);

    page->getTuple(rid.getSlotNum(), tuple);
}

void ExternalMergeSort::close(){
    this->child_executer->close();
    
}

bool ExternalMergeSort::has_column(string col_name){
    string table_name = this->child_executer->getTableHeap()->getTableName();
    for(auto&col:this->get_output_schema()){
        if(table_name+'.'+col.getColName()==col_name)return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
/*
vector<string> tables;
map<string, TableHeap*>tables_map;
map<string, vector<RID>> tables_rids;
map<string, StaticHashIndexWrapper*>tables_indexes;
map<string, BPlusTreeIndexWrapper*>tables_B_indexes;


void createUserTable(BufferPoolManager* BPM, string table_name){
    tables.push_back(table_name);

    Column t1_col1 = Column(TYPE_INT, "user_id", sizeof(int));
    Column t1_col2 = Column(TYPE_STRING, "firstName", 30);
    Column t1_col3 = Column(TYPE_STRING, "lastName", 30);
    Column t1_col4 = Column(TYPE_INT, "age", sizeof(int));
    vector<Column> t1_cols = {t1_col1, t1_col2, t1_col3, t1_col4};

    int t1_first_page_id{-1};
    if(BPM->disk_manager->tables_names.find(table_name) != BPM->disk_manager->tables_names.end()){
        cout << table_name << "found in the database" << endl;
        t1_first_page_id = BPM->disk_manager->tables_names[table_name];
    }

    TableHeap* user_table = new TableHeap(BPM, t1_first_page_id, -1);
    BPM->disk_manager->addTable(table_name, user_table->get_first_page_id());
    user_table->setCols(t1_cols);
    user_table->setTableName(table_name);
    tables_map["User"] = user_table;
}


void insertIntoUserTable(){
    for (int i = 0; i < 7000; i++) {

        Field u1 = Field(TYPE_INT, 100 + i);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i)).c_str()); 
        Field u4 = Field(TYPE_INT, 20 + i%7);               

        Tuple t_user = Tuple({u1, u2, u3, u4});
        RID rid = tables_map["User"]->insertTuple(t_user);
        tables_rids["User"].push_back(rid);
    }
}

int
main(){

    string DB_name = "testSeqScanDB";
    
    DiskManager* dm = new DiskManager(DB_name);
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    // 1. User (user_id int, firstName varchar(30), lastName varchar(30), age int)
    createUserTable(BPM, "User");
    insertIntoUserTable();

    SeqScan* seq_scan = new SeqScan(tables_map["User"]);
    Column sort_key = Column(TYPE_INT, "age", sizeof(int));
    
    ExternalMergeSort* external_merge_sort = new ExternalMergeSort(BPM, seq_scan,sort_key,ASC);

    external_merge_sort->open();

    Tuple* tuple = new Tuple({});
    external_merge_sort->getNext(tuple);

    int counter{0};
    do{
        //tuple->print();
        counter++;
    }while(external_merge_sort->getNext(tuple));

    cout<<counter<<endl;


}
*/