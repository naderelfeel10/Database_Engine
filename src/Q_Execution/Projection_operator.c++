#include<iostream>
#include"Q_Execution/Projection_operator.h"
using namespace std;



void Projection::open(){
    this->child_operator->open();
    vector<Column> child_schema = this->child_operator->get_output_schema();

    this->output_schema.clear();
    this->projection_col_indecies.clear();

    for(auto&col:child_schema)col.printCol();


    for(auto& col_name:this->projection_cols){
        cout<<col_name<<endl;
        for(int i=0;i<child_schema.size();i++){

            if(col_name == child_schema[i].getColName()){
                // if cols match, then save col itself
                this->output_schema.push_back(child_schema[i]);
                // and save it's index
                this->projection_col_indecies.push_back(i);
                break;

            }
        }
    }
    for(auto&col:this->output_schema)col.printCol();

}

bool Projection::getNext(Tuple* tuple){
    
    Tuple tmp_tuple({});
    if(!this->child_operator->getNext(&tmp_tuple)){
        return false;
    }
    vector<Field> res_fields;
    res_fields.reserve(this->projection_col_indecies.size());

    for(auto&index: projection_col_indecies){
        res_fields.push_back(tmp_tuple.fields[index]);
    }

    *tuple = Tuple(res_fields);
    //tuple->print();
    return true;
}

bool Projection::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}
///////////////////////////////////////////

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
    for (int i = 0; i < 70; i++) {

        Field u1 = Field(TYPE_INT, 100 + i);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i)).c_str()); 
        Field u4 = Field(TYPE_INT, 20 + i);               

        Tuple t_user = Tuple({u1, u2, u3, u4});
        RID rid = tables_map1["User"]->insertTuple(t_user);
        tables_rids1["User"].push_back(rid);
    }
}

int main() {
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

    projection->open();

    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});
    for(auto&col:projection_cols)cout<<col<<"             |";
    cout<<endl;
    while (projection->getNext(result_row)) {
        result_row->print();
    }
    cout<<"done"<<endl;

    projection->close();




    return 0;
}
*/