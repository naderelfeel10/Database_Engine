#include"QueryPlan/Planner.hpp"

AbstractPlanNode* Planner::Plan(unique_ptr<BoundStatement> statement){
    switch(statement->type()){
        //if stmt type is select call it's function to handle it
        case BoundStatementType::SELECT:{
            return PlanSelect(dynamic_cast<BoundSelectStatement*>(statement.get()));
        }
        case BoundStatementType::INSERT:{
            auto* insert = dynamic_cast<BoundInsertStatement*>(statement.release());
            return PlanInsert(unique_ptr<BoundInsertStatement>(insert));
        }

        case BoundStatementType::UPDATE:{
            auto* update = dynamic_cast<BoundUpdateStatement*>(statement.release());
            return PlanUpdate(unique_ptr<BoundUpdateStatement>(update));
        }
        case BoundStatementType::DELETE:{
            auto* delete_stmt = dynamic_cast<BoundDeleteStatement*>(statement.release());
            return PlanDelete(unique_ptr<BoundDeleteStatement>(delete_stmt));
        }
        case BoundStatementType::CREATE_TABLE:{
            auto* create_stmt = dynamic_cast<BoundCreateTableStatement*>(statement.release());
            return PlanCreateTable(unique_ptr<BoundCreateTableStatement>(create_stmt));
        }
        default:
            throw runtime_error(" unsupported statement!!!!!!!!!!!");
    }
}


//plan select is the first actual plan 
AbstractPlanNode* Planner::PlanSelect(BoundSelectStatement* statement){

    //the order would be like this :
    //first from table to determine where to fetch the very first data
    int tabl_oid = statement->from_table.table_oid;
    AbstractPlanNode* plan = new SeqScanPlan(tabl_oid);

    //check for joins
    //we might get multiple joins in the same query
    for (auto& join : statement->joins){

        //extract right table
        AbstractPlanNode* right_plan = new SeqScanPlan(join.right_table.table_oid);
        //here plan represent the left side
        plan = new JoinPlan(join.type, join.condition, plan, right_plan);
    }

    //second is where, so we filter onlt needed rows
    if(statement->where != nullptr){
        plan = new FilterPlan(statement->where, plan);
    }

    // group by plan
    if(!statement->group_by.empty()){
        //prepare agg functions like avg, sum ...
        vector<BoundExpression*>functions;

        for(auto&item : statement->select_list){
            BoundExpression* expr = item.expression;
            if(expr->exp_type == BoundExpressionType::FUNCTION){
                functions.push_back(expr);
            }
        }
        
        plan = new GroupByPlan(statement->group_by, functions, plan,statement->having);
    }


    if(!statement->order_by.empty()) {
        plan = new OrderByPlan(statement->order_by[0], plan);
    }

    //then the projection to choose needed cols from the row
    plan = new ProjectionPlan(statement->select_list, plan);

    return plan;
}


AbstractPlanNode* Planner::PlanInsert(unique_ptr<BoundInsertStatement> statement){

    InsertPlan* plan = new InsertPlan(move(statement));
    return dynamic_cast<AbstractPlanNode*>(plan);
}

AbstractPlanNode* Planner::PlanUpdate(unique_ptr<BoundUpdateStatement> statement){

    UpdatePlan* plan = new UpdatePlan(move(statement));
    return dynamic_cast<AbstractPlanNode*>(plan);
}

AbstractPlanNode* Planner::PlanDelete(unique_ptr<BoundDeleteStatement> statement){

    DeletePlan* plan = new DeletePlan(move(statement));
    return dynamic_cast<AbstractPlanNode*>(plan);
}


AbstractPlanNode* Planner::PlanCreateTable(unique_ptr<BoundCreateTableStatement> statement){
    
    if(statement == nullptr){
        throw runtime_error("create stmt is null");
    }

    return new CreateTablePlan(move(statement));
}

/////////////////////////////////////////////////////////////////////////




/*int 
main(){

}*/