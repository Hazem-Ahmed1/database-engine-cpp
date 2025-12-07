#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <string>
#include <vector>
#include <map>
using namespace std;
#include "Table.h"
#include "Condition.h"
#include "Column.h"
#include <cstdlib>


class QueryParser {
private:
    static string trim(const string& str);
    static string toUpper(const string& str);
    static DataType parseDataType(const string& typeStr, int& size);
    static void parseWhereClause(const string& whereStr, vector<Condition>& conditions);

public:
    static Table* parseCreateTable(const string& query);

    static void parseInsert(const string& query,
        string& tableName,
        vector<string>& values);

    static void parseSelect(const string& query,
        string& tableName,
        vector<string>& columns,
        vector<Condition>& conditions);

    static void parseDelete(const string& query,
        string& tableName,
        vector<Condition>& conditions);

    static void parseUpdate(const string& query,
        string& tableName,
        map<string, string>& updates,
        vector<Condition>& conditions);
};

#endif
