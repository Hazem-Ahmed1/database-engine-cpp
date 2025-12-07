#include "Column.h"
#include <string>

using namespace std;

Column::Column(string n, DataType t, int s, bool pk, bool nn)
    : name(n), type(t), size(s), isPrimaryKey(pk), isNotNull(nn) {
}

string Column::getName() const {
    return name;
}

DataType Column::getType() const {
    return type;
}

int Column::getSize() const {
    return size;
}

bool Column::getIsPrimaryKey() const {
    return isPrimaryKey;
}

bool Column::getIsNotNull() const {
    return isNotNull;
}

void Column::setPrimaryKey(bool pk) {
    isPrimaryKey = pk;
}

void Column::setNotNull(bool nn) {
    isNotNull = nn;
}

string Column::getTypeString() const {
    switch (type) {
    case INT:
        return "INT";
    case FLOAT:
        return "FLOAT";
    case VARCHAR:
        return "VARCHAR(" + to_string(size) + ")";
    default:
        return "UNKNOWN";
    }
}

string Column::getFullDefinition() const {
    string def = name + " " + getTypeString();
    if (isPrimaryKey) def += " PRIMARY KEY";
    if (isNotNull)    def += " NOT NULL";
    return def;
}
