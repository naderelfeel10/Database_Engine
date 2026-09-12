#include"Binder/Expression.h"


//2 constructors
ColumnRefExpression::ColumnRefExpression(const string& column):column_name(column){
    type = ExpressionType::COLUMN_REF;
}

ColumnRefExpression::ColumnRefExpression(const string& table, const string& column): table_name(table), column_name(column){
    type = ExpressionType::COLUMN_REF;
}

ConstantExpression::ConstantExpression(const int int_value){
        val_type = TYPE_INT;
        value.int_const = int_value;
        type = ExpressionType::CONSTANT;
}

ConstantExpression::ConstantExpression(const double float_value){
        val_type = TYPE_FLOAT;
        value.float_const = float_value;
        type = ExpressionType::CONSTANT;
}

ConstantExpression::ConstantExpression(const bool bool_value){
        val_type = TYPE_BOOL;
        value.bool_const = bool_value;
        type = ExpressionType::CONSTANT;
}

ConstantExpression::ConstantExpression(const string& str_value) {
        val_type = TYPE_STRING;
        new (&value.str_const)string(str_value);
        type = ExpressionType::CONSTANT;
}


BinaryExpression::BinaryExpression(Expression* left, BinaryOperator op, Expression* right):left(left), right(right), op(op){
        type = ExpressionType::BINARY;
}

ConstantExpression::Value::Value() {
}

ConstantExpression::Value::~Value() {
}
