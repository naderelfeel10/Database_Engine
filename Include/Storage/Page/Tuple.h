#ifndef TUPLE_H
#define TUPLE_H

#include <iostream>
#include <cstring>
#include <vector>
#include "Field.h"


class Tuple {
    private:
        bool is_deleted{false};
        int tulpe_size{};

public:
    std::vector<Field>fields;

    Tuple(std::vector<Field>fields);
    //int getSize() ;
    int getTupleSize() const ;

    void print() ;
    void serialize(char* buffer);
    void deserialize(char* buffer);
    bool get_is_deleted();
    void set_is_deleted(bool new_status);
    

};

#endif 