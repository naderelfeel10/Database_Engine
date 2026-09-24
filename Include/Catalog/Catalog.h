#ifndef CATALOG_H
#define CATALOG_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"Binder/BoundCreateTableStatement.h"
#include"Storage/Table/Column.h"
using namespace std;


struct IndexInfo
{
    int offset = {0};
    
    //uint32_t index_id;
    string name;
    string column_name;
    indexes_t type;
    int root_page;

    void serializeIndex(char* buffer);
    void loadIndex(const char *buffer);
    int getSize();
    
};


//ex : FOREIGN KEY(user_id) REFERENCES users(id)
struct ForeignKeyInfo {
    vector<int> column_ids;
    int referenced_table_id;
    vector<int> referenced_column_ids;
};

struct TableInfo
{
public:
        int offset{0};
        
        //uint32_t table_id;
        string table_name;
        int table_id;
        vector<Column> schema;

        int first_page_id;
        TableHeap* table_heap;
        vector<IndexInfo> indexes;

        //handle table constraints like fk, pk, ..
        vector<ForeignKeyInfo> foreign_keys;
        vector<vector<int>> unique_constraints;
        vector<int> primary_key_columns;

        void serializeTableInfo(char* buffer);
        void loadTableInfo(char *buffer);
        int getSize();
   
        TableHeap*get_table_heap(){
            return this->table_heap;
        }
    void printTableInfo();
};


class Catalog
{
    private:

        // table_name : table_info
        unordered_map<string,TableInfo*> tables;
        unordered_map<int,TableInfo*> tables_ids_map;
        BufferPoolManager* BPM;
        int next_table_id{0};
        int catalog_first_page_id{-1};
        int catalog_last_page_id{-1};

        //int number_of_tables;


    public:
        Catalog(BufferPoolManager* BPM, bool createNew);

        TableInfo* CreateTable(const string& table_name, const vector<Column>&schema);
        TableInfo* CreateTable(const BoundCreateTableStatement& statement);

        void printCatalog(){
            for(auto[table_name, table_info]:tables){
                table_info->printTableInfo();
            }
        }
        void DropTable(string table_name);
        TableInfo* GetTable(string table_name);
        TableInfo* GetTable(int table_id);
        
        bool AddIndex(string table_name, string index_name, string column_name,
                         indexes_t index_type, int root_page);


        IndexInfo* GetIndex(string table_name, string index_name);

        bool TableExists(string table_name);
        bool TableExists(int table_id);
        vector<TableInfo*> GetTables();

        void save_catalog();
        void load_catalog(int catalog_first_page_id);

        unordered_map<string,TableInfo*> getTables(){
            return this->tables;
        }
        BufferPoolManager* getBPM(){
            return this->BPM;
        }


};

#endif