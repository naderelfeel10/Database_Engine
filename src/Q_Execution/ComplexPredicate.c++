#include<iostream>
#include"Q_Execution/ComplexPredicate.h"
#include<cassert>
using namespace std;


ComplexPredicate::ComplexPredicate(AbstractPredicate* left_predicate, AbstractPredicate* right_predicate, ComplexPredicateType type):left_predicate(left_predicate),
right_predicate(right_predicate),c_predicate_type(type){}


AbstractPredicate* ComplexPredicate::getpred(int pred_index){
    if(pred_index==0){
        return this->left_predicate;
    }
    return this->right_predicate;
}
/*
bool ComplexPredicate::checkcomplexPredicate(){

    switch (c_predicate_type)
    {
    case ComplexPredicateType::AND :{
        return this->left_predicate->checkPredicate() &&  this->right_predicate->checkPredicate();
    }
    case ComplexPredicateType::OR :{
        return this->left_predicate->checkPredicate() ||  this->right_predicate->checkPredicate();
    }
    case ComplexPredicateType::NOT :{
        return !this->left_predicate->checkPredicate();
    }
    default:
        cerr<<"wrong complex predicate type"<<endl;
        return false;
    }

}
*/
bool ComplexPredicate::evaluate( Tuple* tuple, vector<Column> cols){
    switch (c_predicate_type)
    {
    case ComplexPredicateType::AND :{
        if(this->right_predicate){
            return this->left_predicate->evaluate(tuple, cols) &&  this->right_predicate->evaluate(tuple, cols);
        }else{
            return this->left_predicate->evaluate(tuple, cols);
        }
    }
    case ComplexPredicateType::OR :{
        if(this->right_predicate){
            return this->left_predicate->evaluate(tuple, cols) ||  this->right_predicate->evaluate(tuple, cols);
        }else{
            return this->left_predicate->evaluate(tuple, cols);
        }
    }
    case ComplexPredicateType::NOT :{
        return !this->left_predicate->evaluate(tuple, cols);
    }
    default:
        cerr<<"wrong complex predicate type"<<endl;
        return false;
    }
}

/*
int 
main(){
    // check int type
    Field* f1 = new Field(TYPE_INT,5);
    Field* f2 = new Field(TYPE_INT,8);

    Column* c1 = new Column(f1,"c1",4);
    Column* c2 = new Column(f2,"c2",4);

    Predicate* predicate1 = new Predicate(c1,c2,PredicateType::LT);
    //cout<<"check if f1 and f2 are equal"<<endl;
    //cout<<predicate1->checkPredicate()<<endl;

    //cout<<"============================"<<endl;
    // check float type
    Field* f3 = new Field(TYPE_FLOAT,5.3);
    Field* f4 = new Field(TYPE_FLOAT,8.5);

    Column* c3 = new Column(f3,"c3",4);
    Column* c4 = new Column(f4,"c4",4);

    Predicate* predicate2 = new Predicate(c3,c4,PredicateType::LT);
    //cout<<"check if f3 and f4 are equal"<<endl;
    //cout<<predicate2->checkPredicate()<<endl;

    //cout<<"============================"<<endl;
    // check str type
    Field* f5 = new Field(TYPE_STRING,"nader");
    Field* f6 = new Field(TYPE_STRING,"nader");

    Column* c5 = new Column(f5,"c5",4);
    Column* c6 = new Column(f6,"c6",4);

    Predicate* predicate3 = new Predicate(c5,c6,PredicateType::EQ);
    //cout<<"check if f5 and f6 are equal"<<endl;
    //cout<<predicate3->checkPredicate()<<endl;

    // check if (f1 < 8) and (f5 == "nader") ;
    ComplexPredicate* complex_predicate = new ComplexPredicate(predicate1, predicate3,ComplexPredicateType::AND);
    cout<<"check (f1 < 8) and (f5 == 'nader') "<<endl;
    cout<<complex_predicate->checkcomplexPredicate();


}
*/