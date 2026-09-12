#include<iostream>
#include"Q_Execution/hash_join.h"
#include<chrono>
using namespace std;


HashJoin::HashJoin(AbstractExecuter* outer_table,AbstractExecuter* inner_table, AbstractPredicate* join_condition, string col_name):
outer_table(outer_table), inner_table(inner_table),join_condition(join_condition)
,join_col_name(col_name) ,curr_inner_tuple({}),outer_tuple({}){

}

void HashJoin::open(){
    this->outer_table->open();
    this->inner_table->open();
    this->set_output_schema();
}

void HashJoin::close(){
    this->outer_table->close();
    this->inner_table->close();
}

void HashJoin::set_output_schema(){
    vector<Column> outer_schema = this->outer_table->get_output_schema();
    vector<Column> inner_schema = this->inner_table->get_output_schema();

    // change col name to be table_name.col_name
    // that's how i can differentiate between cols with same names in diff tables
    
    /*
    string table_name = outer_table->getTableHeap()->getTableName();
    for(int i=0; i<outer_schema.size();i++){
        string col_name = outer_schema[i].getColName();
        outer_schema[i].setColName(table_name+'.'+col_name);
    }

    table_name = inner_table->getTableHeap()->getTableName();
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

vector<Column> HashJoin::get_output_schema(){
    return this->output_schema;
}


bool HashJoin::getNext(Tuple*tuple){

    // dummy tuple to hold outer table tuples
    int join_col_index = this->outer_table->getTableHeap()->getColIndex(this->join_col_name);

    // first i need to build the hash table over the join key
    if(!is_hash_built){
        while(outer_table->getNext(&outer_tuple)){
            Field hashed_field = outer_tuple.fields[join_col_index];
            this->hash_map[hashed_field].push_back(outer_tuple);
        }
        is_hash_built = true;

        if(!this->inner_table->getNext(&this->curr_inner_tuple)){
            return false;
        }
    }

    join_col_index = this->inner_table->getTableHeap()->getColIndex(this->join_col_name);
    // after building the whole hash map, it's time to match and return tuples

    
    Field search_field = curr_inner_tuple.fields[join_col_index];
    while(true){
        

        auto it = hash_map.find(search_field);

        if(it != hash_map.end() && tuple_index < it->second.size()){
            Tuple& tmp_tuple = it->second[tuple_index];  // reference, no copy
            // i will extract the fields, concat then create new tuple with them
            vector<Field> fields1 = tmp_tuple.fields;
            vector<Field> fields2 = curr_inner_tuple.fields;
            fields1.insert(fields1.end(), fields2.begin(), fields2.end());

            Tuple res_tuple = Tuple(fields1);
            *tuple = res_tuple;
            this->tuple_index++;
            return true;
        }

        this->tuple_index =0;
        if(!this->inner_table->getNext(&this->curr_inner_tuple)){
            return false;
        }

        search_field = curr_inner_tuple.fields[join_col_index];

    }


    
}


bool HashJoin::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}

TableHeap* HashJoin::getTableHeap(){
    return nullptr;
}

TableHeap* HashJoin::getOuterTableHeap(){
    return this->outer_table->getTableHeap();
};

TableHeap* HashJoin::getInnerTableHeap(){
    return this->inner_table->getTableHeap();
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
cout << "\n--- Inserting 10 records into Order table ---" << endl;
    for (int i = 0; i < 100; i++) {
        Field o1 = Field(TYPE_INT, 9000 + i);          
        Field o2 = Field(TYPE_INT, 100 + (i%1000));                
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

int main() {

    ios_base::sync_with_stdio(false);
    cout.tie(nullptr);

    string DB_name = "testSeqScanDB";
    DiskManager* dm = new DiskManager(DB_name);
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    createUserTable(BPM, "User");
    insertIntoUserTable(); 

    AbstractExecuter* seq_scan = new SeqScan(tables_map1["User"]);

    Column* col_id = new Column(TYPE_INT, "user_id", 4);    //TUPLE[0]
    Column* col_first = new Column(TYPE_STRING, "firstName", 30); //TUPLE[1]
    Column* col_last = new Column(TYPE_STRING, "lastName", 30);   //TUPLE[2]

    //(TUPLE[0] > 102) AND (TUPLE[0] <= 107)
    Column* const_102 = new Column(new Field(TYPE_INT, 102), "C102",4);
    AbstractPredicate* leaf1 = new Predicate(col_id, const_102, PredicateType::GT);

    Column* const_107 = new Column(new Field(TYPE_INT, 107), "C107",4);
    AbstractPredicate* leaf2 = new Predicate(col_id, const_107, PredicateType::LE);

    AbstractPredicate* and_gate = new ComplexPredicate(leaf1, leaf2, ComplexPredicateType::AND);
    AbstractPredicate* leaf4 = new Predicate(col_last, col_first, PredicateType::EQ);
    AbstractPredicate* root_expression = new ComplexPredicate(and_gate, leaf4, ComplexPredicateType::OR);



    AbstractExecuter* select = new Select(seq_scan, root_expression);

    vector<string>projection_cols = {"user_id","firstName"};
    Projection* projection = new Projection(select, projection_cols);


    createOrderTable(BPM, "Order");
    insertIntoOrderTable();
    //buildOrderIndex(tables_map1["Order"],tables_rids1["Order"], BPM);

    AbstractExecuter* order_seq_scan = new SeqScan(tables_map1["Order"]);

    Column* order_col_id = new Column(TYPE_INT, "Order.user_id", 4);
    Column* user_col_id = new Column(TYPE_INT, "User.user_id", 4);

    AbstractPredicate* join_pred = new Predicate(order_col_id, user_col_id, PredicateType::EQ);

    AbstractExecuter* hash_join = new HashJoin(seq_scan, order_seq_scan, join_pred, "user_id");

    auto st1 = chrono::high_resolution_clock::now();


    hash_join->open();

    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    
    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});

    for(auto&col:hash_join->get_output_schema()){
        cout<<col.getColName()<<" ";
    }
    cout<<endl;
    int counter{0};
    while (hash_join->getNext(result_row)) {
        result_row->print();
        counter++;
    }
    cout<<"done"<<endl;
    cout<<"counter : "<<counter<<endl;

    hash_join->close();

    cout << "searching using hash join duration: " << duration1.count() << " microseconds" << endl;

    end1 = chrono::high_resolution_clock::now();
    duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    cout << "searching using hash join duration: " << duration1.count() << " microseconds" << endl;


    return 0;
}
*/