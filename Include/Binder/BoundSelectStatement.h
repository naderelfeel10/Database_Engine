#ifndef SELECT_H
#define SELECT_H

#include"Expression.h"
#include"BoundExpression.h"
#include"BoundStatement.h"

#include<iostream>
#include<vector>
using namespace std;

struct SelectItem {
    Expression* expression;
    // each item is gonna have an alias for it's name
    // to handle diff names like (select name as "Name") or select age+1,
    string alias;
};

enum class JoinType {
    INNER,
    LEFT,
    RIGHT
};

class BoundJoinClause{

    public:
        JoinType type;
        BoundTable right_table;
        BoundExpression* condition;
        
        BoundJoinClause(JoinType type, BoundTable& right_table, BoundExpression*condition);
};


struct BoundSelectItem {
    BoundExpression* expression;
    string alias;
};

// order by is just asc or desc 
enum class OrderType { ASC, DESC };

class BoundOrderBy{ 
    public:
        BoundExpression* expression;// a col to sort on
        OrderType order_type = OrderType::ASC;

        BoundOrderBy(BoundExpression* expr, OrderType order_type);
};



/*
    in a select statement we expect alot of diff options :

    1. select items like [select name, age, salary]
    2."from table", where we use to fetch tuples
    3.might be join operations
    4. where clause and it's predicate
    5. might have aggregations and group by
    6.might have order by
    7. limit and offset
    // still more options to do later 

*/

class BoundSelectStatement : public BoundStatement { 

    public:
        //items to project on
        vector<BoundSelectItem> select_list;
        //table to fetch data from
        BoundTable from_table;

        //check if there are joins
        vector<BoundJoinClause> joins;  
        // check for where clause to filter based on it
        BoundExpression* where = nullptr;
        // aggregations
        vector<BoundExpression*> group_by;
        // agg filter
        BoundExpression* having = nullptr;
        //sorting 
        vector<BoundOrderBy> order_by;
        //limit and it's offset
        BoundExpression* limit = nullptr;
        BoundExpression* offset = nullptr;
    
    
    BoundStatementType type() const override;

    //just printing   
    void PrintTree() const override;
};

#endif

