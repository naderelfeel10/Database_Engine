#ifndef BOUND_DROP_TABLE_STATEMENT_H
#define BOUND_DROP_TABLE_STATEMENT_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"

#include <string>
#include <vector>

using namespace std;


class BoundDropTableStatement : public BoundStatement { 

public:

    string table_name;    
    BoundDropTableStatement();
    BoundDropTableStatement(string table_name);

    BoundStatementType type() const override;

    void PrintTree() const override ;
};

#endif