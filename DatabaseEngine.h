#ifndef DATABASEENGINE_H
#define DATABASEENGINE_H

#include <string>
#include <map>
#include <cstdlib> 

using namespace std;

class Table;

class DatabaseEngine {
private:
    map<string, Table*> tables;

    bool isValidInt(const string& str);
    bool isValidFloat(const string& str);

public:
    DatabaseEngine();
    ~DatabaseEngine();

    void createTable(const string& query);
    void insertInto(const string& query);
    void selectFrom(const string& query);
    void deleteFrom(const string& query);
    void updateTable(const string& query);
    void listTables();

    void saveToDisk(const string& filename = "database.db") const;
    void loadFromDisk(const string& filename = "database.db");
};

#endif
