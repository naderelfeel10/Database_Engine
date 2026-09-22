#include <gtest/gtest.h>

#include "Buffer/BufferPoolManager.h"
#include "Storage/Disk/DiskManager.h"
#include "Storage/Page/page.h"



class BPMTest : public ::testing::Test {


protected:

    DiskManager* dm;
    BufferPoolManager* bpm;



    void SetUp() override
    {

        dm = new DiskManager("test_db");


        bpm = new BufferPoolManager(dm);

    }



    void TearDown() override
    {

        delete bpm;

        delete dm;

    }

};





TEST_F(BPMTest, CreateNewPage)
{

    int page_id = bpm->newPage();


    EXPECT_GT(page_id,0);


    char* buffer =
        bpm->fetchPage(page_id);


    EXPECT_NE(buffer,nullptr);

}



TEST_F(BPMTest, FetchExistingPage)
{

    int page_id =
        bpm->newPage();



    char* page =
        bpm->fetchPage(page_id);



    EXPECT_NE(page,nullptr);



}



TEST_F(BPMTest, CreateMultiplePages)
{


    std::vector<int> ids;


    for(int i=0;i<10;i++)
    {
        ids.push_back(
            bpm->newPage()
        );
    }


    for(auto id:ids)
    {
        EXPECT_NE(
            bpm->fetchPage(id),
            nullptr
        );
    }

}



TEST_F(BPMTest, ModifyDirtyPagePersistence)
{

    int page_id =
        bpm->newPage();



    char* buffer =
        bpm->fetchPage(page_id);



    strcpy(buffer,"HELLO DATABASE");



    bpm->markAsDirty(page_id);



    delete bpm;



    bpm =
        new BufferPoolManager(dm);



    char* new_buffer =
        bpm->fetchPage(page_id);



    EXPECT_STREQ(
        new_buffer,
        "HELLO DATABASE"
    );

}




TEST_F(BPMTest, EvictionWorks)
{

    std::vector<int> ids;



    for(int i=0;i<BUFFER_SIZE+5;i++)
    {
        ids.push_back(
            bpm->newPage()
        );
    }



    for(auto id:ids)
    {

        EXPECT_NE(
            bpm->fetchPage(id),
            nullptr
        );

    }

}





TEST_F(BPMTest, DeletePage)
{

    int page_id =
        bpm->newPage();


    EXPECT_NE(
        bpm->fetchPage(page_id),
        nullptr
    );


    bpm->deletePage(page_id);

    EXPECT_NE(
        bpm->fetchPage(page_id),
        nullptr
    );
}




TEST_F(BPMTest, DirtyPageFlushOnShutdown)
{


    int page_id =
        bpm->newPage();



    char* buffer =
        bpm->fetchPage(page_id);



    strcpy(buffer,"persistent data");



    bpm->markAsDirty(page_id);



    delete bpm;



    bpm =
        new BufferPoolManager(dm);



    char* loaded =
        bpm->fetchPage(page_id);



    EXPECT_STREQ(
        loaded,
        "persistent data"
    );

}



TEST_F(BPMTest, FetchInvalidPage)
{

    EXPECT_EQ(
        bpm->fetchPage(-1),
        nullptr
    );

}



TEST_F(BPMTest, UnpinPage)
{

    int id =
        bpm->newPage();


    bpm->unpinPage(id,false);


    SUCCEED();

}
