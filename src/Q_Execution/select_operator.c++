#include<iostream>
#include"Q_Execution/select_operator.h"
using namespace std;



void Select::open(){
    this->child_operator->open();
}
void Select::close(){
    this->child_operator->close();
}

//pass table cols 
bool Select::getNext(Tuple* tuple){
    //get the next tuple using the child operator
    // now the tuple has the data of the next tuple, i will check if it satisfies the condition

     // extract cols from table heap in child
    //vector<Column>cols = (this->child_operator)->getTableHeap()->getCols();
    vector<Column>cols = this->child_operator->get_output_schema();
    while(this->child_operator->getNext(tuple)){
        //tuple->print();
        //this->curr_rid.print();
        if(this->predicate->evaluate(tuple, cols)){
            //update curr rid with the newely fetched tuple
            //push to rids_vector
            
            //it has to be seq_scan not join or somthing
            SeqScan* seq_scan = dynamic_cast<SeqScan*>(this->child_operator);
            
            if(seq_scan){
                this->curr_rid = seq_scan->get_prev_rid();
                //this->curr_rid.print();
                this->select_tuples_rids.push_back(curr_rid);
            }
            return true;
        }
    }

    return false;
}


bool Select::has_column(string col_name){
    //string table_name = this->child_operator->getTableHeap()->getTableName();
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}
///////////////////////////////
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

    AbstractExecuter* seq_scan = new SeqScan(tables_map1["User"]);
    // i will create predicate for user_id=104
    Field* user_id_field = new Field(TYPE_INT);
    Column* user_id_col = new Column(user_id_field,"user_id",4);

    Field* field104 = new Field(TYPE_INT,104);
    Column* col104 = new Column(field104,"null",4);

    Predicate* pred1 = new Predicate(user_id_col, col104,PredicateType::EQ);
    ComplexPredicate* complex_pred1 = new ComplexPredicate(pred1, pred1,ComplexPredicateType::AND);

    Select* select = new Select(seq_scan,complex_pred1);
    select->open();

    cout<<"select result"<<endl;
    bool res{true};
    while(res){
        Tuple* t1 = new Tuple({});
        res = select->getNext(t1);
        if(res)
            t1->print();
    }

    cout<<"done"<<endl;

}
*/
/*
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

    // TUPLE[2] == TUPLE[1]
    AbstractPredicate* leaf4 = new Predicate(col_last, col_first, PredicateType::EQ);

    //master ROOT NODE(Assembles Branch A OR Branch B)
    AbstractPredicate* root_expression = new ComplexPredicate(and_gate, leaf4, ComplexPredicateType::OR);


    Select* select = new Select(seq_scan, root_expression);
    select->open();

    cout<<"executing the query"<<endl;
    Tuple* result_row = new Tuple({});
    while (select->getNext(result_row)) {
        result_row->print();
    }
    cout<<"done"<<endl;

    select->close();
    return 0;
}
*/