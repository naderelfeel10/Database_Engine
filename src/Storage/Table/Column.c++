#include<iostream>
#include"Storage/Table/Column.h"
using namespace std;


Column::Column(const Field* field, std::string col_name,int col_max_size = -1 ){
    this->field = new Field(*field);
    this->field_type = field->getFieldType();
    this->col_name = col_name;
    this->col_max_size = col_max_size;
}

string Column::getColName(){return this->col_name;}
Field* Column::getField(){return this->field;}




void Column::serializeCol(char* buffer){
    
    cout<<"serializing col"<<endl;
    int offset = 0;
    
    // save col type
    switch(field_type){
        case TYPE_INT:{buffer[offset]='I';break;}
        case TYPE_STRING:{buffer[offset]='S';break;} 
        case TYPE_FLOAT:{buffer[offset]='F';break;} 
        case TYPE_BOOL:{buffer[offset]='B';break;} 
        default : break;
    }
    offset+=1;

    // save some meta data
    memcpy(buffer+offset, &this->col_max_size, sizeof(col_max_size));
    offset+=sizeof(col_max_size);

    int col_name_size = col_name.length();
    memcpy(buffer+offset, &col_name_size, sizeof(int));
    offset +=sizeof(int);

    memcpy(buffer+offset, col_name.c_str(), col_name_size);
    offset += col_name_size;

    // then serialize the field itself
    if(field !=nullptr){
        this->field->serialize(buffer+offset);
        offset+=field->getSerializedSize();
    }
    cout<<"col is serialized successfuly"<<endl;

}

void Column::deSerializeCol(char* buffer){

    int offset = 0;
    // get meta data
    char type = buffer[offset];
    cout<<"type : "<<type<<endl;
    switch(type){
        case 'I':{this->field_type=TYPE_INT;break;}
        case 'S':{this->field_type=TYPE_STRING; break;} 
        case 'F':{this->field_type=TYPE_FLOAT;break;} 
        case 'B':{this->field_type=TYPE_BOOL;break;} 
        default : break;
    }
    offset+=1;

    memcpy(&this->col_max_size, buffer+offset, sizeof(col_max_size));
    offset+=sizeof(col_max_size);

    int col_name_size;
    memcpy(&col_name_size,buffer+offset, sizeof(col_name_size));
    offset += sizeof(col_name_size);

    this->col_name.assign(buffer + offset, col_name_size);
    offset += col_name_size;

    // get the actual field
    if(field!=nullptr){
        this->field->deserialize(buffer+offset);
        offset+=this->field->getSerializedSize();
    }
}

int Column::getColSize() {
    return 1 + sizeof(this->col_max_size) + sizeof(int) + this->col_name.length()+ 
           (field == nullptr ? 0 : this->field->getSerializedSize());
}
/*
// c1 = c2;
// operator overloading 
Column& Column::operator=(const Column& other) {
    if (this == &other) return *this; 

    delete this->field;

    // make a deep copy intp the new one
    this->col_name = other.col_name;
    this->col_max_size = other.col_max_size;

    this->field = new Field(*other.field);

    return *this;
}
*/

//= operator overloading, fixing nullptr field
Column& Column::operator=(const Column& other){
    if(this == &other) return *this;

    field_type = other.field_type;
    col_name = other.col_name;
    col_max_size = other.col_max_size;

    //here i have to check it there is a field, because it's used in schema and schema doesnot have field
    if(other.field != nullptr){
        //if null create a dummy one with same values
        if(field == nullptr)
            field = new Field(*other.field);
        else
            *field = *other.field;
    }
    else{
        delete field;
        field = nullptr;
    }

    return *this;
}

/*
// making printing more readable
void Column::printCol() {
    cout<<"col name : "<<this->col_name<<endl;
    cout<<"col type : "<<getFieldTypeasString(this->field_type)<<endl;
    cout<<"------------" <<endl;
    
    
    //auto val = this->field->getFieldValue();
//
    //if (const int* i = get_if<int>(&val)) {
    //    cout << *i;
    //} 
    //else if(const double* f = get_if<double>(&val)) {
    //    cout << *f;
    //} 
    //else if(const bool* b = get_if<bool>(&val)) {
    //    cout << (*b ? "true" : "false");
    //} 
    //else if(const std::string* s = get_if<std::string>(&val)) {
    //    cout << *s;
    //}
//
    //cout << "\n------------" << endl;
    
}
*/
void Column::printCol() {
    // Prints cleanly inline as: name:type(size)
    cout << this->col_name << ":" << getFieldTypeasString(this->field_type) << "(" << this->col_max_size << ")";
}
/*
int main(){
    Field* f1 = new Field(TYPE_INT, 3);
    Column* col1 = new Column(f1,"user_id",4);
    col1->printCol();
    
    char* buffer1 = new char[100];
    col1->serializeCol(buffer1);

    Field* f2 = new Field(TYPE_INT);
    Column* col2 = new Column(f2,"");
    col2->deSerializeCol(buffer1);
    col2->printCol();
}
*/