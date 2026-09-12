#include"Binder/BoundInsertStatement.h"


BoundInsertStatement::BoundInsertStatement(){    
}
BoundInsertStatement::BoundInsertStatement(BoundTable* table, vector<Column> columns, vector<BoundExpression*> values)
        :table(table), columns(columns), values(values){
}

BoundStatementType BoundInsertStatement::type()const {
            return BoundStatementType::INSERT;
}

BoundInsertStatement& BoundInsertStatement::operator=(const BoundInsertStatement& other){
        if(this != &other){
            this->table = other.table;
            this->columns = other.columns;
            this->values = other.values;
        }
        return *this;
}


//just printing
void BoundInsertStatement::PrintTree() const  {
        cout << "BoundInsertStatement\n";

        // INTO
        cout << "|-- INTO\n";

        if (table) {
            table->printTable();
        } else {
            cout << "|   NULL\n";
        }
}
