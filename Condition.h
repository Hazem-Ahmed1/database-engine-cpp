#ifndef CONDITION_H
#define CONDITION_H

#include <string>
using namespace std;

#include "Column.h"

class Condition {
public:
    string columnName;
    string op;
    string value;

    Condition(string col, string operation, string val);

    bool evaluate(const string& actualValue, DataType type) const;
};

#endif
