#ifndef BOUND_CONTEXT_H
#define BOUND_CONTEXT_H

#include"Expression.h"
#include"BoundExpression.h"

#include<iostream>
#include<vector>
#include<map>

using namespace std;

//the context represents the currently visible namespace
class BindContext {

public:
    void AddTable(const BoundTable& table);
    BoundColumnRef* ResolveColumn(const string& table_name, const string& column_name);

    vector<BoundTable> tables;
    map<int, BoundTable>tables_map;
    BoundTable getBoundTable(int table_id);
};

#endif