#include"Storage/Page/Tuple.h"
#include<iostream>
#include<gtest/gtest.h>
#include <cstring>
using namespace std;



TEST(TupleTest, CreateTuple)
{

    std::vector<Field> fields =
    {
        Field(TYPE_INT,10),
        Field(TYPE_STRING,"Ahmed"),
        Field(TYPE_FLOAT,5.5),
        Field(TYPE_BOOL,true)
    };


    Tuple tuple(fields);


    EXPECT_EQ(
        tuple.getTupleSize(),
        1 + sizeof(int)
        +
        fields[0].getSerializedSize()
        +
        fields[1].getSerializedSize()
        +
        fields[2].getSerializedSize()
        +
        fields[3].getSerializedSize()
    );
}



TEST(TupleTest, SerializeDeserializeTuple)
{

    Tuple original(
    {
        Field(TYPE_INT,100),
        Field(TYPE_STRING,"Database"),
        Field(TYPE_FLOAT,3.14),
        Field(TYPE_BOOL,true)
    });


    int size = original.getTupleSize();


    char* buffer = new char[size];


    original.serialize(buffer);

    Tuple restored({});

    restored.deserialize(buffer);

    delete []buffer;

    EXPECT_EQ(
        restored.getTupleSize(),
        original.getTupleSize()
    );
}



TEST(TupleTest, TupleDeletedFlag)
{

    Tuple tuple(
    {
        Field(TYPE_INT,5)
    });


    EXPECT_FALSE(
        tuple.get_is_deleted()
    );

    tuple.set_is_deleted(true);

    EXPECT_TRUE(
        tuple.get_is_deleted()
    );
}


TEST(TupleTest, EmptyTuple)
{

    Tuple tuple({});

    EXPECT_GT(
        tuple.getTupleSize(),
        0
    );
}



TEST(TupleTest, MultipleStringFields)
{

    Tuple original(
    {
        Field(TYPE_STRING,"Ahmed"),
        Field(TYPE_STRING,"Database"),
        Field(TYPE_STRING,"Engine")
    });

    int size = original.getTupleSize();

    char* buffer = new char[size];

    original.serialize(buffer);

    Tuple restored({});
    restored.deserialize(buffer);

    delete []buffer;

    EXPECT_EQ(
        restored.getTupleSize(),
        original.getTupleSize()
    );

    EXPECT_EQ(
        original.fields[0],
        restored.fields[0]   
    );
    EXPECT_EQ(
        original.fields[1],
        restored.fields[1]   
    );
    EXPECT_EQ(
        original.fields[2],
        restored.fields[2]   
    );
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}