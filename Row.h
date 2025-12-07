#ifndef ROW_H
#define ROW_H

#include <string>
#include <vector>
using namespace std;

class Row {
private:
    vector<string> values;

public:
    Row();

    void addValue(const string& value);

    vector<string>& getValues();
    const vector<string>& getValues() const;

    string getValue(int index) const;
    void setValue(int index, const string& value);

    int getValueCount() const;
};

#endif
