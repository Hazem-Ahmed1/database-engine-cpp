#ifndef COLUMN_H
#define COLUMN_H

#include <string>

enum DataType {
    INT,
    FLOAT,
    VARCHAR
};

class Column {
private:
    std::string name;
    DataType type;
    int size;
    bool isPrimaryKey;
    bool isNotNull;

public:
    Column(std::string n, DataType t, int s = 0, bool pk = false, bool nn = false);

    std::string getName() const;
    DataType getType() const;
    int getSize() const;
    bool getIsPrimaryKey() const;
    bool getIsNotNull() const;

    void setPrimaryKey(bool pk);
    void setNotNull(bool nn);

    std::string getTypeString() const;
    std::string getFullDefinition() const;
};

#endif
