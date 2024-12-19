#pragma once
#include <iostream>
#include "NODE.h"
#include "DELETE.h"
#include "INSERT.h"


using namespace std;


void SELECT(const string& query, const TableJson& json_table);
bool ISSAME(const TableJson& json_table, const string& table1, const string& table2, const string& column1, const string& column2);
bool ISVALUE(const TableJson& json_table, const string& table, const string& column, const string& s);
void CROSSJOIN(const TableJson& json_table, const string& table1, const string& table2, const string& column1, const string& column2);
bool ISDOT(const string& indication);
string DELQUOTES(const string& indication);
void SEPARDOT(const string& word, string& table, string& column, const TableJson& json_table);