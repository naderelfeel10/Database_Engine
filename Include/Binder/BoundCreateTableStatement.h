#ifndef BOUND_CREATE_TABLE_STATEMENT_H
#define BOUND_CREATE_TABLE_STATEMENT_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"

#include <string>
#include <vector>

using namespace std;



struct BoundForeignKey {
    vector<string> columns;
    string referenced_table;
    vector<string> referenced_columns;
};


enum class BoundConstraintType {
    PRIMARY_KEY,
    UNIQUE,
    FOREIGN_KEY
};


//table constraints : 
//type like : pk, fk, ..
//vector of col names
//referenced table name if found
//referenced cols if found

struct BoundTableConstraint {

    BoundConstraintType type;
    vector<string> columns;
    string referenced_table;
    vector<string> referenced_columns;
};



//table has :

//table_name
//schema
//bool to check if exists
//vector of cols
//vector of constraints

class BoundCreateTableStatement : public BoundStatement { 

public:

    string table_name;
    string schema_name;
    bool if_not_exists;

    vector<Column> columns;
    vector<BoundTableConstraint> constraints;

    BoundCreateTableStatement();

    BoundStatementType type() const override;

    void PrintTree() const override ;
};

#endif