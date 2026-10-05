#include"Q_Execution/drop_table_executer.h"
#include<iostream>
using namespace std;


DropTable::DropTable(Catalog* catalog, const BoundDropTableStatement& statement): catalog(catalog){
    

    drop_table(catalog, statement);

    dropped=true;
}


void DropTable::drop_table(Catalog* catalog, const BoundDropTableStatement& statement){

    string table_name = statement.table_name;
    catalog->DropTable(table_name);
    
}

bool DropTable::is_dropped(){return this->dropped;}

bool DropTable::getNext(Tuple* tuple){return false;};

TableHeap* DropTable::getTableHeap(){
    return nullptr;
}

vector<Column> DropTable::get_output_schema(){return {};}

bool DropTable::has_column(string col_name){return false;};

Tuple DropTable::get_tuple(){
    return Tuple({});
}
