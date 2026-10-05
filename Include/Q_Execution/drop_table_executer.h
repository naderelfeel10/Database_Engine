#ifndef DROP_TABLE_OPERATOR_H
#define DROP_TABLE_OPERATOR_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractExecuter.h"
#include"ComplexPredicate.h"
#include"seq_scan_operator.h"
#include"AbstractPredicate.h"
#include"Catalog/Catalog.h"
#include"Binder/BoundDropTableStatement.h"
using namespace std;

class DropTable : public AbstractExecuter{
    private:
        Catalog* catalog;
        bool dropped{false};

    public:

        DropTable(Catalog* catalog, const BoundDropTableStatement& statement);
        bool is_dropped();
        void open(){};
        void close(){};
        bool getNext(Tuple* tuple);
        
        bool update_tuple(RID rid, Tuple tuple);
        void drop_table(Catalog* catalog, const BoundDropTableStatement& statement);
        TableHeap* getTableHeap();
        vector<Column> get_output_schema();
        Tuple get_tuple();
        bool has_column(string col_name);
        
};

#endif
