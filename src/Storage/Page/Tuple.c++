#include<iostream>
#include<vector>
#include "Storage/Page/Tuple.h"
using namespace std;


Tuple::Tuple(vector<Field> fields){
    for(auto& field : fields) {
        this->fields.push_back(field); 
        tulpe_size += field.getSerializedSize(); 
    }

}
/*
void Tuple::print(){
        cout<<"is deleted << "<<this->get_is_deleted()<<endl;
        cout<<"tuple size << "<<this->tulpe_size<<endl;

        for(auto& field : this->fields) {
            field.print();
    }
}
*/
// a better way to print
void Tuple::print() {
    std::cout << "| ";
    for (auto& field : this->fields)
        field.print();
    std::cout << '\n';  // '\n' is faster than std::endl (no flush)
}

int Tuple::getTupleSize() const {
    int total = sizeof(this->is_deleted) + sizeof(this->tulpe_size);
    for (const auto& field : fields) {
        total += field.getSerializedSize();
    }
    return total;
}

void Tuple::serialize(char* buffer){
    int offset = 0;
    buffer[0] = is_deleted?1:0;
    offset +=1;

    memcpy(buffer+offset, &tulpe_size, sizeof(tulpe_size));
    offset +=sizeof(tulpe_size);

    for(auto&field:this->fields){
        field.serialize(buffer+offset);
        offset+=field.getSerializedSize();
    }

}
void Tuple::deserialize(char* buffer){
    int offset = 0;
    is_deleted =  buffer[0]?true:false;
    offset +=1;

    memcpy(&tulpe_size,buffer+offset,  sizeof(tulpe_size));
    offset +=sizeof(tulpe_size);

        fields.clear();
        while(offset<tulpe_size){
            Field tmp(TYPE_INT);
            tmp.deserialize(buffer+offset);
            //tmp.print();
            fields.push_back(tmp);
            offset+=tmp.getSerializedSize();
        }
}


bool Tuple::get_is_deleted(){
    return this->is_deleted;
}
void Tuple::set_is_deleted(bool new_status){
    this->is_deleted = new_status;
}

/*
int main(){
    vector<Field> myFields = {
        Field(TYPE_INT, 101),
        Field(TYPE_STRING, "Nader"),
        Field(TYPE_FLOAT, 99.5)
    };
    Tuple t1(myFields);
    t1.print();

    char* buffer = new char[t1.getTupleSize()+1];
    t1.serialize(buffer);
    for(int i = 0; i < t1.getTupleSize(); i++) {
    // Cast to unsigned char to see the actual byte values (0-255)
    cout << (int)(unsigned char)buffer[i] << " ";
    }
    cout<<endl;

    Tuple t2({});
    t2.deserialize(buffer);
    t2.print();
}
*/