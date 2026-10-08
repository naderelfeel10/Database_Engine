#include<iostream>
#include"Q_Execution/limit_operator.h"
using namespace std;



Limit::Limit(AbstractExecuter* child, Column* limit, Column* offset){
    
    this->child_operator = child;
    this->skipped = 0;
    this->returned = 0;

    Field* col_field = limit->getField();
    int field_value = col_field->getFieldValueInt();

    this->limit = field_value;

    this->offset =0;

    if(offset != nullptr){

        col_field = offset->getField();
        field_value = col_field->getFieldValueInt();

        this->offset = field_value;
    }


    delete col_field;
}

void Limit::open(){
    this->child_operator->open();
}
void Limit::close(){
    this->child_operator->close();
}

//pass table cols 
bool Limit::getNext(Tuple* tuple){

    //get the next tuple using the child operator
    //skip all tuples while increamnting the skipped counter, till it reaches the offset
    while(skipped < offset){

        Tuple* tmp_tuple = new Tuple({});
        //if all child consumed, return false
        if(child_operator->getNext(tmp_tuple) == false){
            return false;
        }
        //else just increase the skipped counter, then move ot next
        skipped++;
    }

    //after reaching the offset, start returning tuples
    //stop when done
    if(returned >= limit){
        return false;
    }

    //if all child consumed, return false
    if(child_operator->getNext(tuple) == false){
        return false;
    }

    returned++;

    return true;
}


bool Limit::has_column(string col_name){
    //string table_name = this->child_operator->getTableHeap()->getTableName();
    for(auto&col:this->get_output_schema()){
        if(col.getColName()==col_name)return true;
    }
    return false;
}
