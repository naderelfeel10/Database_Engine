#pragma once

#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Catalog\Catalog.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\QueryPlan\AbstractPlanNode.hpp"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Binder\BindContext.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\seq_scan_operator.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\Projection_operator.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\ComplexPredicate.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\Predicate.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Storage\Table\Column.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Storage\Page\Field.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\select_operator.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Binder\binder.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\QueryPlan\Planner.hpp"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\AbstractExecuter.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\Nested_loop_join.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\ExternalMergeSortExecuter.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\SortAggregateExecuter.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\insert_statement_executer.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Binder\BoundUpdateStatement.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\update_statement_executer.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\delete_statement_executer.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Q_Execution\create_table_executer.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\TransactionManager\Transaction_manager.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\parser\external\sql-parser\src\sql\SQLStatement.h"

//in this executer i will use it to convert from BoundedStmts into actual component i use, then call it's operator
class ExecutorFactory{
private:
    Catalog* catalog;
    BindContext* context;
    TransactionManager* txn_manager;

public:

    ExecutorFactory(TransactionManager* txn_manager, Catalog* catalog, BindContext* context):txn_manager(txn_manager),catalog(catalog), context(context){}
    AbstractExecuter* createExecutor(AbstractPlanNode* plan);

    bool execute_txn(const hsql::SQLStatement* stmt);
    
    AbstractPredicate* build_predicate(BoundExpression* expression, AbstractExecuter* child);
    AbstractPredicate* build_join_predicate(BoundExpression* expression, AbstractExecuter* left_child, AbstractExecuter* right_child);

    Column* expr_to_col(BoundExpression* expr,AbstractExecuter* child);
    Column* const_to_col(BoundConstantExpression* expr);

    Column* expr_to_join_col(BoundExpression* expr,AbstractExecuter* left_child,AbstractExecuter* right_child);
};