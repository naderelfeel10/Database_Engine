#include"Binder/BoundUpdateStatement.h"


BoundUpdateStatement::BoundUpdateStatement():table(nullptr),where(nullptr){}

BoundStatementType BoundUpdateStatement::type() const{
    return BoundStatementType::UPDATE;
}

void BoundUpdateStatement::PrintTree() const  {
    cout << "update statement tree" << endl;
}
/*
void BoundUpdateStatement::PrintTree() const  {
    cout << "update statement tree" << endl;

    // Table
    cout << "|-- table" << endl;
    if (table != nullptr) {
        table->PrintTree();
    } else {
        cout << "|   |-- NULL" << endl;
    }

    // Columns and values
    cout << "|-- set" << endl;

    for (size_t i = 0; i < columns.size(); i++) {
        cout << "|   |-- column: " << columns[i].getName() << endl;

        cout << "|   |   |-- value" << endl;
        if (i < values.size() && values[i] != nullptr) {
            values[i]->PrintTree();
        } else {
            cout << "|   |       |-- NULL" << endl;
        }
    }

    // WHERE
    cout << "|-- where" << endl;

    if (where != nullptr) {
        where->PrintTree();
    } else {
        cout << "|   |-- NULL" << endl;
    }
}*/