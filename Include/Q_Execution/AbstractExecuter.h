#ifndef ABSTRACT_EXECUTER_H
#define ABSTRACT_EXECUTER_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
using namespace std;

class AbstractExecuter{
    public:
    virtual void open()=0;
    virtual void close()=0;
    virtual bool getNext(Tuple*tuple)=0;
    
    virtual TableHeap* getTableHeap()=0;
    virtual vector<Column> get_output_schema()=0;
    virtual bool has_column(string col_name)=0;

};

#endif
