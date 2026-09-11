#ifndef CATALOG_H
#define CATALOG_H

#include"../Storage/Table/TableIterator.h"
#include"../Storage/Table/TableHeap.h"
#include"../Storage/Table/RID.h"
#include"../Storage/Table/Column.h"
#include"../Storage/Indexing/Index.h"
#include"../Storage/Indexing/StaticHashIndexWrapper.h"
#include"../Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Binder\BoundCreateTableStatement.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Storage\Table\Column.h"
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

        //just printing
        void printTableInfo(){
            cout << "========================================\n";
            cout << "              TABLE INFO\n";
            cout << "========================================\n";
        
            cout << "Table Name     : " << table_name << '\n';
            cout << "Table ID       : " << table_id << '\n';
            cout << "First Page ID  : " << first_page_id << '\n';
            cout << "Offset         : " << offset << '\n';
        
            // ---------------- Schema ----------------
            cout << "\n--------------- SCHEMA ----------------\n";
        
            cout << "Number of Columns: " << schema.size() << '\n';
        
            for(auto&col:schema){
                col.printCol();
            }
        
            // ---------------- Indexes ----------------
            cout << "\n--------------- INDEXES ----------------\n";
        
            cout << "Number of Indexes: " << indexes.size() << '\n';
        
            for (size_t i = 0; i < indexes.size(); ++i)
            {
                const IndexInfo& index = indexes[i];
            
                cout << "\nIndex [" << i << "]\n";
                cout << "  Name        : " << index.name << '\n';
                cout << "  Column      : " << index.column_name << '\n';
                cout << "  Type        : " << static_cast<int>(index.type) << '\n';
                cout << "  Root Page   : " << index.root_page << '\n';
                cout << "  Offset      : " << index.offset << '\n';
            }
        
            // ---------------- Foreign Keys ----------------
            cout << "\n----------- FOREIGN KEYS ---------------\n";
        
            cout << "Number of Foreign Keys: " << foreign_keys.size() << '\n';
        
            for (size_t i = 0; i < foreign_keys.size(); ++i)
            {
                const ForeignKeyInfo& fk = foreign_keys[i];
            
                cout << "\nForeign Key [" << i << "]\n";
            
                cout << "  Columns       : ";
                for (size_t j = 0; j < fk.column_ids.size(); ++j)
                {
                    cout << fk.column_ids[j];
                    if (j + 1 < fk.column_ids.size())
                        cout << ", ";
                }
                cout << '\n';
            
                cout << "  Referenced Table ID: "
                     << fk.referenced_table_id << '\n';
            
                cout << "  Referenced Columns : ";
                for (size_t j = 0; j < fk.referenced_column_ids.size(); ++j)
                {
                    cout << fk.referenced_column_ids[j];
                
                    if (j + 1 < fk.referenced_column_ids.size())
                        cout << ", ";
                }
                cout << '\n';
            }
        
            // ---------------- Unique Constraints ----------------
            cout << "\n---------- UNIQUE CONSTRAINTS ----------\n";
        
            cout << "Number of Unique Constraints: "
                 << unique_constraints.size() << '\n';
        
            for (size_t i = 0; i < unique_constraints.size(); ++i)
            {
                cout << "  Unique [" << i << "] Columns: ";
            
                for (size_t j = 0; j < unique_constraints[i].size(); ++j)
                {
                    cout << unique_constraints[i][j];
                
                    if (j + 1 < unique_constraints[i].size())
                        cout << ", ";
                }
            
                cout << '\n';
            }
        
            // ---------------- Primary Key ----------------
            cout << "\n----------- PRIMARY KEY ----------------\n";
        
            cout << "Primary Key Columns: ";
        
            if (primary_key_columns.empty())
            {
                cout << "NONE";
            }
            else
            {
                for (size_t i = 0; i < primary_key_columns.size(); ++i)
                {
                    cout << primary_key_columns[i];
                
                    if (i + 1 < primary_key_columns.size())
                        cout << ", ";
                }
            }
        
            cout << '\n';
        
            cout << "========================================\n";
        }
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