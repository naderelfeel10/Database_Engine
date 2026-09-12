#include<iostream>
#include"Q_Execution/MergeJoinExecuter.h"
#include<chrono>
using namespace std;

MergeJoin::MergeJoin(BufferPoolManager*BPM, AbstractExecuter* outer_table, AbstractExecuter* inner_table, string join_col_name):
            BPM(BPM),outer_table(outer_table),inner_table(inner_table),join_col_name(join_col_name){

}

void MergeJoin::open(){

    // i need to sort both tables based on joining key
    this->outer_table->open();
    this->inner_table->open();

    this->curr_outer_tuple = Tuple({});
    //sort outer table based on join col
    // find the col:
    this->outer_col_index = this->outer_table->getTableHeap()->getColIndex(this->join_col_name);
    Column sort_key = this->outer_table->get_output_schema()[outer_col_index];

    // sort outer table
    this->outer_table_sorted = new ExternalMergeSort(BPM, outer_table, sort_key, ASC);
    // get first tuple from the outer  table
    this->outer_table_sorted->open();
    this->outer_table_sorted->getNext(&this->curr_outer_tuple);
    //this->curr_outer_tuple.print();
    int counter{0};
    /*
    Tuple* result_row = new Tuple({});
    while (outer_table_sorted->getNext(result_row)) {
        result_row->print();
        counter++;
    }*/
    cout<<counter<<endl;
    cout<<"outer tuples num : "<<outer_table_sorted->getTableHeap()->get_tuples_num()<<endl;

    cout<<"------------------------------------------------------------------------"<<endl;
    //sort inner table based on join col
    this->inner_col_index = this->inner_table->getTableHeap()->getColIndex(this->join_col_name);
    cout<<this->inner_table->get_output_schema().size()<<endl;
    Column sort_key2 = this->inner_table->get_output_schema()[inner_col_index];
    
    this->inner_table_sorted = new ExternalMergeSort(BPM, inner_table, sort_key2, ASC);
    this->inner_table_sorted->open();
    this->inner_table_sorted->getNext(&this->t2);
    
    counter=0;
    /*Tuple* result_row1 = new Tuple({});
    while (inner_table_sorted->getNext(result_row1)) {
        result_row1->print();
        counter++;
    }*/
    cout<<counter<<endl;
    cout<<"inner tuples num : "<<inner_table_sorted->getTableHeap()->get_tuples_num()<<endl;
    this->set_output_schema();
}

void MergeJoin::set_output_schema(){
    vector<Column> outer_schema = this->outer_table->get_output_schema();
    vector<Column> inner_schema = this->inner_table->get_output_schema();

    /*
    string table_name = this->outer_table->getTableHeap()->getTableName();
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

    outer_schema.insert(outer_schema.end(), inner_schema.begin(), inner_schema.end());
    this->table_schema = outer_schema;
}

vector<Column> MergeJoin::get_output_schema(){
    return this->table_schema;
}

bool MergeJoin::getNext(Tuple* tuple){
    // i need to pointers, on for each table
    static int left_pointer{0};
    static int right_pointer{0};
    //compute size of each table
    int outer_num_tuples = this->outer_table->getTableHeap()->get_tuples_num();
    int inner_num_tuples = this->inner_table->getTableHeap()->get_tuples_num();
    
    if(!res_batch.empty()){
        *tuple = res_batch.back();
        res_batch.pop_back();
        return true;
    }
    inner_matched_tuples.clear();

    //loop through tables
    while(left_pointer< outer_num_tuples && right_pointer< inner_num_tuples){

        //dummy tuples to hold 2 tables data

        //check if both fields are equal

        //this->curr_outer_tuple.print();
        //t2.print();
        //this->curr_outer_tuple.fields[this->outer_col_index].print();
        //t2.fields[this->inner_col_index].print();
        if(this->curr_outer_tuple.fields.size()==0 || this->t2.fields.size()==0){
            break;
        }
        if(this->curr_outer_tuple.fields[this->outer_col_index] == t2.fields[this->inner_col_index]){

            // if matched, then output the result
            // if yes, we need to merge both fields into full tuple
            vector<Field> inner_fields = t2.fields;

            //push to res_set
            inner_matched_tuples.push_back(inner_fields);

            int outer_ = this->inner_table_sorted->getNext(&t2);
            if(!outer_){
                break;
            }
            right_pointer++;
        }
        //else if outer table currenct tuple is greater than the inner one:
        // then advance the inner pointer
        else if(curr_outer_tuple.fields[this->outer_col_index] > t2.fields[this->inner_col_index]){
            right_pointer++;
        }
        // if inner table is larger, then advance the outer one
        else if(curr_outer_tuple.fields[this->outer_col_index] < t2.fields[this->inner_col_index]){
            left_pointer++;
        }

        //while((curr_outer_tuple.fields[this->outer_col_index] == t2.fields[this->inner_col_index]) && left_pointer<outer_num_tuples){
            int jj = right_pointer;

            // from this while loop i need to get all outer tuple matchs into inner_matched_tuples
            while((curr_outer_tuple.fields[this->outer_col_index] == t2.fields[this->inner_col_index]) && jj<inner_num_tuples){
                    //if equal :
                    // 1. output the result:
                    vector<Field> inner_fields = t2.fields;
                    inner_matched_tuples.push_back(inner_fields);
                    
                    t2 = Tuple({});
                    bool inner_ = this->inner_table_sorted->getNext(&t2);
                    if(!inner_){
                        break;
                    }
                    jj++;
            }
            
            /*
            this->curr_outer_tuple.print();
            for(auto&t:inner_matched_tuples){
                t.print();
            }
            */

            // i will just prepare the batch to be printed from inner tuples
            // res is like this : (outer_tuple + innner_tuple)
            for(auto& inner:inner_matched_tuples){
                vector<Field>outer_fields = this->curr_outer_tuple.fields;
                vector<Field>inner_fields = inner.fields;
            
                outer_fields.insert(outer_fields.end(), inner_fields.begin(), inner_fields.end());
                Tuple res_tuple = Tuple(outer_fields);
                //push to res_tuple
                res_batch.push_back(res_tuple);
            }

            left_pointer++;
            // then we move to the next outer tuple
            bool outer_ = this->outer_table_sorted->getNext(&this->curr_outer_tuple);
            if(!outer_){
                break;
            }
            // i need to check whether the next outer tuple is equal to the prev outer
            // if equal i need to matched it with same inner matched tuples
            if(!inner_matched_tuples.empty()){

            Field first_matched_field = inner_matched_tuples.front().fields[this->inner_col_index];
            
            while((this->curr_outer_tuple.fields[this->outer_col_index] == first_matched_field) && left_pointer<outer_num_tuples){
                for(auto& inner:inner_matched_tuples){
                    vector<Field>outer_fields = this->curr_outer_tuple.fields;
                    vector<Field>inner_fields = inner.fields;
                    
                    outer_fields.insert(outer_fields.end(), inner_fields.begin(), inner_fields.end());
                    Tuple res_tuple = Tuple(outer_fields);
                    //push to res_tuple
                    res_batch.push_back(res_tuple);
                }

                // fetch the next outer tuple, check if it matches also
                left_pointer++;
                outer_ = this->outer_table_sorted->getNext(&this->curr_outer_tuple);
                if(!outer_){
                    break;
                }
            }
            right_pointer = jj;

        }
        //}

        if(!res_batch.empty()){
            *tuple = res_batch.back();
            res_batch.pop_back();
            return true;
        }

        //get inner tuple into t2
        //this->inner_table_sorted->getNext(&t2);
        //right_pointer++;
    }

    return false;
}

void MergeJoin::close(){
    this->outer_table->close();
    this->inner_table->close();
}

bool MergeJoin::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}


TableHeap* MergeJoin::getTableHeap(){
    return nullptr;
}

TableHeap* MergeJoin::getOuterTableHeap(){
    return this->outer_table->getTableHeap();
}

TableHeap* MergeJoin::getInnerTableHeap(){
    return this->inner_table->getTableHeap();
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/*
vector<string> tables1;
map<string, TableHeap*>tables_map1;
map<string, vector<RID>> tables_rids1;
map<string, StaticHashIndexWrapper*>tables_indexes1;
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
        Field o2 = Field(TYPE_INT, 100 + (i%1000));                
        Field o3 = Field(TYPE_FLOAT, static_cast<float>(150.75 + (i * 20.5))); 
        Field o4 = Field(TYPE_BOOL, (i % 2 == 0));          

        Tuple t_order = Tuple({o1, o2, o3, o4});
        RID rid = tables_map1["Order"]->insertTuple(t_order);
        tables_rids1["Order"].push_back(rid);
    }
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


    createOrderTable(BPM, "Order");
    insertIntoOrderTable();

    AbstractExecuter* order_seq_scan = new SeqScan(tables_map1["Order"]);

    Column* order_col_id = new Column(TYPE_INT, "user_id", 4);
    Column* user_col_id = new Column(TYPE_INT, "user_id", 4);

    //AbstractPredicate* join_pred = new Predicate(order_col_id, user_col_id, PredicateType::EQ);

    MergeJoin* merge_join = new MergeJoin(BPM, seq_scan,order_seq_scan, user_col_id->getColName());

    auto st1 = chrono::high_resolution_clock::now();

    merge_join->open();

  
    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);

    
    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});
    
    for(auto&col:merge_join->get_output_schema()){
        cout<<col.getColName()<<" ";
    }
    cout<<endl;
    int counter{0};
    while (merge_join->getNext(result_row)) {
        result_row->print();
        counter++;
    }
    cout<<"done"<<endl;
    cout<<"counter :"<<counter<<endl;
    cout << "open time : " << duration1.count() << " microseconds" << endl;
    
    merge_join->close();

    end1 = chrono::high_resolution_clock::now();
    duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    cout << "equi join using merge join duration: " << duration1.count() << " microseconds" << endl;


    return 0;
}
*/