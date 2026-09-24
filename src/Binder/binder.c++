#include"Binder/binder.h"

Binder::Binder(Catalog* catalog, BindContext* context){
    this->catalog = catalog;
    this->context = context;
}

unique_ptr<BoundStatement> Binder::bind(const hsql::SQLStatement* statement) {
    
    cout<<"stmt type : " <<statement->type()<<endl;
    switch (statement->type()) {
        case hsql::kStmtSelect:{

            auto* select_statement = static_cast<const hsql::SelectStatement*>(statement);

            return unique_ptr<BoundStatement>(BindSelect(select_statement));
        }
        
        case hsql::kStmtInsert:{
            auto* insert_statement = static_cast<const hsql::InsertStatement*>(statement);

            return unique_ptr<BoundStatement>(BindInsert(insert_statement));
        
        }
        
        case hsql::kStmtUpdate:{
            auto* update_statement = static_cast<const hsql::UpdateStatement*>(statement);

            return unique_ptr<BoundStatement>(BindUpdate(update_statement));

        }
        case hsql::kStmtDelete:{
            auto* delete_statement = static_cast<const hsql::DeleteStatement*>(statement);
            return unique_ptr<BoundStatement>(BindDelete(delete_statement));

        }
        /*case hsql::kStmtCreate:{
            auto* create_statement = static_cast<const hsql::CreateStatement*>(statement);
            return unique_ptr<BoundStatement>(bindCreateTable(create_statement));

        }*/
        case hsql::kStmtCreate:{
            auto* create_statement = static_cast<const hsql::CreateStatement*>(statement);
            return unique_ptr<BoundStatement>(bindCreate(create_statement));
        }
        /*
        default:
            throw BinderException(
                "Unsupported statement");

        */

        default: throw runtime_error("Unsupported statement type");
    }
}



/*
    in a select statement we expect alot of diff options :

    1. select items like [select name, age, salary]
    2."from table", where we use to fetch tuples
    3.might be join operations
    4. where clause and it's predicate
    5. might have aggregations and group by
    6.might have order by
    7. limit and offset
    // still more options to do later 

*/

BoundSelectStatement* Binder::BindSelect(const hsql::SelectStatement* statement){

    //check if null statement
    if(statement ==nullptr)
        throw runtime_error("cannot bind null SELECT statement");
    

    // main components of this statement :
    // boundSelectStatement is the output I need to return 
    auto* bound = new BoundSelectStatement();

    //FROM
    BindFrom(statement, *bound);

    //JOIN
    if(statement->fromTable->join != nullptr){
        BoundJoinClause bind_join = BindJoin(statement->fromTable->join);
        bound->joins.push_back(bind_join);
    }


    //select list :

    //loop through each expr in select list, resolve it's expression, then add it to bound list
    for(auto&expr : *statement->selectList){
        BoundSelectItem item;
        //resolve the expression binder
        item.expression = BindExpression(expr);
        
        item.alias = expr->alias?expr->alias :"";
        bound->select_list.push_back(item);
    }


    //WHERE, where is just an expression 
    if(statement->whereClause != nullptr)
        bound->where = BindExpression(statement->whereClause);

    //GROUP-BY
    if(statement->groupBy!= nullptr){
        for(auto& expression : *statement->groupBy->columns){
            bound->group_by.push_back(BindExpression(expression));
        }
    }
    //TODO
    //check if projected cols are the same cols in group by
    //check that where clause appears before group by
    //check that having valuse appears after group by

    //HAVING
    if(statement->groupBy && statement->groupBy->having != nullptr){
        bound->having = BindExpression(statement->groupBy->having);
    }

    // ORDER BY
    BindOrderBy(statement, *bound);

    // LIMIT adn OFSSET
    BindLimitOffset(statement, *bound);

    bound->PrintTree();
    return bound;
}

BoundExpression* Binder::BindExpression(hsql::Expr* expression){

    if (expression == nullptr) {
        return nullptr;
    }
    cout<<"expr type : "<<expression->type<<endl;
    switch (expression->type) {
        case hsql::kExprColumnRef:
            return BindColumnRef(expression);

        case hsql::kExprLiteralInt:
            return BindIntegerLiteral(expression);

        case hsql::kExprLiteralFloat:
            return BindFloatLiteral(expression);

        case hsql::kExprLiteralString:
            return BindStringLiteral(expression);

        case hsql::kExprOperator:
            return BindOperator(expression);

        case hsql::kExprFunctionRef:
            return BindFunction(expression);

        default:

            throw std::runtime_error(
                "Unsupported expression"
            );
    }
}


BoundExpression* Binder::BindColumnRef(const hsql::Expr* expression){
    //chech if null expression
    if(expression == nullptr)
        throw runtime_error("cannot bind null column expression");
    
    //check if type is col_ref
    if(!expression->isType(hsql::kExprColumnRef)){
        throw runtime_error("expression is not a column reference");
    }

    //extract column name from parser AST
    string column_name = expression->getName();
    cout<< "col name : "<< column_name<<endl;

    if (column_name.empty()) {
        throw runtime_error("column name is empty");
    }
    //get table_name
    string table_name = expression->table? expression->table : "NULL";
    cout<< "table name : "<< table_name<<endl;

    //resolve col_ref from the context of the binder
    BoundColumnRef* col_ref;
    col_ref = context->ResolveColumn(table_name, column_name);

    col_ref->PrintTree("|", true);

    return col_ref;

}

//bind constants integer, float, and strings
//find integer 
BoundExpression* Binder::BindIntegerLiteral(hsql::Expr* expression) {
    if(expression == nullptr){
        return nullptr;
    }
    //if not null, then fetch ival from the expression
    cout<<expression->ival<<endl;
    return new BoundConstantExpression(expression->ival);
}

BoundExpression* Binder::BindFloatLiteral(hsql::Expr* expression) {
    if(expression == nullptr){
        return nullptr;
    }
    cout<<expression->fval<<endl;
    return new BoundConstantExpression(expression->fval);
}

BoundExpression* Binder::BindStringLiteral(hsql::Expr* expression){
    //check if null first 
    if(expression == nullptr)return nullptr;
    
    if(expression->name == nullptr)return nullptr;
    
    string name = string(expression->name);
    cout<<name<<endl;

    return new BoundConstantExpression(name);
}

// a helper function to convert from ast operator to my defined operators
BoundOperatorType Binder::BindBinaryOperator(hsql::OperatorType op){

    switch (op) {
        case hsql::kOpEquals:
            return BoundOperatorType::EQ;

        case hsql::kOpNotEquals:
            return BoundOperatorType::NE;

        case hsql::kOpGreater:
            return BoundOperatorType::GT;

        case hsql::kOpGreaterEq:
            return BoundOperatorType::GE;

        case hsql::kOpLess:
            return BoundOperatorType::LT;

        case hsql::kOpLessEq:
            return BoundOperatorType::LE;

        case hsql::kOpAnd:
            return BoundOperatorType::AND;

        case hsql::kOpOr:
            return BoundOperatorType::OR;

        case hsql::kOpPlus:
            return BoundOperatorType::ADD;

        case hsql::kOpMinus:
            return BoundOperatorType::SUB;

        default:
            throw runtime_error("undefined binary operator");
    }
}

//bind operator for where clause:
// to handle expressions like (where id=10 and age>30 or salary>10000)
BoundExpression* Binder::BindOperator(const hsql::Expr* expression){

    // check if null first
    if (expression == nullptr) {
        throw std::runtime_error(
            "cannot bind null operator"
        );
    }

    // operator has left, right and the operator
    BoundExpression* left = BindExpression(expression->expr);
    BoundExpression* right = BindExpression(expression->expr2);
    BoundOperatorType op = BindBinaryOperator(expression->opType);

    //return type might be :
    //1. bool : if it's like (a and b) for where clauses
    //2.might have a value like int or string
    FieldType return_type;
    switch (op) {

        //these types return bool result
        case BoundOperatorType::AND:
        case BoundOperatorType::OR:
        case BoundOperatorType::EQ:
        case BoundOperatorType::NE:
        case BoundOperatorType::GT:
        case BoundOperatorType::GE:
        case BoundOperatorType::LT:

        case BoundOperatorType::LE:{ return_type = TYPE_BOOL;
            break;
        }
        // these types return a value
        // example in order by clauses : order by (age+10)
        case BoundOperatorType::ADD:
        case BoundOperatorType::SUB:
        case BoundOperatorType::MUL:
        case BoundOperatorType::DIV:{
            return_type = left->return_type;
            break;
        }

        default:
            throw runtime_error(" unsupported operator");
    }

    if(left){
        cout<<"left is not null"<<endl;
    }

    if(right){
        cout<<"right is not null"<<endl;
    }

    return new BoundBinaryExpression(left, op, right, return_type);
}



void Binder::BindFrom( const hsql::SelectStatement* statement, BoundSelectStatement& bound){

    //check if it has from table first
    if(statement->fromTable == nullptr) {
        throw runtime_error("select statement has no FROM table");
    }
    //fetch the table to extract info
    const hsql::TableRef* table =
        statement->fromTable;

    //2 types of binding the form:
    //1. normal select and the type would be kTableName
    // example : FROM User u
    if(table->type == hsql::kTableName){
        //bind as a single table
        bound.from_table = BindTable(table);
        context->AddTable(bound.from_table);
        return;
    }

    //if it's a join type, so it would contain multiple tables:
    // example :FROM User u INNER JOIN Orders o 
    if (table->type == hsql::kTableJoin) {

        //extract the base table
        const hsql::TableRef* left = table->join->left;
        //this would result to the base normal case of single table
        if (left->type != hsql::kTableName) {
            throw runtime_error("unsupported left side of join");
        }
        //now bind it
        bound.from_table = BindTable(left);
        context->AddTable(bound.from_table);

        return;
    }

    throw runtime_error("unsupported FROM table type");

}

// a helper function to table single table
BoundTable Binder::BindTable(const hsql::TableRef* table){

    //check uf null
    if(table == nullptr){
        throw runtime_error("null table");
    }
    //it must be KTableName type, not join or cross product
    if(table->type != hsql::kTableName){
        throw runtime_error("expected table name");
    }

    //extract table name

    // then fetch from catalog
    string table_name = table->name;

    string alias;

    if(table->alias != nullptr) {
        alias = table->alias->name;
    }

    // now extract the table from the catalog, 
    TableInfo* table_info = catalog->GetTable(table_name);

    // check if table name exists in the catalog
    if(!table_info){
        throw runtime_error("table does not exist: " +table_name);
    }

    //constuct the result
    BoundTable result;

    result.table_oid = table_info->table_id;
    result.table_name = table_name;
    result.alias = alias;
    result.schema = table_info->schema;

    result.printTable();
    return result;
}


// join :
//type conversion from AST type and my defined type
JoinType Binder::BindJoinType(hsql::JoinType type){

    switch (type){
        case hsql::kJoinInner:
            return JoinType::INNER;

        case hsql::kJoinLeft:
            return JoinType::LEFT;

        case hsql::kJoinRight:
            return JoinType::RIGHT;

        default:
            throw runtime_error("unsupported join type");
    }
}


BoundExpression* Binder::BindFunction(hsql::Expr* expression){

    if(expression == nullptr){
        return nullptr;
    }

    string function_name = expression->name;

    //to upper case normalization
    for(auto&c:function_name) {
        c = toupper(c);
    }

    AggregateType aggregate_type;

    //bound agg type bsed on it's name 
    if(function_name == "COUNT"){
        aggregate_type = COUNT;
    }
    else if(function_name == "SUM"){
        aggregate_type = SUM;
    }
    else if(function_name == "AVG"){
        aggregate_type = AVG;
    }
    else if(function_name == "MAX"){
        aggregate_type = MAX;
    }
    else if(function_name == "MIN"){
        aggregate_type = MIN;
    }
    else{
        cerr<<"unsupported function:"<<function_name<<endl;
        return nullptr;
    }


    // function argument, the first only that's what the executer support for now
    hsql::Expr* argument_expr = (*expression->exprList)[0];

    // bind the argument normally
    BoundExpression* argument = BindExpression(argument_expr);
    //check if null
    if(argument == nullptr){
        return nullptr;
    }

    BoundColumnRef* col = dynamic_cast<BoundColumnRef*>(argument);

    // return type
    FieldType return_type = col->return_type;

    return new BoundFunctionExpression(function_name, aggregate_type, col);
}
/*
// binding join clause
BoundJoinClause Binder::BindJoin(const hsql::JoinDefinition* join){

    //join has left, right, condition, and join type
    // first get right table

    string alias = join->right->alias?join->right->alias->name:"";
    string table_name = join->right->name;
    cout<<"right table name : "<<table_name<<endl;
    // now extract the table from the catalog, 
    TableInfo* table_info = catalog->GetTable(table_name);

    // check if table name exists in the catalog
    if(!table_info){
        throw runtime_error(" table does not exist: " +table_name);
    }
    // now bound the table 
    BoundTable bound_table;

    bound_table.table_oid = table_info->table_id;
    bound_table.table_name = table_name;
    bound_table.alias = alias;
    bound_table.schema = table_info->schema;
    
    bound_table.printTable();

    // last thing to add the table to the context
    context->AddTable(bound_table);

    BoundExpression* condition = nullptr;
    // extract on operator
    if(join->condition != nullptr){
        //extract the condition
        condition = BindExpression(join->condition);
        //it has to be bool
        if(condition->return_type != TYPE_BOOL){
            throw runtime_error("JOIN condition must evaluate to BOOLEAN");
        }
    }

    JoinType type =BindJoinType(join->type);

    BoundJoinClause result(type, bound_table, condition);
    cout<<"join bound table : "<<bound_table.table_name<<endl;
    
    
    return result;
}
*/

BoundJoinClause Binder::BindJoin(const hsql::JoinDefinition* join){
    if(join == nullptr){
        throw runtime_error("null JOIN definition");
    }

    if(join->right == nullptr){
        throw runtime_error("no right table");
    }
    //bind right table
    BoundTable right_table = BindTable(join->right);

    right_table.printTable();
    //add it to the context
    context->AddTable(right_table);

    BoundExpression* condition = nullptr;
    if(join->condition != nullptr){

        cout<<"binding join condition..."<<endl;
        condition = BindExpression(join->condition);

        if(condition == nullptr){
            throw runtime_error("failed to bind join condition");
        }
        if(condition->return_type != TYPE_BOOL){
            throw runtime_error("join condition must evaluate to bool");
        }
    }
    //bind join type
    JoinType type = BindJoinType(join->type);

    return BoundJoinClause(type,right_table,condition);
}


// order by :
void Binder::BindOrderBy(const hsql::SelectStatement* statement, BoundSelectStatement& bound){

    //first check if null
    if(statement->order == nullptr){
        return;
    }
    // order by might have multiple col like : order by age, salary
    for(const auto& order : *statement->order){

        if(order == nullptr){
            continue;
        }
        //bind the expression being sorted
        BoundExpression* expression =BindExpression(order->expr);
        //order type ASC or DESC
        OrderType order_type;

        switch(order->type){
            case hsql::kOrderAsc:{
                order_type = OrderType::ASC;
                break;
            }
            case hsql::kOrderDesc:{
                order_type = OrderType::DESC;
                break;
            }
            default:
                throw runtime_error("unsupported ORDER BY type");
        
        }
        //now push to the order by vector
        bound.order_by.emplace_back(expression,order_type);
    }
}


//limit adn offset binder
void Binder::BindLimitOffset(const hsql::SelectStatement* statement, BoundSelectStatement& bound){
    //check if null 
    if(statement->limit == nullptr){
        return;
    }

    // stmt.limit is LimitDescription and it has limit expression 
    //so if not null, just bind it as expression
    if(statement->limit->limit != nullptr){
        bound.limit =BindExpression(statement->limit->limit);
    }

    // now offset, and it's also an Expr, so just bind
    if(statement->limit->offset != nullptr){

        bound.offset =BindExpression(statement->limit->offset);
    }
}


//bind Insert
/* to bind insert stmt we need :
    1. bound the table as bound table
    2. Bound the Cols as colref
    3. bound values as expressions
*/
BoundInsertStatement* Binder::BindInsert(const hsql::InsertStatement* statement){
    
    
    BoundInsertStatement* bound = new BoundInsertStatement();
    
    //check if null statement
    if(statement ==nullptr)
        throw runtime_error("cannot bind null insert statement");

    //bound table
    if(statement->tableName == nullptr){
        throw runtime_error("table name can't be null");
    }

    //prepare bound_table from table_info 
    string table_name = string(statement->tableName);
    TableInfo* table_info  = this->catalog->GetTable(table_name);

    BoundTable* bound_table = new BoundTable();
    
    bound_table->table_name = table_name;
    bound_table->table_oid = table_info->table_id;
    bound_table->schema = table_info->schema;

    bound_table->printTable();

    //prepare cols:
    //insert stmt has a vector of col names 
    
    //extract cols from schema
    vector<Column> target_cols;
    vector<BoundExpression*>values; 

    int stmt_values_index{};

    //[10, "nader"];
    for(auto& col:bound_table->schema){
        cout<<col.getColName()<<endl;
        bool found = false;

        //loop throug cols, if found push to target_cols, else push null col
        if(statement->columns){
            for (char* col_name:*statement->columns){

                if(string(col_name)==col.getColName()){
                    found = true;
                    break;
                }
            }
        }
        
        if(found){
            //push to cols
            target_cols.push_back(col);
            //push to values
            BoundExpression* bound_expr = this->BindExpression(statement->values->at(stmt_values_index));
            stmt_values_index++;

            values.push_back(bound_expr);
        }else{
            Column null_col = Column(TYPE_NULL,col.getColName(), col.getColSize());
            target_cols.push_back(null_col);
            //push nullptr into values
            values.push_back(nullptr);
        }
    }

    for(auto&col:target_cols)col.printCol();
    for(auto&expr:values){
        if(expr)
            expr->PrintTree();
    }
    bound_table->printTable();


    //binding values
    
    //create bound insert then return it 

    bound->table = bound_table;
    bound->columns = target_cols;
    bound->values = values;

    bound->PrintTree();

    return bound;

}

BoundUpdateStatement* Binder::BindUpdate(const hsql::UpdateStatement* statement){
    //check if null
    if(statement == nullptr)
        throw runtime_error("can't bind null update statement");

    //table can't be null
    if (statement->table == nullptr)
        throw runtime_error("this table cannot be null");


    //fetch table info from table name
    BoundUpdateStatement* bound = new BoundUpdateStatement();
    string table_name = statement->table->name;
    TableInfo* table_info = catalog->GetTable(table_name);

    //check if table is in catalog
    if(!table_info)
        throw runtime_error("this table does not exist "+table_name);

    //bound table from table info
    BoundTable* bound_table = new BoundTable();

    bound_table->table_name = table_name;
    bound_table->table_oid = table_info->table_id;
    bound_table->schema = table_info->schema;

    //add to the context
    this->context->AddTable(*bound_table);

    //the desired res is like this :
    // update User sett user_id=9, firstName='nader' where User.user_id > 10;
    //cols :[user_id, firstName, NULL, NULL]
    //values:[9, nader, nullptr, nullptr]

     for(auto& column : bound_table->schema){
        bool found = false;

        BoundExpression* bound_value = nullptr;

        // search whether this schema 
        for(auto* update : *statement->updates){

            if(update == nullptr)
                throw runtime_error("invalid update syntax");


            string update_column = update->column;
            if(column.getColName()==update_column){
                
                if(found){
                    throw runtime_error("column updated more than once:"+update_column);
                }

                found = true;
                bound_value = BindExpression(update->value);
                break;
            }
        }

        bound->columns.push_back(column);
        bound->values.push_back(bound_value);
    }


    //bind where also
    if(statement->where != nullptr){
        bound->where = BindExpression(statement->where);
    }else{
        bound->where = nullptr;
    }

    bound->table = bound_table;

    for(auto&col:bound->columns)col.printCol();
    for(auto&expr:bound->values){
        if(expr)
            expr->PrintTree();
    }
    bound_table->printTable();

    return bound;
}


//delete binder: 
//delete from User where user_id = 10;
//we need table and where condition
BoundDeleteStatement* Binder::BindDelete(const hsql::DeleteStatement* statement){
    //check if null
    if(statement == nullptr)
        throw runtime_error("null delete statement");

    if(statement->tableName == nullptr)
        throw runtime_error("table cannot be null");

    //fetch the table
    string table_name = statement->tableName;
    //cout<<table_name;
    TableInfo* table_info = catalog->GetTable(table_name);

    if(table_info == nullptr){
        throw runtime_error("table does not exist:"+table_name);
    }

    //create bound delete statement
    BoundDeleteStatement* bound = new BoundDeleteStatement();

    BoundTable* bound_table = new BoundTable();
    
    //bound table from table_info
    bound_table->table_name = table_name;
    bound_table->table_oid = table_info->table_id;
    bound_table->schema = table_info->schema;
    bound->table = bound_table;

    context->AddTable(*bound_table);
    bound_table->printTable();

    //bind where condition
    if(statement->expr != nullptr){
        bound->where =BindExpression(statement->expr);
    }
    //else means delete every thing
    else{
        bound->where = nullptr;
    }


    return bound;
}



FieldType Binder::convertColumnType(const hsql::ColumnType& type){

    switch (type.data_type) {

        case hsql::DataType::INT:
            return TYPE_INT;
        case hsql::DataType::DOUBLE:
            return TYPE_FLOAT;
        case hsql::DataType::TEXT:
            return TYPE_STRING;
        case hsql::DataType::BOOLEAN:
            return TYPE_BOOL;
        default:
            throw runtime_error("unsupported column type");
    }
}


BoundStatement* Binder::bindCreate(const hsql::CreateStatement* statement){

    //check for the type first, then call the needed function
    switch (statement->type){
        
    case hsql::kCreateTable:{
        return this->bindCreateTable(statement);
    }

    case hsql::kCreateIndex:{
        return this->bindCreateIndex(statement);
    }
    
    default:
        throw runtime_error("create type is undefined");
        break;
    }
}
BoundCreateTableStatement*Binder::bindCreateTable(const hsql::CreateStatement* statement){

    //check if null
    if(statement == nullptr)
        throw runtime_error("cannot bind null CREATE statement");

    //check if table name exists
    if(statement->tableName==nullptr){
        throw runtime_error("table name is missing");
    }

    string table_name = statement->tableName;
    cout<<table_name<<endl;

    //check if table already exists
    //this returns nullptr if not found
    TableInfo* existing = catalog->GetTable(table_name);

    if(existing!=nullptr&& !statement->ifNotExists){
        throw runtime_error("table already exists"+table_name);
    }

    //now bound the statement
    BoundCreateTableStatement* bound = new BoundCreateTableStatement();

    //bind basic meta data like table name and the schema
    bound->table_name = table_name;
    bound->if_not_exists =statement->ifNotExists;
    cout<<bound->if_not_exists<<endl;

    if(statement->schema != nullptr){
        bound->schema_name = statement->schema;
    }

    //bind the cols and constraits 
    if(statement->columns == nullptr || statement->columns->empty()){
        throw runtime_error("table cols can't be empty");
    }

    //extract col names from stmt->cols then feed to teh bound
    unordered_set<string> column_names;

    for(auto* column_def :*statement->columns){

        if(column_def == nullptr)throw runtime_error("invalid column definition");
        if(column_def->name == nullptr)throw runtime_error("column name cannot be null");

        //prepare each col then push to the bound
        string column_name = column_def->name;
        cout<<column_name<<endl;
        //set is better to check uniqueness in col names
        //so check if already exists
        if(!column_names.insert(column_name).second){
            throw runtime_error("duplicate column:"+column_name);
        }

        //bound table needs a vector of Columns
        FieldType field_type =convertColumnType(column_def->type);
        size_t column_size;

        switch(column_def->type.data_type){

            case hsql::DataType::INT:
                column_size = sizeof(int);
                break;
            case hsql::DataType::DOUBLE:
                column_size = sizeof(double);
                break;
            case hsql::DataType::TEXT:
                column_size = 30;
                break;
            case hsql::DataType::BOOLEAN:
                column_size = sizeof(bool);
                break;
            default:
                throw runtime_error("unsupported column type");

        }

        Column column(field_type, column_name,column_size);

        //initial field values
        if(!column_def->nullable){
            column.setNull(false);
        }else{
            column.setNull(true);
        }

        //push to the bound
        bound->columns.push_back(column);

        //prepare constraints
        if(column_def->column_constraints!=nullptr){
            //loop through the set of constraints
            for(auto&constraint:*column_def->column_constraints){
                //constraint type may be : PK, FK, unique
                switch(constraint){
                    case hsql::ConstraintType::PrimaryKey:{
                        //pk has type, cols, and constraints
                        BoundTableConstraint pk;

                        pk.type =BoundConstraintType::PRIMARY_KEY;
                        pk.columns.push_back(column_name);
                        //push to bound constraints
                        bound->constraints.push_back(pk);
                        
                        for(auto&col_name:pk.columns)cout<<col_name<<" ";
                        break;
                    }
                    case hsql::ConstraintType::Unique:{
                        BoundTableConstraint unique;

                        unique.type = BoundConstraintType::UNIQUE;
                        unique.columns.push_back(column_name);
                        bound->constraints.push_back(unique);

                        for(auto&col_name:unique.columns)cout<<col_name<<" ";
                        break;
                    }
                    //null and not null is represented inside the col itself
                    case hsql::ConstraintType::NotNull:
                    case hsql::ConstraintType::Null:
                        //break;
                        break;
                    //col0level and table level will be handled 
                    case hsql::ConstraintType::ForeignKey:
                        break;

                    default:
                        break;
                }
            }
        }
        
        //for col-level references 
        //col_def has a list of references
        if(column_def->references != nullptr){

            for(auto*reference:*column_def->references){
                if(reference==nullptr)
                    continue;
                //it's fk constraint
                BoundTableConstraint fk;
                fk.type = BoundConstraintType::FOREIGN_KEY;
                fk.columns.push_back(column_name);

                //it must mention table name, can't be null
                if(reference->table == nullptr)
                    throw runtime_error("foreign key referenced table is null");

                fk.referenced_table = reference->table;

                //refernce has some cols, for each one push to fk constraint referenced cols
                if(reference->columns != nullptr){
                    for(char* ref_column :*reference->columns){
                        cout<<ref_column<<endl;
                        fk.referenced_columns.push_back(ref_column);
                    }
                }

                //finally push this fk into bound constraints
                bound->constraints.push_back(fk);
            }
        }
    }
    
    //that was col-level handling
    //but also table itself has some constraints to handle
    if(statement->tableConstraints != nullptr){
        for(auto* table_constraint:*statement->tableConstraints){
        
            if(table_constraint == nullptr)
                continue;
  
            BoundTableConstraint constraint;
            //handle type
            switch(table_constraint->type){
                case hsql::ConstraintType::PrimaryKey:
                    constraint.type = BoundConstraintType::PRIMARY_KEY;
                    break;
                case hsql::ConstraintType::Unique:
                    constraint.type = BoundConstraintType::UNIQUE;
                    break;
                case hsql::ConstraintType::ForeignKey:
                    constraint.type = BoundConstraintType::FOREIGN_KEY;
                    break;

                default:
                    throw runtime_error("unsupported table constraint");
            }

            //handle col_names
            if(table_constraint->columnNames != nullptr){
                for(char* column_name:*table_constraint->columnNames){
                    cout<<column_name<<endl;
                    constraint.columns.push_back(column_name);
                }
            }
            //handle fks
            auto* foreign_key =dynamic_cast<hsql::ForeignKeyConstraint*>(table_constraint);

            //check if it has a fk
            if(foreign_key != nullptr){

                if(foreign_key->references == nullptr){
                    throw runtime_error("foreign key has no ref clause");
                }
                if(foreign_key->references->table == nullptr){
                    throw runtime_error("foreign key ref table is null");
                }
                //fetch referenced-table
                constraint.referenced_table = foreign_key->references->table;
                //fetch cols as well
                if(foreign_key->references->columns != nullptr){
                    for(char* column :*foreign_key->references->columns){
                        cout<<column<<endl;
                        constraint.referenced_columns.push_back(column);
                    }
                }
            }

            bound->constraints.push_back(constraint);
        }
    }


    return bound;
}

BoundCreateIndexStatement*Binder::bindCreateIndex(const hsql::CreateStatement* statement){

    //we need to fetch index_name, table name, and cols names 

    //check if nullptr
    if(statement == nullptr){
        throw runtime_error("stmt is null");
    }

    //fetch index _name from ast
    //check if found first
    if(statement->indexName == nullptr){
        throw runtime_error("no index name");
    }

    string index_name = statement->indexName;

    //then bind table_name 
    if(statement->tableName==nullptr){
        throw runtime_error("table name is missing");
    }

    string table_name = statement->tableName;

    if(statement->indexColumns == nullptr){
        throw runtime_error("index cols is nullptr");
    }
    
    if(statement->indexColumns->empty()){
        throw runtime_error("index cols size can't be zero");
    }

    //symantic check
    //check if table_name is valid
    TableInfo* table = catalog->GetTable(table_name);

    if(table == nullptr){
        throw runtime_error(table_name +" table "+"does not exist");
    }


    BoundCreateIndexStatement* bound = new BoundCreateIndexStatement();

    bound->index_name = index_name;
    bound->table_name = table_name;
    bound->if_not_exists = statement->ifNotExists;

    //now bind cols 

    unordered_set<string> column_names;

    for(char* column : *statement->indexColumns){

        if (column == nullptr){
            throw runtime_error("index column cannot be null");
        }

        string column_name = column;

        //check if the col found in the table first
        bool found = false;

        for(Column table_column : table->schema){
            
            if(table_column.getColName() == column_name){
                found = true;
                bound->columns.push_back(table_column);
                break;
            }
        }
        //if not found, then throw error
        if(found == false){
            throw runtime_error("col not found");
        }
    }


    return bound;
}


/*
int
main(){

    DiskManager* dm = new DiskManager("catalog.db");
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    Catalog* catalog = new Catalog(BPM, true);

    string table_name = "User";

    Column t1_col1 = Column(TYPE_INT, "user_id", sizeof(int));
    Column t1_col2 = Column(TYPE_STRING, "firstName", 30);
    Column t1_col3 = Column(TYPE_STRING, "lastName", 30);
    Column t1_col4 = Column(TYPE_INT, "age", sizeof(int));
    vector<Column> user_schema = {t1_col1, t1_col2, t1_col3, t1_col4};

    Column t3_col1 = Column(TYPE_INT, "order_id", sizeof(int));
    Column t3_col2 = Column(TYPE_INT, "user_id", sizeof(int));
    Column t3_col3 = Column(TYPE_FLOAT, "totalAmount", sizeof(float));
    Column t3_col4 = Column(TYPE_BOOL, "isShipped", sizeof(bool));
    vector<Column> order_schema = {t3_col1, t3_col2, t3_col3, t3_col4};


    catalog->CreateTable(table_name, user_schema);
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

    const std::string sql = "SELECT u.user_id, u.firstName from User as u "
                            "inner join Orders on u.user_id = Orders.user_id "
                            "WHERE u.user_id = 2 order by u.user_id limit 10 offset 5;";

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


    


}*/