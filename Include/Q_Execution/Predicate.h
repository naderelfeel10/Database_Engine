#ifndef PREDICATE_H
#define PREDICATE_H


#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractPredicate.h"
using namespace std;


enum class PredicateType {
    EQ,
    NE, 
    GT, 
    GE, 
    LT, 
    LE 
};

class Predicate:public AbstractPredicate{
    private:
        Column* left_col;
        Column* right_col;
        PredicateType predicate_type;

        template<typename T>
        bool compareValues(T v1, T v2);

    public:
        Predicate(Column* left_col, Column* right_col, PredicateType type):left_col(left_col),
        right_col(right_col),predicate_type(type){}

        bool checkPredicate();
        bool evaluate(Tuple* tuple, vector<Column> cols);  

        Column* getcol(int col_index);
};

#endif