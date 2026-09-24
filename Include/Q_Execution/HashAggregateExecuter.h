#ifndef HASH_AGG_EXECUTER_H
#define HASH_AGG_EXECUTER_H

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
#include"Q_Execution/SortAggregateExecuter.h"
using namespace std;

/*
enum AggregateType {COUNT, SUM, AVG, MAX, MIN};

struct AggValues{
    long counter{0};
    double value{0.0};

};

struct GroupingFunction{
    AggregateType grouping_type;
    int function_key;
};
*/
class HashAggregateExecuter: public AbstractExecuter{
    
    private:
        //passed in the the constuctor
        AbstractExecuter* table;
        vector<int> grouping_keys;
        vector<GroupingFunction> grouping_functions;
        BufferPoolManager* BPM;
        // i insert and update group state in this map
        map<vector<Field>, vector<AggValues>>groups_map;
        //this will hold data from groups_map after finishing hashing
        vector<vector<Field>>groups;
        vector<vector<AggValues>>states;

        vector<AggValues>agg_state;
        vector<Field> curr_group;

        Tuple curr_tuple = Tuple({});
        bool has_tuple;
        int curr_group_index{0};
        
        
        vector<Column>output_schema;

    public:
        HashAggregateExecuter(BufferPoolManager* BPM, AbstractExecuter*table, vector<int> grouping_keys, vector<GroupingFunction> grouping_functions);

        void open();
        void close();
        bool getNext(Tuple*tuple);

        TableHeap* getTableHeap();
        void set_output_schema();
        vector<Column> get_output_schema();

        vector<Field> get_grouping_fields(Tuple tuple);
        void update_aggregate();
        Tuple get_output_tuple(vector<Field>curr_grouping_fields,vector<AggValues>agg_state);
        bool is_same_group();
        string get_function_string(GroupingFunction func);

        bool has_column(string col_name);

};

#endif
