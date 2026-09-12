#include"Binder/BoundDeleteStatement.h"



BoundDeleteStatement::BoundDeleteStatement() :table(nullptr), where(nullptr){}

BoundStatementType BoundDeleteStatement::type() const {

    return BoundStatementType::DELETE;
}


void BoundDeleteStatement::PrintTree() const  {

    cout << "|-- BoundDeleteStatement" << endl;

    if (table) {
        cout << "|   |-- Table: "
             << table->table_name << endl;

        cout << "|   |-- table_oid: "
             << table->table_oid << endl;

    }
    if (where) {
        cout << "|   |-- WHERE" << endl;
        where->PrintTree();
    }

    else {
        cout << "|   |-- WHERE: NULL (delete all)" << endl;
    }
}


