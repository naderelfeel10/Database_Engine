#ifndef UPDATE_H
#define UPDATE_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"
#include"BoundSelectStatement.h"


#include<iostream>
#include<vector>
using namespace std;


class BoundUpdateStatement : public BoundStatement {
public:
    BoundTable* table;

    //cols to update
    vector<Column> columns;
    //values to update with
    vector<BoundExpression*> values;
    //the condtion
    BoundExpression* where;

    BoundUpdateStatement();

    BoundStatementType type() const override;
    void PrintTree()const override;
};

#endif