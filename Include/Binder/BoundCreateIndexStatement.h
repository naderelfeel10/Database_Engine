#ifndef BOUND_CREATE_INDEX_H
#define BOUND_CREATE_INDEX_H

#include "BoundStatement.h"
#include"Storage/Table/Column.h"
#include <string>
#include <vector>

using namespace std;


/*
    index_name,
    table the index built on
    cols the index built on

*/
class BoundCreateIndexStatement : public BoundStatement{

public:
    string index_name;
    string table_name;

    vector<Column> columns;

    bool if_not_exists;

    BoundCreateIndexStatement();

    BoundStatementType type() const override;

    void PrintTree() const override;
};

#endif