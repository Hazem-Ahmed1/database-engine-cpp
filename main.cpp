#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

#include "Column.h"
#include "Row.h"
#include "Table.h"
#include "Condition.h"
#include "QueryParser.h"
#include "DatabaseEngine.h"

#include <sys/stat.h>
#include <direct.h>   // _mkdir

using namespace std;

// ================== DB FOLDER HELPERS ==================

const string BASE_DB_FOLDER = "databases";

bool directoryExists(const string& path) {
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) {
        return false; // cannot access
    }
    return (info.st_mode & _S_IFDIR) != 0;
}

void createDirectoryIfNotExists(const string& path) {
    if (!directoryExists(path)) {
        _mkdir(path.c_str());
    }
}

string trimString(const string& str) {
    int first = 0;
    int last = (int)str.size() - 1;

    while (first <= last && isspace((unsigned char)str[first])) first++;
    while (last >= first && isspace((unsigned char)str[last])) last--;

    if (first > last) return "";
    return str.substr(first, last - first + 1);
}

string getDatabaseFolder(const string& dbName) {
    return BASE_DB_FOLDER + "\\" + dbName;
}

string getDatabaseFile(const string& dbName) {
    return getDatabaseFolder(dbName) + "\\database.db";
}

// ================== HELP TEXT ==================

void printHelp() {
    cout << "\nSupported commands:" << endl;
    cout << "  CREATE DATABASE db_name" << endl;
    cout << "  USE db_name" << endl;
    cout << "  CREATE TABLE table_name (col1 type1 [PRIMARY KEY] [NOT NULL], ...)" << endl;
    cout << "  INSERT INTO table_name VALUES (val1, val2, ...)" << endl;
    cout << "  SELECT * FROM table_name [WHERE condition]" << endl;
    cout << "  SELECT col1, col2 FROM table_name [WHERE condition]" << endl;
    cout << "  UPDATE table_name SET col1=val1, col2=val2 [WHERE condition]" << endl;
    cout << "  DELETE FROM table_name" << endl;
    cout << "  DELETE FROM table_name WHERE condition" << endl;
    cout << "  LIST TABLES" << endl;
    cout << "  HELP" << endl;
    cout << "  EXIT" << endl;
    cout << "\nSupported types: INT, FLOAT, VARCHAR(size)" << endl;
    cout << "Supported operators in WHERE: =, !=, <, >, <=, >=" << endl;
}

// ================== MAIN ==================

int main() {
    DatabaseEngine db;

    // default database
    string currentDatabase = "master";

    // make sure base folder and master DB folder exist
    createDirectoryIfNotExists(BASE_DB_FOLDER);
    createDirectoryIfNotExists(getDatabaseFolder(currentDatabase));

    // load master database if file exists
    db.loadFromDisk(getDatabaseFile(currentDatabase));

    printHelp();
    cout << "\nExamples:" << endl;
    cout << "  CREATE DATABASE shop" << endl;
    cout << "  USE shop" << endl;
    cout << "  CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL, age INT)" << endl;
    cout << "  INSERT INTO users VALUES (1, John, 25)" << endl;
    cout << "  SELECT * FROM users" << endl;
    cout << "  DELETE FROM users WHERE age < 30" << endl;
    cout << endl;

    string line;
    bool shouldExit = false;

    while (!shouldExit) {
        cout << "dbms[" << currentDatabase << "]> ";
        getline(cin, line);

        if (line.empty()) continue;

        // -------- split input line into commands by ';' --------
        vector<string> commands;
        string temp;

        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == ';') {
                string cmd = trimString(temp);
                if (!cmd.empty()) commands.push_back(cmd);
                temp.clear();
            }
            else {
                temp += c;
            }
        }
        // last part (no trailing ;)
        temp = trimString(temp);
        if (!temp.empty()) commands.push_back(temp);

        if (commands.empty()) continue;

        // -------- process each command separately --------
        for (size_t ci = 0; ci < commands.size(); ++ci) {
            string query = commands[ci];

            // build uppercase version for matching
            string upperQuery = query;
            transform(upperQuery.begin(), upperQuery.end(),
                upperQuery.begin(), ::toupper);

            if (upperQuery == "EXIT" || upperQuery == "QUIT") {
                db.saveToDisk(getDatabaseFile(currentDatabase));
                cout << "Goodbye!" << endl;
                shouldExit = true;
                break; // break command loop
            }
            else if (upperQuery.find("CREATE DATABASE") == 0) {
                string dbName = query.substr(15);      // after "CREATE DATABASE"
                dbName = trimString(dbName);

                if (dbName.empty()) {
                    cout << "Error: Database name is required." << endl;
                }
                else {
                    string folder = getDatabaseFolder(dbName);
                    if (directoryExists(folder)) {
                        cout << "Error: Database '" << dbName << "' already exists." << endl;
                    }
                    else {
                        createDirectoryIfNotExists(folder);
                        cout << "Database '" << dbName
                            << "' created in folder '" << folder << "'." << endl;
                    }
                }
            }
            else if (upperQuery.find("USE ") == 0) {
                string dbName = query.substr(4);       // after "USE "
                dbName = trimString(dbName);

                if (dbName.empty()) {
                    cout << "Error: Database name is required." << endl;
                }
                else {
                    string folder = getDatabaseFolder(dbName);
                    if (!directoryExists(folder)) {
                        cout << "Error: Database '" << dbName << "' does not exist." << endl;
                    }
                    else {
                        // save current DB before switching
                        db.saveToDisk(getDatabaseFile(currentDatabase));
                        currentDatabase = dbName;
                        db.loadFromDisk(getDatabaseFile(currentDatabase));
                        cout << "Switched to database '" << currentDatabase << "'." << endl;
                    }
                }
            }
            else if (upperQuery == "LIST TABLES") {
                db.listTables();
            }
            else if (upperQuery.find("CREATE TABLE") == 0) {
                db.createTable(query);
                db.saveToDisk(getDatabaseFile(currentDatabase));
            }
            else if (upperQuery.find("INSERT INTO") == 0) {
                db.insertInto(query);
                db.saveToDisk(getDatabaseFile(currentDatabase));
            }
            else if (upperQuery.find("SELECT") == 0) {
                db.selectFrom(query);
            }
            else if (upperQuery.find("UPDATE") == 0) {
                db.updateTable(query);
                db.saveToDisk(getDatabaseFile(currentDatabase));
            }
            else if (upperQuery.find("DELETE") == 0) {
                db.deleteFrom(query);
                db.saveToDisk(getDatabaseFile(currentDatabase));
            }
            else if (upperQuery == "HELP") {
                printHelp();
            }
            else {
                cout << "Unknown command: " << query
                    << "\nType 'HELP' for a list of commands or 'EXIT' to quit." << endl;
            }

            cout << endl;
        }
    }

    return 0;
}
