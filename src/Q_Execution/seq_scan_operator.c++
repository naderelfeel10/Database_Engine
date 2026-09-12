#include<iostream>
#include"Q_Execution/seq_scan_operator.h"
using namespace std;


SeqScan::SeqScan(TableHeap* table_heap){
    this->Table_heap = table_heap;
    open();
}
void SeqScan::open(){
    this->curr_rid_pointer = this->Table_heap->getStartingRID();
    //update seq_scan output schema and append table_name at the begining :

    this->output_schema = this->Table_heap->get_output_schema();
    string table_name = this->Table_heap->getTableName();

    for(int i=0;i<this->output_schema.size(); i++){
        //table_name.col_name
        string new_name = table_name+'.'+this->output_schema[i].getColName();
        this->output_schema[i].setColName(new_name);
        this->output_schema[i].printCol();
    }

    //for(auto&col:this->output_schema)col.printCol();
}
void SeqScan::close(){
    this->curr_rid_pointer = RID(-1,-1);
}

bool SeqScan::getNext(Tuple* tuple){

    int lst_page_id = this->Table_heap->getStoppigRID().getPageId();

    while(true){

        int curr_page_id = this->curr_rid_pointer.getPageId();
        int curr_slot_num = this->curr_rid_pointer.getSlotNum();


        //this->curr_rid_pointer.print();
        // fetch the page 
        char* page_buffer = this->Table_heap->BPM->fetchPage(curr_page_id);
        if (page_buffer == nullptr) {
            return false; 
        }
        PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
        Page* page = reinterpret_cast<Page*>(page_buffer);

        int num_of_tuples = pageHeader->num_tuples;

        int next_page_id = pageHeader->next_page_id;
        int next_slot_num = curr_rid_pointer.getSlotNum()+1;

        //cout<<next_page_id<<", "<<next_slot_num<<endl;

        // if this is the last slot in the page
        // then increment page_id
        if(curr_slot_num >= num_of_tuples ){
            //this->curr_rid_pointer = RID(next_page_id,0);
            curr_page_id=next_page_id;
            //if next page is beyond the last page in the table
            // then return EOF 
            if(curr_page_id>lst_page_id || curr_page_id==-1){
                tuple = nullptr;
                return false;
            }else{
                // if it's not the last page in the table, then get the first slot in it
                this->curr_rid_pointer = RID(next_page_id,0);
                Tuple* tmp_tuple = nullptr;
                tmp_tuple = this->Table_heap->getTuple(this->curr_rid_pointer);
                
                this->prev_rid_pointer = this->curr_rid_pointer;
                this->curr_rid_pointer = RID(next_page_id,1);
                //nullptr means it's deleted
                if(tmp_tuple == nullptr){
                    continue;
                }
                *tuple = *tmp_tuple;
                return true;
            }

        }
        Tuple* tmp_tuple = nullptr;
        tmp_tuple = this->Table_heap->getTuple(this->curr_rid_pointer);
        
        this->prev_rid_pointer = this->curr_rid_pointer;
        this->curr_rid_pointer = RID(curr_page_id,next_slot_num);

        if(tmp_tuple == nullptr){
            continue;
        }
        //this->curr_rid_pointer.print();
        //this->prev_rid_pointer.print();

        //tuple->print();
        *tuple = *tmp_tuple;
        return true;
    }
}

bool SeqScan::has_column(string col_name){
    string table_name = this->Table_heap->getTableName();
    for(auto&col:this->get_output_schema()){
        if(table_name+'.'+col.getColName()==col_name)return true;
    }
    return false;
}

TableHeap* SeqScan::getTableHeap(){
    return this->Table_heap;
}

vector<Column> SeqScan::get_output_schema(){
    return this->output_schema;
}

vector<RID> SeqScan::get_table_rids(){
    this->table_rids = this->Table_heap->getTableRIDS();

    for(auto&rid:this->table_rids)rid.print();
    return this->table_rids;
}

RID SeqScan::get_curr_rid(){
    return this->curr_rid_pointer;
}

RID SeqScan::get_prev_rid(){
    return this->prev_rid_pointer;
}

///////////////////////////////
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
    cout << "--- Inserting 10 records into User table ---" << endl;
    for (int i = 0; i < 70; i++) {

        Field u1 = Field(TYPE_INT, 100 + i);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i)).c_str()); 
        Field u4 = Field(TYPE_INT, 20 + i);               

        Tuple t_user = Tuple({u1, u2, u3, u4});
        RID rid = tables_map["User"]->insertTuple(t_user);
        tables_rids["User"].push_back(rid);
    }
}
*/
/*
int
main(){

    string DB_name = "testSeqScanDB";
    
    DiskManager* dm = new DiskManager(DB_name);
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    // 1. User (user_id int, firstName varchar(30), lastName varchar(30), age int)
    createUserTable(BPM, "User");
    insertIntoUserTable();

    SeqScan* seq_scan = new SeqScan(tables_map["User"]);
    bool res{true};
    while(res){
    Tuple* t1 = new Tuple({});
    res = seq_scan->getNext(t1);
    if(res)
        t1->print();
    }


}
*/