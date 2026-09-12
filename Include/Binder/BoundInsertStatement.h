#ifndef INSERT_H
#define INSERT_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"
#include"BoundSelectStatement.h"


#include<iostream>
#include<vector>
using namespace std;


//bound insert statement is gonna be like :
//insert into User (user_id, age) values [1,20],[2,30];
class BoundInsertStatement : public BoundStatement {

public:
    //table to insert into
    //cols to insert into 
    //values 

    BoundTable* table;
    vector<Column> columns;
    vector<BoundExpression*> values;

    BoundInsertStatement();
    BoundInsertStatement(BoundTable* table, vector<Column> columns, vector<BoundExpression*> values);

    BoundStatementType type() const override;

    BoundInsertStatement& operator=(const BoundInsertStatement& other);


    //just printing
    void PrintTree() const override;
};

#endif