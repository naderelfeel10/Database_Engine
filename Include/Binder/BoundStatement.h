#ifndef BOUND_STATEMENT_H
#define BOUND_STATEMENT_H

#include"../../parser/external/sql-parser/src/SQLParser.h"
#include"../../parser/external/sql-parser/src/SQLParserResult.h"

using namespace std;

enum class BoundStatementType {
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE_TABLE,
    DROP_TABLE
};


class BoundStatement {
public:
    virtual BoundStatementType
    type() const = 0;
    // just printing
    virtual void PrintTree() const = 0;
};

#endif