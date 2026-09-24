#include <iostream>

#include "external/sql-parser/src/SQLParser.h"
#include "external/sql-parser/src/SQLParserResult.h"

int main() {
    const std::string sql =
        "SELECT firstName FROM User WHERE user_id = 105;";

    hsql::SQLParserResult result;

    hsql::SQLParser::parse(sql, &result);

    if (!result.isValid()) {
        std::cerr << "Parse failed:\n";
        std::cerr << result.errorMsg() << '\n';
        return 1;
    }

    std::cout << "Parse successful!\n";
    std::cout << "Statements: " << result.size() << '\n';

    

    return 0;
}