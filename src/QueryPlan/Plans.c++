#include"QueryPlan/Plans.hpp"

SeqScanPlan::SeqScanPlan(int table_oid):table_oid(table_oid){
    type = PlanType::SEQ_SCAN;
}

//just printing
void SeqScanPlan::PrintTree(int indent) const {

    PrintIndent(indent);
    cout << "SeqScan"
         << " [table_oid=" << table_oid << "]"
         << endl;
    
}



FilterPlan::FilterPlan(BoundExpression* predicate, AbstractPlanNode* child):predicate(predicate),child(child){
    type = PlanType::FILTER;
}

    //printing
void FilterPlan::PrintTree(int indent)const {

        PrintIndent(indent);

        cout << "Filter";
        cout << endl;
        if (child != nullptr) {
            child->PrintTree(indent + 1);
        }
}



    
ProjectionPlan::ProjectionPlan(const vector<BoundSelectItem>& expressions, AbstractPlanNode* child):expressions(expressions),child(child){
    type = PlanType::PROJECTION;
}

//printing
void ProjectionPlan::PrintTree(int indent )const  {

        PrintIndent(indent);

        cout << "Projection";
        cout << endl;
        if (child != nullptr) {
            child->PrintTree(indent + 1);
        }
}



JoinPlan::JoinPlan(JoinType join_type, BoundExpression* condition, AbstractPlanNode* left, AbstractPlanNode* right){

        this->type = PlanType::JOIN;
        this->join_type = join_type;
        this->condition = condition;
        this->left = left;
        this->right = right;
}

    //getters
    JoinType JoinPlan::getJoinType(){
        return join_type;
    }

    BoundExpression* JoinPlan::getCondition(){
        return condition;
    }

    AbstractPlanNode* JoinPlan::getLeft(){
        return left;
    }

    AbstractPlanNode* JoinPlan::getRight(){
        return right;
    }

//just printing
void JoinPlan::PrintTree(int indent) const{

        PrintIndent(indent);

        cout << "JOIN";

        switch (join_type) {

            case JoinType::INNER:
                cout << " [INNER]";
                break;

            case JoinType::LEFT:
                cout << " [LEFT]";
                break;

            case JoinType::RIGHT:
                cout << " [RIGHT]";
                break;
        }

        cout << endl;


        // Print join condition
        if (condition != nullptr) {

            PrintIndent(indent + 1);
            cout << "Condition:" << endl;

            condition->PrintTree(
                "|   ",
                true
            );
        }


        // Print left child
        PrintIndent(indent + 1);
        cout << "Left:" << endl;

        if (left != nullptr) {
            left->PrintTree(indent + 2);
        }


        // Print right child
        PrintIndent(indent + 1);
        cout << "Right:" << endl;

        if (right != nullptr) {
            right->PrintTree(indent + 2);
        }
}
    


    
    OrderByPlan::OrderByPlan(BoundOrderBy order_by, AbstractPlanNode* child):order_by(order_by),child(child){
        this->type = PlanType::SORT;
    }
    BoundOrderBy OrderByPlan::getOrderBy(){
        return order_by;
    }
    AbstractPlanNode* OrderByPlan::getChild(){
        return child;
    }
    BoundExpression* OrderByPlan::getExpression(){
        return this->order_by.expression;
    }
    OrderType OrderByPlan::getOrderType(){
        return this->order_by.order_type;
    }

    //just printing
    void OrderByPlan::PrintTree(int indent )const {
        PrintIndent(indent);

        cout << "ORDER BY";

        // Print ASC / DESC
        switch (order_by.order_type) {
            case OrderType::ASC:
                cout << " [ASC]";
                break;

            case OrderType::DESC:
                cout << " [DESC]";
                break;
        }

        cout << endl;

        // Print sort expression
        if (order_by.expression != nullptr) {

            PrintIndent(indent + 1);
            cout << "Expression:" << endl;

            order_by.expression->PrintTree(
                "|   ",
                true
            );
        }

        // Print child
        PrintIndent(indent + 1);
        cout << "Child:" << endl;

        if (child != nullptr) {
            child->PrintTree(indent + 2);
        }
    }

    



GroupByPlan::GroupByPlan(vector<BoundExpression*> grouping_keys,vector<BoundExpression*> grouping_functions,
                AbstractPlanNode* child, BoundExpression*having) : grouping_keys(grouping_keys),

            grouping_functions(grouping_functions),child(child),having(having){
        this->type = PlanType::AGGREGATION;
}

vector<BoundExpression*> GroupByPlan::getGroupingKeys(){
        return grouping_keys;
} 
    
vector<BoundExpression*> GroupByPlan::getGroupingFunctions(){
        return grouping_functions;
}

AbstractPlanNode* GroupByPlan::getChild(){
        return child;
}

BoundExpression* GroupByPlan::getHaving(){
        return this->having;
}

//just printing
void GroupByPlan::PrintTree(int indent) const  {

        PrintIndent(indent);

        cout << "GROUP BY" << endl;

        // Print grouping keys
        if (!grouping_keys.empty()) {

            PrintIndent(indent + 1);
            cout << "Grouping Keys:" << endl;

            for (auto* key : grouping_keys) {

                if (key != nullptr) {
                    key->PrintTree(
                        "|   ",
                        true
                    );
                }
            }
        }

        // Print grouping functions
        if (!grouping_functions.empty()) {

            PrintIndent(indent + 1);
            cout << "Grouping Functions:" << endl;

            for (auto* func : grouping_functions) {

                if (func != nullptr) {
                    func->PrintTree(
                        "|   ",
                        true
                    );
                }
            }
        }

        // Print child
        PrintIndent(indent + 1);
        cout << "Child:" << endl;

        if (child != nullptr) {
            child->PrintTree(indent + 2);
        }
}





    InsertPlan::InsertPlan(unique_ptr<BoundInsertStatement> stmt)
        :bound_insert(move(stmt)) {
        this->type = PlanType::INSERT;
    }

    void InsertPlan::PrintTree(int indent)const {
        cout<<"insert plan"<<endl;
    }




    UpdatePlan::UpdatePlan(unique_ptr<BoundUpdateStatement> stmt)
        :bound_update(move(stmt)) {
        this->type = PlanType::UPDATE;
    }

    void UpdatePlan::PrintTree(int indent)const {
        cout<<"update plan"<<endl;
    }




DeletePlan::DeletePlan(unique_ptr<BoundDeleteStatement> statement):bound_delete(move(statement)){
    type = PlanType::DELETE;
}

BoundDeleteStatement* DeletePlan::getBoundDelete() const{
    return bound_delete.get();
}

//just printing
void DeletePlan::PrintTree(int indend) const {
    cout << "|-- DeletePlan" << endl;

    if (bound_delete) {
        bound_delete->PrintTree();
    }
}



CreateTablePlan::CreateTablePlan(unique_ptr<BoundCreateTableStatement> statement):bound_create_table(move(statement)){
    type = PlanType::CREATE_TABLE;
}

BoundCreateTableStatement* CreateTablePlan::getBoundCreateTable() const{
    return bound_create_table.get();
}

//just printing
void CreateTablePlan::PrintTree(int ident) const {

        cout << "|-- CreateTablePlan"
             << endl;

        if (bound_create_table != nullptr) {

            bound_create_table->PrintTree();
        }
}
