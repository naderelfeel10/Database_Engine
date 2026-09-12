#ifndef Index_H
#define Index_H


#include"Storage/Page/Tuple.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include <string>
using namespace std;

class Index{
    public:
    virtual void Insert(Field&field,std::string col_name, std::vector<Column> tuple_cols, RID rid)=0;
    virtual void Delete(Field&field)= 0;
    virtual vector<RID>  Search(Field&field)const=0;
    virtual string get_index_col_name()=0;

    //virtual void print()const=0;
    //virtual void serialize(char data[]);
    //virtual void deserialize(char data[]);
};

#endif