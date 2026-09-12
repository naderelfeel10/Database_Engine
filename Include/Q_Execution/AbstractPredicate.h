#ifndef ABSTRACT_PREDICATE_H
#define ABSTRACT_PREDICATE_H

#include <vector>
#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"

class AbstractPredicate {
    public:
    virtual ~AbstractPredicate() = default;
    virtual bool evaluate(Tuple* tuple,  std::vector<Column> cols) = 0;
};

#endif