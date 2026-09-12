#ifndef EXPR_H
#define EXPR_H


#include"Storage\Page\Field.h"
#include<iostream>
using namespace std;

enum class ExpressionType {
    COLUMN_REF,
    CONSTANT,
    BINARY,
    UNARY,
    FUNCTION,
    STAR
};


struct Expression {
    ExpressionType type;
    virtual ~Expression() = default;
};

struct ColumnRefExpression : Expression {
    //col representation as table_name and col_name
    string table_name;     
    string column_name;

    //2 constructors
    ColumnRefExpression(const string& column);
    ColumnRefExpression(const string& table, const string& column);
};


struct ConstantExpression : Expression {

    FieldType val_type;
    // union of possible values the const might have:
    // int, double, cool, string
    union Value{
        int int_const;
        double float_const;
        bool bool_const;
        string str_const;

        Value();
        ~Value();
    }value;

    ConstantExpression(const int int_value);

    ConstantExpression(const double float_value);

    ConstantExpression(const bool bool_value);

    ConstantExpression(const string& str_value);

};

enum class BinaryOperator{
    EQ,
    NE, 
    GT, 
    GE, 
    LT, 
    LE 
};

struct BinaryExpression : Expression{

    // a simple representation of the predicate
    Expression* left;
    Expression* right;

    BinaryOperator op;

    BinaryExpression(Expression* left, BinaryOperator op, Expression* right);

};
#endif