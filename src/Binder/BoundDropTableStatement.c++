#include"Binder/BoundDropTableStatement.h"

BoundDropTableStatement::BoundDropTableStatement():table_name(""){}

BoundDropTableStatement::BoundDropTableStatement(string table_name):table_name(table_name){}


BoundStatementType BoundDropTableStatement::type() const  {
        return BoundStatementType::DROP_TABLE;
}

//just printing
void BoundDropTableStatement::PrintTree() const {

    cout<<"|-- BoundTableTableStatement"<<endl;
    cout<<"|   |-- table_name: "<<table_name<<endl;


}
