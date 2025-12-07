#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <map>

using namespace std;   // ✔ You asked for this

#include "Column.h"
#include "Row.h"
#include "Condition.h"

class Table {
private:
    string tableName;
    vector<Column> columns;
    vector<Row> rows;
    int primaryKeyIndex;

public:
    Table(string name);

    void addColumn(const Column& col);
    void addRow(const Row& row);

    string getTableName() const;
    vector<Column>& getColumns();
    const vector<Column>& getColumns() const;

    vector<Row>& getRows();
    const vector<Row>& getRows() const;

    int getColumnCount() const;
    int getRowCount() const;
    int getPrimaryKeyIndex() const;

    int getColumnIndex(const string& colName) const;
    bool hasPrimaryKey(const string& value) const;

    void display() const;
    void displayData(const vector<int>& columnIndices = vector<int>()) const;

    int deleteRows(const vector<Condition>& conditions);
    int updateRows(const map<string, string>& updates,
        const vector<Condition>& conditions);
};

#endif
