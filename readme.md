<p align="center">
  <img src="assets/logo.svg" width="600">
</p>

# Database Engine

A production-oriented, disk-based relational database engine built from scratch in modern C++. The project implements the core architecture of a real-world DBMS, including SQL parsing, semantic analysis, query planning, Volcano-style execution, storage management, indexing, transaction processing, and crash recovery.

The engine provides a complete pipeline from SQL statements to persistent data storage, with support for catalog management, buffer pool management, slotted-page storage, hash and B+ tree indexing, multiple join algorithms, aggregation, sorting, transaction lifecycle management, and Write-Ahead Logging (WAL) with redo/undo recovery.

The goal of this project is to explore and implement the internal mechanisms behind database systems such as PostgreSQL and MySQL, focusing on how modern relational database engines process queries, manage memory and disk, maintain consistency, and recover from failures.

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────┐
│          SQL Query String                           │
└────────────────────┬────────────────────────────────┘
                     │
          ┌──────────▼──────────┐
          │    Parser (AST)     │
          └──────────┬──────────┘
                     │
          ┌──────────▼──────────┐
          │  Binder (Semantic)  │  ← Catalog Lookup
          └──────────┬──────────┘
                     │
          ┌──────────▼──────────┐
          │   Planner (Logic)   │
          └──────────┬──────────┘
                     │
          ┌──────────▼──────────────────┐
          │ ExecutorFactory + Txn ID    │  ← Transaction Manager
          └──────────┬──────────────────┘
                     │
          ┌──────────▼──────────────────┐
          │  Query Execution            │  ← Volcano Model
          │  Log Operations to WAL       │  ← Write-Ahead Log
          └──────────┬──────────────────┘
                     │
          ┌──────────▼──────────────────┐
          │  Commit/Abort + Recovery    │  ← Transaction Manager
          └──────────┬──────────────────┘
                     │
          ┌──────────▼──────────────────┐
          │   Storage Layer             │  ← Buffer Pool, Indexing, Disk I/O
          └─────────────────────────────┘
                     │
          ┌──────────▼──────────────────┐
          │  Result Tuples / Status     │
          └─────────────────────────────┘
```

---

## ✨ Features

### SQL Support
- **SELECT** with column projection
- **WHERE** clauses with complex predicates (AND, OR, comparison operators)
- **JOIN** (INNER, LEFT, RIGHT) with multiple join algorithms
- **GROUP BY** with multiple columns
- **HAVING** clauses for aggregate filtering
- **ORDER BY** (ASC / DESC) with external merge sort
- **INSERT INTO**: insert row into a table 
- **UPDATE**: update a row or more in a table   
- **DELETE FROM**: delete a row or more from a table   
- **CREATE INDEX** builds an index on the col (B+ Tree, Hash)
- **DROP INDEX** drops the index build on a col
- **CREATE TABLE**: create table with schema and constraints 
- **DROP TABLE**: drop table from the db 
- **CREATE DATABASE**: create new db file on HD
- **DROP DATABASE**: drop/remove the whole db
- **CONNECT DATABASE**: connect to one of systems db


### Query Execution
- **Volcano-style iterator model** — composable, streaming operators
- **Multiple join algorithms**: Nested Loop, Indexed NLJ, Hash Join, Merge Join
- **Hash & Sort-based aggregation**
- **External merge sort** for datasets larger than memory
- **Index-accelerated lookups** — Static Hash (O(1)) + B+ Tree (O(log n))

### Transaction Support & ACID Guarantees
- **Transaction Manager** — BEGIN, COMMIT, ABORT lifecycle management
- **Write-Ahead Logging (WAL)** — durability via persistent operation log
- **Crash Recovery** — redo committed transactions, undo uncommitted ones
- **Serializable Isolation Level** — transactions execute sequentially
- **Full ACID Compliance** — all changes durable before COMMIT returns

### Storage & Persistence
- **Disk-backed persistence** — 4KB slotted pages
- **Buffer Pool Manager** with LRU eviction
- **Full crash recovery** — tables and indexes rebuilt from disk on restart
- **Hash and B+ Tree indexing** with auto-persistence
- **Write-Ahead Log recovery** — automatic redo/undo on system restart

---

## 🔧 Core Components

### Compilation Pipeline

| Component | Purpose | 
|-----------|---------|
| **Parser** | AST generation from SQL strings |
| **Binder** | Semantic analysis & type checking |
| **Catalog** | Schema metadata persistence |
| **Planner** | Logical query plan generation |
| **ExecutorFactory** | Physical operator instantiation |

### Transaction & Recovery

| Component | Purpose | 
|-----------|---------|
| **Transaction Manager** | BEGIN, COMMIT, ABORT lifecycle |
| **Transaction** | Tracks txn state, isolation level, write set |
| **Write-Ahead Log (WAL)** | Logs all operations before execution |
| **WAL Recovery** | Redo/Undo on system restart |

### Execution Engine

| Component | Purpose |
|-----------|---------|
| **Sequential Scan** | Full table iteration |
| **Filter** | WHERE clause evaluation |
| **Projection** | SELECT column filtering |
| **Nested Loop Join** | Pairwise row matching |
| **Indexed Nested Loop Join** | Index-assisted joins |
| **Hash Join** | Hash table–based joins |
| **Merge Join** | Pre-sorted join execution |
| **Hash Aggregation** | GROUP BY (in-memory) |
| **Sort Aggregation** | GROUP BY (external sort) |
| **External Merge Sort** | ORDER BY (disk-resident data) |

### Storage Layer

| Component | Purpose |
|-----------|---------|
| **Disk Manager** | Page I/O and persistence |
| **Buffer Pool Manager** | LRU in-memory cache |
| **Table Heap** | Row storage & CRUD |
| **Static Hash Index** | O(1) lookups |
| **B+ Tree Index** | O(log n) lookups + range scans |

---

## 📐 Project Structure

```
Database_Engine/
|
├── Include(header files)/
|
├── parser(external-component)/
|
└── src/
    |
    ├── Binder/
    │   ├── BindContext.c++
    │   ├── binder.c++
    │   ├── BoundCreateIndexStatement.c++
    │   ├── BoundCreateTableStatement.c++
    │   ├── BoundDeleteStatement.c++
    │   ├── BoundDropTableStatement.c++
    │   ├── BoundExpression.c++
    │   ├── BoundInsertStatement.c++
    │   ├── BoundSelectStatement.c++
    │   ├── BoundUpdateStatement.c++
    │   └── Expression.c++
    |
    ├── Buffer/
    │   ├── BufferPoolManager.c++
    │   └── LRU_replacement.c++
    |
    ├── Catalog/
    │   └── Catalog.c++
    |
    ├── Executer/
    │   ├── ExecuterFactory.c++
    |
    ├── Q_Execution/
    │   ├── ComplexPredicate.c++
    │   ├── create_index_operator.c++
    │   ├── create_table_executer.c++
    │   ├── delete_statement_executer.c++
    │   ├── drop_table_executer.c++
    │   ├── ExternalMergeSortExecuter.c++
    │   ├── hash_join.c++
    │   ├── HashAggregateExecuter.c++
    │   ├── IndexedNested_loop_join.c++
    │   ├── insert_statement_executer.c++
    │   ├── MergeJoinExecuter.c++
    │   ├── Nested_loop_join.c++
    │   ├── Predicate.c++
    │   ├── Projection_operator.c++
    │   ├── select_operator.c++
    │   ├── seq_scan_operator.c++
    │   ├── SortAggregateExecuter.c++
    │   └── update_statement_executer.c++
    |
    ├── QueryPlan/
    │   ├── Planner.c++
    │   └── Plans.c++
    |
    ├── Recovery/
    │   ├── WAL_manager.c++
    │   ├── WAL_record.c++
    │   └── WAL_recovery.c++
    |
    ├── Storage/
    │   ├── Disk/
    │   │   └── DiskManager.c++
    |   |
    │   ├── Indexing/
    │   │   ├── BPlusTreeIndex.c++
    │   │   ├── BPlusTreeIndexWrapper.c++
    │   │   ├── static_hash_index.c++
    │   │   └── StaticHashIndexWrapper.c++
    |   |
    │   ├── Page/
    │   │   ├── catalog.db
    │   │   ├── Field.c++
    │   │   ├── page.c++
    │   │   └── Tuple.c++
    |   |
    │   └── Table/
    │       ├── Column.c++
    │       ├── RID.c++
    │       ├── TableHeap.c++
    │       └── TableIterator.c++
    |
    ├── TransactionManager/
    │   ├── Transaction_manager.c++
    │   └── Transaction.c++
    |
    └── test/
        ├── test_loading_DB.c++
        ├── test_multiple_tables.c++
        ├── test_table_load_store.c++
        └── testTable_iterator.c++

```

---

## 🔄 Transaction Lifecycle & Recovery

### Transaction States
```
BEGIN
  ↓
RUNNING  (Operations logged to WAL, not yet durable)
  ↓
COMMIT / ABORT
  ↓
COMMITTED / ABORTED (Written to WAL, now durable)
```

### Write-Ahead Logging (WAL)

Every transaction operation is logged **before** execution:

```
[BEGIN, txn_id=1]
  [INSERT, txn_id=1, table_id=1, rid=(0,0), tuple=(1, 'Alice', 25)]
  [UPDATE, txn_id=1, table_id=1, rid=(0,1), old=(2, 'Bob', 30), new=(2, 'Bob', 31)]
[COMMIT, txn_id=1]  ← Durability point
```

### Crash Recovery

**On system restart:**

```
1. Read all WAL records
   ├─ COMMITTED transactions: Redo all operations
   └─ UNCOMMITTED transactions: Undo all operations

2. Restore database to consistent state

3. Resume normal execution
```

**Example:**
- Transaction 1 (COMMITTED): Changes replayed ✅
- Transaction 2 (UNCOMMITTED): Changes rolled back ✅
- Database restored to last consistent state ✅

---

## 📊 Performance

### Join Algorithm Benchmarks

**Test Setup:**
- Users: 10,000 rows
- Orders: 1,000,000 rows  
- Join condition: `user_id` (high skew)

| Algorithm | Execution Time | Overhead |
|-----------|--------|----------|
| Hash Join | **22 sec** | Minimal |
| Indexed NLJ | 46 sec | Index build overhead |
| Sort-Merge | 78 sec | Sorting phases |
| Nested Loop | ⚠️ Infeasible | Too many comparisons |

[Link to Detailed Benchmarks]

---

## 🏛️ Design Principles

### 1. **Separation of Concerns**
- **Frontend** (Parser) ← Syntactic analysis
- **Middle-end** (Binder, Planner) ← Semantic analysis & optimization
- **Backend** (Executor, Storage, Recovery) ← Physical execution & durability

### 2. **Volcano Iterator Model**
Every operator exposes `open() → getNext() → close()`, enabling:
- Streaming data flow (no materialization)
- Composable operator chains
- Pipeline parallelism (future)

### 3. **Write-Ahead Logging (WAL)**
All changes logged **before** execution ensures:
- **Durability**: Committed changes survive crashes
- **Atomicity**: Partial failures roll back cleanly
- **Consistency**: Database always in valid state

### 4. **Disk-First Design**
- All data structures persist to disk
- Buffer pool minimizes I/O via LRU
- Crash recovery is automatic

### 5. **Production Patterns**
Mirrors real DBMS internals:
- PostgreSQL's transaction manager & WAL recovery
- MySQL's redo/undo log architecture
- SQLite's rollback journal approach

---

## 🛠️ Tech Stack

- **Language:** C++17
- **Build System:** CMake
- **External Dependencies:** SQL Parser (forked submodule)
- **Testing:** Custom integration tests

---

## 🔮 Future Roadmap

### Phase 2: Query Optimization
- [ ] Cost-based query planner (Selinger algorithm)
- [ ] Cardinality estimation & statistics
- [ ] Predicate pushdown & join reordering

### Phase 3: Concurrency & Isolation
- [ ] Read Committed & Snapshot isolation levels
- [ ] MVCC (Multi-Version Concurrency Control)
- [ ] Lock manager with deadlock detection
- [ ] Concurrent transaction execution

### Phase 4: Advanced Features
- [ ] Window functions (OVER clauses)
- [ ] Correlated subqueries
- [ ] Common Table Expressions (CTEs)
- [ ] Stored procedures

### Phase 5: Scalability
- [ ] Parallel query execution
- [ ] Distributed query processing
- [ ] Network-based replication

---

<div align="center">

**Built with ☕ and 💻 from Egypt**

</div>