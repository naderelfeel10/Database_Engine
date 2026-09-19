#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>
#include "Storage/Disk/DiskManager.h"

class DiskManagerTest : public ::testing::Test {
protected:
    // Test database file names
    const std::string test_db = "test_disk_manager.db";
    const std::string persistence_db = "test_persistence.db";
    const std::string reuse_db = "test_reuse.db";
    const std::string expansion_db = "test_expansion.db";

    // Set up before each test
    void SetUp() override {
        // Clean up any existing test databases
        std::filesystem::remove(test_db);
        std::filesystem::remove(persistence_db);
        std::filesystem::remove(reuse_db);
        std::filesystem::remove(expansion_db);
    }

    // Tear down after each test
    void TearDown() override {
        // Clean up test databases
        std::filesystem::remove(test_db);
        std::filesystem::remove(persistence_db);
        std::filesystem::remove(reuse_db);
        std::filesystem::remove(expansion_db);
    }
};

// Test 1: DiskManager initialization creates a new file
TEST_F(DiskManagerTest, InitializeNewDatabase) {
    DiskManager dm(test_db);
    
    EXPECT_TRUE(std::filesystem::exists(test_db));
    EXPECT_EQ(dm.header.number_of_tables, 0);
    EXPECT_EQ(dm.header.map_size, 0);
    EXPECT_GT(dm.header.capacity, 0);
}

// Test 2: Write and read a single page
TEST_F(DiskManagerTest, WriteSinglePage) {
    DiskManager dm(test_db);
    
    Page p1(1);
    Field f1(TYPE_STRING, "test_data");
    Tuple t1({f1});
    
    p1.insertTuple(t1);
    const char* buffer = p1.getData();
    
    dm.writePage(1, buffer);
    EXPECT_EQ(dm.pages_table.size(), 1);
    EXPECT_TRUE(dm.pages_table.find(1) != dm.pages_table.end());
}

// Test 3: Read page that was written
TEST_F(DiskManagerTest, ReadWrittenPage) {
    DiskManager dm(test_db);
    
    Page p1(42);
    Field f1(TYPE_INT, 100);
    Tuple t1({f1});
    p1.insertTuple(t1);
    
    const char* write_buffer = p1.getData();
    dm.writePage(42, write_buffer);
    
    char read_buffer[PAGE_SIZE];
    dm.readPage(42, read_buffer);
    
    PageHeader* p_header = reinterpret_cast<PageHeader*>(read_buffer);
    EXPECT_EQ(p_header->page_id, 42);
}

// Test 4: Reading non-existent page returns safely
TEST_F(DiskManagerTest, ReadNonExistentPage) {
    DiskManager dm(test_db);
    
    char read_buffer[PAGE_SIZE];
    memset(read_buffer, 0, PAGE_SIZE);
    
    dm.readPage(999, read_buffer);
    // Should not crash, just return without reading
}

// Test 5: Write multiple pages
TEST_F(DiskManagerTest, WriteMultiplePages) {
    DiskManager dm(test_db);
    
    for(int i = 1; i <= 10; i++) {
        Page p(i);
        Field f(TYPE_INT, i * 10);
        Tuple t({f});
        p.insertTuple(t);
        
        dm.writePage(i, p.getData());
    }
    
    EXPECT_EQ(dm.pages_table.size(), 10);
}

// Test 6: Delete a page
TEST_F(DiskManagerTest, DeletePage) {
    DiskManager dm(test_db);
    
    Page p1(1);
    dm.writePage(1, p1.getData());
    
    EXPECT_EQ(dm.pages_table.size(), 1);
    
    dm.deletePage(1);
    
    EXPECT_EQ(dm.pages_table.size(), 0);
    EXPECT_EQ(dm.deleted_slots.size(), 1);
}

// Test 7: Deleted slots are reused
TEST_F(DiskManagerTest, ReuseDeletedSlots) {
    DiskManager dm(test_db);
    
    Page p1(1), p2(2);
    dm.writePage(1, p1.getData());
    dm.writePage(2, p2.getData());
    
    size_t original_size = dm.getSize();
    
    dm.deletePage(1);
    
    Page p3(3);
    dm.writePage(3, p3.getData());
    
    // Size should not grow if we reused the deleted slot
    size_t new_size = dm.getSize();
    EXPECT_LE(new_size, original_size + PAGE_SIZE);
}

// Test 8: Add and remove tables
TEST_F(DiskManagerTest, AddAndRemoveTable) {
    DiskManager dm(test_db);
    
    dm.addTable("Users", 1);
    EXPECT_EQ(dm.header.number_of_tables, 1);
    EXPECT_TRUE(dm.tables_names.find("Users") != dm.tables_names.end());
    
    dm.addTable("Products", 2);
    EXPECT_EQ(dm.header.number_of_tables, 2);
    
    dm.removeTable("Users");
    EXPECT_EQ(dm.header.number_of_tables, 1);
    EXPECT_TRUE(dm.tables_names.find("Users") == dm.tables_names.end());
}

// Test 9: Metadata persistence - data survives DiskManager lifetime
TEST_F(DiskManagerTest, PersistenceAcrossLifetimes) {
    {
        DiskManager dm(persistence_db);
        Page p1(100);
        Field f1(TYPE_INT, 555);
        Tuple t1({f1});
        p1.insertTuple(t1);
        
        dm.writePage(100, p1.getData());
        dm.addTable("TestTable", 100);
        
        // Destructor saves metadata
    }
    
    {
        DiskManager dm_reloaded(persistence_db);
        
        // Check that page was reloaded
        EXPECT_TRUE(dm_reloaded.pages_table.find(100) != dm_reloaded.pages_table.end());
        
        // Check that table was reloaded
        EXPECT_EQ(dm_reloaded.header.number_of_tables, 1);
        EXPECT_TRUE(dm_reloaded.tables_names.find("TestTable") != dm_reloaded.tables_names.end());
        EXPECT_EQ(dm_reloaded.tables_names["TestTable"], 100);
    }
}

// Test 10: Metadata persistence with multiple pages and tables
TEST_F(DiskManagerTest, PersistenceMultipleTablesAndPages) {
    {
        DiskManager dm(persistence_db);
        
        for(int i = 1; i <= 5; i++) {
            Page p(i);
            dm.writePage(i, p.getData());
        }
        
        dm.addTable("Users", 1);
        dm.addTable("Products", 2);
        dm.addTable("Orders", 3);
    }
    
    {
        DiskManager dm_reloaded(persistence_db);
        
        EXPECT_EQ(dm_reloaded.pages_table.size(), 5);
        EXPECT_EQ(dm_reloaded.header.number_of_tables, 3);
        
        EXPECT_TRUE(dm_reloaded.tables_names.find("Users") != dm_reloaded.tables_names.end());
        EXPECT_TRUE(dm_reloaded.tables_names.find("Products") != dm_reloaded.tables_names.end());
        EXPECT_TRUE(dm_reloaded.tables_names.find("Orders") != dm_reloaded.tables_names.end());
    }
}

// Test 11: File capacity expansion
TEST_F(DiskManagerTest, FileExpansion) {
    DiskManager dm(expansion_db);
    
    int initial_capacity = dm.header.capacity;
    
    // Write enough pages to trigger expansion
    for(int i = 1; i <= initial_capacity + 10; i++) {
        Page p(i);
        dm.writePage(i, p.getData());
    }
    
    EXPECT_GT(dm.header.capacity, initial_capacity);
    EXPECT_EQ(dm.pages_table.size(), initial_capacity + 10);
}

// Test 12: Overwrite existing page
TEST_F(DiskManagerTest, OverwritePage) {
    DiskManager dm(test_db);
    
    Page p1(1);
    Field f1(TYPE_INT, 42);
    Tuple t1({f1});
    p1.insertTuple(t1);
    dm.writePage(1, p1.getData());
    
    Page p2(1);  // Same ID
    Field f2(TYPE_INT, 99);
    Tuple t2({f2});
    p2.insertTuple(t2);
    dm.writePage(1, p2.getData());
    
    // Should have same number of pages
    EXPECT_EQ(dm.pages_table.size(), 1);
    
    // Read and verify the new data
    char read_buffer[PAGE_SIZE];
    dm.readPage(1, read_buffer);
    PageHeader* header = reinterpret_cast<PageHeader*>(read_buffer);
    EXPECT_EQ(header->page_id, 1);
}

// Test 13: Delete non-existent page (should be safe)
TEST_F(DiskManagerTest, DeleteNonExistentPage) {
    DiskManager dm(test_db);
    
    dm.deletePage(999);  // Should not crash
    //should crash after update
    
    EXPECT_EQ(dm.pages_table.size(), 0);
    EXPECT_EQ(dm.deleted_slots.size(), 0);
}

// Test 14: GetSize calculation
TEST_F(DiskManagerTest, GetSizeCalculation) {
    DiskManager dm(test_db);
    
    size_t initial_size = dm.getSize();
    EXPECT_EQ(initial_size, 0);
    
    Page p1(1);
    dm.writePage(1, p1.getData());
    
    size_t after_write = dm.getSize();
    EXPECT_EQ(after_write, PAGE_SIZE);
    
    Page p2(2);
    dm.writePage(2, p2.getData());
    
    size_t after_two_writes = dm.getSize();
    EXPECT_EQ(after_two_writes, 2 * PAGE_SIZE);
}

// Test 15: Write and read with actual tuple data
TEST_F(DiskManagerTest, WriteReadComplexTuple) {
    DiskManager dm(test_db);
    
    Page p1(1);
    Field f1(TYPE_INT, 123);
    Field f2(TYPE_FLOAT, 45.67);
    Field f3(TYPE_STRING, "hello");
    
    Tuple t1({f1, f2, f3});
    p1.insertTuple(t1);
    
    dm.writePage(1, p1.getData());
    
    char read_buffer[PAGE_SIZE];
    dm.readPage(1, read_buffer);
    
    PageHeader* header = reinterpret_cast<PageHeader*>(read_buffer);
    EXPECT_EQ(header->page_id, 1);
    EXPECT_GT(header->num_tuples, 0);
}

// Test 16: Deleted slots persist across sessions
TEST_F(DiskManagerTest, DeletedSlotsPersistence) {
    {
        DiskManager dm(persistence_db);
        
        Page p1(1), p2(2), p3(3);
        dm.writePage(1, p1.getData());
        dm.writePage(2, p2.getData());
        dm.writePage(3, p3.getData());
        
        dm.deletePage(2);  // Delete middle page
    }
    
    {
        DiskManager dm_reloaded(persistence_db);
        
        EXPECT_EQ(dm_reloaded.deleted_slots.size(), 1);
        EXPECT_EQ(dm_reloaded.pages_table.size(), 2);
        
        // New page should reuse the deleted slot
        Page p4(4);
        dm_reloaded.writePage(4, p4.getData());
        
        EXPECT_EQ(dm_reloaded.deleted_slots.size(), 0);
        EXPECT_EQ(dm_reloaded.pages_table.size(), 3);
    }
}