#ifndef SEQ_SCAN_PLAN_NODE_H
#define SEQ_SCAN_PLAN_NODE_H

#include"AbstractPlanNode.hpp"
#include"Q_Execution/HashAggregateExecuter.h"

using namespace std;

//first type is seq_scan
class SeqScanPlan : public AbstractPlanNode{

public:

    int table_oid;
    SeqScanPlan(int table_oid);

    //just printing
    void PrintTree(int indent = 0) const override;
};


//now filter and it need the child node and a condition to filter on
class FilterPlan : public AbstractPlanNode {

public:

    BoundExpression* predicate;
    AbstractPlanNode* child;
    
    FilterPlan(BoundExpression* predicate, AbstractPlanNode* child);

    //printing
    void PrintTree(int indent = 0)const override;
};

//projection and it need cols to select and the child node
class ProjectionPlan :public AbstractPlanNode{

public:

    //vector of cols
    vector<BoundSelectItem> expressions;
    AbstractPlanNode* child;
    
    ProjectionPlan(const vector<BoundSelectItem>& expressions, AbstractPlanNode* child);
    //printing
    void PrintTree(int indent = 0)const override ;
};


//join plan, it has join type, condtion, left and right childs
class JoinPlan : public AbstractPlanNode{
private:
    JoinType join_type;
    BoundExpression* condition;
    AbstractPlanNode* left;
    AbstractPlanNode* right;

public:
    JoinPlan(JoinType join_type, BoundExpression* condition, AbstractPlanNode* left, AbstractPlanNode* right);

    //getters
    JoinType getJoinType();

    BoundExpression* getCondition();

    AbstractPlanNode* getLeft();

    AbstractPlanNode* getRight();

    //just printing
    void PrintTree(int indent = 0) const override;
};

//same idea with order_By
class OrderByPlan : public AbstractPlanNode{

private:

    BoundOrderBy order_by;
    AbstractPlanNode* child;

public:
    
    OrderByPlan(BoundOrderBy order_by, AbstractPlanNode* child);
    BoundOrderBy getOrderBy();
    AbstractPlanNode* getChild();
    BoundExpression* getExpression();
    OrderType getOrderType();

    //just printing
    void PrintTree(int indent = 0)const override;
    
};


//planning agg 
class GroupByPlan : public AbstractPlanNode{

private:
    //group by needs a vector of keys to group on and vector of grouping functions 
    vector<BoundExpression*> grouping_keys;
    vector<BoundExpression*> grouping_functions;
    BoundExpression* having;

    AbstractPlanNode* child;

public:

    GroupByPlan(vector<BoundExpression*> grouping_keys,vector<BoundExpression*> grouping_functions,
                AbstractPlanNode* child, BoundExpression*having);

    vector<BoundExpression*> getGroupingKeys();
    
    vector<BoundExpression*> getGroupingFunctions();

    AbstractPlanNode* getChild();
    BoundExpression* getHaving();
    //just printing
    void PrintTree(int indent = 0) const override ;
};



class InsertPlan : public AbstractPlanNode {

public:
    unique_ptr<BoundInsertStatement> bound_insert;

    InsertPlan(unique_ptr<BoundInsertStatement> stmt);

    void PrintTree(int indent = 0)const override;

};

class UpdatePlan : public AbstractPlanNode {

public:
    unique_ptr<BoundUpdateStatement> bound_update;

    UpdatePlan(unique_ptr<BoundUpdateStatement> stmt);

    void PrintTree(int indent = 0)const override;

};


class DeletePlan : public AbstractPlanNode {
public:

    unique_ptr<BoundDeleteStatement> bound_delete;

    DeletePlan(unique_ptr<BoundDeleteStatement> statement);

    BoundDeleteStatement* getBoundDelete() const;
    //just printing
    void PrintTree(int indend=0) const override;
};


class CreateTablePlan : public AbstractPlanNode{

private:
    unique_ptr<BoundCreateTableStatement> bound_create_table;

public:

    CreateTablePlan(unique_ptr<BoundCreateTableStatement> statement);

    BoundCreateTableStatement* getBoundCreateTable() const;
    //just printing
    void PrintTree(int ident) const override;
};


class CreateIndexPlan : public AbstractPlanNode{

private:
    unique_ptr<BoundCreateIndexStatement> bound_create_index;

public:

    CreateIndexPlan(unique_ptr<BoundCreateIndexStatement> statement);

    BoundCreateIndexStatement* getBoundCreateIndex() const;
    //just printing
    void PrintTree(int ident) const override;
};

#endif