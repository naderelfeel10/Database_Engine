#include <gtest/gtest.h>

#include "Buffer/LRU_replacement.h"

#include <cstring>


class LRUTest : public ::testing::Test {

protected:

    LRU* lru;


    void SetUp() override
    {
        lru = new LRU(3);
    }


    void TearDown() override
    {
        delete lru;
    }


    char* createValue(const char* str)
    {
        char* buffer = new char[64];
        strcpy(buffer,str);
        return buffer;
    }

};



TEST_F(LRUTest, InsertAndRetrieve)
{
    char* value = createValue("page1");


    lru->put_frame(1,value);


    ASSERT_NE(lru->get_frame(1),nullptr);
    EXPECT_STREQ(lru->get_frame(1),"page1");
}



TEST_F(LRUTest, NonExistingFrameReturnsNull)
{

    EXPECT_EQ(lru->get_frame(100),nullptr);

}



TEST_F(LRUTest, MultipleInsertions)
{

    lru->put_frame(1,createValue("one"));
    lru->put_frame(2,createValue("two"));
    lru->put_frame(3,createValue("three"));


    EXPECT_STREQ(lru->get_frame(1),"one");
    EXPECT_STREQ(lru->get_frame(2),"two");
    EXPECT_STREQ(lru->get_frame(3),"three");

}



TEST_F(LRUTest, LRU_EvictsLeastRecentlyUsed)
{

    lru->put_frame(1,createValue("one"));
    lru->put_frame(2,createValue("two"));
    lru->put_frame(3,createValue("three"));


    /*
        order:

        head
          |
          3
          2
          1
          |
        tail

    */


    int removed = lru->evict_frame();


    EXPECT_EQ(removed,1);


    EXPECT_EQ(lru->get_frame(1),nullptr);

    EXPECT_NE(lru->get_frame(2),nullptr);
    EXPECT_NE(lru->get_frame(3),nullptr);

}



TEST_F(LRUTest, AccessChangesPriority)
{

    lru->put_frame(1,createValue("one"));
    lru->put_frame(2,createValue("two"));
    lru->put_frame(3,createValue("three"));



    // make page 1 recently used

    lru->get_frame(1);



    /*
       New order:

       1
       3
       2

    */


    int removed = lru->evict_frame();


    EXPECT_EQ(removed,2);

}



TEST_F(LRUTest, UpdatingExistingFrameMovesItToFront)
{

    lru->put_frame(1,createValue("old"));

    lru->put_frame(2,createValue("two"));

    lru->put_frame(1,createValue("new"));



    int removed = lru->evict_frame();



    EXPECT_EQ(removed,2);


    EXPECT_STREQ(
        lru->get_frame(1),
        "new"
    );

}



TEST_F(LRUTest, RemoveSpecificFrame)
{

    lru->put_frame(1,createValue("one"));
    lru->put_frame(2,createValue("two"));


    lru->remove_frame(1);


    EXPECT_EQ(
        lru->get_frame(1),
        nullptr
    );


    EXPECT_NE(
        lru->get_frame(2),
        nullptr
    );

}



TEST_F(LRUTest, FillCapacityAndEvictMany)
{

    lru->put_frame(1,createValue("1"));
    lru->put_frame(2,createValue("2"));
    lru->put_frame(3,createValue("3"));


    EXPECT_EQ(lru->evict_frame(),1);
    EXPECT_EQ(lru->evict_frame(),2);
    EXPECT_EQ(lru->evict_frame(),3);

}



TEST_F(LRUTest, EvictEmptyLRU)
{

    EXPECT_EQ(
        lru->evict_frame(),
        -1
    );

}