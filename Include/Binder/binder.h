#ifndef BINDER_H
#define BINDER_H

#include"Catalog/Catalog.h"
#include"../../parser/external/sql-parser/src/SQLParser.h"
#include"../../parser/external/sql-parser/src/SQLParserResult.h"
#include"BoundStatement.h"
#include"/home/elfeel/Desktop/SWE/Database_Engine/parser/external/sql-parser/src/sql/SelectStatement.h"
#include"BoundExpression.h"
#include"BoundSelectStatement.h"
#include"BindContext.h"
#include"BoundSelectStatement.h"
#include"../../parser/external/sql-parser/src/SQLParser.h"
#include"../../parser/external/sql-parser/src/sql/SQLStatement.h"
#include"Q_Execution/SortAggregateExecuter.h"
#include"Binder/BoundInsertStatement.h"
#include"../../parser/external/sql-parser/src/sql/InsertStatement.h"
#include"Binder/BoundUpdateStatement.h"
#include"Binder/BoundDeleteStatement.h"
#include"Binder/BoundCreateTableStatement.h"

using namespace std;


class Binder {
private:
    Catalog* catalog;
    BindContext* context;

public:
    Binder(Catalog* catalog, BindContext* context);
    
    std::unique_ptr<BoundStatement> bind(const hsql::SQLStatement* statement);
    
    //bind selectr statement
    BoundSelectStatement* BindSelect(const hsql::SelectStatement* statement);
    BoundInsertStatement* BindInsert(const hsql::InsertStatement* statement);
    BoundUpdateStatement* BindUpdate(const hsql::UpdateStatement* statement);
    BoundDeleteStatement* BindDelete(const hsql::DeleteStatement* statement);

    //
    BoundCreateTableStatement* bindCreateTable(const hsql::CreateStatement* statement);
    FieldType convertColumnType(const hsql::ColumnType& type);

    
    //sub functions used in main ones
    BoundExpression* BindExpression( hsql::Expr* expression);
    //expression types thaat i need to bind
    BoundExpression* BindColumnRef(const hsql::Expr* expression);

    BoundExpression* BindIntegerLiteral(hsql::Expr* expression);
    BoundExpression* BindFloatLiteral(hsql::Expr* expression);
    BoundExpression* BindStringLiteral(hsql::Expr* expression);
    BoundExpression* BindOperator(const hsql::Expr* expression);
    BoundExpression* BindFunction(hsql::Expr* expression);

    BoundOperatorType BindBinaryOperator(hsql::OperatorType op);
    //BoundExpression* BindOperator(const hsql::Expr* expression);
    //select statement bind sub-functions
    void BindFrom( const hsql::SelectStatement* statement, BoundSelectStatement& bound);

    BoundJoinClause BindJoin(const hsql::JoinDefinition* join);
    JoinType BindJoinType(hsql::JoinType type);

    BoundTable BindTable(const hsql::TableRef* table);

    void BindOrderBy(const hsql::SelectStatement* statement, BoundSelectStatement& bound);
    void BindLimitOffset(const hsql::SelectStatement* statement, BoundSelectStatement& bound);
};


#endif