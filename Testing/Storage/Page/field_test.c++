#include"Storage/Page/Field.h"
#include<iostream>
#include<gtest/gtest.h>
#include <cstring>
using namespace std;


TEST(FieldTest, CreateIntegerField)
{
    Field f(TYPE_INT, 100);

    EXPECT_EQ(f.getFieldType(), TYPE_INT);
    EXPECT_EQ(f.getFieldValueInt(), 100);
    EXPECT_FALSE(f.isNull());
}


TEST(FieldTest, CreateFloatField)
{
    Field f(TYPE_FLOAT, 3.14);

    EXPECT_EQ(f.getFieldType(), TYPE_FLOAT);

    EXPECT_NEAR(
        f.getFieldValueFloat(),
        3.14,
        0.001
    );

    EXPECT_FALSE(f.isNull());
}


TEST(FieldTest, CreateBoolField)
{
    Field f(TYPE_BOOL, true);

    EXPECT_EQ(f.getFieldType(), TYPE_BOOL);
    EXPECT_TRUE(f.getFieldValueBool());
    EXPECT_FALSE(f.isNull());
}


TEST(FieldTest, CreateStringField)
{
    Field f(TYPE_STRING, "Ahmed");

    EXPECT_EQ(f.getFieldType(), TYPE_STRING);

    EXPECT_STREQ(
        f.getFieldValueStr(),
        "Ahmed"
    );

    EXPECT_FALSE(f.isNull());
}



TEST(FieldTest, NullField)
{
    Field f(TYPE_INT);

    EXPECT_TRUE(f.isNull());
}



TEST(FieldTest, CopyConstructorInteger)
{
    Field original(TYPE_INT,50);

    Field copy(original);


    EXPECT_EQ(
        copy.getFieldValueInt(),
        50
    );

    EXPECT_EQ(
        copy.getFieldType(),
        TYPE_INT
    );
}



TEST(FieldTest, CopyConstructorString)
{
    Field original(TYPE_STRING,"Database");


    Field copy(original);


    EXPECT_STREQ(
        original.getFieldValueStr(),
        copy.getFieldValueStr()
    );


    //verify deep copy
    EXPECT_NE(
        original.getFieldValueStr(),
        copy.getFieldValueStr()
    );
}



TEST(FieldTest, AssignmentOperator)
{
    Field a(TYPE_INT,10);
    Field b(TYPE_INT,20);

    b=a;

    EXPECT_EQ(
        b.getFieldValueInt(),
        10
    );
}



TEST(FieldTest, EqualityOperator)
{
    Field a(TYPE_INT,100);

    Field b(TYPE_INT,100);

    Field c(TYPE_INT,200);


    EXPECT_TRUE(a==b);

    EXPECT_FALSE(a==c);
}



TEST(FieldTest, ComparisonOperators)
{
    Field a(TYPE_INT,10);

    Field b(TYPE_INT,20);


    EXPECT_TRUE(a<b);

    EXPECT_TRUE(b>a);

    EXPECT_TRUE(a<=b);

    EXPECT_TRUE(b>=a);
}



TEST(FieldTest, StringComparison1)
{
    Field a(TYPE_STRING,"Ahmed");

    Field b(TYPE_STRING,"Ziad");



    EXPECT_TRUE(a<b);

    EXPECT_TRUE(b>a);

    EXPECT_TRUE(a<=b);

    EXPECT_TRUE(b>=a);
}

TEST(FieldTest, StringComparison2)
{
    Field a(TYPE_STRING);
    a.set_null(true);

    Field b(TYPE_STRING,"Ziad");
    b.set_null(true);

    EXPECT_TRUE(a==b);

}

TEST(FieldTest, StringComparison3)
{
    Field a(TYPE_STRING);
    a.set_null(true);

    Field b(TYPE_STRING,"Ziad");
    b.set_null(false);

    EXPECT_FALSE(a==b);

}

TEST(FieldTest, StringComparison4)
{
    Field a(TYPE_STRING);
    a.set_null(false);

    Field b(TYPE_STRING,"Ziad");
    b.set_null(true);

    EXPECT_FALSE(a==b);

}


TEST(FieldTest, IntegerSerialization)
{
    Field original(TYPE_INT,123);

    char buffer[100];
    original.serialize(buffer);

    Field restored(TYPE_INT);
    restored.deserialize(buffer);

    EXPECT_TRUE(
        original==restored
    );
}



TEST(FieldTest, StringSerialization)
{
    Field original(
        TYPE_STRING,
        "ELFEEL"
    );

    char buffer[100];

    original.serialize(buffer);
    //diff datatype to check if it would change
    Field restored(TYPE_INT);

    restored.deserialize(buffer);
    EXPECT_TRUE(
        original==restored
    );
}


TEST(FieldTest, FloatSerialization)
{
    Field original(TYPE_FLOAT,55.5);


    char buffer[100];


    original.serialize(buffer);


    Field restored(TYPE_FLOAT);
    restored.deserialize(buffer);

    EXPECT_NEAR(
        restored.getFieldValueFloat(),
        55.5,
        0.001
    );
}

/*int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/