#include "DatabaseEngine.h"
#include "Table.h"
#include "QueryParser.h"
#include "Condition.h"
#include "Column.h"
#include "Row.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
using namespace std;

DatabaseEngine::DatabaseEngine() {
}

DatabaseEngine::~DatabaseEngine() {
    map<string, Table*>::iterator it;
    for (it = tables.begin(); it != tables.end(); ++it) {
        delete it->second;
    }
    tables.clear();
}

bool DatabaseEngine::isValidInt(const string& str) {
    if (str.empty()) return false;

    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;

    if (start >= str.length()) return false;

    for (size_t i = start; i < str.length(); i++) {
        if (!isdigit((unsigned char)str[i])) return false;
    }
    return true;
}

bool DatabaseEngine::isValidFloat(const string& str) {
    if (str.empty()) return false;

    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;

    if (start >= str.length()) return false;

    bool hasDecimal = false;
    bool hasDigit = false;

    for (size_t i = start; i < str.length(); i++) {
        if (isdigit((unsigned char)str[i])) {
            hasDigit = true;
        }
        else if (str[i] == '.') {
            if (hasDecimal) return false;
            hasDecimal = true;
        }
        else {
            return false;
        }
    }

    return hasDigit;
}

void DatabaseEngine::createTable(const string& query) {
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

    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::insertInto(const string& query) {
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

        const vector<Column>& columns = table->getColumns();

        for (int i = 0; i < (int)values.size(); i++) {
            const Column& col = columns[i];
            string value = values[i];

            // NOT NULL
            if (col.getIsNotNull() && value.empty()) {
                cout << "Error: Column '" << col.getName() << "' cannot be NULL" << endl;
                return;
            }

            // PRIMARY KEY uniqueness
            if (col.getIsPrimaryKey()) {
                if (table->hasPrimaryKey(value)) {
                    cout << "Error: Duplicate PRIMARY KEY value '" << value << "'" << endl;
                    return;
                }
            }

            // Type checks
            if (col.getType() == INT) {
                if (!isValidInt(value)) {
                    cout << "Error: Column '" << col.getName()
                        << "' expects INT but got '" << value << "'" << endl;
                    return;
                }
            }
            else if (col.getType() == FLOAT) {
                if (!isValidFloat(value)) {
                    cout << "Error: Column '" << col.getName()
                        << "' expects FLOAT but got '" << value << "'" << endl;
                    return;
                }
            }
            else if (col.getType() == VARCHAR) {
                if ((int)value.length() > col.getSize()) {
                    cout << "Error: Column '" << col.getName()
                        << "' VARCHAR(" << col.getSize() << ") exceeded. Got "
                        << value.length() << " characters" << endl;
                    return;
                }
            }
        }

        Row row;
        for (int i = 0; i < (int)values.size(); i++) {
            row.addValue(values[i]);
        }

        table->addRow(row);

        cout << "[" << table->getRowCount() << "] Row inserted successfully into '"
            << tableName << "'!" << endl;

    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::selectFrom(const string& query) {
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

        // No WHERE
        if (conditions.empty()) {
            if (columns.empty()) {
                table->displayData();
            }
            else {
                vector<int> colIndices;
                for (int i = 0; i < (int)columns.size(); i++) {
                    int idx = table->getColumnIndex(columns[i]);
                    if (idx == -1) {
                        cout << "Error: Column '" << columns[i] << "' does not exist!" << endl;
                        return;
                    }
                    colIndices.push_back(idx);
                }
                table->displayData(colIndices);
            }
            return;
        }

        // With WHERE
        cout << "\nTable: " << tableName << endl;
        cout << "--------------------------------------" << endl;

        const vector<Column>& tableCols = table->getColumns();
        vector<int> displayCols;

        if (columns.empty()) {
            for (int i = 0; i < (int)tableCols.size(); i++) {
                displayCols.push_back(i);
            }
        }
        else {
            for (int i = 0; i < (int)columns.size(); i++) {
                int idx = table->getColumnIndex(columns[i]);
                if (idx == -1) {
                    cout << "Error: Column '" << columns[i] << "' does not exist!" << endl;
                    return;
                }
                displayCols.push_back(idx);
            }
        }

        // headers
        for (int i = 0; i < (int)displayCols.size(); i++) {
            cout << tableCols[displayCols[i]].getName();
            if (i < (int)displayCols.size() - 1) cout << " | ";
        }
        cout << endl;

        for (int i = 0; i < (int)displayCols.size(); i++) {
            cout << "----------";
            if (i < (int)displayCols.size() - 1) cout << "-+-";
        }
        cout << endl;

        int count = 0;
        const vector<Row>& rows = table->getRows();

        for (int r = 0; r < (int)rows.size(); r++) {
            const Row& row = rows[r];
            bool matchesAll = true;

            for (int c = 0; c < (int)conditions.size(); c++) {
                const Condition& cond = conditions[c];
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
                for (int i = 0; i < (int)displayCols.size(); i++) {
                    cout << row.getValue(displayCols[i]);
                    if (i < (int)displayCols.size() - 1) cout << " | ";
                }
                cout << endl;
                count++;
            }
        }

        if (count == 0) {
            cout << "No matching rows found." << endl;
        }

        cout << "\nRows returned: " << count << endl;

    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::deleteFrom(const string& query) {
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

        cout << "[" << deletedCount << "] Row(s) deleted from '"
            << tableName << "'!" << endl;

    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::updateTable(const string& query) {
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
        map<string, string>::const_iterator it;
        for (it = updates.begin(); it != updates.end(); ++it) {
            const string& colName = it->first;
            const string& value = it->second;

            int colIndex = table->getColumnIndex(colName);
            if (colIndex == -1) {
                cout << "Error: Column '" << colName << "' does not exist!" << endl;
                return;
            }

            const Column& col = table->getColumns()[colIndex];

            if (col.getType() == INT && !isValidInt(value)) {
                cout << "Error: Invalid INT value for column '" << colName << "'" << endl;
                return;
            }
            if (col.getType() == FLOAT && !isValidFloat(value)) {
                cout << "Error: Invalid FLOAT value for column '" << colName << "'" << endl;
                return;
            }
        }

        int updatedCount = table->updateRows(updates, conditions);

        cout << "[" << updatedCount << "] Row(s) updated in '"
            << tableName << "'!" << endl;

    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::dropTable(const string& query) {
    try {
        string tableName = QueryParser::parseDropTable(query);

        if (tables.find(tableName) == tables.end()) {
            cout << "Error: Table '" << tableName << "' does not exist!" << endl;
            return;
        }

        delete tables[tableName];
        
        tables.erase(tableName);

        cout << "Table '" << tableName << "' dropped successfully!" << endl;
    }
    catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}

void DatabaseEngine::listTables() {
    if (tables.empty()) {
        cout << "No tables in database." << endl;
        return;
    }

    cout << "Tables in database:" << endl;

    map<string, Table*>::iterator it;
    for (it = tables.begin(); it != tables.end(); ++it) {
        cout << "  - " << it->first << " ("
            << it->second->getRowCount() << " rows)" << endl;
    }
}

void DatabaseEngine::saveToDisk(const string& filename) const {
    ofstream out(filename.c_str());
    if (!out) {
        cout << "Warning: Could not open '" << filename << "' for saving.\n";
        return;
    }

    // Number of tables
    out << tables.size() << '\n';

    map<string, Table*>::const_iterator it;
    for (it = tables.begin(); it != tables.end(); ++it) {
        Table* t = it->second;

        out << "TABLE\n";
        out << t->getTableName() << '\n';

        // Columns
        const vector<Column>& cols = t->getColumns();
        out << cols.size() << '\n';

        for (size_t i = 0; i < cols.size(); ++i) {
            const Column& col = cols[i];

            out << col.getName() << '\n';
            out << (int)col.getType() << ' '
                << col.getSize() << ' '
                << (col.getIsPrimaryKey() ? 1 : 0) << ' '
                << (col.getIsNotNull() ? 1 : 0) << '\n';
        }

        // Rows
        const vector<Row>& rows = t->getRows();
        out << rows.size() << '\n';

        for (size_t r = 0; r < rows.size(); ++r) {
            const Row& row = rows[r];

            int valueCount = row.getValueCount();
            out << valueCount << '\n';

            for (int v = 0; v < valueCount; ++v) {
                out << row.getValue(v) << '\n';
            }
        }
    }
}

void DatabaseEngine::loadFromDisk(const string& filename) {
    ifstream in(filename.c_str());
    if (!in) {
        return; // No DB file, first run
    }

    // Clear old tables
    map<string, Table*>::iterator itold;
    for (itold = tables.begin(); itold != tables.end(); ++itold) {
        delete itold->second;
    }
    tables.clear();

    string line;
    if (!getline(in, line)) return;
    if (line.empty()) return;

    int tableCount = stoi(line);

    for (int ti = 0; ti < tableCount; ++ti) {
        string marker;
        if (!getline(in, marker)) break;
        if (marker != "TABLE") break;

        string tableName;
        if (!getline(in, tableName)) break;

        if (!getline(in, line)) break;
        int colCount = stoi(line);

        Table* table = new Table(tableName);

        // Columns
        for (int ci = 0; ci < colCount; ++ci) {
            string colName;
            if (!getline(in, colName)) { delete table; return; }

            if (!getline(in, line)) { delete table; return; }

            istringstream iss(line);
            int typeInt, size, pk, nn;
            iss >> typeInt >> size >> pk >> nn;

            DataType dt = (DataType)typeInt;
            Column col(colName, dt, size, pk != 0, nn != 0);
            table->addColumn(col);
        }

        // Rows
        if (!getline(in, line)) { delete table; return; }
        int rowCount = stoi(line);

        for (int ri = 0; ri < rowCount; ++ri) {
            if (!getline(in, line)) { delete table; return; }

            int valueCount = stoi(line);
            Row row;

            for (int vi = 0; vi < valueCount; ++vi) {
                string val;
                if (!getline(in, val)) { delete table; return; }
                row.addValue(val);
            }

            table->addRow(row);
        }

        tables[tableName] = table;
    }

    cout << "Database loaded from '" << filename << "'.\n";
}
