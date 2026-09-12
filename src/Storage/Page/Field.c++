#include<iostream>
#include<string>
#include<cstring>
#include "Storage/Page/Field.h"
using namespace std;

//constrcutors to accecpt all types of data :
//INT field
Field::Field(FieldType type,int value){
    fieldType  = FieldType(type);
    VALUE_INT = value;
    this->size = sizeof(value);
}
//double field
Field::Field(FieldType type,double value){
    fieldType  = FieldType(type);
    this->size = sizeof(value);
    VALUE_FLOAT = value;
    
}
//bool field
Field::Field(FieldType type,bool value){
    fieldType  = FieldType(type);
    VALUE_BOOL = value;
    this->size = sizeof(value);
}
//string field
Field::Field(FieldType type,const char* value){
    fieldType  = FieldType(type);
    this->size = strlen(value);
    //VALUE_STRING = value;
    this->VALUE_STRING = new char[this->size + 1];
    strcpy((char*)this->VALUE_STRING, value);
}

Field::Field(const Field& other) {

    this->fieldType = other.fieldType;
    this->size = other.size;
    this->is_null = other.is_null;

    if (fieldType == TYPE_STRING && other.VALUE_STRING != nullptr) {

        this->VALUE_STRING = new char[size + 1];
        memcpy((void*)this->VALUE_STRING, other.VALUE_STRING, size + 1);
    }
    else {
        this->VALUE_STRING = nullptr; // Or copy other primitive types
        this->VALUE_INT = other.VALUE_INT;
        this->VALUE_FLOAT = other.VALUE_FLOAT;
        this->VALUE_BOOL = other.VALUE_BOOL;
    }
}


Field::Field(FieldType type):fieldType(type){

        this->is_null = true;
        this->size = 0;

        switch (type)
        {
        case TYPE_INT:
            VALUE_INT = 0;
            break;
        case TYPE_FLOAT:
            VALUE_FLOAT = 0.0;
            break;
        case TYPE_STRING:
            VALUE_STRING = nullptr;
            break;
        case TYPE_BOOL:
            VALUE_BOOL = false;
            break;
        default:
            break;
        }

}

FieldType Field::getFieldType()const{
    return this->fieldType;
}

bool Field::isNull(){
    return this->is_null;
}

int Field::getSize(){
     return this->size;
}


int Field::getSerializedSize() const {
    return 1 + 1+ 4 + size; 
}

// making printing more readable
/*
void Field::print()  {

        switch (fieldType) {
            case TYPE_INT:
                cout <<"value : " << VALUE_INT<<endl;
                cout<<"size : "<<size<<endl;
                break;

            case TYPE_FLOAT: 
                if (this->is_null) {
                    cout << "value : [NULL]" << endl;
                    cout << "size : 0" << endl;
                } else {
                    cout << "value : " << VALUE_FLOAT << endl;
                    cout << "size : " << size << endl;
                }
                break;

            case TYPE_STRING: 
                if (this->is_null) {
                    cout << "value : [NULL]" << endl;
                    cout << "size : 0" << endl;
                } else {
                    cout << "value : " << VALUE_STRING << endl;
                    cout << "size : " << size << endl;
                }
                break;

            case TYPE_BOOL:   
                cout <<"value : " << VALUE_BOOL<<endl;
                cout<<"size : "<<size<<endl;
                break;

        }
    cout << endl;
    
    }
*/
/*
void Field::print() {
    const int VAL_WIDTH = 15;
    const int SIZE_WIDTH = 8;

    // 1. Extract logic to a helper for better clarity
    auto getDisplayValue = [&]() -> std::string {
        if (this->is_null) return "[NULL]";

        switch (fieldType) {
            case TYPE_INT:    return std::to_string(VALUE_INT);
            case TYPE_FLOAT:  return std::to_string(VALUE_FLOAT);
            case TYPE_BOOL:   return (VALUE_BOOL ? "true" : "false");
            case TYPE_STRING: return (VALUE_STRING ? VALUE_STRING : "[EMPTY]");
            default:          return "??";
        }
    };

    // 2. Format the output clearly
    std::string displayValue = getDisplayValue();
    int displaySize          = this->is_null ? 0 : this->size;

    std::cout << "| " << std::left  << std::setw(VAL_WIDTH)  << displayValue 
              << " | " << std::right << std::setw(SIZE_WIDTH) << displaySize 
              << " |"  << std::endl;
}*/

// a better way to print
void Field::print() {
    const int VAL_WIDTH = 15;
    
    switch (fieldType) {
        case TYPE_INT:   std::cout << std::left << std::setw(VAL_WIDTH) << VALUE_INT;   break;
        case TYPE_FLOAT: std::cout << std::left << std::setw(VAL_WIDTH) << VALUE_FLOAT; break;
        case TYPE_BOOL:  std::cout << std::left << std::setw(VAL_WIDTH) << (VALUE_BOOL ? "true" : "false"); break;
        case TYPE_STRING: std::cout << std::left << std::setw(VAL_WIDTH) << (VALUE_STRING ? VALUE_STRING : "[EMPTY]"); break;
        case TYPE_NULL: std::cout << std::left << std::setw(VAL_WIDTH) << "NULL"; break;
        default:         std::cout << std::left << std::setw(VAL_WIDTH) << "??"; break;
    }
    std::cout << " | ";
}




int Field::getFieldValueInt()const {
    return VALUE_INT;
}

float Field::getFieldValueFloat()const {
    return VALUE_FLOAT;
}

bool Field::getFieldValueBool()const {
    return VALUE_BOOL;
}

const char* Field::getFieldValueStr()const {
    return VALUE_STRING;
}

/*
FieldValue Field::getFieldValue()const{
    if(this->fieldType == TYPE_INT) return this->getFieldValueInt();
    if(this->fieldType == TYPE_FLOAT) return this->getFieldValueFloat();
    if(this->fieldType == TYPE_BOOL) return this->getFieldValueBool();

    return string(this->getFieldValueStr());

}
*/
// updated version
FieldValue Field::getFieldValue() const
{
    switch (fieldType)
    {
    case TYPE_INT:
        return VALUE_INT;
    case TYPE_FLOAT:
        return VALUE_FLOAT;
    case TYPE_BOOL:
        return VALUE_BOOL;
    case TYPE_STRING:
        return string(VALUE_STRING);

    case TYPE_NULL:
        return nullptr;
    default:
        throw runtime_error("unknown fieldtype");
    }
}


void Field::serialize(char* buffer){

        //starting with offset=1, to save offset=0 for field type
        int offset = 1;
        //save size
        memcpy(buffer+offset,&this->size,sizeof(size));
        offset+=sizeof(size);
        //save is_null
        memcpy(buffer+offset,&this->is_null,sizeof(is_null));
        offset+=sizeof(is_null);

        switch (fieldType)
        {
        case TYPE_INT:{
            buffer[0] = 'I';

            if(!is_null)
                memcpy(buffer+offset,&VALUE_INT,sizeof(VALUE_INT));
            else 
                memset(buffer+offset,0,sizeof(VALUE_INT));
            
            offset+=sizeof(VALUE_INT);

            break;
        }
        case TYPE_FLOAT:{
            buffer[0] = 'F';

            if(!is_null)
                memcpy(buffer+offset,&VALUE_FLOAT,sizeof(VALUE_FLOAT));
            else 
                memset(buffer+offset,0,sizeof(VALUE_FLOAT));
            
            offset+=sizeof(VALUE_FLOAT);

            break;
        }
        case TYPE_BOOL:{
            buffer[0] = 'B';

            if(!is_null)
                memcpy(buffer+offset,&VALUE_BOOL,sizeof(VALUE_BOOL));
            else 
                memset(buffer+offset,0,sizeof(VALUE_BOOL));
            
            offset+=sizeof(VALUE_BOOL);

            break;
        }
        case TYPE_STRING:{
            buffer[0] = 'S';

            if(!is_null)
                memcpy(buffer+offset,VALUE_STRING,size);
            else 
                memset(buffer+offset,0,size);

            buffer[offset+size] = '\0';
            offset += size;
            break;
        }
        //no need, keep if for now
        case TYPE_NULL:{
            //type byte is N means null
            buffer[0] = 'N';
            break;
        }
        default:
            throw runtime_error("incorrect data type");
            break;
        }
    }


void Field::deserialize(char* buffer){
        //fetch type from first bit
        char type = buffer[0];
        int offset = 1;
        
        memcpy(&this->size,buffer+offset,sizeof(size));
        offset+=sizeof(size);

        memcpy(&this->is_null,buffer+offset,sizeof(is_null));
        offset+=sizeof(is_null);

        switch (type)
        {
            //handle null fields
        case 'N':{
            this->fieldType = TYPE_NULL;
            this->is_null = true;
            this->size = 0;
            break;
        }
        case 'I':{
            this->fieldType = TYPE_INT;
            memcpy(&this->VALUE_INT,buffer+offset,sizeof(VALUE_INT));
            offset+=sizeof(VALUE_INT);
            break;
        }
        case 'F':{
            this->fieldType = TYPE_FLOAT;
            memcpy(&this->VALUE_FLOAT,buffer+offset,sizeof(VALUE_FLOAT));
            offset+=sizeof(VALUE_FLOAT);
            break;
        }
        case 'B':{
            this->fieldType = TYPE_BOOL;
            memcpy(&this->VALUE_BOOL,buffer+offset,sizeof(VALUE_BOOL));
            offset+=sizeof(VALUE_BOOL);
            break;
        }
        case 'S':{
            this->fieldType = TYPE_STRING;
            VALUE_STRING = new char[size+1];
            //copy base size withoud the terminator
            memcpy((void*)VALUE_STRING,buffer+offset,size);
            //then add the terminator manually
            ((char*)VALUE_STRING)[size] = '\0';

            offset+=size;
            break;
        }

        default:
            throw runtime_error("field type is not found");
            break;
        }

}


void Field::setValue(int value){
    this->VALUE_INT = value;
    this->fieldType = TYPE_INT;
    this->is_null = false;
}


void Field::setValue(double value){
    this->VALUE_FLOAT = value;
    this->fieldType = TYPE_FLOAT;
    this->is_null = false;
}


void Field::setValue(bool value){
    this->VALUE_BOOL = value;
    this->fieldType = TYPE_BOOL;
    this->is_null = false;
}
/*
void Field::setValue(const char* value){
    
    char* new_string = new char[strlen(value) + 1];
    strcpy(new_string, value);

    this->VALUE_STRING = new_string;
    this->is_null = false;
}
*/
void Field::setValue(const char* value) {


    if (value == nullptr) {
        this->is_null = true;
        return;
    }

    int len = strlen(value);
    char* new_string = new char[len + 1];
    
    strcpy(new_string, value);

    this->VALUE_STRING = new_string;
    this->fieldType = TYPE_STRING;
    this->is_null = false;
}




/*
Field& Field::operator=(const Field& other) {
    if (this == &other) return *this;

    if (this->fieldType == TYPE_STRING && this->VALUE_STRING != nullptr) {
        delete[] this->VALUE_STRING;
        this->VALUE_STRING = nullptr;
    }

    this->fieldType = other.fieldType;
    this->size = other.size;
    this->is_null = other.is_null;

    if (other.fieldType == TYPE_STRING && other.VALUE_STRING != nullptr) {
        this->VALUE_STRING = new char[other.size + 1];
        memcpy((void*)this->VALUE_STRING, other.VALUE_STRING, other.size + 1);
        
    } else {
        this->VALUE_INT = other.VALUE_INT;
        this->VALUE_FLOAT = other.VALUE_FLOAT;
        this->VALUE_BOOL = other.VALUE_BOOL;
    }

    return *this;
}
*/
Field& Field::operator=(const Field& other) {
    if (this == &other) return *this;

    // 1. SAFELY delete the string ONLY if the current type actually WAS a string
    if (this->fieldType == TYPE_STRING && this->VALUE_STRING != nullptr) {
        delete[] this->VALUE_STRING;
        this->VALUE_STRING = nullptr;
    }

    // 2. Copy metadata flags
    this->fieldType = other.fieldType;
    this->size = other.size;
    this->is_null = other.is_null;

    // 3. Copy the union data safely based STRICTLY on the source type tag
    if (!other.is_null) {
        switch (other.fieldType) {
            case TYPE_STRING:
                if (other.VALUE_STRING != nullptr) {
                    this->VALUE_STRING = new char[other.size + 1];
                    memcpy((void*)this->VALUE_STRING, other.VALUE_STRING, other.size + 1);
                } else {
                    this->VALUE_STRING = nullptr;
                }
                break;
                
            case TYPE_INT:
                this->VALUE_INT = other.VALUE_INT; // Only write to the active variant
                break;
                
            case TYPE_FLOAT:
                this->VALUE_FLOAT = other.VALUE_FLOAT;
                break;
                
            case TYPE_BOOL:
                this->VALUE_BOOL = other.VALUE_BOOL;
                break;
        }
    }

    return *this;
}

bool Field::operator==(const Field& other)const{

    if(this->fieldType != other.getFieldType()){
        return false;
    }

    //check if is_null first
    if(this->is_null){
        if(other.is_null){
            return true;
        }else{
            return false;
        }
    }

    if(other.is_null){
        if(this->is_null){
            return true;
        }else{
            return false;
        }
    }

    switch (other.getFieldType())
    {
    case TYPE_INT:
        return (this->getFieldValueInt() == other.getFieldValueInt());
        break;
    
    case TYPE_BOOL:
        return (this->getFieldValueBool() == other.getFieldValueBool());
        break;
    
    case TYPE_FLOAT:
        return (this->getFieldValueFloat() == other.getFieldValueFloat());
        break;

    case TYPE_STRING:
        return ( string(this->getFieldValueStr()) == string(other.getFieldValueStr()) );
        break;

    default:
        return false;
        break;
    }
}

bool Field::operator>(const Field& other)const{
    if(this == &other) return false;
    if(this->fieldType != other.getFieldType()) return false;
    
    //check for nulls
    if(this->is_null || other.is_null){
        throw runtime_error("null fields can't be compared");
    }
    switch(other.fieldType){
        case TYPE_INT:{
            return (this->getFieldValueInt() > other.getFieldValueInt());
            break;
        }
        case TYPE_FLOAT:{
            return (this->getFieldValueFloat() > other.getFieldValueFloat());
            break;
        }
        case TYPE_BOOL:{
            return (this->getFieldValueBool() > other.getFieldValueBool());
            break;
        }
        case TYPE_STRING:{
            return (string(this->getFieldValueStr()) > string(other.getFieldValueStr()) );
            break;
        }
        default:
        return false;
    }
}

bool Field::operator<(const Field& other)const{
    if(this == &other) return false;
    if(this->fieldType != other.getFieldType()) return false;

    //check for nulls
    if(this->is_null || other.is_null){
        throw runtime_error("null fields can't be compared");
    }

    switch(other.fieldType){
        case TYPE_INT:{
            return (this->getFieldValueInt() < other.getFieldValueInt());
            break;
        }
        case TYPE_FLOAT:{
            return (this->getFieldValueFloat() < other.getFieldValueFloat());
            break;
        }
        case TYPE_BOOL:{
            return (this->getFieldValueBool() < other.getFieldValueBool());
            break;
        }
        case TYPE_STRING:{
            return (string(this->getFieldValueStr()) < string(other.getFieldValueStr()) );
            break;
        }
        default:
        return false;
    }
}

bool Field::operator>=(const Field& other)const{
    if(this == &other) return true;
    if(this->fieldType != other.getFieldType()) return false;

    //check for nulls
    if(this->is_null || other.is_null){
        throw runtime_error("null fields can't be compared");
    }

    switch(other.fieldType){
        case TYPE_INT:{
            return (this->getFieldValueInt() >= other.getFieldValueInt());
            break;
        }
        case TYPE_FLOAT:{
            return (this->getFieldValueFloat() >= other.getFieldValueFloat());
            break;
        }
        case TYPE_BOOL:{
            return (this->getFieldValueBool() >= other.getFieldValueBool());
            break;
        }
        case TYPE_STRING:{
            return (string(this->getFieldValueStr()) >= string(other.getFieldValueStr()) );
            break;
        }
        default:
        return false;
    }
}

bool Field::operator<=(const Field& other)const{
    if(this == &other) return true;
    if(this->fieldType != other.getFieldType()) return false;

    //check for nulls
    if(this->is_null || other.is_null){
        throw runtime_error("null fields can't be compared");
    }
    
    switch(other.fieldType){
        case TYPE_INT:{
            return (this->getFieldValueInt() <= other.getFieldValueInt());
            break;
        }
        case TYPE_FLOAT:{
            return (this->getFieldValueFloat() <= other.getFieldValueFloat());
            break;
        }
        case TYPE_BOOL:{
            return (this->getFieldValueBool() <= other.getFieldValueBool());
            break;
        }
        case TYPE_STRING:{
            return (string(this->getFieldValueStr()) <= string(other.getFieldValueStr()) );
            break;
        }
        default:
        return false;
    }
}


Field::~Field(){
    if(fieldType == TYPE_STRING && VALUE_STRING != nullptr){
        delete[]VALUE_STRING;
        //cout<<"deleted successfuly!"<<endl;
    }
}

/*
int main(){

    FieldType int_field = TYPE_INT;
    Field f1(int_field,5);
    f1.print();

    FieldType string_field = TYPE_STRING;
    Field f2(string_field,"nader");
    f2.print();

    FieldType float_field = TYPE_FLOAT;
    Field f3(float_field,6.4);
    f3.print();

    FieldType bool_field = TYPE_BOOL;
    Field f4(bool_field,true);
    f4.print();

    Field f5(string_field);
    f5.print();

    // testing serialize 
    char* buffer = new char[100];
    f1.serialize(buffer);
    for(int i = 0; i < 10; i++) {
    // Cast to unsigned char to see the actual byte values (0-255)
    cout << (int)(unsigned char)buffer[i] << " ";
    }
    cout<<endl;
    delete []buffer;

    char* buffer2 = new char[100];
    f2.serialize(buffer2);
    for(int i = 0; i < 12; i++) {
    // Cast to unsigned char to see the actual byte values (0-255)
    cout << (int)(unsigned char)buffer2[i] << " ";
    }
    cout<<endl;

    Field* f6 = new Field(int_field);
    f6->deserialize(buffer2);
    f6->print();
    delete f6;

    //

    Field dm_f1(TYPE_STRING,"nader");
    Field dm_f2(TYPE_STRING,"nader");
    if(dm_f1 == dm_f2){
        cout<<"f1 = f2";
    }else{
        cout<<"f1 != f2";
    }
}
*/