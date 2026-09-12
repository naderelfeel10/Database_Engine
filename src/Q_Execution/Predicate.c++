#include<iostream>
#include"Q_Execution/Predicate.h"
#include<cassert>
using namespace std;



template<typename T>
bool Predicate::compareValues(T v1, T v2){
    //return comparision type
    switch(predicate_type){
        case PredicateType::EQ:
            return v1==v2;
        case PredicateType::NE:
            return v1!=v2;
        case PredicateType::GT:
            return v1>v2;
        case PredicateType::GE:
            return v1>=v2;        
        case PredicateType::LT:
            return v1<v2;        
        case PredicateType::LE:
            return v1<=v2;
    }
    return false;
}

bool Predicate::checkPredicate(){

    assert(this->left_col != nullptr && "left_col is null!");
    assert(this->right_col != nullptr && "right_col is null!");

    Field* left_field = this->left_col->getField();
    Field* right_field = this->right_col->getField();

    FieldType type1 = left_field->getFieldType();
    FieldType type2 = right_field->getFieldType();

    // check if types match
    assert(type1 == type2);
    //check all types i support
    switch (type1)
    {
    case TYPE_INT:{
        int value1 = left_field->getFieldValueInt();
        int value2 = right_field->getFieldValueInt();
        return this->compareValues(value1, value2);
    }
    case TYPE_BOOL:{
        bool value1 = left_field->getFieldValueBool();
        bool value2 = right_field->getFieldValueBool();
        return this->compareValues(value1, value2);
    }
    case TYPE_FLOAT:{
        double value1 = left_field->getFieldValueFloat();
        double value2 = right_field->getFieldValueFloat();
        return this->compareValues(value1, value2);
    }
    case TYPE_STRING:{
        string_view value1 = left_field->getFieldValueStr();
        string_view value2 = right_field->getFieldValueStr();
        return this->compareValues(value1, value2);
    }
    default:{
        cerr<<"wrong field type"<<endl;
        return false;
    }
    
}
    
}
bool Predicate::evaluate( Tuple* tuple, vector<Column> cols)
{
    Field* active_left_field = nullptr;
    Field* active_right_field = nullptr;

    for (size_t i = 0; i < cols.size(); ++i)
    {
        // cout << this->left_col->getColName()
        //      << ",,,," << cols[i].getColName() << endl;

        if (this->left_col->getColName() == cols[i].getColName())
        {
            active_left_field = new Field(tuple->fields[i]);
            //active_left_field->print();
        }

        // cout << this->right_col->getColName()
        //      << " ,,,, " << cols[i].getColName() << endl;

        if (this->right_col->getColName() == cols[i].getColName())
        {
            active_right_field = new Field(tuple->fields[i]);
            //active_left_field->print();
        }
    }

    // If any col is const, its name contains "_const_"
    if (left_col->getColName().find("const") != string::npos)
    {
        active_left_field = left_col->getField();
    }

    if (right_col->getColName().find("const") != string::npos)
    {
        active_right_field = right_col->getField();
    }

    active_left_field =
        active_left_field ? active_left_field : this->left_col->getField();

    active_right_field =
        active_right_field ? active_right_field : this->right_col->getField();

    assert(
        active_left_field != nullptr &&
        active_right_field != nullptr &&
        "column names not found in row metadata!"
    );

    string left_col_name = left_col->getColName();
    string right_col_name = right_col->getColName();

    // int left_col_size = left_col->getColSize();
    // int right_col_size = right_col->getColSize();

    int left_col_size =
        sizeof(active_left_field->getFieldType());

    int right_col_size =
        sizeof(active_right_field->getFieldType());

    // delete left_col;
    // delete right_col;

    left_col = new Column(
        active_left_field,
        left_col_name,
        left_col_size
    );

    right_col = new Column(
        active_right_field,
        right_col_name,
        right_col_size
    );

    //left_col->printCol();
    //right_col->printCol();

    //left_col->getField()->print();
    //right_col->getField()->print();

    // return this->compareValues(
    //     active_left_field,
    //     active_right_field
    // );

    return this->checkPredicate();
}

Column* Predicate::getcol(int col_index){
    if(col_index==0)return this->left_col;
        return this->right_col;
}
/*
bool Predicate::evaluate(Tuple* tuple,  vector<Column> cols)
{
    //tuple->print();
    Field* active_left_field = nullptr;
    Field* active_right_field = nullptr;

    // =========================
    // LEFT OPERAND
    // =========================

    if (left_col->getColName().find("_const_") != string::npos)
    {
        active_left_field = left_col->getField();
    }
    else
    {
        for (size_t i = 0; i < cols.size(); ++i)
        {
            if (left_col->getColName() == cols[i].getColName())
            {
                active_left_field = new Field(tuple->fields[i]);
                break;
            }
        }
    }

    // =========================
    // RIGHT OPERAND
    // =========================

    if (right_col->getColName().find("_const_") != string::npos)
    {
        active_right_field = right_col->getField();
    }
    else
    {
        for (size_t i = 0; i < cols.size(); ++i)
        {
            if (right_col->getColName() == cols[i].getColName())
            {
                active_right_field = &tuple->fields[i];
                break;
            }
        }
    }

    // =========================
    // VALIDATION
    // =========================

    if (active_left_field == nullptr)
    {
        throw runtime_error(
            "Left column not found: " +
            left_col->getColName()
        );
    }

    if (active_right_field == nullptr)
    {
        throw runtime_error(
            "Right column not found: " +
            right_col->getColName()
        );
    }

    //cout << "LEFT: ";
    //active_left_field->print();

    //cout << "RIGHT: ";
    //active_right_field->print();

    // =========================
    // COMPARE DIRECTLY
    // =========================

    FieldType left_type = active_left_field->getFieldType();
    FieldType right_type = active_right_field->getFieldType();

    if (left_type != right_type)
    {
        throw runtime_error("Predicate type mismatch");
    }

    switch (left_type)
    {
        case TYPE_INT:
            return compareValues(
                active_left_field->getFieldValueInt(),
                active_right_field->getFieldValueInt()
            );

        case TYPE_FLOAT:
            return compareValues(
                active_left_field->getFieldValueFloat(),
                active_right_field->getFieldValueFloat()
            );

        case TYPE_BOOL:
            return compareValues(
                active_left_field->getFieldValueBool(),
                active_right_field->getFieldValueBool()
            );

        case TYPE_STRING:
            return compareValues(
                active_left_field->getFieldValueStr(),
                active_right_field->getFieldValueStr()
            );

        default:
            throw runtime_error("Unsupported field type");
    }
}
*/

/*
int 
main(){
    // check int type
    Field* f1 = new Field(TYPE_INT,5);
    Field* f2 = new Field(TYPE_INT,8);

    Column* c1 = new Column(f1,"c1",4);
    Column* c2 = new Column(f2,"c2",4);

    Predicate* predicate1 = new Predicate(c1,c2,PredicateType::EQ);
    cout<<"check if f1 and f2 are equal"<<endl;
    cout<<predicate1->checkPredicate()<<endl;

    cout<<"============================"<<endl;
    // check float type
    Field* f3 = new Field(TYPE_FLOAT,5.3);
    Field* f4 = new Field(TYPE_FLOAT,8.5);

    Column* c3 = new Column(f3,"c3",4);
    Column* c4 = new Column(f4,"c4",4);

    Predicate* predicate2 = new Predicate(c3,c4,PredicateType::LT);
    cout<<"check if f3 and f4 are equal"<<endl;
    cout<<predicate2->checkPredicate()<<endl;

    cout<<"============================"<<endl;
    // check str type
    Field* f5 = new Field(TYPE_STRING,"nader");
    Field* f6 = new Field(TYPE_STRING,"not_nader");

    Column* c5 = new Column(f5,"c5",4);
    Column* c6 = new Column(f6,"c6",4);

    Predicate* predicate3 = new Predicate(c5,c6,PredicateType::EQ);
    cout<<"check if f5 and f6 are equal"<<endl;
    cout<<predicate3->checkPredicate()<<endl;

}
*/