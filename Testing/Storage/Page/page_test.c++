#include <gtest/gtest.h>

#include "Storage/Page/page.h"

#include <vector>
#include <cstring>


static Tuple CreateSimpleTuple()
{
    return Tuple(
    {
        Field(TYPE_INT,100),
        Field(TYPE_STRING,"Ahmed"),
        Field(TYPE_FLOAT,55.5),
        Field(TYPE_BOOL,true)
    });
}



static Tuple CreateLargeTuple()
{
    std::string big(500,'A');

    return Tuple(
    {
        Field(TYPE_INT,999),
        Field(TYPE_STRING,big.c_str())
    });
}



TEST(PageTest, PageInitialization)
{
    Page page(10);

    PageHeader* header =
        reinterpret_cast<PageHeader*>(page.getData());


    EXPECT_EQ(header->page_id,10);

    EXPECT_EQ(header->num_tuples,0);

    EXPECT_EQ(header->num_deleted_tuples,0);

    EXPECT_EQ(header->free_space_pointer,PAGE_SIZE);
}



TEST(PageTest, InsertSingleTuple)
{
    Page page(1);


    Tuple tuple = CreateSimpleTuple();


    int slot =
        page.insertTuple(tuple);


    EXPECT_EQ(slot,0);



    Tuple result({});


    bool found =
        page.getTuple(slot,result);


    EXPECT_TRUE(found);


    EXPECT_FALSE(
        result.get_is_deleted()
    );
}



TEST(PageTest, InsertMultipleTuples)
{
    Page page(1);


    Tuple tuple = CreateSimpleTuple();



    for(int i=0;i<60;i++)
    {
        int slot =
            page.insertTuple(tuple);

        EXPECT_EQ(slot,i);
    }


    for(int i=0;i<60;i++)
    {
        Tuple result({});

        EXPECT_TRUE(
            page.getTuple(i,result)
        );
    }
}


TEST(PageTest, InsertUntilFull)
{
    Page page(1);


    Tuple tuple =
    CreateLargeTuple();


    int inserted=0;


    while(true)
    {
        int slot =
            page.insertTuple(tuple);


        if(slot==-1)
            break;


        inserted++;
    }


    EXPECT_GT(
        inserted,
        0
    );
}



TEST(PageTest, RetrieveInvalidSlot)
{
    Page page(1);


    Tuple result({});


    EXPECT_FALSE(
        page.getTuple(
            100,
            result
        )
    );


    EXPECT_FALSE(
        page.getTuple(
            -1,
            result
        )
    );
}



TEST(PageTest, DeleteTuple)
{
    Page page(1);


    Tuple tuple =
        CreateSimpleTuple();



    int slot =
        page.insertTuple(tuple);


    EXPECT_TRUE(
        page.deleteTuple(slot)
    );



    Tuple result({});


    EXPECT_FALSE(
        page.getTuple(slot,result)
    );



    PageHeader* header =
        reinterpret_cast<PageHeader*>(page.getData());


    EXPECT_EQ(
        header->num_deleted_tuples,
        1
    );
}



TEST(PageTest, DeleteSameTupleTwice)
{
    Page page(1);


    int slot =
        page.insertTuple(
            CreateSimpleTuple()
        );


    EXPECT_TRUE(
        page.deleteTuple(slot)
    );


    EXPECT_FALSE(
        page.deleteTuple(slot)
    );
}



TEST(PageTest, DeleteInvalidSlot)
{
    Page page(1);


    EXPECT_FALSE(
        page.deleteTuple(5)
    );
}



TEST(PageTest, UpdateSameSizeTuple)
{
    Page page(1);

    Tuple oldTuple =
    {
        {
            Field(TYPE_INT,10),
            Field(TYPE_STRING,"AAA")
        }
    };


    int slot =
        page.insertTuple(oldTuple);



    Tuple newTuple =
    {
        {
            Field(TYPE_INT,20),
            Field(TYPE_STRING,"BBB")
        }
    };


    int result =
        page.updateTuple(
            slot,
            newTuple
        );


    EXPECT_EQ(
        result,
        slot
    );



    Tuple read({});


    EXPECT_TRUE(
        page.getTuple(slot,read)
    );


    EXPECT_EQ(
        read.fields[0].getFieldValueInt(),
        20
    );
    
    EXPECT_STREQ(
        read.fields[1].getFieldValueStr(),
        "BBB"
    );
}



TEST(PageTest, UpdateBiggerTuple)
{
    Page page(1);


    Tuple oldTuple(
    {
        Field(TYPE_INT,1),
        Field(TYPE_STRING,"A")
    });

    int slot =
        page.insertTuple(oldTuple);



    Tuple bigger(
    {
        Field(TYPE_INT,2),
        Field(TYPE_STRING,
              "This is a much bigger string")
    });



    int newSlot =
        page.updateTuple(
            slot,
            bigger
        );


    EXPECT_NE(
        newSlot,
        -1
    );

    Tuple result({});

    EXPECT_TRUE(
        page.getTuple(
            newSlot,
            result
        )
    );

    EXPECT_EQ(
        result.fields[0].getFieldValueInt(),
        2
    );
}



/*TEST(PageTest, InsertRawData)
{
    Page page(1);


    char data[]="Hello Page";


    int slot =
        page.insertData(
            data,
            strlen(data)+1
        );


    EXPECT_EQ(slot,0);



    char* result=nullptr;


    EXPECT_TRUE(
        page.getIndexData(
            slot,
            result
        )
    );


    EXPECT_STREQ(
        result,
        "Hello Page"
    );
}
*/


TEST(PageTest, GetIndexDataInvalid)
{
    Page page(1);

    char* buffer=nullptr;

    EXPECT_FALSE(
        page.getIndexData(
            5,
            buffer
        )
    );
}


TEST(PageTest, DeletedTupleCannotBeRead)
{
    Page page(1);

    int slot =
        page.insertTuple(
            CreateSimpleTuple()
        );

    page.deleteTuple(slot);

    Tuple result({});

    EXPECT_FALSE(
        page.getTuple(
            slot,
            result
        )
    );
}



TEST(PageTest, GetAllFieldsFromColumn)
{
    Page page(1);



    page.insertTuple(
        Tuple({
            Field(TYPE_INT,10),
            Field(TYPE_STRING,"A")
        })
    );


    page.insertTuple(
        Tuple({
            Field(TYPE_INT,20),
            Field(TYPE_STRING,"B")
        })
    );


    auto result =
        page.get_field_from_all_tuples(0);


    EXPECT_EQ(
        result.size(),
        2
    );


    EXPECT_EQ(
        result[0].getFieldValueInt(),
        10
    );


    EXPECT_EQ(
        result[1].getFieldValueInt(),
        20
    );
}



TEST(PageTest, GetCustomColumns)
{
    Page page(1);

    page.insertTuple(
        Tuple({
            Field(TYPE_INT,10),
            Field(TYPE_STRING,"A"),
            Field(TYPE_BOOL,true)
        })
    );


    auto result =
        page.get_custom_fields_from_all_tuples(
            {0,2}
        );

    EXPECT_EQ(
        result.size(),
        1
    );

    EXPECT_EQ(
        result[0][0].getFieldValueInt(),
        10
    );

    EXPECT_TRUE(
        result[0][1].getFieldValueBool()
    );
}


TEST(PageTest, PageHandlesDifferentTupleSizes)
{
    Page page(1);

    EXPECT_NE(
        page.insertTuple(
            Tuple({
                Field(TYPE_STRING,"A")
            })
        ),
        -1
    );



    EXPECT_NE(
        page.insertTuple(
            Tuple({
                Field(TYPE_STRING,
                "A very very long string")
            })
        ),
        -1
    );
}



TEST(PageTest, DeletedFlagInsideTuple)
{
    Page page(1);


    int slot =
        page.insertTuple(
            CreateSimpleTuple()
        );


    page.deleteTuple(slot);



    Tuple result({});


    page.getTuple(slot,result);


    EXPECT_TRUE(
        result.get_is_deleted()
    );
}

/*int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/