#ifndef DELETE_H
#define DELETE_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"
#include"BoundSelectStatement.h"


#include<iostream>
#include<vector>
using namespace std;


class BoundDeleteStatement : public BoundStatement {
public:
    BoundTable* table;
    BoundExpression* where;

    BoundDeleteStatement();

    BoundStatementType type()const override;

    //just printing

    void PrintTree() const override;
};

#endif