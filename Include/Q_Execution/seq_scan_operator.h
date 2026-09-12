#ifndef SEQ_SCAN_H
#define SEQ_SCAN_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractExecuter.h"
using namespace std;

class SeqScan: public AbstractExecuter{
    private:
        TableHeap* Table_heap;
        vector<Column> output_schema;

        RID  curr_rid_pointer = RID(-1,-1);
        RID  prev_rid_pointer = RID(-1,-1);

        //holds the rids of the all table tuples
        vector<RID>table_rids;
    public:
        SeqScan(TableHeap* table_heap);
        void open()override;
        void close()override;
        bool getNext(Tuple*tuple)override;
        TableHeap* getTableHeap()override;
        vector<Column> get_output_schema();

        bool has_column(string col_name);

        vector<RID> get_table_rids();
        RID get_curr_rid();
        RID get_prev_rid();
};

#endif
