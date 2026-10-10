#ifndef COL_H
#define COL_H

#include <iostream>
#include <string>
#include "Storage/Page/Field.h"
#include <variant>

class Column{

    private:
        Field* field;
        FieldType field_type;
        std::string col_name;
        int col_max_size;
        bool is_nullable{true};

    public:

        Column(const Field* field, string col_name,int col_max_size);
        Column(FieldType f_type, string col_name,int col_max_size):field(nullptr),field_type(f_type),col_name(col_name),col_max_size(col_max_size){};
        //Column(){}
        Column(): field(nullptr), field_type(TYPE_INT), col_name(""), col_max_size(0){}


        string getColName();
        void setColName(string col_name){this->col_name=col_name;}

        Field* getField();
        FieldType get_field_type(){
            return field_type;
        }
        void serializeCol(char* data);
        void deSerializeCol(char* data);

        Column& operator=(const Column& other);

        int getColSize();
        FieldType getColType(){return this->field_type;}
        void set_is_nullable(bool value){
            this->is_nullable = value;
        }
        bool get_is_nullable(){return this->is_nullable;}
        void printCol();

};

#endif