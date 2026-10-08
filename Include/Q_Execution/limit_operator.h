#ifndef LIMIT_H
#define LIMIT_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractExecuter.h"
using namespace std;

class Limit: public AbstractExecuter{
    private:
        AbstractExecuter* child_operator;
        
        int limit;
        int offset;

        int skipped;
        int returned;

        RID curr_rid = RID(-1,-1);

    public:
        Limit(AbstractExecuter* child, Column* limit, Column* offset);
        
        void open()override;
        void close()override;
        bool getNext(Tuple*tuple)override;
        TableHeap* getTableHeap(){return child_operator->getTableHeap();}
        vector<Column> get_output_schema(){return child_operator->get_output_schema();}

        bool has_column(string col_name);

};

#endif
