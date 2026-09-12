#ifndef C_PREDICATE_H
#define C_PREDICATE_H


#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"Predicate.h"
#include"AbstractPredicate.h"
using namespace std;


enum class ComplexPredicateType {
    AND,
    OR,
    NOT
};

class ComplexPredicate:public AbstractPredicate{
    private:
        AbstractPredicate* left_predicate;
        AbstractPredicate* right_predicate;
        ComplexPredicateType c_predicate_type;
        
    public:
        ComplexPredicate(AbstractPredicate* left_predicate, AbstractPredicate* right_predicate, ComplexPredicateType type);

        bool checkcomplexPredicate();
        bool evaluate(Tuple* tuple, vector<Column> cols);

        AbstractPredicate* getpred(int pred_index);
};

#endif