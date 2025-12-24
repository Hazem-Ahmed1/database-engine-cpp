#include "Table.h"
#include <iostream>

using namespace std;

Table::Table(string name)
    : tableName(name), primaryKeyIndex(-1) {
}

void Table::addColumn(const Column& col) {
    if (col.getIsPrimaryKey()) {
        primaryKeyIndex = (int)columns.size();
    }
    columns.push_back(col);
}

void Table::addRow(const Row& row) {
    rows.push_back(row);
}

string Table::getTableName() const {
    return tableName;
}

vector<Column>& Table::getColumns() {
    return columns;
}

const vector<Column>& Table::getColumns() const {
    return columns;
}

vector<Row>& Table::getRows() {
    return rows;
}

const vector<Row>& Table::getRows() const {
    return rows;
}

int Table::getColumnCount() const {
    return (int)columns.size();
}

int Table::getRowCount() const {
    return (int)rows.size();
}

int Table::getPrimaryKeyIndex() const {
    return primaryKeyIndex;
}

#include <algorithm>
#include <cctype>

// helper for case insensitive string compare
bool iequals(const string& a, const string& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

int Table::getColumnIndex(const string& colName) const {
    for (int i = 0; i < (int)columns.size(); i++) {
        if (iequals(columns[i].getName(), colName)) {
            return i;
        }
    }
    return -1;
}

bool Table::hasPrimaryKey(const string& value) const {
    if (primaryKeyIndex == -1) return false;

    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i].getValue(primaryKeyIndex) == value) {
            return true;
        }
    }
    return false;
}

void Table::display() const {
    cout << "Table '" << tableName << "' created successfully!" << endl;
    cout << "Columns:" << endl;
    for (int i = 0; i < (int)columns.size(); i++) {
        cout << "  - " << columns[i].getFullDefinition() << endl;
    }
}

void Table::displayData(const vector<int>& columnIndices) const {
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
    }
    else {
        for (int r = 0; r < (int)rows.size(); r++) {
            const Row& row = rows[r];
            for (int i = 0; i < (int)displayCols.size(); i++) {
                cout << row.getValue(displayCols[i]);
                if (i < (int)displayCols.size() - 1) cout << " | ";
            }
            cout << endl;
        }
    }

    cout << "\nTotal rows: " << rows.size() << endl;
}

int Table::deleteRows(const vector<Condition>& conditions) {
    if (conditions.empty()) {
        // DELETE * (all rows)
        int count = (int)rows.size();
        rows.clear();
        return count;
    }

    vector<Row> newRows;
    int deletedCount = 0;

    for (int r = 0; r < (int)rows.size(); r++) {
        const Row& row = rows[r];
        bool matchesAll = true;

        for (int c = 0; c < (int)conditions.size(); c++) {
            const Condition& cond = conditions[c];
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
        }
        else {
            deletedCount++;
        }
    }

    rows = newRows;
    return deletedCount;
}

int Table::updateRows(const map<string, string>& updates,
    const vector<Condition>& conditions) {
    int updatedCount = 0;

    for (int r = 0; r < (int)rows.size(); r++) {
        Row& row = rows[r];
        bool matchesAll = true;

        // Check if row matches all conditions
        for (int c = 0; c < (int)conditions.size(); c++) {
            const Condition& cond = conditions[c];
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
            map<string, string>::const_iterator it;
            for (it = updates.begin(); it != updates.end(); ++it) {
                int colIndex = getColumnIndex(it->first);
                if (colIndex != -1) {
                    row.setValue(colIndex, it->second);
                }
            }
            updatedCount++;
        }
    }

    return updatedCount;
}
