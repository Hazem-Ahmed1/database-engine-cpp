#include "QueryParser.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdlib>  // for atoi


using namespace std;

string QueryParser::trim(const string& str) {
    int first = 0;
    int last = (int)str.length() - 1;

    while (first <= last && isspace((unsigned char)str[first])) first++;
    while (last >= first && isspace((unsigned char)str[last])) last--;

    if (first > last) return "";
    return str.substr(first, last - first + 1);
}

string QueryParser::toUpper(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

DataType QueryParser::parseDataType(const string& typeStr, int& size) {
    string upper = toUpper(typeStr);
    size = 0;

    if (upper == "INT") {
        return INT;
    }
    else if (upper == "FLOAT") {
        return FLOAT;
    }
    else if (upper.find("VARCHAR") == 0) {
        size_t start = upper.find('(');
        size_t end = upper.find(')');
        if (start != string::npos && end != string::npos) {
            size = stoi(upper.substr(start + 1, end - start - 1));
        }
        else {
            size = 255;
        }
        return VARCHAR;
    }


    throw runtime_error("Unknown data type: " + typeStr);
}

Table* QueryParser::parseCreateTable(const string& query) {
    string upperQuery = toUpper(query);
    size_t tablePos = upperQuery.find("TABLE");
    size_t nameStart = query.find_first_not_of(" \t", tablePos + 5);
    size_t parenStart = query.find('(', nameStart);

    if (nameStart == string::npos || parenStart == string::npos) {
        throw runtime_error("Invalid CREATE TABLE syntax");
    }

    string tableName = trim(query.substr(nameStart, parenStart - nameStart));
    Table* table = new Table(tableName);

    size_t parenEnd = query.find_last_of(')');
    if (parenEnd == string::npos) {
        delete table;
        throw runtime_error("Missing closing parenthesis");
    }

    string columnDefs = query.substr(parenStart + 1, parenEnd - parenStart - 1);

    stringstream ss(columnDefs);
    string columnDef;
    int pkCount = 0;

    while (getline(ss, columnDef, ',')) {
        columnDef = trim(columnDef);
        string upperDef = toUpper(columnDef);

        stringstream colStream(columnDef);
        string colName, colType;
        colStream >> colName >> colType;

        if (colName.empty() || colType.empty()) {
            delete table;
            throw runtime_error("Invalid column definition: " + columnDef);
        }

        // Check for constraints
        bool isPK = (upperDef.find("PRIMARY KEY") != string::npos);
        bool isNN = (upperDef.find("NOT NULL") != string::npos);

        if (isPK) {
            pkCount++;
            if (pkCount > 1) {
                delete table;
                throw runtime_error("Table can have only one PRIMARY KEY");
            }
        }

        int size;
        DataType type = parseDataType(colType, size);
        Column col(colName, type, size, isPK, isNN);
        table->addColumn(col);
    }

    if (table->getColumnCount() == 0) {
        delete table;
        throw runtime_error("Table must have at least one column");
    }

    return table;
}

void QueryParser::parseInsert(const string& query,
    string& tableName,
    vector<string>& values) {
    string upperQuery = toUpper(query);
    size_t intoPos = upperQuery.find("INTO");
    size_t tableStart = query.find_first_not_of(" \t", intoPos + 4);
    size_t valuesPos = upperQuery.find("VALUES", tableStart);

    if (tableStart == string::npos || valuesPos == string::npos) {
        throw runtime_error("Invalid INSERT syntax");
    }

    tableName = trim(query.substr(tableStart, valuesPos - tableStart));

    size_t parenStart = query.find('(', valuesPos);
    size_t parenEnd = query.find_last_of(')');

    if (parenStart == string::npos || parenEnd == string::npos) {
        throw runtime_error("Missing parentheses in VALUES");
    }

    string valueStr = query.substr(parenStart + 1, parenEnd - parenStart - 1);

    stringstream ss(valueStr);
    string value;

    while (getline(ss, value, ',')) {
        values.push_back(trim(value));
    }
}

void QueryParser::parseSelect(const string& query,
    string& tableName,
    vector<string>& columns,
    vector<Condition>& conditions) {
    string upperQuery = toUpper(query);

    // Find FROM
    size_t fromPos = upperQuery.find("FROM");
    if (fromPos == string::npos) {
        throw runtime_error("SELECT must have FROM clause");
    }

    // Extract columns (SELECT ... FROM)
    string colPart = trim(query.substr(6, fromPos - 6)); // after "SELECT"

    if (colPart == "*") {
        // Select all columns
        columns.clear();
    }
    else {
        // Parse specific columns
        stringstream ss(colPart);
        string col;
        while (getline(ss, col, ',')) {
            columns.push_back(trim(col));
        }
    }

    // Extract table name
    size_t wherePos = upperQuery.find("WHERE", fromPos);
    size_t tableEnd = (wherePos != string::npos) ? wherePos : query.length();
    tableName = trim(query.substr(fromPos + 4, tableEnd - (fromPos + 4)));

    // Parse WHERE clause if exists
    if (wherePos != string::npos) {
        parseWhereClause(query.substr(wherePos + 5), conditions);
    }
}

void QueryParser::parseDelete(const string& query,
    string& tableName,
    vector<Condition>& conditions) {
    string upperQuery = toUpper(query);

    size_t fromPos = upperQuery.find("FROM");

    if (fromPos == string::npos) {
        throw runtime_error("DELETE must have FROM clause");
    }

    // Extract table name
    size_t wherePos = upperQuery.find("WHERE", fromPos);
    size_t tableEnd = (wherePos != string::npos) ? wherePos : query.length();
    tableName = trim(query.substr(fromPos + 4, tableEnd - (fromPos + 4)));

    // Parse WHERE clause if exists
    if (wherePos != string::npos) {
        parseWhereClause(query.substr(wherePos + 5), conditions);
    }
    // If no WHERE clause and DELETE *, conditions will be empty (delete all)
}

void QueryParser::parseUpdate(const string& query,
    string& tableName,
    map<string, string>& updates,
    vector<Condition>& conditions) {
    string upperQuery = toUpper(query);

    size_t updatePos = upperQuery.find("UPDATE");
    size_t setPos = upperQuery.find("SET", updatePos);

    if (setPos == string::npos) {
        throw runtime_error("UPDATE must have SET clause");
    }

    // Extract table name
    tableName = trim(query.substr(updatePos + 6, setPos - (updatePos + 6)));

    // Find WHERE position
    size_t wherePos = upperQuery.find("WHERE", setPos);
    size_t setEnd = (wherePos != string::npos) ? wherePos : query.length();

    // Parse SET clause
    string setPart = query.substr(setPos + 3, setEnd - (setPos + 3));
    stringstream ss(setPart);
    string assignment;

    while (getline(ss, assignment, ',')) {
        assignment = trim(assignment);
        size_t eqPos = assignment.find('=');
        if (eqPos != string::npos) {
            string col = trim(assignment.substr(0, eqPos));
            string val = trim(assignment.substr(eqPos + 1));
            updates[col] = val;
        }
    }

    // Parse WHERE clause if exists
    if (wherePos != string::npos) {
        parseWhereClause(query.substr(wherePos + 5), conditions);
    }
}

void QueryParser::parseWhereClause(const string& whereStr,
    vector<Condition>& conditions) {
    string trimmed = trim(whereStr);

    // Simple parser: splits by AND (doesn't handle OR yet)
    vector<string> parts;
    size_t pos = 0;
    string temp = trimmed;
    string upperTemp = toUpper(temp);

    while ((pos = upperTemp.find(" AND ")) != string::npos) {
        parts.push_back(temp.substr(0, pos));
        temp = temp.substr(pos + 5);
        upperTemp = toUpper(temp);
    }
    parts.push_back(temp);

    for (size_t i = 0; i < parts.size(); i++) {
        string condStr = trim(parts[i]);

        // Find operator
        string op;
        size_t opPos = string::npos;

        if ((opPos = condStr.find("!=")) != string::npos) {
            op = "!=";
        }
        else if ((opPos = condStr.find("<=")) != string::npos) {
            op = "<=";
        }
        else if ((opPos = condStr.find(">=")) != string::npos) {
            op = ">=";
        }
        else if ((opPos = condStr.find("=")) != string::npos) {
            op = "=";
        }
        else if ((opPos = condStr.find("<")) != string::npos) {
            op = "<";
        }
        else if ((opPos = condStr.find(">")) != string::npos) {
            op = ">";
        }

        if (opPos != string::npos) {
            string col = trim(condStr.substr(0, opPos));
            string val = trim(condStr.substr(opPos + op.length()));
            conditions.push_back(Condition(col, op, val));
        }
    }
}

string QueryParser::parseDropTable(const string& query) {
    string upperQuery = toUpper(query);
    size_t tablePos = upperQuery.find("TABLE");

    if (tablePos == string::npos) {
        throw runtime_error("DROP TABLE syntax error");
    }

    string tableName = trim(query.substr(tablePos + 5)); // after "TABLE"
    
    // Remove trailing semicolon if present (though main loop handles splitting by ;)
    if (!tableName.empty() && tableName[tableName.length() - 1] == ';') {
        tableName = tableName.substr(0, tableName.length() - 1);
    }
    tableName = trim(tableName);

    if (tableName.empty()) {
        throw runtime_error("Table name missing in DROP TABLE command");
    }

    return tableName;
}
