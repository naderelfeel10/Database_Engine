#ifndef PLANER_NODE_H
#define PLANER_NODE_H

#include"AbstractPlanNode.hpp"
#include"Plans.hpp"
#include"Binder/BoundInsertStatement.h"
using namespace std;

// the planner which takes an input statment, determines it's type then choose how to handle it
class Planner {

public:

    AbstractPlanNode* Plan(unique_ptr<BoundStatement> statement);

private:

    AbstractPlanNode* PlanSelect(BoundSelectStatement* statement);
    AbstractPlanNode* PlanInsert(unique_ptr<BoundInsertStatement> statement);
    AbstractPlanNode* PlanUpdate(unique_ptr<BoundUpdateStatement> statement);
    AbstractPlanNode* PlanDelete(unique_ptr<BoundDeleteStatement> statement);
    
    AbstractPlanNode* PlanCreateTable(unique_ptr<BoundCreateTableStatement> statement);
    AbstractPlanNode* PlanCreateIndex(unique_ptr<BoundCreateIndexStatement> statement);


};

#endif