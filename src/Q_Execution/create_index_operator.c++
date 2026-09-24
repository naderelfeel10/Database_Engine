#include"Q_Execution/create_index_operator.h"


CreateIndex::CreateIndex(TransactionManager* txn_manager, WALManager* wal_manager,
                    TableHeap* table_heap, indexes_t index_type, string col_name, int index_size){

    this->table_heap = table_heap;
    this->txn_manager = txn_manager;
    this->wal_manager = wal_manager;

    cout << "wal_manager = " << wal_manager << endl;
    cout << "table_heap  = " << table_heap << endl;

    if(this->wal_manager)
        this->wal_manager->set_table_heap(table_heap);
    
    //create the index
    this->table_heap->createIndex(index_type, col_name, index_size);

    //after creating the index, insert the data into it

    Index* static_hash_index_wrapper =  table_heap->indexes_map[col_name][0];
    TableIterator table_iterator = TableIterator(table_heap,table_heap->getStartingRID(), table_heap->getStoppigRID());

    //loop throgh all tuples, and insert the index_col into the index
    for(;!table_iterator.end();++table_iterator){

        Tuple t = *table_iterator;
        
        int counter{0};
        for(auto& col:table_heap->getCols()){

            if(col.getColName()==col_name){
                //t.fields[counter].print();
                static_cast<StaticHashIndexWrapper*>(static_hash_index_wrapper)->Insert(t.fields[counter],col_name,table_heap->getCols(),table_iterator.getCurrRIDPointer());
                break;
            }

            counter++;

        }
    
    }
    

}