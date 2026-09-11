#include<iostream>
#include"Catalog.h"

vector<int> column_names_to_indexes(vector<string> names, vector<Column> schema);

Catalog::Catalog(BufferPoolManager* BPM, bool createNew):BPM(BPM){

    if(createNew){
        // allocate a page for first and last  page
        this->catalog_first_page_id = this->BPM->newPage();
        this->catalog_last_page_id = this->BPM->newPage();

        cout<<"first page id : "<<catalog_first_page_id<<endl;;
        cout<<"last page id : "<<catalog_last_page_id<<endl;

        //update next pointer of  first page to the last one
        char* buffer = this->BPM->fetchPage(catalog_first_page_id);
        PageHeader* header = reinterpret_cast<PageHeader*>(buffer);

        header->next_page_id = catalog_last_page_id;
        //mark as dirty just to be saved to disk later
        this->BPM->markAsDirty(catalog_first_page_id);
    
    }
}
/*
TableInfo* Catalog::CreateTable(string table_name, vector<Column>schema){
    //check if table already exists :
    if(this->tables.find(table_name) != tables.end()){
        cerr<<"this table already exists"<<endl;
        return nullptr;       
    }else{
        //prepare table meta data, then create tableInfo
        auto* info = new TableInfo();
        info->table_name = table_name;
        info->schema = schema;
        cout<<info->schema.size()<<", "<<schema.size()<<endl;

        TableHeap* heap = new TableHeap(BPM, -1, -1);

        heap->setCols(schema);
        cout<<"schema size : "<<heap->get_output_schema().size()<<endl;

        info->table_heap = heap;
        info->first_page_id = heap->get_first_page_id();

        tables[table_name] = info;


        return info;
    }

}
*/

//create table refactor
TableInfo* Catalog::CreateTable(const string& table_name,const vector<Column>& schema){
    //check if table already exists :
    if(tables.find(table_name) != tables.end()){
        cerr<<"Table already exists: "<<table_name<<endl;
        return nullptr;
    }

    //create the physical table storage
    TableHeap* heap = new TableHeap(BPM, -1, -1);

    //configure table heap by setting the name and schema
    heap->setTableName(table_name);
    heap->setCols(schema);

    //create catalog metadata
    TableInfo* info = new TableInfo();

    info->table_name = table_name;
    info->table_id = ++next_table_id;
    info->schema = schema;
    info->table_heap = heap;
    info->first_page_id = heap->get_first_page_id();

    heap->setTableId(info->table_id);

    tables[table_name] = info;
    tables_ids_map[info->table_id] = info;

    return info;
}


//this function is to create table from it's bounded stmt :
//it should resolve meta data like table_name, id, schema ..
//create the tableheap
//resolve constraints like pk, fk, unique
TableInfo* Catalog::CreateTable(const BoundCreateTableStatement& statement){
    
    //check if table already exists in our database
    string table_name = statement.table_name;
    auto it = tables.find(table_name);

    //if found. check if the query has (if not exist)
    if(it != tables.end()){
        //if not exists, so just return the found tableInfo
        if(statement.if_not_exists){
            return it->second;
        }
        
        throw runtime_error("this table already exists "+table_name);
    }

    //resolve and validate schema
    if(statement.columns.empty()){
        throw runtime_error("empty schema");
    }
    //create tableheap to be represented on hd
    TableHeap* heap =new TableHeap(BPM, -1, -1);

    heap->setTableName(table_name);
    heap->setCols(statement.columns);

    //create tableInfo and fill with basic table metadata
    TableInfo* info = new TableInfo();

    info->table_id = ++next_table_id;
    info->table_name = table_name;
    info->schema =statement.columns;
    info->table_heap = heap;
    info->first_page_id = heap->get_first_page_id();

    heap->setTableId(info->table_id);

    //constraints resolving
    for(const auto& constraint :statement.constraints){
        //resolve col_indexes
        vector<int> column_indexes = column_names_to_indexes(constraint.columns,statement.columns);

        //then assigne it based on it's type
        switch(constraint.type){
            case BoundConstraintType::PRIMARY_KEY:{
                info->primary_key_columns =column_indexes;
                break;
            }
            case BoundConstraintType::UNIQUE:{
                info->unique_constraints.push_back(column_indexes);
                break;
            }
            case BoundConstraintType::FOREIGN_KEY:{
                ForeignKeyInfo fk;
                //col-level ref
                fk.column_ids = column_names_to_indexes(constraint.columns,statement.columns);

                string referenced_table_name = constraint.referenced_table;
                TableInfo* referenced_table_info = GetTable(referenced_table_name);

                if(referenced_table_info == nullptr){
                    throw runtime_error("referenced table does not exist");
                }

                int table_id = referenced_table_info->table_id;
                fk.referenced_table_id = table_id;

                fk.referenced_column_ids = column_names_to_indexes(constraint.referenced_columns, referenced_table_info->schema);
                
                info->foreign_keys.push_back(fk);
                break;
            }
        }
    }

    tables.emplace(table_name,info);
    tables_ids_map.emplace(info->table_id, info);
    return info;
}

//simple function to convert from schema of col_names into it's col_id
vector<int> column_names_to_indexes(vector<string> names, vector<Column> schema){
    vector<int> indexes;

    for(const string& name:names){
        int index = -1;
        for(int i{};i<schema.size();i++){
            if(schema[i].getColName()==name){
                index = i;
                break;
            }
        }
        //col_name not found
        if(index == -1){
            throw runtime_error("column does not exist "+name);
        }
        indexes.push_back(index);
    }

    return indexes;
}


void Catalog::DropTable(string table_name){
    TableInfo* info = tables[table_name];
    //delete the actual table from heap
    info->table_heap->deleteTableHeap();
    //delete from teh catalog
    tables.erase(table_name);

}

TableInfo* Catalog::GetTable(string table_name){
    if(tables.find(table_name) != tables.end())
        return tables[table_name];

    return nullptr;
}

TableInfo* Catalog::GetTable(int table_id){
    if(tables_ids_map.find(table_id) != tables_ids_map.end())
        return tables_ids_map[table_id];

    return nullptr;
}


bool Catalog::TableExists(string table_name){
    if(tables.find(table_name) != tables.end())
        return true;

    return false;
}

bool Catalog::TableExists(int table_id){
    if(tables_ids_map.find(table_id) != tables_ids_map.end())
        return true;

    return false;
}
vector<TableInfo*> Catalog::GetTables(){
    vector<TableInfo*> res;
    for(auto&[table_name, info] : this->tables){
        res.push_back(info);
    }
    return res;
}

void Catalog::save_catalog(){
    //create a buffer to serialize data into
    char* buffer = new char[PAGE_SIZE];
    int index{0};
    
    //save next_table_id
    memcpy(buffer+index, &next_table_id, sizeof(next_table_id));
    index += sizeof(next_table_id);

    //save first_page_id
    memcpy(buffer+index, &catalog_first_page_id, sizeof(catalog_first_page_id));
    index += sizeof(catalog_first_page_id);
    
    //save last_page_id
    memcpy(buffer+index, &catalog_last_page_id, sizeof(catalog_last_page_id));
    index += sizeof(catalog_last_page_id);

    //save tables size
    int number_of_tables = tables.size();
    cout<<number_of_tables<<endl;
    memcpy(buffer+index, &number_of_tables, sizeof(number_of_tables));
    index += sizeof(number_of_tables);

    //save actual table 
    for(auto&[table_name, tableInfo]:this->tables){
        tableInfo->serializeTableInfo(buffer+index);
        index+=tableInfo->getSize();
    }

    //now copy the whole buffer into the first page then save it to the disk
    char* page_buffer = this->BPM->fetchPage(catalog_first_page_id);
    memcpy(page_buffer, buffer, PAGE_SIZE);
    // mark as direty jsut to be presistent on disk
    BPM->markAsDirty(catalog_first_page_id);
    cout << "Saving to page: " << catalog_first_page_id << endl;
    //delete the buffer allocated
    delete[]buffer;

}

void Catalog::load_catalog(int page_id=1){
    //load the page 
    //by default it's the second page in the whole file (page_id =1)
    char* buffer = this->BPM->fetchPage(page_id);
    int offset{0};
    cout << "Loading from page: " << page_id << endl;
    //load data 

    //load first_page_id
    memcpy(&this->catalog_first_page_id,buffer+offset, sizeof(catalog_first_page_id));
    offset += sizeof(catalog_first_page_id);

    //load last_page_id
    memcpy(&this->catalog_last_page_id,buffer+offset, sizeof(catalog_last_page_id));
    offset += sizeof(catalog_last_page_id);

    //load tables size
    int number_of_tables;
    memcpy(&number_of_tables, buffer+offset, sizeof(number_of_tables));
    offset += sizeof(number_of_tables);

    cout<<"number of tables : "<<number_of_tables<<endl;
    tables.clear();
    tables_ids_map.clear();
    //laod actual table 
    for(int i=0;i<number_of_tables;i++){
        TableInfo* table_info = new TableInfo();
        table_info->loadTableInfo(buffer+offset);
        offset+=table_info->getSize();

        string table_name=  table_info->table_name;
        tables[table_name] = table_info;
        tables_ids_map[table_info->table_id] = table_info;
    }

}
///  ////////
/////


    void IndexInfo::serializeIndex(char* buffer){
        //int index{0};
        offset = 0;

        // save index_name len (to be able to load it dynamically)
        int index_name_size = name.length();
        memcpy(buffer+offset, &index_name_size, sizeof(index_name_size));
        offset += sizeof(index_name_size);
 
        //save index name
        memcpy(buffer+offset, name.data(), name.size());
        offset += name.size();

        //same with col_name that the index is built on
        int col_name_size = column_name.length();
        memcpy(buffer+offset, &col_name_size, sizeof(col_name_size));
        offset += sizeof(col_name_size);

        //save column name
        memcpy(buffer+offset, column_name.data(), column_name.size());
        offset += column_name.size();

        // save the type of the index
        memcpy(buffer+offset, &type, sizeof(type));
        offset += sizeof(type);

        //save first page_id of the index where so coiuld be loaded later
        memcpy(buffer+offset, &root_page, sizeof(root_page));
        offset += sizeof(root_page);
    }

    void IndexInfo::loadIndex(const char *buffer){
        offset = 0;

        //load index name length
        int len; 
        memcpy(&len, buffer +offset, sizeof(len));
        offset += sizeof(len);

        //load index name
        name.assign(buffer+offset, len);
        offset += len;

        //load col name size
        memcpy(&len, buffer + offset, sizeof(len));
        offset += sizeof(len);

        // load col name
        column_name.assign(buffer + offset, len);
        offset += len;

        //load index type
        memcpy(&type, buffer + offset, sizeof(type));
        offset += sizeof(type);

        //load index page_id
        memcpy(&root_page, buffer + offset, sizeof(root_page));
        offset += sizeof(root_page);
    }

    int IndexInfo::getSize(){return offset;}


//////////////////////////////////////


        void TableInfo::serializeTableInfo(char* buffer){
            offset = 0;
            // save table_name length
            int table_name_size = table_name.length();
            memcpy(buffer+offset, &table_name_size, sizeof(table_name_size));
            offset += sizeof(table_name_size);

            //save table_name
            memcpy(buffer+offset, table_name.data(), table_name.size());
            offset += table_name.size();

            //save table_id
            memcpy(buffer+offset, &table_id, sizeof(table_id));
            offset += sizeof(table_id);

            //save first page_id of the table
            memcpy(buffer+offset, &first_page_id, sizeof(first_page_id));
            offset += sizeof(first_page_id);

            //save number of cols in schema
            int schema_size = schema.size();
            memcpy(buffer+offset, &schema_size, sizeof(schema_size));
            offset += sizeof(schema_size);

            // save each col
            for(auto& c : this->schema){
                c.printCol();
                switch (c.getColType()){
                case TYPE_INT:
                    {buffer[offset] = 'I'; break;}
                case TYPE_STRING:
                    {buffer[offset] = 'S'; break;}
                case TYPE_FLOAT:
                    {buffer[offset] = 'F'; break;}
                case TYPE_BOOL:
                    {buffer[offset] = 'B'; break;}
                default:
                    cerr<<"incorrect field type"<<endl;
                    break;
                }
                offset+=1;
                cout<<"from save table meta"<<endl;
                c.serializeCol(buffer+ offset);
                offset += c.getColSize();
            }

            //save number of indexes
            int indexes_size = indexes.size();
            memcpy(buffer+offset, &indexes_size, sizeof(indexes_size));
            offset += sizeof(indexes_size);

            // save table indexes:
            for(auto&index:indexes){
                index.serializeIndex(buffer+offset);
                offset += index.getSize();
            }

        }

        void TableInfo::loadTableInfo(char *buffer){
            offset = 0;
        
            int len;
            //load table name len
            memcpy(&len, buffer + offset, sizeof(len));
            offset += sizeof(len);

            //load table name
            table_name.assign(buffer + offset, len);
            offset += len;
        
            //load table_id
            memcpy(&table_id, buffer + offset, sizeof(table_id));
            offset += sizeof(table_id);

            //load the first page id
            memcpy(&first_page_id, buffer + offset, sizeof(first_page_id));
            offset += sizeof(first_page_id);
        
            //load schema size
            int schema_size;
            memcpy(&schema_size, buffer + offset, sizeof(schema_size));
            offset += sizeof(schema_size);
        
            schema.clear();
            //load every col and add it to the schema

            for(int i=0;i<schema_size;i++){
                char type = buffer[offset];
                cout<<"Typeee : "<<type<<endl;
                FieldType field_type;
                switch (type)
                {
                case 'I':
                    {field_type =  TYPE_INT; break;}
                case 'S':
                    {field_type =  TYPE_STRING; break;}
                case 'F':
                    {field_type =  TYPE_FLOAT; break;}
                case 'B':
                    {field_type =  TYPE_BOOL; break;}
                default:
                    cerr<<"invalid field type"<<endl;
                    break;
                }
                offset+=1;
                Column c(field_type,"",1);
                c.deSerializeCol(buffer+ offset);
                //c.getField()->print();
                //c.printCol();
                this->schema.push_back(c);
                offset += c.getColSize();
            }

            //load index size
            int indexes_size;
            memcpy(&indexes_size, buffer + offset, sizeof(indexes_size));
            offset += sizeof(indexes_size);
        
            indexes.clear();
            // load all indexes
            for (int i = 0; i < indexes_size; i++){
                IndexInfo idx;
                idx.loadIndex(buffer + offset);
                // add index size then push table indexes
                offset += idx.getSize();
                indexes.push_back(idx);
            }
        }

        int TableInfo::getSize(){return offset;}

/*
int
main(){

    
    
    DiskManager* dm = new DiskManager("catalog.db");
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    Catalog* catalog = new Catalog(BPM, true);

    string table_name = "User";

    Column t1_col1 = Column(TYPE_INT, "user_id", sizeof(int));
    Column t1_col2 = Column(TYPE_STRING, "firstName", 30);
    Column t1_col3 = Column(TYPE_STRING, "lastName", 30);
    Column t1_col4 = Column(TYPE_INT, "age", sizeof(int));
    vector<Column> user_schema = {t1_col1, t1_col2, t1_col3, t1_col4};

    catalog->CreateTable(table_name, user_schema);
    catalog->CreateTable("Order", user_schema);

    cout<<endl;
    for(auto&[table_name, table_info]: catalog->getTables()){
        cout<<table_name<<endl;
        cout<<table_info->table_name<<endl;
        for(auto&col: table_info->schema){
            col.printCol();
        }
        cout<<endl;
    }
    cout<<"--------------"<<endl;

    catalog->save_catalog();
    
    
     
    //delete catalog;
    BPM->~BufferPoolManager();
    dm->~DiskManager();
    
    
   */

    /*
    DiskManager* dm2 = new DiskManager("catalog.db");
    BufferPoolManager* BPM2 = new BufferPoolManager(dm2);

    Catalog* catalog2 = new Catalog(BPM2, false);
    //retest after loading
    catalog2->load_catalog();

    for(auto&[table_name, table_info]: catalog2->getTables()){
        cout<<table_name<<endl;
        cout<<table_info->table_name<<endl;
        for(auto&col: table_info->schema){
            col.printCol();
        }
        cout<<endl;
    }
    cout<<"--------------"<<endl;

    */
    

//}
