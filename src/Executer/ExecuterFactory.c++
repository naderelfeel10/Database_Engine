#include"Executer/ExecutorFactory.h"
#include<iostream>
using namespace std;


ExecutorFactory::ExecutorFactory(TransactionManager* txn_manager, Catalog* catalog, BindContext* context, WALManager* wal_manager):txn_manager(txn_manager),
    catalog(catalog), context(context){
        this->wal_manager = wal_manager;
        cout << "wal_manager = " << wal_manager << endl;
        cout << "wal_manager = " << this->wal_manager << endl;

}
//create the factory executer based on plan 
AbstractExecuter* ExecutorFactory::createExecutor(AbstractPlanNode* plan){
    
    switch(plan->type){

        //fist case if seq scan
        // i will prepare the table_heap then feed it as a prameter to it's operator
        case PlanType::SEQ_SCAN:{

            auto* scan_plan = static_cast<SeqScanPlan*>(plan);

            //resolve table name from context, then get table heap from catalog
            BoundTable bound_table = context->getBoundTable(scan_plan->table_oid);
            string table_name = bound_table.table_name;

            // get table_info, then get the table heap
            TableInfo* table_info = catalog->GetTable(table_name);
            TableHeap* table_heap = table_info->get_table_heap();
            
            //i need to pass the alias if exists
            if(bound_table.alias.size()>0)
                table_heap->setTableName(bound_table.alias);
            
            cout<<bound_table.alias<<" || "<<table_heap->getTableName()<<endl;

            assert(table_heap->get_output_schema().size() > 0);

            //check if null
            if(table_heap == nullptr){
                throw runtime_error("table heap not found");
            }
            
            // then return the Executer
            return new SeqScan(table_heap);
        }


        case PlanType::PROJECTION:{
                
            auto* projection_plan = static_cast<ProjectionPlan*>(plan);
            
            //the child is also a child executer, so i will keep calling it recursevly 
            AbstractExecuter* child = createExecutor(projection_plan->child);
                
            vector<string> projection_cols;
                
            //loop through all select items in the projection plan
            //projection only needs vector of col names, and the child executer
            for(const auto& item : projection_plan->expressions){
            
                //it might be col_ref, or agg function, so we have to consider both
                //col ref
                BoundColumnRef* column_ref = dynamic_cast<BoundColumnRef*>(item.expression);
                //if not null means it's col_ref
                if(column_ref != nullptr){
                    string col_name = column_ref->table_name+"."+column_ref->column_name;
                    projection_cols.push_back(col_name);
                    continue;
                }
            
                // agg function:
                BoundFunctionExpression* function = dynamic_cast<BoundFunctionExpression*>(item.expression);
                //if not null means it's a function
                if(function != nullptr){
                    BoundColumnRef* argument = dynamic_cast<BoundColumnRef*>(function->argument);
                
                    if(argument == nullptr){
                        throw runtime_error("agg argument is not a column");
                    }
                    //col_name like this AVG(salary)
                    string col_name = function->function_name + "(" +argument->column_name + ")";

                    cout<<col_name<<endl;
                
                    projection_cols.push_back(col_name);
                    continue;
                }
            
                throw runtime_error("unsupported projection expression");
            }

            for(auto&col_name:projection_cols)cout<<col_name<<" ";
        
            //now return the operator
            return new Projection(child, projection_cols);
        }


        //select case
        case PlanType::FILTER:{

            //select needs child executer and a predicate to filer on
            auto* filter_plan = static_cast<FilterPlan*>(plan);

            //keep calling recursevly
            AbstractExecuter* child = createExecutor(filter_plan->child);
            //then constrcut the predicate from bound 
            AbstractPredicate* predicate = build_predicate(filter_plan->predicate, child);

            return new Select(child, predicate);
        }

        case PlanType::JOIN:{
            //cast into join plan
            JoinPlan* join_plan = static_cast<JoinPlan*>(plan);
            //then create executers for left adn right childs
            AbstractExecuter* left_child = createExecutor(join_plan->getLeft());
            AbstractExecuter* right_child = createExecutor(join_plan->getRight());
            
            //then build the predicate
            AbstractPredicate* predicate = build_join_predicate(join_plan->getCondition(),left_child, right_child);


            return new NestedLoopJoin(left_child, right_child, predicate, INNER_JOIN);


        }
        //now order by, it's based on one col only
        case PlanType::SORT:{
            auto* order_plan = static_cast<OrderByPlan*>(plan);
                
            //call the child recursevily adn resolve sorting key
            AbstractExecuter* child = createExecutor(order_plan->getChild());
            Column* sort_key = expr_to_col(order_plan->getExpression(), child);
            cout<<sort_key->getColName()<<endl;
            
            //resolve sorting method
            sorting_methods method;
            
            if(order_plan->getOrderType() == OrderType::ASC){
                method = ASC;
            }
            else{
                method = DESC;
            }
        
            return new ExternalMergeSort(catalog->getBPM(), child, *sort_key, method);
        }
        //agg functions
        case PlanType::AGGREGATION:{
            //prepare child executer
            auto* group_by_plan = static_cast<GroupByPlan*>(plan);
            AbstractExecuter* child = createExecutor(group_by_plan->getChild());

            //sort_agg needs a vector of col indexes 
            //also needs a vector of agg functions which is (function_type, col_index)

            //loop through each item in gourping keys and get it's col_oid
            vector<int>grouping_cols;
            for(auto&item : group_by_plan->getGroupingKeys()){
                BoundColumnRef* col_ref = dynamic_cast<BoundColumnRef*>(item);
                grouping_cols.push_back(col_ref->column_oid);
            }
            
            vector<GroupingFunction>grouping_functions;

            for(auto& item:group_by_plan->getGroupingFunctions()){

                //prepare the function adn the col it'c col parameter 
                BoundFunctionExpression* func = dynamic_cast<BoundFunctionExpression*>(item);
                BoundColumnRef* col = dynamic_cast<BoundColumnRef*>(func->argument);
                
                //prepare grouing funcion
                GroupingFunction grouping_func;
                grouping_func.grouping_type = func->function_type;
                grouping_func.function_key = col->column_oid;

                cout<<grouping_func.grouping_type<<" | "<<grouping_func.function_key<<endl;

                grouping_functions.push_back(grouping_func);

            }
            //prepare having clause as a prediacte
            AbstractPredicate* having_predicate = build_predicate(group_by_plan->getHaving(), child);
            group_by_plan->getHaving()->PrintTree();
            
            return new SortAggregateExecuter(this->catalog->getBPM(),child, grouping_cols, grouping_functions,having_predicate);

        }
        //////////////////////////////////////////////////////  
        case PlanType::INSERT:{

            auto* insert_plan = static_cast<InsertPlan*>(plan);

            //resolve table name from context, then get table heap from catalog
            BoundTable* bound_table = insert_plan->bound_insert->table;
            string table_name = bound_table->table_name;

            bound_table->printTable();
            cout<<table_name<<bound_table->table_oid<<endl;
            
            // get table_info, then get the table heap
            TableInfo* table_info = catalog->GetTable(table_name);
            cout<<table_info->table_name<<endl;
            TableHeap* table_heap = table_info->get_table_heap();

            //then resolve the tuple:
            vector<Field>to_insert_cols;
            for (size_t i = 0;i < insert_plan->bound_insert->columns.size();i++){
                
                Column& column = insert_plan->bound_insert->columns[i];
                BoundExpression* expression = insert_plan->bound_insert->values[i];
                column.printCol();
                
                //if expr is null, then col is null
                if(expression==nullptr){
                    Field null_field = Field(TYPE_NULL);
                    to_insert_cols.push_back(null_field);
                }else{
                    expression->PrintTree();
                    
                    //else if it has a value, convert it into a                 
                    BoundConstantExpression* const_expr = dynamic_cast<BoundConstantExpression*>(expression);
                    Column* col = const_to_col(const_expr);
                    col->setColName(column.getColName());
                    
                    col->printCol();
                    to_insert_cols.push_back(*col->getField());
                }  

            }
            Tuple tuple = Tuple(to_insert_cols);
            tuple.print();
            cout<<"wall manager"<<wal_manager<<endl;
            cout<<"wall manager"<<this->wal_manager<<endl;
            return new InsertTuple(wal_manager, txn_manager, table_heap, tuple);
        }
        /*
            BoundTable* table;
            vector<Column> columns;
            vector<BoundExpression*> values;
            BoundExpression* where;
        */
        case PlanType::UPDATE:{

            UpdatePlan* update_plan  = static_cast<UpdatePlan*>(plan);

            //resolve table name from context, then get table heap from catalog
            BoundTable* bound_table = update_plan->bound_update->table;
            string table_name = bound_table->table_name;

            bound_table->printTable();
            cout<<table_name<<bound_table->table_oid<<endl;
            
            // get table_info, then get the table heap
            TableInfo* table_info = catalog->GetTable(table_name);
            cout<<table_info->table_name<<endl;
            TableHeap* table_heap = table_info->get_table_heap();

            //prepare the child executer form where we will fetch all tuples and check againest the predict 
            SeqScan* seq_scan = new SeqScan(table_heap);
            //prepare the predict (where condition)
            AbstractPredicate* predicate = build_predicate(update_plan->bound_update->where, seq_scan);

            //this select executer will check againest the predict and return only to_update tuples 
            Select* select_executer = new Select(seq_scan, predicate);

            //we need to prepare the tuple to update data with 
            //we have cols, and values 

            //fetch all tuples from select_executer
            //for every tuple to update, we have only some cols to update
            // so the original cols should remain the same non-changed
            Tuple dummy_tuple({});
            AbstractExecuter* update_tuple_executer = new UpdateTuple(wal_manager, txn_manager, table_heap);

            //for(auto&rid:seq_scan->get_table_rids())rid.print();

            while(select_executer->getNext(&dummy_tuple)){
                
                //dummy_tuple.print();
                //fetch the rid 
                RID curr_rid = select_executer->get_curr_rid();
                //curr_rid.print();
                
                //this index represent curr field in the actual tuple 
                int field_index{};

                vector<Field>new_col_fields;
                for (size_t i = 0;i < update_plan->bound_update->columns.size();i++){

                    Column& column = update_plan->bound_update->columns[i];
                    BoundExpression* expression = update_plan->bound_update->values[i];
                    //column.printCol();

                    //if expr is null, then col
                    if(expression==nullptr){
                        Field field = dummy_tuple.fields[field_index];
                        new_col_fields.push_back(field);
                    }else{
                        //expression->PrintTree();

                        //else if it has a value, convert it into a                 
                        BoundConstantExpression* const_expr = dynamic_cast<BoundConstantExpression*>(expression);
                        Column* col = const_to_col(const_expr);
                        col->setColName(column.getColName());

                        //col->printCol();
                        new_col_fields.push_back(*col->getField());

                    }
                    //move to next field in the tuple wheather it's null or not 
                    field_index++;
                }

                Tuple new_tuple = Tuple(new_col_fields);
                //new_tuple.print();

                //now we have the new_tuple, and the rid, just update it
                static_cast<UpdateTuple*>(update_tuple_executer)->update_tuple(curr_rid, new_tuple);
            }

            return update_tuple_executer;
            

        }

        case PlanType::DELETE:{
            auto* delete_plan = static_cast<DeletePlan*>(plan);
            
            //resolve table name from context, then get table heap from catalog
            BoundTable* bound_table = delete_plan->bound_delete->table;
            string table_name = bound_table->table_name;

            //bound_table->printTable();
            //cout<<table_name<<bound_table->table_oid<<endl;
            
            // get table_info, then get the table heap
            TableInfo* table_info = catalog->GetTable(table_name);
            cout<<table_info->table_name<<endl;
            TableHeap* table_heap = table_info->get_table_heap();

            //prepare the child executer form where we will fetch all tuples and check againest the predict 
            SeqScan* seq_scan = new SeqScan(table_heap);

            Select* select_executer = nullptr;
            //prepare the predict (where condition)
            if(delete_plan->bound_delete->where){
                AbstractPredicate* predicate = build_predicate(delete_plan->bound_delete->where, seq_scan);

                //this select executer will check againest the predict and return only to_update tuples 
                select_executer = new Select(seq_scan, predicate);
            }
            Tuple dummy_tuple({});
            AbstractExecuter* delete_tuple_executer = new DeleteTuple(wal_manager, txn_manager, table_heap);

            if(select_executer){
            
                while(select_executer->getNext(&dummy_tuple)){
                    //fetch the rid 
                    RID curr_rid = select_executer->get_curr_rid();
                    static_cast<DeleteTuple*>(delete_tuple_executer)->delete_tuple(curr_rid);
                }
            }else{
                while(seq_scan->getNext(&dummy_tuple)){
                    //fetch the rid 
                    RID curr_rid = select_executer->get_curr_rid();
                    static_cast<DeleteTuple*>(delete_tuple_executer)->delete_tuple(curr_rid);
                }
            }
            return delete_tuple_executer;

        }

        case PlanType::CREATE_TABLE:{
            CreateTablePlan* create_plan = static_cast<CreateTablePlan*>(plan);

            AbstractExecuter* create_table_executer = new CreateTable(catalog, *create_plan->getBoundCreateTable());
            cout<<static_cast<CreateTable*>(create_table_executer)->is_created()<<endl;

            //TODO : create a createTable executer and return it (done)
            //return just null for now
            return create_table_executer;
        }
        
        case PlanType::CREATE_INDEX:{

            CreateIndexPlan* create_index_plan = static_cast<CreateIndexPlan*>(plan);

            string table_name = create_index_plan->getBoundCreateIndex()->table_name;
            cout<<table_name<<endl;

            TableInfo* table_info = catalog->GetTable(table_name);
            cout<<table_info->table_name<<endl;
            TableHeap* table_heap = table_info->get_table_heap();

            //prepare args:

            string index_name = create_index_plan->getBoundCreateIndex()->index_name;
            string col_name = create_index_plan->getBoundCreateIndex()->columns[0].getColName();
            int index_size = 200;
            indexes_t index_type = indexes_t::STATIC_HASH_INDEX;


            AbstractExecuter* create_index_executer = new CreateIndex(this->txn_manager, this->wal_manager,
                                                         table_heap,index_type,col_name,index_size);

            Index* index = table_heap->getIndex(col_name, index_type);

            if(index != nullptr){
                int first_page_id = table_heap->indexes_pages_ids[pair(col_name, index_type)]->index_first_page_id;
                this->catalog->AddIndex(table_name, index_name, col_name, index_type, first_page_id);
            }else{
                throw runtime_error("error while creating the index");
            }

            return create_index_executer;

        }
        default:{
            throw runtime_error("invalid planType");
            return nullptr;
        }

    }
}


AbstractPredicate* ExecutorFactory::build_predicate(BoundExpression* expression, AbstractExecuter* child){
    //check if null
    if(expression == nullptr){
        return nullptr;
    }

    expression->PrintTree();
    // this has to be binary expression
    if(expression->exp_type != BoundExpressionType::BINARY){
        throw runtime_error("predicate has to be binary");
    }

    auto* binary=static_cast<BoundBinaryExpression*>(expression);
    //build the predicate based on it's type recursively
    switch (binary->op){
        //complex predicate 
        //and, or, not
        case BoundOperatorType::AND:{
            AbstractPredicate* left_child = build_predicate(binary->left,child );
            AbstractPredicate* right_child = build_predicate(binary->right,child );

            return new ComplexPredicate(left_child, right_child, ComplexPredicateType::AND);
        }

        case BoundOperatorType::OR:{
            AbstractPredicate* left_child = build_predicate(binary->left, child);
            AbstractPredicate* right_child = build_predicate(binary->right, child);

            return new ComplexPredicate(left_child, right_child, ComplexPredicateType::OR);
        }

        case BoundOperatorType::NOT:{
            AbstractPredicate* left_child = build_predicate(binary->left, child);

            return new ComplexPredicate(left_child, nullptr, ComplexPredicateType::NOT);
        }

        //now simple predicates like user_id = 1;

        case BoundOperatorType::EQ:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            Column* left_col = expr_to_col(binary->left, child);
            left_col->printCol();
            Column* right_col = expr_to_col(binary->right, child);
            right_col->printCol();

            return new Predicate(left_col, right_col, PredicateType::EQ);
        }
        case BoundOperatorType::NE:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            Column* left_col = expr_to_col(binary->left, child);
            Column* right_col = expr_to_col(binary->right, child);

            return new Predicate(left_col, right_col, PredicateType::NE);
        }

        case BoundOperatorType::GT:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            Column* left_col = expr_to_col(binary->left, child);
            left_col->printCol();
            
            Column* right_col = expr_to_col(binary->right, child);
            right_col->printCol();

            return new Predicate(left_col, right_col, PredicateType::GT);
        }

        case BoundOperatorType::GE:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            
            Column* left_col = expr_to_col(binary->left, child);
            left_col->printCol();

            Column* right_col = expr_to_col(binary->right, child);
            right_col->printCol();

            return new Predicate(left_col, right_col, PredicateType::GE);
        }

        case BoundOperatorType::LT:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            Column* left_col = expr_to_col(binary->left, child);
            Column* right_col = expr_to_col(binary->right, child);

            return new Predicate(left_col, right_col, PredicateType::LT);
        }

        case BoundOperatorType::LE:{
            //string col_name = dynamic_cast<BoundColumnRef*>(binary->left)->column_name;
            Column* left_col = expr_to_col(binary->left, child);
            Column* right_col = expr_to_col(binary->right, child);

            return new Predicate(left_col, right_col, PredicateType::LE);
        }

        default:
            throw runtime_error(
                "Unsupported predicate operator"
            );
    }
}


AbstractPredicate* ExecutorFactory::build_join_predicate(BoundExpression* expression,AbstractExecuter* left_child,AbstractExecuter* right_child){
    if (expression == nullptr) {
        return nullptr;
    }

    if (expression->exp_type != BoundExpressionType::BINARY) {
        throw runtime_error("Join predicate must be binary");
    }

    auto* binary =static_cast<BoundBinaryExpression*>(expression);

    switch(binary->op){

        case BoundOperatorType::EQ:{
            binary->PrintTree();
            Column* left_col = expr_to_join_col(binary->left, left_child, right_child);
            Column* right_col = expr_to_join_col(binary->right, left_child, right_child);

            return new Predicate(left_col, right_col, PredicateType::EQ);
        }

        case BoundOperatorType::NE:{

            Column* left_col =expr_to_join_col(binary->left, left_child, right_child);

            Column* right_col =expr_to_join_col(binary->right, left_child, right_child);

            return new Predicate(left_col, right_col, PredicateType::NE);
        }

        case BoundOperatorType::GT:{

            Column* left_col =expr_to_join_col(binary->left, left_child, right_child);
            Column* right_col = expr_to_join_col(binary->right, left_child, right_child);

            return new Predicate(left_col, right_col, PredicateType::GT);
        }

        default:
            throw runtime_error("unsupported join predicate");
    }
}


Column* ExecutorFactory::expr_to_join_col(BoundExpression* expr,AbstractExecuter* left_child,AbstractExecuter* right_child){
    
    if(expr == nullptr){
        throw runtime_error("Expression is null");
    }

    switch (expr->exp_type) {

        case BoundExpressionType::COLUMN_REF: {

            auto* col_ref = static_cast<BoundColumnRef*>(expr);

            AbstractExecuter* target_child = nullptr;

            string res_col_name = col_ref->table_name+'.'+col_ref->column_name;

            //for join, I have to determine if it's left or right col
            if(left_child->has_column(res_col_name)){
                target_child = left_child;
            }
            else if(right_child->has_column(res_col_name)){
                target_child = right_child;
            }

            //col not found
            else
            throw runtime_error("column is not found ....");
            
            vector<Column> schema = target_child->get_output_schema();

            for(auto&col:target_child->get_output_schema())col.printCol();

            if(col_ref->column_oid < 0 || col_ref->column_oid >= schema.size()){
                throw runtime_error("invlaid col oid");
            }
            Column* res_col = new Column(schema[col_ref->column_oid]);
            
            res_col->setColName(res_col_name);
            //return new Column(schema[col_ref->column_oid]);
            cout<<res_col->getColName()<<endl;
            return res_col;
        }

        //normal const handling 
        case BoundExpressionType::CONSTANT: {

            auto* constant = static_cast<BoundConstantExpression*>(expr);

            return const_to_col(constant);
        }

        default:
            throw runtime_error(
                "Unsupported join expression"
            );
    }
}


// expression can't just be passed to select operator
// i have to transform it from expression into col, then pass it into the operator
Column* ExecutorFactory::expr_to_col(BoundExpression* expr,AbstractExecuter* child){

    //based on expression type, choose how to handle it
    // might be col_ref, or const
    cout<<expr->exp_type<<endl;
    switch(expr->exp_type){
        //fist case if col_ref, ex: "user_id"
        case BoundExpressionType::COLUMN_REF:{

            BoundColumnRef* col_ref = static_cast<BoundColumnRef*>(expr);
            cout<<col_ref->column_name<<endl;

            if(child == nullptr){
                throw runtime_error("child executer iis null");
            }
             //fetch schema from the child
            vector<Column> schema = child->get_output_schema();
            for(auto&col : schema)col.printCol();
            
            //cout<<"schema size : "<<schema.size()<<endl;
            //cout<<"col ref : "<<col_ref->column_name<<", "<<col_ref->column_oid<<endl;
            
            //then select the needed col
            Column* result = new Column(schema[col_ref->column_oid]);
            result->printCol();

            return result;
        }

        // case 2 is const values, like : integer or string value
        case BoundExpressionType::CONSTANT:{

            BoundConstantExpression* constant = static_cast<BoundConstantExpression*>(expr);
            //convert this concrete value into actual col
            Column* result = const_to_col(constant);
            result->printCol();

            return result;
        }
        //handle functions and aggregations like COUNT,SUM
        case BoundExpressionType::FUNCTION: {

            BoundFunctionExpression* function = static_cast<BoundFunctionExpression*>(expr);

            cout<<function->function_name<<endl;
            
            string col_name;

            if (function->argument != nullptr){
                BoundColumnRef* col_ref = dynamic_cast<BoundColumnRef*>(function->argument);

                col_name = function->function_name+"("+col_ref->column_name+")";
            }

            Column* res_col = new Column(function->return_type, col_name, sizeof(function->return_type));
            res_col->printCol();
            
            return res_col;
        }
            
    

   


        default:
            throw runtime_error("invalid expression");
    }
}


//wrap const values into a col
Column* ExecutorFactory::const_to_col(BoundConstantExpression* expr){
        
    //convert based on return type
    //int, bool, float or string
    switch(expr->return_type){
        
        case FieldType::TYPE_INT:{
            //construct the field containing typen and value
            Field* f = new Field(TYPE_INT, static_cast<int>(expr->value.int_const));
            //then col has field, name and size
            Column* col = new Column(f,"_const_int_", sizeof(int));
            return col;
        }

        case FieldType::TYPE_FLOAT:{
            //construct the field containing typen and value
            Field* f = new Field(TYPE_FLOAT, static_cast<double>(expr->value.float_const));
            //then col has field, name and size
            Column* col = new Column(f,"_const_float_", sizeof(double));
            return col;
        }

        case FieldType::TYPE_BOOL:{
            //construct the field containing typen and value
            Field* f = new Field(TYPE_BOOL, static_cast<bool>(expr->value.bool_const));
            //then col has field, name and size
            Column* col = new Column(f,"_const_bool_", sizeof(bool));
            return col;
        }

        case FieldType::TYPE_STRING:{
            //construct the field containing typen and value
            Field* f = new Field(TYPE_STRING, static_cast<const char*>(expr->value.str_const.c_str()));
            //then col has field, name and size
            Column* col = new Column(f,"_const_str_", expr->value.str_const.size());
            return col;
        } 

        default:
            break;
        }
}

bool ExecutorFactory::execute_txn(const hsql::SQLStatement* stmt){

    Transaction* curr_txn = this->txn_manager->get_current_transaction();
    
    if(stmt->type() == hsql::kStmtTransaction){
        auto* txn_stmt = static_cast<const hsql::TransactionStatement*>(stmt);

        switch(txn_stmt->command){

            case hsql::kBeginTransaction:{
                cout<<"txn begining"<<endl;
                curr_txn = txn_manager->begin();
                int txn_id = curr_txn->GetTransactionId();

                WALRecord record{LogType::BEGIN, txn_id, -1, RID(-1, -1), Tuple({}), Tuple({})};
                this->wal_manager->add_record(record);
                return true;
            }

            case hsql::kCommitTransaction:{
                cout<<"txn commiting"<<endl;
                txn_manager->commit(curr_txn);
                int txn_id = curr_txn->GetTransactionId();

                WALRecord record{LogType::COMMIT, txn_id, -1, RID(-1, -1), Tuple({}), Tuple({})};
                this->wal_manager->add_record(record);
                return true;
            }

            case hsql::kRollbackTransaction:
                cout<<"txn rolling back"<<endl;
                txn_manager->abort(curr_txn);
                int txn_id = curr_txn->GetTransactionId();

                WALRecord record{LogType::ABORT, txn_id, -1, RID(-1, -1), Tuple({}), Tuple({})};
                this->wal_manager->add_record(record);
                return true;
        }

        return false;
    }
}

/// dummy insertions
void InsertIntoUserTable(TableHeap* user_table){
    for(int i = 0; i < 70; i++){

        Field u1(TYPE_INT, 100 + i);
        Field u2(TYPE_STRING,("First_" + to_string(i)).c_str());
        Field u3(TYPE_STRING,("Last_" + to_string(i)).c_str());
        Field u4(TYPE_INT, 20 + i);

        Tuple tuple({u1, u2, u3, u4});
        RID rid = user_table->insertTuple(tuple);

    }
}

vector<Column> CreateUserSchema(){
    return{
        Column(TYPE_INT, "user_id", sizeof(int)),
        Column(TYPE_STRING, "firstName", 30),
        Column(TYPE_STRING, "lastName", 30),
        Column(TYPE_INT, "age", sizeof(int))
    };
}
vector<Column> CreateOrdersSchema(){
    return {
        Column(TYPE_INT, "order_id", sizeof(int)),
        Column(TYPE_INT, "user_id", sizeof(int)),
        Column(TYPE_FLOAT, "totalAmount", sizeof(float)),
        Column(TYPE_BOOL, "isShipped", sizeof(bool))
    };
}

/// dummy order insertions
void InsertIntoOrdersTable(TableHeap* orders_table) {

    int order_id = 1;

    for (int user_id = 100; user_id < 170; user_id++) {

        // Some users have 1 order, some 2, some 3
        int num_orders = (user_id % 3) + 1;

        for (int j = 0; j < num_orders; j++) {

            Field o1(TYPE_INT, order_id);                        
            Field o2(TYPE_INT, user_id);                  

            float amount = 50.0f + (user_id - 100) * 10.5f + j * 25.0f;
            Field o3(TYPE_FLOAT, amount);                  

            bool shipped = ((order_id % 2) == 0);
            Field o4(TYPE_BOOL, shipped);              

            Tuple tuple({o1, o2, o3, o4});

            RID rid = orders_table->insertTuple(tuple);

            order_id++;
        }
    }
}
/*
int
main(){

    DiskManager* dm = new DiskManager("catalog.db");
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    Catalog* catalog = new Catalog(BPM, true);

    vector<Column>user_schema = CreateUserSchema();
    catalog->CreateTable("User", user_schema);

    TableHeap* user_table_heap = catalog->GetTable("User")->get_table_heap();

    vector<Column>order_schema = CreateOrdersSchema();
    catalog->CreateTable("Orders", order_schema);

    cout<<endl;
    for(auto&[table_name, table_info]: catalog->getTables()){
        cout<<table_name<<endl;
        cout<<table_info->table_name<<endl;
        for(auto&col: table_info->schema){
            col.printCol();
        }
        cout<<endl;
    }
    cout<<"--------------"<<endl;

    //catalog->save_catalog();

    //const std::string sql = "SELECT u.user_id, u.firstName from User as u "
    //                        "inner join Orders on u.user_id = Orders.user_id "
    //                        "WHERE u.user_id = 2 order by u.user_id limit 10 offset 5;";
                            

    const string sql = "select u.firstName from User u where u.user_id > 10;";

    hsql::SQLParserResult result;

    hsql::SQLParser::parse(sql, &result);

    BindContext* context = new BindContext();
    Binder* binder = new Binder(catalog, context);

    const hsql::SQLStatement* stmt = result.getStatement(0);
    unique_ptr<BoundStatement> bound_stmt =  binder->bind(stmt);

    cout<<context->tables.size()<<endl;

    for(auto&table:context->tables){
        table.printTable();
    }

    cout<<endl<<"================================================================================================="<<endl;
    bound_stmt->PrintTree();
    cout<<endl<<"================================================================================================="<<endl;
    
    //create the plan
    Planner* planner = new Planner();
    AbstractPlanNode* plan = planner->Plan(move(bound_stmt));
    
    //print plan tree
    plan->PrintTree();

    //executer factory
    ExecutorFactory factory(catalog, context);
    Projection* executor = dynamic_cast<Projection*>(factory.createExecutor(plan));

    //execution
    executor->open();

    cout<<"\n==========output==========\n";

    Tuple tuple({});

    while(executor->getNext(&tuple)){
        tuple.print();
    }

    executor->close();

    cout<<"=============================\n";
    


}
*/

//main rewrite

int main()
{
    DiskManager* dm = new DiskManager("catalog.db");

    BufferPoolManager* BPM = new BufferPoolManager(dm);

    Catalog* catalog = new Catalog(BPM, true);

    vector<Column> user_schema = CreateUserSchema();

    TableInfo* user_info =catalog->CreateTable("User", user_schema);

    if(user_info == nullptr){
        cerr<<"Failed to create User table"<<endl;
        return 0;
    }


    TableHeap* user_table = user_info->table_heap;

    InsertIntoUserTable(user_table);



    vector<Column> order_schema = CreateOrdersSchema();

    TableInfo* orders_info = catalog->CreateTable("Orders", order_schema);

    if(orders_info == nullptr){
        cerr<<"Failed to create Orders table"<<endl;
        return 0;
    }

    TableHeap* order_table = orders_info->table_heap;

    //InsertIntoOrdersTable(order_table);

    cout<<"\n==========CATALOG=========="<<endl;

    for (auto& [table_name, table_info] : catalog->getTables()) {

        cout<<"Table: "<<table_name << endl;
        cout<<"OID: "<<table_info->first_page_id<<endl;
        cout<<"Schema:\n";
        for(auto& col : table_info->schema){
            col.printCol();
        }
        cout << endl;
    }

    cout<<"=============================\n";

    /*

    //const std::string sql = "SELECT u.user_id, u.firstName from User as u "
    //                        "inner join Orders on u.user_id = Orders.user_id "
    //                        "WHERE u.user_id = 2 order by u.user_id limit 10 offset 5;";
                            

    //const string sql = "select u.firstName, Orders.isShipped from User u where u.user_id > 10 inner join Orders on u.user_id=Orders.user_id;";
    
    //string sql;
    cout<<"ELFEEL_DB> ";
    //getline(cin, sql);

    //const string sql = "SELECT u.user_id, Orders.user_id, u.firstName from User u inner join Orders on u.user_id = Orders.user_id where u.user_id > 120;";
    //const string sql = "SELECT u.user_id,u.age from User u order by u.age DESC;";
    //const string sql = "SELECT u.user_id, u.firstName, AVG(u.age), SUM(u.age) from User u group by u.user_id, u.firstName ;";
    //const string sql = "SELECT u.user_id, u.firstName, AVG(u.age), SUM(u.age)"
    //        "from User u group by u.user_id, u.firstName having SUM(u.age)>70 AND AVG(u.age)>=30.1;";


    // select u.user_id, u.firstName from User u where u.user_id>150;
    // insert into User (user_id, firstName) values(170, 'elfeel');


    //const string sql = "insert into User (user_id, firstName) values (3,\'nader\');";

    const string sql = "update User set user_id = 6, firstName='nader' where User.user_id > 150;";
    const string sql = "delete from User where User.user_id=700;";

    const string sql = "delete from User where User.user_id=700;";

    

    hsql::SQLParserResult result;

    hsql::SQLParser::parse(sql, &result);

    BindContext* context = new BindContext();
    Binder* binder = new Binder(catalog, context);

    const hsql::SQLStatement* stmt = result.getStatement(0);
    unique_ptr<BoundStatement> bound_stmt =  binder->bind(stmt);

    cout<<context->tables.size()<<endl;

    for(auto&table:context->tables){
        table.printTable();
    }

    cout<<endl<<"================================================================================================="<<endl;
    bound_stmt->PrintTree();
    cout<<endl<<"================================================================================================="<<endl;
    
    //create the plan
    Planner* planner = new Planner();
    AbstractPlanNode* plan = planner->Plan(move(bound_stmt));
    
    //print plan tree
    plan->PrintTree();

    //executer factory
    ExecutorFactory factory(catalog, context);
    

    InsertTuple* executor = dynamic_cast<InsertTuple*>(factory.createExecutor(plan));


    if(executor->is_inserted()){
        cout<<"1 row inserted successfully."<<endl;
        executor->get_tuple().print();
    }
    

    UpdateTuple* executor = dynamic_cast<UpdateTuple*>(factory.createExecutor(plan));

    //execution
    //executor->open();



    cout<<"\n==========output==========\n";



    Tuple tuple({});

    for(auto&col:executor->get_output_schema()){
        cout<<col.getColName()<<"   | ";
    }
    cout<<endl;

    while(executor->getNext(&tuple)){
        tuple.print();
    }

    executor->close();

    cout<<"=============================\n";

    */



string sql;
//string sql = "CREATE TABLE students (name TEXT, student_number INTEGER, city TEXT, grade DOUBLE);";

//string sql = "CREATE TABLE students (name TEXT, student_number INTEGER NOT NULL, city TEXT, grade DOUBLE PRIMARY KEY UNIQUE);";
//string sql = "insert into students (name, student_number, city, grade) values ('nader', 1, 'cairo', 3.12);";
BindContext* context = new BindContext();
TransactionManager* txn_manager = new TransactionManager();

const char* table_name = "/home/elfeel/Desktop/SWE/Database_Engine/src/Executer/wal.bin";
WALRecovery* recovery_manager = new WALRecovery(table_name);
//recovery_manager->~WALRecovery();
WALManager* wal_manager = new WALManager(catalog, recovery_manager,nullptr,table_name);
wal_manager->recover();
ExecutorFactory factory(txn_manager, catalog, context, wal_manager);
Transaction* curr_txn = txn_manager->get_current_transaction();
while (true) {

    catalog->printCatalog();
    cout << "\nELFEEL_DB> ";

    if (!getline(cin, sql))
        break;

    // Ignore empty input
    if (sql.empty())
        continue;

    // Optional: exit command
    if (sql == "exit" || sql == "quit")
        break;

    try {

        // ============================================================
        // 1. Parse
        // ============================================================

        hsql::SQLParserResult result;

        hsql::SQLParser::parse(sql, &result);

        if (result.size() == 0) {
            cout << "No SQL statement found." << endl;
            continue;
        }


        // ============================================================
        // 2. Bind
        // ============================================================




        Binder* binder = new Binder(catalog, context);

        const hsql::SQLStatement* stmt = result.getStatement(0);
        curr_txn = txn_manager->get_current_transaction();
        if(stmt->type() == hsql::kStmtTransaction){
            bool is_txn = factory.execute_txn(stmt);
            curr_txn = txn_manager->get_current_transaction();
            if(is_txn){
                continue;
            }
        }
        else if(curr_txn==nullptr){
            cout<<"curr txn is null"<<endl;
            if(stmt->type() == hsql::kStmtInsert || stmt->type() == hsql::kStmtUpdate || stmt->type() == hsql::kStmtDelete){

                txn_manager->begin();
                curr_txn = txn_manager->get_current_transaction();
                int txn_id = curr_txn->GetTransactionId();
                WALRecord record{LogType::BEGIN, txn_id, -1, RID(-1, -1), Tuple({}), Tuple({})};
                wal_manager->add_record(record);

            }

        }

        unique_ptr<BoundStatement> bound_stmt =
            binder->bind(stmt);


        // ============================================================
        // Debug: Binding information
        // ============================================================

        cout << "\nTables in Bind Context: "
             << context->tables.size()
             << endl;

        for (auto& table : context->tables) {
            table.printTable();
        }

        cout << "\n"
             << "================================================================================================="
             << endl;

        bound_stmt->PrintTree();

        cout << "\n"
             << "================================================================================================="
             << endl;


        // ============================================================
        // 3. Create Plan
        // ============================================================

        Planner* planner = new Planner();

        AbstractPlanNode* plan =
            planner->Plan(move(bound_stmt));


        // ============================================================
        // 4. Print Plan
        // ============================================================

        plan->PrintTree();


        // ============================================================
        // 5. Create Executor
        // ============================================================


        //AbstractExecuter* executor = factory.createExecutor(plan);


        // ============================================================
        // 6. Execute
        // ============================================================

        cout << "\n========== output ==========\n";

    //AbstractExecuter* executor = factory.createExecutor(plan);

        switch (plan->type){
        
            case PlanType::INSERT:{
                InsertTuple* insert_executor = dynamic_cast<InsertTuple*>(factory.createExecutor(plan));
            
                insert_executor->open();
            
                if (insert_executor->is_inserted()) {
                    cout << "1 row inserted successfully.\n";
                    insert_executor->get_tuple().print();
                }
            
                insert_executor->close();
                break;
            }
        
        
            case PlanType::UPDATE: {
                UpdateTuple* update_executor = dynamic_cast<UpdateTuple*>(factory.createExecutor(plan));

                break;
            }
            case PlanType::DELETE: {
                DeleteTuple* delete_executor = dynamic_cast<DeleteTuple*>(factory.createExecutor(plan));

                break;
            }

            case PlanType::CREATE_TABLE: {
                CreateTable* create_executor = dynamic_cast<CreateTable*>(factory.createExecutor(plan));
                catalog->printCatalog();
                break;
            }

            case PlanType::CREATE_INDEX: {
                CreateIndex* create_executor = dynamic_cast<CreateIndex*>(factory.createExecutor(plan));
                catalog->printCatalog();
                break;
            }
        case PlanType::SEQ_SCAN: {
        
            SeqScan* seq_scan_executor = dynamic_cast<SeqScan*>(factory.createExecutor(plan));

        
            seq_scan_executor->open();
        
            cout << "\n========== output ==========\n";
        
            for (auto& col : seq_scan_executor->get_output_schema()) {
                cout << col.getColName() << "   | ";
            }
        
            cout << '\n';
        
            Tuple tuple({});
        
            while (seq_scan_executor->getNext(&tuple)) {
                tuple.print();
            }
        
            seq_scan_executor->close();
        
            cout << "=============================\n";
        
            break;
        }


        case PlanType::PROJECTION: {
        
            Projection* projection_executor = dynamic_cast<Projection*>(factory.createExecutor(plan));

            projection_executor->open();
        
            cout << "\n========== output ==========\n";
        
            for (auto& col : projection_executor->get_output_schema()) {
                cout << col.getColName() << "   | ";
            }
        
            cout << '\n';
        
            Tuple tuple({});
        
            while (projection_executor->getNext(&tuple)) {
                tuple.print();
            }
        
            projection_executor->close();
        
            cout << "=============================\n";
        
            break;
        }


        case PlanType::FILTER: {
        
            Select* filter_executor = dynamic_cast<Select*>(factory.createExecutor(plan));
        
            filter_executor->open();
        
            cout << "\n========== output ==========\n";
        
            for (auto& col : filter_executor->get_output_schema()) {
                cout << col.getColName() << "   | ";
            }
        
            cout << '\n';
        
            Tuple tuple({});
        
            while (filter_executor->getNext(&tuple)) {
                tuple.print();
            }
        
            filter_executor->close();
        
            cout << "=============================\n";
        
            break;
        }
        
            default:
                cout << "Unsupported plan type.\n";
                break;
        }


        if(stmt->type() != hsql::kStmtTransaction){
                if(stmt->type() == hsql::kStmtInsert || stmt->type() == hsql::kStmtUpdate || stmt->type() == hsql::kStmtDelete){
            
                    curr_txn = txn_manager->get_current_transaction();
                    int txn_id = curr_txn->GetTransactionId();
                    txn_manager->commit(curr_txn);
                    WALRecord record{LogType::COMMIT,txn_id, -1, RID(-1, -1), Tuple({}), Tuple({})};
                    wal_manager->add_record(record);
                }
            
        }
    }

        catch (const exception& e) {

        cout << "\nExecution Error: "
             << e.what()
             << endl;
    }
}

}
