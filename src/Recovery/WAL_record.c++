#include"WAL_record.h"

//this record shall be flushed on disk immediatly
//serialize it into stream of bytes
void WALRecord::serialize_WAL_record(char* buffer){
    int offset{};
    //save type
    memcpy(buffer+offset, &this->type, sizeof(type));
    offset += sizeof(type);

    //save txn_id
    memcpy(buffer+offset, &this->transaction_id, sizeof(transaction_id));
    offset += sizeof(transaction_id);
    
    //save rid
    rid.serialize(buffer+offset);
    offset+=rid.getSerializedSize();

    //save old tuple size
    int old_tuple_size = old_tuple.getTupleSize();
    //memcpy(buffer+offset, &old_tuple_size, sizeof(old_tuple_size));
    //offset += sizeof(old_tuple_size);

    //save old and new tuples
    old_tuple.serialize(buffer+offset);
    offset += old_tuple_size;
    
    //save new tuple size
    int new_tuple_size = new_tuple.getTupleSize();
    //memcpy(buffer+offset, &new_tuple_size, sizeof(new_tuple_size));
    //offset += sizeof(new_tuple_size);

    new_tuple.serialize(buffer+offset);
    offset += new_tuple_size;
}

//serialize it into stream of bytes
void WALRecord::deSerialize_WAL_record(char* buffer){
    int offset{};
    //load type
    memcpy(&this->type, buffer+offset, sizeof(type));
    offset += sizeof(type);

    //load txn_id
    memcpy( &this->transaction_id, buffer+offset, sizeof(transaction_id));
    offset += sizeof(transaction_id);

    //save rid
    rid.deserialize(buffer+offset);
    offset+=rid.getSerializedSize();
    rid.print();

    //load old and new tuples
    old_tuple.deserialize(buffer+offset);
    offset += old_tuple.getTupleSize();

    old_tuple.print();

    new_tuple.deserialize(buffer+offset);
    offset += new_tuple.getTupleSize();

    new_tuple.print();
}

int WALRecord::get_record_size(){
    int size{};

    size += sizeof(this->type);
    size += sizeof(this->transaction_id);
    size += this->rid.getSerializedSize();
    size += this->old_tuple.getTupleSize();
    size += this->new_tuple.getTupleSize();

    return size;

}


/*int
main(){

}
*/