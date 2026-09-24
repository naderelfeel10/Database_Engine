#ifndef ABSTRACT_PLAN_NODE_H
#define ABSTRACT_PLAN_NODE_H

#include"Binder/BoundExpression.h"
#include"Binder/BoundSelectStatement.h"
#include"Binder/BoundInsertStatement.h"
#include"Binder/BoundUpdateStatement.h"
#include"Binder/BoundDeleteStatement.h"
#include"Binder/BoundCreateTableStatement.h"
#include"Binder/BoundCreateIndexStatement.h"


using namespace std;

//multiple plan types like seq_scan, project, filer, ...
enum class PlanType {
    SEQ_SCAN,
    FILTER,
    PROJECTION,
    JOIN,
    AGGREGATION,
    SORT,
    LIMIT,
    //
    INSERT,
    UPDATE,
    DELETE,
    CREATE_TABLE,
    CREATE_INDEX

};


class AbstractPlanNode{

public:
    PlanType type;
    virtual ~AbstractPlanNode() = default;

    virtual void PrintTree(int indent = 0) const = 0;
    
    static void PrintIndent(int indent){
        for(int i = 0; i < indent; i++){
            cout << "  ";
        }
    }

};

#endif