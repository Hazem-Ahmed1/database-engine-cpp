#include "Condition.h"
#include <cstdlib>  

using namespace std;

Condition::Condition(string col, string operation, string val)
    : columnName(col), op(operation), value(val) {
}

bool Condition::evaluate(const string& actualValue, DataType type) const {

    if (type == INT || type == FLOAT) {
        double actual = atof(actualValue.c_str());
        double expected = atof(value.c_str());

        if (op == "=")  return actual == expected;
        if (op == "!=") return actual != expected;
        if (op == "<")  return actual < expected;
        if (op == ">")  return actual > expected;
        if (op == "<=") return actual <= expected;
        if (op == ">=") return actual >= expected;

        return false;
    }

    if (op == "=")  return actualValue == value;
    if (op == "!=") return actualValue != value;
    if (op == "<")  return actualValue < value;
    if (op == ">")  return actualValue > value;
    if (op == "<=") return actualValue <= value;
    if (op == ">=") return actualValue >= value;

    return false;
}
