#include"Binder/BoundCreateIndexStatement.h"
#include<iostream>
BoundCreateIndexStatement::BoundCreateIndexStatement():table_name(""),if_not_exists(false){
}

BoundStatementType BoundCreateIndexStatement::type() const  {
        return BoundStatementType::CREATE_INDEX;
}

//just printing
void BoundCreateIndexStatement::PrintTree() const {

    cout << "|-- BoundCreateIndexStatement" << endl;
    cout << "|   |-- table_name: "
         << table_name << endl;

    cout << "|   |-- if_not_exists: "
         << (if_not_exists ? "true" : "false")
         << endl;

   
}