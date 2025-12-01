#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

// ============= DATA TYPE ENUM =============
enum DataType {
    INT,
    FLOAT,
    VARCHAR
};

// ============= COLUMN CLASS (WITH CONSTRAINTS) =============
class Column {
private:
    string name;
    DataType type;
    int size;
    bool isPrimaryKey;
    bool isNotNull;

public:
    Column(string n, DataType t, int s = 0, bool pk = false, bool nn = false)
            : name(n), type(t), size(s), isPrimaryKey(pk), isNotNull(nn) {}

    string getName() const { return name; }
    DataType getType() const { return type; }
    int getSize() const { return size; }
    bool getIsPrimaryKey() const { return isPrimaryKey; }
    bool getIsNotNull() const { return isNotNull; }

    void setPrimaryKey(bool pk) { isPrimaryKey = pk; }
    void setNotNull(bool nn) { isNotNull = nn; }

    string getTypeString() const {
        switch(type) {
            case INT: return "INT";
            case FLOAT: return "FLOAT";
            case VARCHAR: return "VARCHAR(" + to_string(size) + ")";
            default: return "UNKNOWN";
        }
    }

    string getFullDefinition() const {
        string def = name + " " + getTypeString();
        if (isPrimaryKey) def += " PRIMARY KEY";
        if (isNotNull) def += " NOT NULL";
        return def;
    }
};

// ============= CONDITION CLASS (FOR WHERE CLAUSE) =============
class Condition {
public:
    string columnName;
    string op;  // =, !=, <, >, <=, >=
    string value;

    Condition(string col, string operation, string val)
            : columnName(col), op(operation), value(val) {}

    bool evaluate(const string& actualValue, DataType type) const {
        if (type == INT || type == FLOAT) {
            double actual = stod(actualValue);
            double expected = stod(value);

            if (op == "=") return actual == expected;
            if (op == "!=") return actual != expected;
            if (op == "<") return actual < expected;
            if (op == ">") return actual > expected;
            if (op == "<=") return actual <= expected;
            if (op == ">=") return actual >= expected;
        } else {
            // VARCHAR comparison
            if (op == "=") return actualValue == value;
            if (op == "!=") return actualValue != value;
            if (op == "<") return actualValue < value;
            if (op == ">") return actualValue > value;
            if (op == "<=") return actualValue <= value;
            if (op == ">=") return actualValue >= value;
        }
        return false;
    }
};

// ============= ROW CLASS =============
class Row {
private:
    vector<string> values;

public:
    void addValue(const string& value) {
        values.push_back(value);
    }

    vector<string>& getValues() {
        return values;
    }

    const vector<string>& getValues() const {
        return values;
    }

    string getValue(int index) const {
        if (index >= 0 && index < (int)values.size()) {
            return values[index];
        }
        return "";
    }

    void setValue(int index, const string& value) {
        if (index >= 0 && index < (int)values.size()) {
            values[index] = value;
        }
    }

    int getValueCount() const {
        return values.size();
    }
};

// ============= TABLE CLASS =============
class Table {
private:
    string tableName;
    vector<Column> columns;
    vector<Row> rows;
    int primaryKeyIndex;

public:
    Table(string name)
            : tableName(name), primaryKeyIndex(-1) {}

    void addColumn(const Column& col) {
        if (col.getIsPrimaryKey()) {
            primaryKeyIndex = columns.size();
        }
        columns.push_back(col);
    }

    void addRow(const Row& row) {
        rows.push_back(row);
    }

    string getTableName() const { return tableName; }
    vector<Column>& getColumns() { return columns; }
    const vector<Column>& getColumns() const { return columns; }
    vector<Row>& getRows() { return rows; }
    const vector<Row>& getRows() const { return rows; }

    int getColumnCount() const { return columns.size(); }
    int getRowCount() const { return rows.size(); }
    int getPrimaryKeyIndex() const { return primaryKeyIndex; }

    int getColumnIndex(const string& colName) const {
        for (int i = 0; i < (int)columns.size(); i++) {
            if (columns[i].getName() == colName) {
                return i;
            }
        }
        return -1;
    }

    bool hasPrimaryKey(const string& value) const {
        if (primaryKeyIndex == -1) return false;

        for (const auto& row : rows) {
            if (row.getValue(primaryKeyIndex) == value) {
                return true;
            }
        }
        return false;
    }

    void display() const {
        cout << "Table '" << tableName << "' created successfully!" << endl;
        cout << "Columns:" << endl;
        for (const auto& col : columns) {
            cout << "  - " << col.getFullDefinition() << endl;
        }
    }

    void displayData(const vector<int>& columnIndices = {}) const {
        cout << "\nTable: " << tableName << endl;
        cout << "--------------------------------------" << endl;

        vector<int> displayCols = columnIndices;
        if (displayCols.empty()) {
            for (int i = 0; i < (int)columns.size(); i++) {
                displayCols.push_back(i);
            }
        }

        // Print headers
        for (int i = 0; i < (int)displayCols.size(); i++) {
            cout << columns[displayCols[i]].getName();
            if (i < (int)displayCols.size() - 1) cout << " | ";
        }
        cout << endl;

        for (int i = 0; i < (int)displayCols.size(); i++) {
            cout << "----------";
            if (i < (int)displayCols.size() - 1) cout << "-+-";
        }
        cout << endl;

        if (rows.empty()) {
            cout << "No data in table." << endl;
        } else {
            for (const auto& row : rows) {
                for (int i = 0; i < (int)displayCols.size(); i++) {
                    cout << row.getValue(displayCols[i]);
                    if (i < (int)displayCols.size() - 1) cout << " | ";
                }
                cout << endl;
            }
        }

        cout << "\nTotal rows: " << rows.size() << endl;
    }

    int deleteRows(const vector<Condition>& conditions) {
        if (conditions.empty()) {
            // DELETE * (all rows)
            int count = rows.size();
            rows.clear();
            return count;
        }

        vector<Row> newRows;
        int deletedCount = 0;

        for (const auto& row : rows) {
            bool matchesAll = true;

            for (const auto& cond : conditions) {
                int colIndex = getColumnIndex(cond.columnName);
                if (colIndex == -1) continue;

                string actualValue = row.getValue(colIndex);
                DataType colType = columns[colIndex].getType();

                if (!cond.evaluate(actualValue, colType)) {
                    matchesAll = false;
                    break;
                }
            }

            if (!matchesAll) {
                newRows.push_back(row);
            } else {
                deletedCount++;
            }
        }

        rows = newRows;
        return deletedCount;
    }

    int updateRows(const map<string, string>& updates, const vector<Condition>& conditions) {
        int updatedCount = 0;

        for (auto& row : rows) {
            bool matchesAll = true;

            // Check if row matches all conditions
            for (const auto& cond : conditions) {
                int colIndex = getColumnIndex(cond.columnName);
                if (colIndex == -1) continue;

                string actualValue = row.getValue(colIndex);
                DataType colType = columns[colIndex].getType();

                if (!cond.evaluate(actualValue, colType)) {
                    matchesAll = false;
                    break;
                }
            }

            if (matchesAll) {
                // Update the row
                for (const auto& update : updates) {
                    int colIndex = getColumnIndex(update.first);
                    if (colIndex != -1) {
                        row.setValue(colIndex, update.second);
                    }
                }
                updatedCount++;
            }
        }

        return updatedCount;
    }
};

// ============= QUERY PARSER CLASS =============
class QueryParser {
private:
    static string trim(const string& str) {
        int first = 0;
        int last = str.length() - 1;

        while (first <= last && isspace(str[first])) first++;
        while (last >= first && isspace(str[last])) last--;

        if (first > last) return "";
        return str.substr(first, last - first + 1);
    }

    static string toUpper(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    static DataType parseDataType(const string& typeStr, int& size) {
        string upper = toUpper(typeStr);
        size = 0;

        if (upper == "INT") {
            return INT;
        } else if (upper == "FLOAT") {
            return FLOAT;
        } else if (upper.find("VARCHAR") == 0) {
            size_t start = upper.find('(');
            size_t end = upper.find(')');
            if (start != string::npos && end != string::npos) {
                size = stoi(upper.substr(start + 1, end - start - 1));
            } else {
                size = 255;
            }
            return VARCHAR;
        }

        throw runtime_error("Unknown data type: " + typeStr);
    }

public:
    static Table* parseCreateTable(const string& query) {
        size_t tablePos = toUpper(query).find("TABLE");
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

            int size;
            DataType type = parseDataType(colType, size);
            table->addColumn(Column(colName, type, size, isPK, isNN));
        }

        if (table->getColumnCount() == 0) {
            delete table;
            throw runtime_error("Table must have at least one column");
        }

        return table;
    }

    static void parseInsert(const string& query, string& tableName, vector<string>& values) {
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

    static void parseSelect(const string& query, string& tableName,
                            vector<string>& columns, vector<Condition>& conditions) {
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
        } else {
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

    static void parseDelete(const string& query, string& tableName, vector<Condition>& conditions) {
        string upperQuery = toUpper(query);

        // Check for DELETE *
        size_t starPos = upperQuery.find("DELETE *");
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

    static void parseUpdate(const string& query, string& tableName,
                            map<string, string>& updates, vector<Condition>& conditions) {
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

    static void parseWhereClause(const string& whereStr, vector<Condition>& conditions) {
        string trimmed = trim(whereStr);

        // Simple parser: splits by AND (doesn't handle OR yet)
        stringstream ss(trimmed);
        string condStr;

        // For now, handle single condition or multiple conditions with AND
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

        for (const auto& part : parts) {
            string cond = trim(part);

            // Find operator
            string op;
            size_t opPos = string::npos;

            if ((opPos = cond.find("!=")) != string::npos) {
                op = "!=";
            } else if ((opPos = cond.find("<=")) != string::npos) {
                op = "<=";
            } else if ((opPos = cond.find(">=")) != string::npos) {
                op = ">=";
            } else if ((opPos = cond.find("=")) != string::npos) {
                op = "=";
            } else if ((opPos = cond.find("<")) != string::npos) {
                op = "<";
            } else if ((opPos = cond.find(">")) != string::npos) {
                op = ">";
            }

            if (opPos != string::npos) {
                string col = trim(cond.substr(0, opPos));
                string val = trim(cond.substr(opPos + op.length()));
                conditions.push_back(Condition(col, op, val));
            }
        }
    }
};

// ============= DATABASE ENGINE CLASS =============
class DatabaseEngine {
private:
    map<string, Table*> tables;

    bool isValidInt(const string& str) {
        if (str.empty()) return false;

        size_t start = 0;
        if (str[0] == '-' || str[0] == '+') start = 1;

        if (start >= str.length()) return false;

        for (size_t i = start; i < str.length(); i++) {
            if (!isdigit(str[i])) return false;
        }
        return true;
    }

    bool isValidFloat(const string& str) {
        if (str.empty()) return false;

        size_t start = 0;
        if (str[0] == '-' || str[0] == '+') start = 1;

        if (start >= str.length()) return false;

        bool hasDecimal = false;
        bool hasDigit = false;

        for (size_t i = start; i < str.length(); i++) {
            if (isdigit(str[i])) {
                hasDigit = true;
            } else if (str[i] == '.') {
                if (hasDecimal) return false;
                hasDecimal = true;
            } else {
                return false;
            }
        }

        return hasDigit;
    }

public:
    ~DatabaseEngine() {
        for (auto& pair : tables) {
            delete pair.second;
        }
        tables.clear();
    }

    void createTable(const string& query) {
        try {
            Table* table = QueryParser::parseCreateTable(query);
            string tableName = table->getTableName();

            if (tables.find(tableName) != tables.end()) {
                delete table;
                cout << "Error: Table '" << tableName << "' already exists!" << endl;
                return;
            }

            tables[tableName] = table;
            table->display();

        } catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void insertInto(const string& query) {
        try {
            string tableName;
            vector<string> values;

            QueryParser::parseInsert(query, tableName, values);

            if (tables.find(tableName) == tables.end()) {
                cout << "Error: Table '" << tableName << "' does not exist!" << endl;
                return;
            }

            Table* table = tables[tableName];

            if ((int)values.size() != table->getColumnCount()) {
                cout << "Error: Expected " << table->getColumnCount()
                     << " values but got " << values.size() << endl;
                return;
            }

            // Validate constraints and data types
            const vector<Column>& columns = table->getColumns();

            for (int i = 0; i < (int)values.size(); i++) {
                const Column& col = columns[i];
                string value = values[i];

                // Check NOT NULL
                if (col.getIsNotNull() && value.empty()) {
                    cout << "Error: Column '" << col.getName() << "' cannot be NULL" << endl;
                    return;
                }

                // Check PRIMARY KEY uniqueness
                if (col.getIsPrimaryKey()) {
                    if (table->hasPrimaryKey(value)) {
                        cout << "Error: Duplicate PRIMARY KEY value '" << value << "'" << endl;
                        return;
                    }
                }

                // Validate data type
                if (col.getType() == INT) {
                    if (!isValidInt(value)) {
                        cout << "Error: Column '" << col.getName()
                             << "' expects INT but got '" << value << "'" << endl;
                        return;
                    }
                } else if (col.getType() == FLOAT) {
                    if (!isValidFloat(value)) {
                        cout << "Error: Column '" << col.getName()
                             << "' expects FLOAT but got '" << value << "'" << endl;
                        return;
                    }
                } else if (col.getType() == VARCHAR) {
                    if ((int)value.length() > col.getSize()) {
                        cout << "Error: Column '" << col.getName()
                             << "' VARCHAR(" << col.getSize() << ") exceeded. Got "
                             << value.length() << " characters" << endl;
                        return;
                    }
                }
            }

            Row row;
            for (const auto& value : values) {
                row.addValue(value);
            }

            table->addRow(row);

            cout << "[" << table->getRowCount() << "] Row inserted successfully into '"
                 << tableName << "'!" << endl;

        } catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void selectFrom(const string& query) {
        try {
            string tableName;
            vector<string> columns;
            vector<Condition> conditions;

            QueryParser::parseSelect(query, tableName, columns, conditions);

            if (tables.find(tableName) == tables.end()) {
                cout << "Error: Table '" << tableName << "' does not exist!" << endl;
                return;
            }

            Table* table = tables[tableName];

            // If no WHERE clause, display all rows
            if (conditions.empty()) {
                if (columns.empty()) {
                    table->displayData();
                } else {
                    // Get column indices for specific columns
                    vector<int> colIndices;
                    for (const auto& colName : columns) {
                        int idx = table->getColumnIndex(colName);
                        if (idx != -1) {
                            colIndices.push_back(idx);
                        }
                    }
                    table->displayData(colIndices);
                }
                return;
            }

            // Filter rows based on WHERE conditions
            cout << "\nTable: " << tableName << endl;
            cout << "--------------------------------------" << endl;

            // Print headers
            const vector<Column>& tableCols = table->getColumns();
            vector<int> displayCols;

            if (columns.empty()) {
                for (int i = 0; i < (int)tableCols.size(); i++) {
                    displayCols.push_back(i);
                }
            } else {
                for (const auto& colName : columns) {
                    int idx = table->getColumnIndex(colName);
                    if (idx != -1) displayCols.push_back(idx);
                }
            }

            for (size_t i = 0; i < displayCols.size(); i++) {
                cout << tableCols[displayCols[i]].getName();
                if (i < displayCols.size() - 1) cout << " | ";
            }
            cout << endl;

            for (size_t i = 0; i < displayCols.size(); i++) {
                cout << "----------";
                if (i < displayCols.size() - 1) cout << "-+-";
            }
            cout << endl;

            // Display filtered rows
            int count = 0;
            const vector<Row>& rows = table->getRows();

            for (const auto& row : rows) {
                bool matchesAll = true;

                for (const auto& cond : conditions) {
                    int colIndex = table->getColumnIndex(cond.columnName);
                    if (colIndex == -1) continue;

                    string actualValue = row.getValue(colIndex);
                    DataType colType = tableCols[colIndex].getType();

                    if (!cond.evaluate(actualValue, colType)) {
                        matchesAll = false;
                        break;
                    }
                }

                if (matchesAll) {
                    for (size_t i = 0; i < displayCols.size(); i++) {
                        cout << row.getValue(displayCols[i]);
                        if (i < displayCols.size() - 1) cout << " | ";
                    }
                    cout << endl;
                    count++;
                }
            }

            if (count == 0) {
                cout << "No matching rows found." << endl;
            }

            cout << "\nRows returned: " << count << endl;

        } catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void deleteFrom(const string& query) {
        try {
            string tableName;
            vector<Condition> conditions;

            QueryParser::parseDelete(query, tableName, conditions);

            if (tables.find(tableName) == tables.end()) {
                cout << "Error: Table '" << tableName << "' does not exist!" << endl;
                return;
            }

            Table* table = tables[tableName];
            int deletedCount = table->deleteRows(conditions);

            cout << "[" << deletedCount << "] Row(s) deleted from '" << tableName << "'!" << endl;

        } catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void updateTable(const string& query) {
        try {
            string tableName;
            map<string, string> updates;
            vector<Condition> conditions;

            QueryParser::parseUpdate(query, tableName, updates, conditions);

            if (tables.find(tableName) == tables.end()) {
                cout << "Error: Table '" << tableName << "' does not exist!" << endl;
                return;
            }

            Table* table = tables[tableName];

            // Validate updates
            for (const auto& update : updates) {
                int colIndex = table->getColumnIndex(update.first);
                if (colIndex == -1) {
                    cout << "Error: Column '" << update.first << "' does not exist!" << endl;
                    return;
                }

                const Column& col = table->getColumns()[colIndex];

                // Validate data type
                if (col.getType() == INT && !isValidInt(update.second)) {
                    cout << "Error: Invalid INT value for column '" << update.first << "'" << endl;
                    return;
                }
                if (col.getType() == FLOAT && !isValidFloat(update.second)) {
                    cout << "Error: Invalid FLOAT value for column '" << update.first << "'" << endl;
                    return;
                }
            }

            int updatedCount = table->updateRows(updates, conditions);

            cout << "[" << updatedCount << "] Row(s) updated in '" << tableName << "'!" << endl;

        } catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void listTables() {
        if (tables.empty()) {
            cout << "No tables in database." << endl;
            return;
        }

        cout << "Tables in database:" << endl;
        for (const auto& pair : tables) {
            cout << "  - " << pair.first << " ("
                 << pair.second->getRowCount() << " rows)" << endl;
        }
    }
};

// ============= MAIN APPLICATION =============
int main() {
    DatabaseEngine db;

    cout << "========================================" << endl;
    cout << "   C++ OOP DBMS Engine v3.0 COMPLETE   " << endl;
    cout << "========================================" << endl;
    cout << "\nSupported commands:" << endl;
    cout << "  CREATE TABLE table_name (col1 type1 [PRIMARY KEY] [NOT NULL], ...)" << endl;
    cout << "  INSERT INTO table_name VALUES (val1, val2, ...)" << endl;
    cout << "  SELECT * FROM table_name [WHERE condition]" << endl;
    cout << "  SELECT col1, col2 FROM table_name [WHERE condition]" << endl;
    cout << "  UPDATE table_name SET col1=val1, col2=val2 [WHERE condition]" << endl;
    cout << "  DELETE * FROM table_name" << endl;
    cout << "  DELETE FROM table_name WHERE condition" << endl;
    cout << "  LIST TABLES" << endl;
    cout << "  EXIT" << endl;
    cout << "\nSupported types: INT, FLOAT, VARCHAR(size)" << endl;
    cout << "Supported operators in WHERE: =, !=, <, >, <=, >=" << endl;
    cout << "\nExamples:" << endl;
    cout << "  CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL, age INT)" << endl;
    cout << "  INSERT INTO users VALUES (1, John, 25)" << endl;
    cout << "  INSERT INTO users VALUES (2, Sarah, 30)" << endl;
    cout << "  SELECT * FROM users" << endl;
    cout << "  SELECT name, age FROM users WHERE age > 25" << endl;
    cout << "  UPDATE users SET age = 26 WHERE id = 1" << endl;
    cout << "  DELETE FROM users WHERE age < 30" << endl;
    cout << "  DELETE * FROM users\n" << endl;

    string query;

    while (true) {
        cout << "dbms> ";
        getline(cin, query);

        if (query.empty()) continue;

        string upperQuery = query;
        transform(upperQuery.begin(), upperQuery.end(), upperQuery.begin(), ::toupper);

        if (upperQuery == "EXIT" || upperQuery == "QUIT") {
            cout << "Goodbye!" << endl;
            break;
        }
        else if (upperQuery == "LIST TABLES") {
            db.listTables();
        }
        else if (upperQuery.find("CREATE TABLE") == 0) {
            db.createTable(query);
        }
        else if (upperQuery.find("INSERT INTO") == 0) {
            db.insertInto(query);
        }
        else if (upperQuery.find("SELECT") == 0) {
            db.selectFrom(query);
        }
        else if (upperQuery.find("UPDATE") == 0) {
            db.updateTable(query);
        }
        else if (upperQuery.find("DELETE") == 0) {
            db.deleteFrom(query);
        }
        else {
            cout << "Unknown command. Type 'EXIT' to quit." << endl;
        }

        cout << endl;
    }

    return 0;
}