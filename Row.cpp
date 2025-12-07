#include "Row.h"


using namespace std;

Row::Row() {
}

void Row::addValue(const string& value) {
    values.push_back(value);
}

vector<string>& Row::getValues() {
    return values;
}

const vector<string>& Row::getValues() const {
    return values;
}

string Row::getValue(int index) const {
    if (index >= 0 && index < (int)values.size()) {
        return values[index];
    }
    return "";
}

void Row::setValue(int index, const string& value) {
    if (index >= 0 && index < (int)values.size()) {
        values[index] = value;
    }
}

int Row::getValueCount() const {
    return (int)values.size();
}
