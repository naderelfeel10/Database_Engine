#pragma once

#include"Catalog/Catalog.h"
#include"QueryPlan/AbstractPlanNode.hpp"
#include"Binder/BindContext.h"
#include"Q_Execution/seq_scan_operator.h"
#include"Q_Execution/Projection_operator.h"
#include"Q_Execution/ComplexPredicate.h"
#include"Q_Execution/Predicate.h"
#include"Storage/Table/Column.h"
#include"Storage/Page/Field.h"
#include"Q_Execution/select_operator.h"
#include"Binder/binder.h"
#include"QueryPlan/Planner.hpp"
#include"Q_Execution/AbstractExecuter.h"
#include"Q_Execution/Nested_loop_join.h"
#include"Q_Execution/ExternalMergeSortExecuter.h"
#include"Q_Execution/SortAggregateExecuter.h"
#include"Q_Execution/insert_statement_executer.h"
#include"Binder/BoundUpdateStatement.h"
#include"Q_Execution/update_statement_executer.h"
#include"Q_Execution/delete_statement_executer.h"
#include"Q_Execution/create_table_executer.h"
#include"TransactionManager/Transaction_manager.h"
#include"../../parser/external/sql-parser/src/sql/SQLStatement.h"
#include"Recovery/WAL_manager.h"
#include"Q_Execution/create_index_operator.h"

//in this executer i will use it to convert from BoundedStmts into actual component i use, then call it's operator
class ExecutorFactory{
private:
    Catalog* catalog;
    BindContext* context;
    TransactionManager* txn_manager;
    WALManager* wal_manager;

public:

    ExecutorFactory(TransactionManager* txn_manager, Catalog* catalog, BindContext* context, WALManager* wal_manager);

    AbstractExecuter* createExecutor(AbstractPlanNode* plan);

    bool execute_txn(const hsql::SQLStatement* stmt);
    
    AbstractPredicate* build_predicate(BoundExpression* expression, AbstractExecuter* child);
    AbstractPredicate* build_join_predicate(BoundExpression* expression, AbstractExecuter* left_child, AbstractExecuter* right_child);

    Column* expr_to_col(BoundExpression* expr,AbstractExecuter* child);
    Column* const_to_col(BoundConstantExpression* expr);

    Column* expr_to_join_col(BoundExpression* expr,AbstractExecuter* left_child,AbstractExecuter* right_child);
};