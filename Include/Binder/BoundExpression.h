#ifndef BOUND_EXPRESSION_H
#define BOUND_EXPRESSION_H


#include"Storage\Page\Field.h"
#include"Storage\Table\Column.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\parser\external\sql-parser\src\sql\Expr.h"
#include"Q_Execution\SortAggregateExecuter.h"
#include<iostream>
#include<vector>
using namespace std;


enum  BoundExpressionType {
    COLUMN_REF,
    CONSTANT,
    BINARY,
    UNARY,
    FUNCTION,
    STAR
};

class BoundExpression {
public:
    FieldType return_type;
    BoundExpressionType exp_type;
    //virtual ~BoundExpression() = default;
    BoundExpression(BoundExpressionType type);
    //just printing
    //virtual void PrintTree(const string& prefix, bool isLast) const ;
    virtual void PrintTree(const string& prefix = "",bool isLast = true) const;
};



// i want to represent the bounded col like this :
// ColRef("age") : 
    /*BoundColumnRef
        table_oid = 10
        column_oid = 2
        return_type = INT*/

class BoundColumnRef:  public BoundExpression{

public:
    int table_oid;
    int column_oid;

    string table_name;
    string column_name;

    BoundColumnRef();
    //this constuctor recieves input from the parser output, then creates the BoundedCol
    BoundColumnRef(int table_oid, int column_oid, const string& table_name, const string& column_name, FieldType return_type);

    // just printing
    void PrintTree(const string& prefix = "",bool isLast = true) const override;
};

/*
BoundTable
    oid = 10
    table_name = users
    alias = u
*/
class BoundTable{
    public:
        int table_oid;

        string table_name;
        string alias;
        vector<Column> schema;
    /*
    void printTable(){
        cout<<"table oid : "<<table_oid;
    }*/
    // a better printing
    void printTable() const;
};


class BoundConstantExpression : public BoundExpression {

public:
    // union of possible values the const might have:
    // int, double, cool, string
    union Value{
        int64_t int_const;
        double float_const;
        bool bool_const;
        string str_const;

        Value(){};
        ~Value(){};
    }value;


    BoundConstantExpression(const int64_t int_value);

    BoundConstantExpression(const double float_value);

    BoundConstantExpression(const bool bool_value);

    BoundConstantExpression(const string& str_value);

    //just printing
    void PrintTree(const string& prefix = "",bool isLast = true) const override;
};
/*
enum class BinaryOperator{
    EQ,
    NE, 
    GT, 
    GE, 
    LT, 
    LE 
};*/
enum class BoundOperatorType {
    EQ,
    NE,
    GT,
    GE,
    LT,
    LE,
    AND,
    OR,
    NOT,
    ADD,
    SUB,
    MUL,
    DIV
};




class BoundBinaryExpression : public BoundExpression{

public:
    // a simple representation of the predicate
    BoundExpression* left;
    BoundExpression* right;

    BoundOperatorType op;

    BoundBinaryExpression(BoundExpression* left, BoundOperatorType op,BoundExpression* right, FieldType return_type);

    void PrintTree(const string& prefix = "",bool isLast = true) const override;

    
};

//bound functions like count, avg, or defined functions
class BoundFunctionExpression : public BoundExpression{

public:
    string function_name;
    AggregateType function_type;
    BoundExpression* argument;

    BoundFunctionExpression(const string& function_name, AggregateType function_type, BoundExpression* argument);

    void PrintTree(const string& prefix = "",bool isLast = true) const override;
};

#endif