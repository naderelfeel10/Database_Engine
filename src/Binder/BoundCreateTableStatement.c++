#include"Binder/BoundCreateTableStatement.h"

BoundCreateTableStatement::BoundCreateTableStatement():table_name(""),schema_name(""),if_not_exists(false){}

BoundStatementType BoundCreateTableStatement::type() const  {
        return BoundStatementType::CREATE_TABLE;
}

//just printing
void BoundCreateTableStatement::PrintTree() const {

    cout << "|-- BoundCreateTableStatement" << endl;
    cout << "|   |-- table_name: "
         << table_name << endl;

    cout << "|   |-- if_not_exists: "
         << (if_not_exists ? "true" : "false")
         << endl;

   
    if (!constraints.empty()) {
        cout << "|   |-- Constraints"
             << endl;

        for (const auto& constraint : constraints) {
            cout << "|   |   |-- ";

            switch (constraint.type) {
                case BoundConstraintType::PRIMARY_KEY:
                    cout << "PRIMARY KEY";
                    break;

                case BoundConstraintType::UNIQUE:
                    cout << "UNIQUE";
                    break;

                case BoundConstraintType::FOREIGN_KEY:
                    cout << "FOREIGN KEY";
                    break;

            }
            cout << endl;
        }
    }
}