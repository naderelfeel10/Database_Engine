#include"Binder/BindContext.h"

BoundColumnRef* BindContext::ResolveColumn(const string& table_name, const string& column_name){ 
    
    cout<<"ll : ";
    cout<<table_name<<" "<<column_name<<endl;
    // using table_name, and col_name 
    // find the actual ColRef and return it, else return not found or repeated col
    BoundColumnRef* result = nullptr;
    bool found = false;

    for(const auto& table : tables){

        // if table_name is not equal to the targted, continue to the next table
        if (!table_name.empty() &&table.alias != table_name && table.table_name != table_name){
            continue;
        }

        // if the table is found, then check if it has the targted col
        int col_id = 0;
        FieldType col_type;
        bool col_found{false};

        for(Column col: table.schema){
            if(col.getColName() == column_name){
                col_found=true;
                col_type = col.getColType();
                break;
            }
            col_id++;
        }

        if(!col_found)continue;

        //check if col with same name and same table name is fouond
        if(found){
            cerr<<"Ambiguous column: " + column_name<<endl;
        }

        // construct the rsult
        // result->table_oid = table.table_oid;
        // result->column_oid = col_id;

        string resolved_table_name = table.alias.empty()? table.table_name : table.alias;
        // result->column_name = column_name;
        // result->return_type = col_type;

        result = new BoundColumnRef(table.table_oid, col_id, resolved_table_name, column_name, col_type);
        cout<<result->table_name<<", "<<result->table_oid<<", "<<result->column_name<<", "<<result->return_type<<endl;

        found = true;
    }

    if(!found){
        throw runtime_error("column does not exist: " + column_name);
    }
    result->PrintTree("|",true);

    return result;
}

void BindContext::AddTable(const BoundTable& table){
    tables.push_back(table);
    int table_id = table.table_oid;
    tables_map[table_id] = table;
}

BoundTable BindContext::getBoundTable(int table_id){
    return tables_map[table_id];
}
/*int
main(){

}*/