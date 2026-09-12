#include<iostream>
#include"Q_Execution/IndexedNested_loop_join.h"
#include<chrono>
using namespace std;


IndexedNestedLoopJoin::IndexedNestedLoopJoin(BufferPoolManager* BPM, AbstractExecuter* outer_table,AbstractExecuter* inner_table, Index* inner_index, AbstractPredicate* join_condition):
                    BPM(BPM), outer_table(outer_table),inner_table(inner_table),
                    inner_index(inner_index), join_condition(join_condition){
    
    string col_name  = inner_index->get_index_col_name();
    col_index = this->outer_table->getTableHeap()->getColIndex(col_name);
}



void IndexedNestedLoopJoin::open(){
    this->outer_table->open();
    this->set_output_schema();
}

void IndexedNestedLoopJoin::close(){
    this->outer_table->close();
}

void IndexedNestedLoopJoin::set_output_schema(){
    vector<Column> outer_schema = this->outer_table->get_output_schema();
    vector<Column> inner_schema = this->inner_table->getTableHeap()->getCols();

    // change col name to be table_name.col_name
    // that's how i can differentiate between cols with same names in diff tables

    /*
    string table_name = outer_table->getTableHeap()->getTableName();
    for(int i=0; i<outer_schema.size();i++){
        string col_name = outer_schema[i].getColName();
        outer_schema[i].setColName(table_name+'.'+col_name);
    }

    table_name = this->inner_table->getTableHeap()->getTableName();
    for(int i=0; i<inner_schema.size();i++){
        string col_name = inner_schema[i].getColName();
        inner_schema[i].setColName(table_name+'.'+col_name);
    }
    */
   
    // concat
    outer_schema.insert(outer_schema.end(), inner_schema.begin(), inner_schema.end());

    this->output_schema = outer_schema;

    //return output_schema;
}


vector<Column> IndexedNestedLoopJoin::get_output_schema(){
    return this->output_schema;
}


bool IndexedNestedLoopJoin::getNext(Tuple*tuple){
    
    //fetch a tuple from outer table
    // extract key the index is built on
    // search through the index using this key
       // this will return a vector of matched results [t1, t10, t67];
    // return matched tuples one by one 

    while(inner_matches.empty()){
        bool has_outer = this->outer_table->getNext(&this->curr_outer_tuple);
        // if no more outer tuples, then end the join and return false
        if(!has_outer){
            return false;
        }
        //else :
        //fetch matches from the index:
        //[id, name, salary]
        Field search_field = this->curr_outer_tuple.fields[col_index];
        this->inner_matches =  this->inner_index->Search(search_field);
    }

    // if not empty:
    //consume tuples from this inner matches tuples
    RID curr_rid = this->inner_matches.back();
    this->inner_matches.pop_back();
    //fetch the tuple of this RID 
    Tuple inner_tuple = Tuple({});
    this->getTuple(curr_rid, inner_tuple);

    // outer_tuple + inner_tuple
    vector<Field> outer_fields = this->curr_outer_tuple.fields;
    vector<Field> inner_fields = inner_tuple.fields;
    outer_fields.insert(outer_fields.end(), inner_fields.begin(), inner_fields.end());

    *tuple = Tuple(outer_fields);
    return true;
}


void IndexedNestedLoopJoin::getTuple(RID rid, Tuple& tuple) {

    char* page_buffer = this->BPM->fetchPage(rid.getPageId());
    Page* page = reinterpret_cast<Page*>(page_buffer);

    page->getTuple(rid.getSlotNum(), tuple);
}

bool IndexedNestedLoopJoin::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}


TableHeap* IndexedNestedLoopJoin::getTableHeap(){
    return nullptr;
};

TableHeap* IndexedNestedLoopJoin::getOuterTableHeap(){
    return this->outer_table->getTableHeap();
};
////////////////////////////////////////////
///////////////////////////////////////////

/*
vector<string> tables1;
map<string, TableHeap*>tables_map1;
map<string, vector<RID>> tables_rids1;
map<string, StaticHashIndexWrapper*>tables_indexes1;
map<string, Index*>tables_indexes;
map<string, BPlusTreeIndexWrapper*>tables_B_indexes1;


void createUserTable(BufferPoolManager* BPM, string table_name){
    tables1.push_back(table_name);

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
    tables_map1["User"] = user_table;
}


void insertIntoUserTable(){
    cout << "--- Inserting 10 records into User table ---" << endl;
    for (int i = 0; i < 100; i++) {

        Field u1 = Field(TYPE_INT, 100 + i);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i)).c_str()); 
        Field u4 = Field(TYPE_INT, 20 + i);               

        Tuple t_user = Tuple({u1, u2, u3, u4});
        RID rid = tables_map1["User"]->insertTuple(t_user);
        tables_rids1["User"].push_back(rid);
    }
}



void createOrderTable(BufferPoolManager* BPM, string table_name){
    tables1.push_back(table_name);

    Column t3_col1 = Column(TYPE_INT, "order_id", sizeof(int));
    Column t3_col2 = Column(TYPE_INT, "user_id", sizeof(int));
    Column t3_col3 = Column(TYPE_FLOAT, "totalAmount", sizeof(float));
    Column t3_col4 = Column(TYPE_BOOL, "isShipped", sizeof(bool));
    vector<Column> t3_cols = {t3_col1, t3_col2, t3_col3, t3_col4};

    int t3_first_page_id{-1};
    if(BPM->disk_manager->tables_names.find(table_name) != BPM->disk_manager->tables_names.end()){
        cout << table_name << " found in the database" << endl;
        t3_first_page_id = BPM->disk_manager->tables_names[table_name];
    }

    TableHeap* order_table = new TableHeap(BPM, t3_first_page_id, -1);
    BPM->disk_manager->addTable(table_name, order_table->get_first_page_id());
    order_table->setCols(t3_cols);
    order_table->setTableName(table_name);
    tables_map1["Order"] = order_table;
}

void insertIntoOrderTable(){
cout << "\n--- Inserting some records into Order table ---" << endl;
    for (int i = 0; i < 100; i++) {
        Field o1 = Field(TYPE_INT, 9000 + i);          
        Field o2 = Field(TYPE_INT, 100 + (i%10));                
        Field o3 = Field(TYPE_FLOAT, static_cast<float>(150.75 + (i * 20.5))); 
        Field o4 = Field(TYPE_BOOL, (i % 2 == 0));          

        Tuple t_order = Tuple({o1, o2, o3, o4});
        RID rid = tables_map1["Order"]->insertTuple(t_order);
        tables_rids1["Order"].push_back(rid);
    }
}


void buildOrderIndex(TableHeap* order_table, const vector<RID>& order_rids, BufferPoolManager* BPM) {
    if (order_rids.empty()) return;

    string index_col_name = "user_id";
    order_table->createIndex(STATIC_HASH_INDEX, index_col_name, 300);
    
    StaticHashIndexWrapper* s_hash_wrapper = static_cast<StaticHashIndexWrapper*>(
        order_table->getIndex(index_col_name, STATIC_HASH_INDEX)
    );


    TableIterator table_iterator(order_table, order_rids.front(), order_rids.back());

    for (; !table_iterator.end(); ++table_iterator) {
        Tuple tuple = *table_iterator;
        int counter = 0;
        
        for (auto& col : order_table->getCols()) {
            if (col.getColName() == index_col_name) {
                s_hash_wrapper->Insert(
                    tuple.fields[counter], 
                    index_col_name, 
                    order_table->getCols(), 
                    table_iterator.getCurrRIDPointer()
                );
            }
            counter++;
        }
    }
    cout << "Successfully generated Static Hash Index on [Order." << index_col_name << "]" << endl;
    tables_indexes1["Order"] = s_hash_wrapper;
    tables_indexes["Order"] = s_hash_wrapper;
}

 
int
main(){

    ios_base::sync_with_stdio(false);
    cout.tie(nullptr);

    string DB_name = "testSeqScanDB";
    DiskManager* dm = new DiskManager(DB_name);
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    createUserTable(BPM, "User");
    insertIntoUserTable(); 

    AbstractExecuter* seq_scan = new SeqScan(tables_map1["User"]);


    createOrderTable(BPM, "Order");
    insertIntoOrderTable();
    
    auto st1 = chrono::high_resolution_clock::now();
    buildOrderIndex(tables_map1["Order"],tables_rids1["Order"], BPM);
    AbstractExecuter* order_seq_scan = new SeqScan(tables_map1["Order"]);
    Column* order_col_id = new Column(TYPE_INT, "Order.user_id", 4);
    Column* user_col_id = new Column(TYPE_INT, "User.user_id", 4);


    AbstractPredicate* join_pred = new Predicate(order_col_id, user_col_id, PredicateType::EQ);
    AbstractExecuter* indexed_nested_loop_join = new IndexedNestedLoopJoin(BPM,seq_scan, order_seq_scan,tables_indexes1["Order"],join_pred);

    indexed_nested_loop_join->open();

    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});
    int counter{0};

    for(auto&col:indexed_nested_loop_join->get_output_schema()){
        cout<<col.getColName()<<"   |  ";
    }
    cout<<endl;
    while (indexed_nested_loop_join->getNext(result_row)) {
        result_row->print();
        counter++;
    }
    cout<<"done"<<endl;
    cout<<"counter : "<<counter<<endl;
    indexed_nested_loop_join->close();

    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    cout << "searching using indexed nested loop join duration: " << duration1.count() << " microseconds" << endl;


    return 0;
}
*/