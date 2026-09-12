#include"Binder/BoundSelectStatement.h"


BoundJoinClause::BoundJoinClause(JoinType type, BoundTable& right_table, BoundExpression*condition){
    this->type = type;
    this->right_table=right_table;
    this->condition = condition;
}

BoundOrderBy::BoundOrderBy(BoundExpression* expr, OrderType order_type){
    this->expression = expr;
    this->order_type = order_type;
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

BoundStatementType BoundSelectStatement::type() const {
            return BoundStatementType::SELECT;
}


//just printing   
void BoundSelectStatement::PrintTree() const {
        cout << "BoundSelectStatement\n";

        // FROM
        cout << "|-- FROM\n";
        from_table.printTable();
        cout<<'\n';

        // JOINS
        if (!joins.empty()) {
            cout << "|-- JOINS\n";

            for (size_t i = 0; i < joins.size(); i++) {

                cout << "|   "
                     << (i == joins.size() - 1 ? "|-- " : "|-- ")
                     << "JOIN\n";

                cout << "|       Type: ";

                switch (joins[i].type) {
                    case JoinType::INNER:
                        cout << "INNER";
                        break;

                    case JoinType::LEFT:
                        cout << "LEFT";
                        break;

                    case JoinType::RIGHT:
                        cout << "RIGHT";
                        break;
                }

                cout << '\n';

                cout << "|       Table:\n";
                joins[i].right_table.printTable();
                cout<<"\n";
                if (joins[i].condition) {
                    cout << "|       Condition:\n";
                    joins[i].condition->PrintTree("|           ", true);
                }
            }
        }

        // SELECT
        cout << "|-- SELECT\n";

        for (size_t i = 0; i < select_list.size(); i++) {

            cout << "|   "
                 << (i == select_list.size() - 1 ? "|-- " : "|-- ")
                 << "Expression";

            if (!select_list[i].alias.empty()) {
                cout << " AS " << select_list[i].alias;
            }

            cout << '\n';

            if (select_list[i].expression) {
                select_list[i].expression->PrintTree(
                    "|       ",
                    true
                );
            }
        }

        // WHERE
        if (where) {
            cout << "|-- WHERE\n";
            where->PrintTree("|   ", true);
        }

        // GROUP BY
        if (!group_by.empty()) {
            cout << "|-- GROUP BY\n";

            for (size_t i = 0; i < group_by.size(); i++) {
                group_by[i]->PrintTree(
                    "|   ",
                    i == group_by.size() - 1
                );
            }
        }

        // HAVING
        if (having) {
            cout << "|-- HAVING\n";
            having->PrintTree("|   ", true);
        }

        // ORDER BY
        if (!order_by.empty()) {
            cout << "|-- ORDER BY\n";

            for (size_t i = 0; i < order_by.size(); i++) {

                cout << "|   "
                     << (i == order_by.size() - 1 ? "|-- " : "|-- ");

                cout << (order_by[i].order_type == OrderType::ASC
                             ? "ASC"
                             : "DESC")
                     << '\n';

                if (order_by[i].expression) {
                    order_by[i].expression->PrintTree(
                        "|       ",
                        true
                    );
                }
            }
        }

        // LIMIT
        if (limit) {
            cout << "|-- LIMIT\n";
            limit->PrintTree("|   ", true);
        }

        // OFFSET
        if (offset) {
            cout << "|-- OFFSET\n";
            offset->PrintTree("    ", true);
        }
}

