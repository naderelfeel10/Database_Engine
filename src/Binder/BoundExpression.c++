#include"Binder/BoundExpression.h"

BoundExpression::BoundExpression(BoundExpressionType type): exp_type(type){}

//just printing
void BoundExpression::PrintTree(const string& prefix,
                           bool isLast) const {
        cout << prefix
             << (isLast ? "|-- " : "|-- ")
             << "BoundExpression\n";
}

BoundColumnRef::BoundColumnRef(int table_oid, int column_oid, const string& table_name, const string& column_name, FieldType return_type)
        :BoundExpression(BoundExpressionType::COLUMN_REF), table_oid(table_oid), column_oid(column_oid), table_name(table_name), column_name(column_name){

        this->return_type = return_type;
}

// just printing
void BoundColumnRef::PrintTree(const string& prefix, bool isLast) const {
    cout << prefix
         << (isLast ? "|-- " : "|-- ")
         << "BoundColumnRef\n";

    string childPrefix = prefix + (isLast ? "    " : "|   ");

    cout << childPrefix << "|-- table: "
         << table_name << '\n';

    cout << childPrefix << "|-- column: "
         << column_name << '\n';

    cout << childPrefix << "|-- table_oid: "
         << table_oid << '\n';

    cout << childPrefix << "|-- column_oid: "
         << column_oid << '\n';

    cout << childPrefix << "|-- type: "
         << return_type << '\n';
}

void BoundTable::printTable() const {
        std::cout << "Table: " << table_name;
        if (!alias.empty()) {
            std::cout << " AS " << alias;
        }
        std::cout << " (OID: " << table_oid << ")\n";
        
        std::cout << "Schema:\n";
        for(Column col:schema){
            col.printCol();
        }
}


BoundConstantExpression::BoundConstantExpression(const int64_t int_value):BoundExpression(BoundExpressionType::CONSTANT){
        return_type = TYPE_INT;
        value.int_const = int_value;
        exp_type = BoundExpressionType::CONSTANT;
}

BoundConstantExpression::BoundConstantExpression(const double float_value):BoundExpression(BoundExpressionType::CONSTANT){
        return_type = TYPE_FLOAT;
        value.float_const = float_value;
        exp_type = BoundExpressionType::CONSTANT;
}

BoundConstantExpression::BoundConstantExpression(const bool bool_value):BoundExpression(BoundExpressionType::CONSTANT){
        return_type = TYPE_BOOL;
        value.bool_const = bool_value;
        exp_type = BoundExpressionType::CONSTANT;
}

BoundConstantExpression::BoundConstantExpression(const string& str_value):BoundExpression(BoundExpressionType::CONSTANT){
        return_type = TYPE_STRING;
        new (&value.str_const)string(str_value);
        exp_type = BoundExpressionType::CONSTANT;
}

void BoundConstantExpression::PrintTree(const string& prefix ,bool isLast ) const {

        cout << prefix
             << (isLast ? "|-- " : "|-- ")
             << "BoundConstant\n";

        string childPrefix =
            prefix + (isLast ? "    " : "|   ");

        switch (return_type) {

            case FieldType::TYPE_INT:
                cout << childPrefix
                     << "|-- value: "
                     << value.int_const << '\n';
                break;

            case FieldType::TYPE_FLOAT:
                cout << childPrefix
                     << "|-- value: "
                     << value.float_const << '\n';
                break;

            case FieldType::TYPE_BOOL:
                cout << childPrefix
                     << "|-- value: "
                     << (value.bool_const ? "true" : "false") << '\n';
                break;

            case FieldType::TYPE_STRING:
                cout << childPrefix
                     << "|-- value: "
                     << value.str_const << '\n';
                break;
            
            case FieldType::TYPE_NULL:
                cout << "NULL";
                break;
        }
}

static string OperatorToString(BoundOperatorType op) {
    switch (op) {
        case BoundOperatorType::EQ:  return "=";
        case BoundOperatorType::NE:  return "!=";
        case BoundOperatorType::GT:  return ">";
        case BoundOperatorType::GE:  return ">=";
        case BoundOperatorType::LT:  return "<";
        case BoundOperatorType::LE:  return "<=";
        case BoundOperatorType::AND: return "AND";
        case BoundOperatorType::OR:  return "OR";
        case BoundOperatorType::ADD: return "+";
        case BoundOperatorType::SUB: return "-";
        case BoundOperatorType::MUL: return "*";
        case BoundOperatorType::DIV: return "/";
    }

    return "?";
}

BoundBinaryExpression::BoundBinaryExpression(BoundExpression* left, BoundOperatorType op,BoundExpression* right, FieldType return_type)
                        :BoundExpression(BoundExpressionType::BINARY), left(left), right(right),op(op){
        exp_type = BoundExpressionType::BINARY;
        this->return_type = return_type;
}

void BoundBinaryExpression::PrintTree(const string& prefix,
                   bool isLast) const {

        cout << prefix
             << (isLast ? "|-- " : "|-- ")
             << "BoundBinaryExpression\n";


        string childPrefix =
            prefix + (isLast ? "    " : "|   ");

        cout << childPrefix
             << "|-- operator: "
             << OperatorToString(op) << '\n';

        cout << childPrefix
             << "|-- left: ";

        if (left)
            cout << "NOT NULL\n";
        else
            cout << "NULL\n";

        if (left) {
            left->PrintTree(childPrefix + "|   ", true);
        }

        cout << childPrefix
             << "`-- right: ";

        if (right)
            cout << "NOT NULL\n";
        else
            cout << "NULL\n";

        if (right) {
            right->PrintTree(childPrefix + "    ", true);
        }

        
}


BoundFunctionExpression::BoundFunctionExpression(const string& function_name, AggregateType function_type, BoundExpression* argument)
                            :BoundExpression(BoundExpressionType::FUNCTION), function_name(function_name), 
                            function_type(function_type),argument(argument){

        this->function_type = function_type;
        this->return_type = this->argument->return_type;

}


        
void BoundFunctionExpression::PrintTree(const string& prefix,
                   bool isLast) const {

        cout << prefix
             << "|-- Function: ";

        switch (function_type) {
            case COUNT: cout << "COUNT"; break;
            case SUM:   cout << "SUM";   break;
            case AVG:   cout << "AVG";   break;
            case MAX:   cout << "MAX";   break;
            case MIN:   cout << "MIN";   break;
        }

        cout << endl;

        if (argument != nullptr) {
            argument->PrintTree(
                prefix + "    ",
                true
            );
        }
}



/*int
main(){

}*/