#ifndef EXTERNAL_MERGE_SORT
#define EXTERNAL_MERGE_SORT

#include<unordered_map>
#include <map>
#include <vector>
#include <queue>
#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractPredicate.h"
#include"AbstractExecuter.h"
#include"seq_scan_operator.h"
#include"select_operator.h"
#include"Projection_operator.h"
using namespace std;

enum  sorting_methods {ASC,DESC }; 

class ExternalMergeSort :public AbstractExecuter {
        
        private:
            AbstractExecuter* child_executer;
            Column sort_key; // the key iam sorting on
            sorting_methods sorting_method;
            BufferPoolManager* BPM; 

            queue<int>pages_ids;
            vector<Tuple> run_buffer;
            char* tmp_page_buffer;
            int tmp_page_id{-1};
            //RID curr_rid = RID(-1,-1);
            //vector<RID>pages_rids;

            RID  curr_rid_pointer = RID(-1,-1);
            RID  last_rid_pointer = RID(-1,-1);

            vector<Column>output_schema;

        public:
            ExternalMergeSort(BufferPoolManager* BPM,AbstractExecuter* child_executer,Column sort_key,sorting_methods sorting_method);
            void open()override;
            void close()override;
            bool getNext(Tuple*tuple)override;

            TableHeap* getTableHeap();

            vector<Column> get_output_schema();

            void write_run_buffer_on_disk(int col_index, Tuple& tuple);
            void insert_tuple(int& new_page_id,Page*& new_page, PageHeader*& new_page_header, int page_id, int slot_num, int &left, int &right);
            
            void getTuple(RID rid, Tuple& tuple);
            bool has_column(string col_name);


};
#endif