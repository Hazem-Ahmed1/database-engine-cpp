# 🚀 C++ OOP DBMS Engine

A fully functional **mini DBMS (Database Management System)** built using **Object-Oriented Programming in C++**, capable of handling table creation, data insertion, selection, updating, deletion, and SQL-like query parsing — all in memory.

## 📌 Features

### ✅ SQL-like Query Support

The engine supports these commands:
- CREATE DATABASE
- LIST DATABASES
- DROP DATABASE
- USE DATABASE
- CREATE TABLE
- INSERT INTO
- SELECT
- UPDATE
- DELETE
- LIST TABLES
- EXIT

## 🧱 Supported Data Types

- INT
- FLOAT
- VARCHAR(size)
- VARCHAR

## 🔐 Column Constraints

- PRIMARY KEY — ensures uniqueness
- NOT NULL — disallows empty values

## 🔍 WHERE Clause Support

Operators:

=, !=, <, >, <=, >=

Allows filtered queries such as:

SELECT \* FROM users WHERE age > 20

## 🛠️ Installation & Running

### 1️⃣ Compile

g++ -std=c++17 main.cpp -o dbms

### 2️⃣ Run

./dbms

## 📝 Example Queries

-  CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL, age INT)
-  INSERT INTO users VALUES (1, John, 25)
-  INSERT INTO users VALUES (2, Sarah, 30)
-  SELECT _ FROM users
-  SELECT name, age FROM users WHERE age > 25
-  UPDATE users SET age = 26 WHERE id = 1
-  DELETE FROM users WHERE age < 30 AND name="Sarah"
-  DELETE _ FROM users
-  LIST TABLES

## 🧩 Project Architecture Overview

DatabaseEngine -> QueryParser -> Table -> (Columns, Rows) -> Condition/Row

## 🎯 Demonstrated Concepts

- OOP Principles
- SQL-like query parsing
- Validation for INT, FLOAT, VARCHAR
- Constraints handling
- In-memory storage
- Error handling


