#include<iostream>
#include"Q_Execution/Nested_loop_join.h"
#include<chrono>
using namespace std;



NestedLoopJoin::NestedLoopJoin(AbstractExecuter* outer_table,AbstractExecuter* inner_table,
               AbstractPredicate* join_condition, join_types join_type=INNER_JOIN):
               outer_table(outer_table), inner_table(inner_table),
               join_condition(join_condition),join_type(join_type){
        this->open();
}

void NestedLoopJoin::open(){
    this->outer_table->open();
    this->inner_table->open();
    set_output_schema();
}

//Student       //Grade
//[id, name] , [id, grade ]
// Student.id, Student.name, Grade.id, Grade.grade

void NestedLoopJoin::set_output_schema(){
    vector<Column> outer_schema = this->outer_table->get_output_schema();
    vector<Column> inner_schema = this->inner_table->get_output_schema();

    /*string table_name = this->outer_table->getTableHeap()->getTableName();
    for(int i{0};i<outer_schema.size();i++){
        string col_name = outer_schema[i].getColName();
        outer_schema[i].setColName(table_name+'.'+col_name);
    }*/
    
    //"id" >>> "Student.id"

    /*table_name = this->inner_table->getTableHeap()->getTableName();
    for(int i{0};i<inner_schema.size();i++){
        string col_name = inner_schema[i].getColName();
        inner_schema[i].setColName(table_name+'.'+col_name);
    }*/

    // combine:
    outer_schema.insert(outer_schema.end(), inner_schema.begin(), inner_schema.end());
    this->output_schema = outer_schema;
    for(auto&col:this->output_schema)col.printCol();
}


vector<Column> NestedLoopJoin::get_output_schema(){
    
    return this->output_schema;
}



bool NestedLoopJoin::getNext(Tuple* tuple){

    // fetch outer tuple:
    //loop : 
        // loop through all inner table till end
        // match on join condition
        // when inner table is exausted: fetch the next outer tuple
    while(true){

        // check whether inner table is exhasted
        if(!has_inner){
            // if join type is left join, and no inner matches, then add it once with null inner row
            if(this->inner_matches_counter ==0 && (this->join_type == LEFT_JOIN || this->join_type == FULL_JOIN)){
                    
                    inner_matches_counter =-1;
                    int inner_tuple_size = this->inner_table->get_output_schema().size();
                    
                    Field tmp_f = Field(TYPE_STRING,"NULL");
                    //tmp_f.set_null(true);
                    vector<Field> inner_fields(inner_tuple_size, tmp_f);
                    vector<Field> outer_fields = this->outer_tuple.fields;

                    //safety check
                    if(outer_fields.size()==0 || inner_fields.size()==0){
                        continue;
                    } 
                    // now we have the full result 
                    outer_fields.insert(outer_fields.end(), inner_fields.begin(), inner_fields.end());
                    //create tuple to return
                    Tuple res_tuple = Tuple(outer_fields);
                    *tuple = res_tuple;
                    return true;
            }else{

            //fetch next outer tuple
            bool has_outer = this->outer_table->getNext(&this->outer_tuple);
            //if still tuples in outer table, then match with all inner rows
            if(has_outer){
                this->inner_table->open();
                this->has_inner = true;
                this->inner_matches_counter =0;
            }
            //else means no data left in outer table, so the join is done and no more data
            else{
                return false;
            }
        }
        }

        // so there are data left in inner table:
        this->has_inner = this->inner_table->getNext(tuple);
        while(has_inner){
            // outer data + inner data
            vector<Field> outer_fields = this->outer_tuple.fields;
            vector<Field> inner_fields = tuple->fields;
            //safety check
            if(outer_fields.size()==0 || inner_fields.size()==0){
                continue;
            } 
            // now we have the full result 
            outer_fields.insert(outer_fields.end(), inner_fields.begin(), inner_fields.end());
            //create tuple to return
            Tuple res_tuple = Tuple(outer_fields);

            // check join condition: 
            if(this->join_condition->evaluate(&res_tuple, this->output_schema)){
                this->inner_matches_counter++;
                *tuple = res_tuple;
                return true;
            }
            else if(this->join_type == RIGHT_JOIN || this->join_type == FULL_JOIN){

                vector<Field> inner_fields = tuple->fields;
                //this->outer_tuple.print();
                int outer_tuple_size = this->outer_tuple.fields.size();

                //cout<<"outer size :"<<outer_tuple_size<<endl;
                Field tmp_f = Field(TYPE_STRING,"NULL");
                //tmp_f.set_null(true);
                vector<Field> outer_fields1(outer_tuple_size, tmp_f);
                
                // now we have the full result 
                outer_fields1.insert(outer_fields1.end(), inner_fields.begin(), inner_fields.end());
                //create tuple to return
                Tuple res_tuple = Tuple(outer_fields1);
                //for(auto&f:res_tuple.fields)
                    //f.print();
                //res_tuple.print();
                *tuple = res_tuple;
                 return true;
            }
            // fetch the next inner
            this->has_inner = this->inner_table->getNext(tuple);
        }


    }
}


void NestedLoopJoin::close(){
    this->outer_table->close();
    this->inner_table->close();
}

bool NestedLoopJoin::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}


TableHeap* NestedLoopJoin::getTableHeap(){
    return nullptr;
}

TableHeap* NestedLoopJoin::getOuterTableHeap(){
    return this->outer_table->getTableHeap();
}

TableHeap* NestedLoopJoin::getInnerTableHeap(){
    return this->inner_table->getTableHeap();
}
//////////////////////////////
//////////////////////////////

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
    cout << "--- Inserting some records into User table ---" << endl;
    for (int i = 0; i < 1000; i++) {

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
    for (int i = 0; i < 10000; i++) {
        Field o1 = Field(TYPE_INT, 9000 + i);          
        Field o2 = Field(TYPE_INT, 100 + (i%100));                
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

    Column* order_col_id = new Column(TYPE_INT, "Order.user_id", 4);
    Column* user_col_id = new Column(TYPE_INT, "User.user_id", 4);

    //Order.user_id = User.user_id
    AbstractPredicate* join_pred = new Predicate(order_col_id, user_col_id, PredicateType::EQ);
    AbstractExecuter* nested_loop_join = new NestedLoopJoin(seq_scan, order_seq_scan,join_pred, INNER_JOIN);


    auto st1 = chrono::high_resolution_clock::now();

    nested_loop_join->open();
    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});

    for(auto&col:nested_loop_join->get_output_schema()){
        cout<<col.getColName()<<"   | ";
    }
    cout<<endl;

    int counter{0};
    while (nested_loop_join->getNext(result_row)) {
        result_row->print();
        counter++;
    }
    cout<<"done"<<endl;
    cout<<"counter : "<<counter<<endl;

    cout<<"num of user tuples : "<<tables_map1["User"]->get_tuples_num()<<endl;
    cout<<"num of order tuples : "<<tables_map1["Order"]->get_tuples_num()<<endl;

    nested_loop_join->close();

    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    cout << "searching using  nested loop join duration: " << duration1.count() << " microseconds" << endl;


    return 0;
}
*/