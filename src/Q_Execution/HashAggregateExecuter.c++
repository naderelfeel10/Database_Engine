#include<iostream>
#include"Q_Execution/HashAggregateExecuter.h"
#include<chrono>
using namespace std;


HashAggregateExecuter::HashAggregateExecuter(BufferPoolManager* BPM, AbstractExecuter*table, vector<int> grouping_keys, vector<GroupingFunction> grouping_functions ):
    BPM(BPM),table(table), grouping_keys(grouping_keys), grouping_functions(grouping_functions){
        agg_state.assign(grouping_functions.size(), AggValues{});
}

void HashAggregateExecuter::open(){
    this->table->open();
    set_output_schema();

    //fetch a tuple
    this->has_tuple = this->table->getNext(&curr_tuple);

    // if still data then :
    // 1. get the grouping fields of this tuple
    // 2. update it's corresponding value in the hash_map

    while(this->has_tuple){

        this->curr_group.clear();
        this->curr_group = this->get_grouping_fields(curr_tuple);

        //update agg will update the value inside curr_group and agg_state
        if(this->groups_map.find(this->curr_group) == this->groups_map.end()){
            agg_state.assign(grouping_functions.size(), AggValues{});
        }
        else{
            agg_state = this->groups_map[this->curr_group];
        }
        this->update_aggregate();

        //now insert into the map
        this->groups_map[this->curr_group] = this->agg_state;
        this->has_tuple = this->table->getNext(&curr_tuple);
    }

    /*for(auto&agg:this->groups_map){
        Tuple res_tuple = this->get_output_tuple(agg.first, agg.second);
        res_tuple.print();
    }
    */
    for(auto&agg:this->groups_map){
        this->groups.push_back(agg.first);
        this->states.push_back(agg.second);
    }
}


void HashAggregateExecuter::close(){
    this->table->close();
}


void HashAggregateExecuter::set_output_schema(){
    //output grouping vector + grouping functions

    this->output_schema = this->table->get_output_schema();
    /*
    int size = this->grouping_keys.size();
    string table_name = this->table->getTableHeap()->getTableName();
    
    for(int i=0;i<size;i++){
        int col_index = grouping_keys[i];
        Column col = this->table->get_output_schema()[col_index];
        string col_name = table_name+'.'+col.getColName();
        col.setColName(col_name);
        this->output_schema.push_back(col);
    }
    */

    int size = this->grouping_functions.size();
    for(int i{0};i<size;i++){
        int col_index = grouping_functions[i].function_key;
        Column col = this->table->get_output_schema()[col_index];

        string function_name = get_function_string(grouping_functions[i]);
        col.setColName(function_name+'('+col.getColName()+')');
        this->output_schema.push_back(col);
    }
    
    for(auto&col:this->output_schema)col.printCol();
}



vector<Column> HashAggregateExecuter::get_output_schema(){
    return this->output_schema;
}


string HashAggregateExecuter::get_function_string(GroupingFunction func){
    switch (func.grouping_type)
    {
    case SUM:
        return "SUM";
        break;
    case AVG:
        return "AVG";
        break;
    case MAX:
        return "MAX";
        break;
    case MIN:
        return "MIN";
        break;
    case COUNT:
        return "COUNT";
        break;

    default:
        return "COUNT";
        break;
    }

    return "INVALID";
}

vector<Field> HashAggregateExecuter::get_grouping_fields(Tuple tuple){
    vector<Field> tmp_fields_vector;
    for(int&index:this->grouping_keys){
        Field tmp_f =  tuple.fields[index];
        tmp_fields_vector.push_back(tmp_f);
    }
    return tmp_fields_vector;
}


void HashAggregateExecuter::update_aggregate(){
    for(size_t i{0};i<this->grouping_functions.size();i++){
        //fetch function type 
        AggregateType grouping_type = grouping_functions[i].grouping_type;
        int col_index = grouping_functions[i].function_key;
        
        //fetch the new value
        //next_tuple.print();
        Field agg_field = this->curr_tuple.fields[col_index];

        double value;
        //fetch the value based on the field type
        switch (agg_field.getFieldType())
        {
        case TYPE_INT:
            value = agg_field.getFieldValueInt();
            break;
        case TYPE_BOOL:
            value = agg_field.getFieldValueBool();
            break;
        case TYPE_FLOAT:
            value = agg_field.getFieldValueFloat();
            break;
        
        default:
            throw runtime_error("wrong type");
        }

        //update the counter
        agg_state[i].counter++;
        switch (grouping_type)
        {
        case SUM:
        case AVG:
            agg_state[i].value += (value);
            break;
        case MAX:
            agg_state[i].value = max(value, agg_state[i].value);
            break;
        case MIN:
            agg_state[i].value = min(value, agg_state[i].value);
            break;

        default:
            break;
        }
    }
}


Tuple HashAggregateExecuter::get_output_tuple(vector<Field>curr_grouping_fields,vector<AggValues>agg_state){
    
    vector<Field>functions_fields;
    int size =this->grouping_functions.size();
    for(int i{0};i<size;i++){
        AggregateType type = this->grouping_functions[i].grouping_type;
        Field f;
        double value;
        switch (type)
        {
        case SUM:
        case MIN:
        case MAX:
            value = agg_state[i].value;
            f = Field(TYPE_FLOAT, value);
            break;
        case AVG:
            //cout<<this->agg_state[i].value<<" "<<this->agg_state[i].counter*1.0<<endl;
            value = ((agg_state[i].value)/(agg_state[i].counter*1.0));
            //cout<<value<<endl;
            f = Field(TYPE_FLOAT, value);
            break;
        case COUNT:
            value =(agg_state[i].counter);
            f = Field(TYPE_FLOAT, value);
            break;       
        default:
            break;
        }
        functions_fields.push_back(f);
    }

    curr_grouping_fields.insert(curr_grouping_fields.end(), functions_fields.begin(), functions_fields.end());

    return Tuple(curr_grouping_fields);
}



//////
bool HashAggregateExecuter::getNext(Tuple* tuple){
    if(this->curr_group_index >= groups.size()){
        return false;
    }
    vector<Field>group_keys = this->groups[this->curr_group_index];
    vector<AggValues>group_states = this->states[this->curr_group_index];
    curr_group_index++;

    *tuple = this->get_output_tuple(group_keys, group_states);
        return true;
    
}

bool HashAggregateExecuter::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}

TableHeap* HashAggregateExecuter::getTableHeap(){
    return nullptr;
};

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
    for (int i = 0; i < 1000000; i++) {

        Field u1 = Field(TYPE_INT, 100 + i%10);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i%10)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i%10)).c_str()); 
        Field u4 = Field(TYPE_INT, 20 + i);               

        Tuple t_user = Tuple({u1, u2, u3, u4});
        RID rid = tables_map1["User"]->insertTuple(t_user);
        tables_rids1["User"].push_back(rid);
    }
}

int
main(){
    string DB_name = "testSeqScanDB";
    DiskManager* dm = new DiskManager(DB_name);
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    // 1. User (user_id int, firstName varchar(30), lastName varchar(30), age int)
    createUserTable(BPM, "User");
    auto st1 = chrono::high_resolution_clock::now();
    insertIntoUserTable();
    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - st1);
    cout << "inserting 1,000,000 tuple: " << duration1.count() << " microseconds" << endl;

    /*
    AbstractExecuter* seq_scan = new SeqScan(tables_map1["User"]);

    GroupingFunction gf1 = GroupingFunction();
    gf1.grouping_type = SUM;
    gf1.function_key = 3;

    GroupingFunction gf2 = GroupingFunction();
    gf2.grouping_type = COUNT;
    gf2.function_key = 3;

    GroupingFunction gf3 = GroupingFunction();
    gf3.grouping_type = AVG;
    gf3.function_key = 3;

    GroupingFunction gf4 = GroupingFunction();
    gf4.grouping_type = MAX;
    gf4.function_key = 3;

    HashAggregateExecuter* group_by = new HashAggregateExecuter(BPM, seq_scan, {0,1}, {gf1, gf2, gf3, gf4});

    group_by->open();
    cout<<"select result"<<endl;
    bool res{true};
    for(auto&col:group_by->get_output_schema()){
        cout<<col.getColName()<<"       | ";
    }
    cout<<endl;
    while(res){
        Tuple* t1 = new Tuple({});
        res = group_by->getNext(t1);
        if(res)
            t1->print();
    }

    cout<<"done"<<endl;
    */

//}