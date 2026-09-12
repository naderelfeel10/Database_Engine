#include<iostream>
#include"Q_Execution/SortAggregateExecuter.h"
using namespace std;

void SortAggregateExecuter::open(){
    this->table->open();
    set_output_schema();
    //i will sort the table based on grouping key
    Column sorting_col = this->table->get_output_schema()[grouping_keys[0]];

    this->sorted_table = new ExternalMergeSort(BPM, table, sorting_col, ASC);
    this->sorted_table->open();
    // get the first tuple 
    this->has_tuple = this->sorted_table->getNext(&this->next_tuple);
}



void SortAggregateExecuter::close(){
    this->table->close();
}


void SortAggregateExecuter::set_output_schema(){
    //output grouping vector + grouping functions   
    this->output_schema = this->table->get_output_schema();
    /*
    this->output_schema.clear();
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
        cout<<col.getColName()<<endl;
        this->output_schema.push_back(col);
    }

    for(auto&col:this->output_schema)col.printCol();
}



vector<Column> SortAggregateExecuter::get_output_schema(){
    return this->output_schema;
}


string SortAggregateExecuter::get_function_string(GroupingFunction func){
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
        break;
    }

}
vector<Field> SortAggregateExecuter::get_grouping_fields(Tuple tuple){
    vector<Field> tmp_fields_vector;
    for(int&index:this->grouping_keys){
        Field tmp_f =  tuple.fields[index];
        tmp_fields_vector.push_back(tmp_f);
    }
    return tmp_fields_vector;
}


void SortAggregateExecuter::update_aggregate(){
    for(size_t i{0};i<this->grouping_functions.size();i++){
        //fetch function type 
        AggregateType grouping_type = grouping_functions[i].grouping_type;
        int col_index = grouping_functions[i].function_key;
        
        //fetch the new value
        //next_tuple.print();
        Field agg_field = this->next_tuple.fields[col_index];

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

Tuple SortAggregateExecuter::get_output_tuple(){
    
    vector<Field>functions_fields;
    int size =this->grouping_functions.size();
    for(int i{0};i<size;i++){
        AggregateType type = this->grouping_functions[i].grouping_type;

        Column g_col = this->output_schema[this->grouping_functions[i].function_key];
        //g_col.printCol();
        FieldType field_type = g_col.getColType();

        Field f;
        double value;
        switch (type)
        {
        case SUM:
        case MIN:
        case MAX:{
            value = this->agg_state[i].value;
            
            if(field_type == TYPE_INT){
                f = Field(TYPE_INT, static_cast<int>(value));
            }
            else if(field_type==TYPE_FLOAT){
                f = Field(TYPE_FLOAT, value);
            }
            else if(field_type == TYPE_BOOL){
                f = Field(TYPE_BOOL, static_cast<bool>(value));
            }
        
            break;
        }
        case AVG:
            //cout<<this->agg_state[i].value<<" "<<this->agg_state[i].counter*1.0<<endl;
            value = ((this->agg_state[i].value)/(this->agg_state[i].counter*1.0));
            //cout<<value<<endl;
            f = Field(TYPE_FLOAT, value);
            break;
        case COUNT:
            value =(this->agg_state[i].counter);
            f = Field(TYPE_INT, value);
            break;       
        default:
            break;
        }
        functions_fields.push_back(f);
    }
    vector<Field>tmp_grouping_fields = this->curr_grouping_fields;

    tmp_grouping_fields.insert(tmp_grouping_fields.end(), functions_fields.begin(), functions_fields.end());

    Tuple res = Tuple(tmp_grouping_fields);

    //cout << "tuple:" << endl;
    //res.print();
    return res;
}


bool SortAggregateExecuter::is_same_group(){
    for(int i{0};i<this->curr_grouping_fields.size();i++){
        if(this->curr_grouping_fields[i] == this->next_tuple.fields[this->grouping_keys[i]] ){
            continue;
        }
        return false;
    }
    return true;
}
//////
/*
bool SortAggregateExecuter::getNext(Tuple* tuple){
    // check if it still have data
    if(!has_tuple){
         return false;
    }

    // if still data then :
    //1. update the aggregation state;
    //2. get next tuple
        // if no more data : output result and return true;
        // if still data and same group : update result_state 
        // if still data but diff group: then output the result and return true

    agg_state.assign(grouping_functions.size(), AggValues{});
    this->curr_grouping_fields = this->get_grouping_fields(this->next_tuple);
    
    while(this->has_tuple){

        agg_state.assign(grouping_functions.size(), AggValues{});
        curr_grouping_fields = get_grouping_fields(next_tuple);

        while(this->has_tuple){
            //update the result, the fetch the next tupe
            update_aggregate();
            this->has_tuple = this->sorted_table->getNext(&this->next_tuple);
            //if EOF
            if(!has_tuple){
                // *tuple = this->get_output_tuple();
                //return true;

                //handle having
                Tuple* result = new Tuple(this->get_output_tuple());
                //result->print();
                if(having == nullptr || having->evaluate(result,this->output_schema)){
                    *tuple = *result;
                    return true;
                }

            }
            // if same group
            else if(is_same_group()){
                continue;
            }
            //if diff group
            else {

                // *tuple = this->get_output_tuple();
                //agg_state.assign(grouping_functions.size(), AggValues{});
                //return true;
                

                //handle having
                Tuple* result = new Tuple(this->get_output_tuple());
                //result->print();
                //for(auto&col:this->output_schema)col.printCol();
                //cout<<"////"<<endl;
                //for(auto&col:this->table->get_output_schema())col.printCol();
                if(having == nullptr || having->evaluate(result, this->output_schema)){
                    *tuple = *result;
                    return true;
                }
            }

        }
    }
        return false;
}
*/
// getNext rewrite to solve having issues 
bool SortAggregateExecuter::getNext(Tuple* tuple)
{
    if (!has_tuple)
        return false;

    while(has_tuple){

        //we have to reset the agg_state before consuming new group
        agg_state.assign(grouping_functions.size(),AggValues{});

        curr_grouping_fields = get_grouping_fields(next_tuple);

        //fetch all data from one group and update it's agg state
        while(has_tuple){
            update_aggregate();

            //fetch from sorted table
            has_tuple = sorted_table->getNext(&next_tuple);

            //table is done
            if (!has_tuple)
                break;

            //if diff group, then break
            if (!is_same_group())
                break;
        }

        //the group is compele here
        //so get the outputt tuple with the updated agg state then check againest having condition
        Tuple result = get_output_tuple();

        //result.print();

        //evaluate having and return the tuple if true

        if (having == nullptr || having->evaluate(&result, output_schema)){
            *tuple = result;
            return true;
        }

        // having is false 
        // next_tuple is already the first tuple in the next group
        if (!has_tuple)
            return false;
    }

    return false;
}

bool SortAggregateExecuter::has_column(string col_name){
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}
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
    for (int i = 0; i < 100; i++) {

        Field u1 = Field(TYPE_INT, 100 + i%10);                
        Field u2 = Field(TYPE_STRING, ("First_" + to_string(i)).c_str());
        Field u3 = Field(TYPE_STRING, ("Last_" + to_string(i)).c_str()); 
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
    insertIntoUserTable();

    AbstractExecuter* seq_scan = new SeqScan(tables_map1["User"]);

    GroupingFunction gf1 = GroupingFunction();
    gf1.grouping_type = SUM;
    gf1.function_key = 3;

    GroupingFunction gf2 = GroupingFunction();
    gf2.grouping_type = AVG;
    gf2.function_key = 3;
    SortAggregateExecuter* group_by = new SortAggregateExecuter(BPM, seq_scan, {0}, {gf1, gf2});

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

}*/