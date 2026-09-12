#ifndef AGG_EXECUTER_H

#define AGG_EXECUTER_H

#include<unordered_map>
#include <map>
#include <vector>
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
#include"ExternalMergeSortExecuter.h"

using namespace std;

enum AggregateType {COUNT, SUM, AVG, MAX, MIN};

struct AggValues{
    long counter{0};
    double value{0.0};
};

struct GroupingFunction{
    AggregateType grouping_type;
    int function_key;
};


class SortAggregateExecuter: public AbstractExecuter{
    
    private:
        //passed in the the constuctor
        AbstractExecuter* table;
        vector<int> grouping_keys;
        vector<GroupingFunction> grouping_functions;
        BufferPoolManager* BPM;
        //hold new sorted table 
        AbstractExecuter* sorted_table;

        vector<Column> output_schema;
        // keeping state of everything(next tuple, grouping keys, agg_state)
        Tuple next_tuple = Tuple({});
        bool has_tuple;
        vector<Field> curr_grouping_fields;
        vector<AggValues> agg_state;

        //added having
        AbstractPredicate* having{nullptr};


    public:
        SortAggregateExecuter(BufferPoolManager* BPM, AbstractExecuter*table, vector<int> grouping_keys, vector<GroupingFunction> grouping_functions, AbstractPredicate*having):
            BPM(BPM),table(table), grouping_keys(grouping_keys), grouping_functions(grouping_functions),having(having){
                agg_state.assign(grouping_functions.size(), AggValues{});
                this->open();
            };

        void open();
        void close();
        bool getNext(Tuple*tuple);

        TableHeap* getTableHeap(){return nullptr;};
        void set_output_schema();
        vector<Column> get_output_schema();

        vector<Field> get_grouping_fields(Tuple tuple);
        void update_aggregate();
        Tuple get_output_tuple();
        bool is_same_group();
        string get_function_string(GroupingFunction func);

        bool has_column(string col_name);
};

#endif
