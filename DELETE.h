#pragma once
#include <iostream>
#include <fstream>
#include "NODE.h"
#include "INSERT.h"

using namespace std;

bool EXISTCOL(const string& tableName, const string& columnName, Node* Tablehead);
bool CORWHERE(istringstream& iss2, string& table, string& column, string& value, const string& tableName, const TableJson& json_table);
bool DELROWS(const string& tableName, const string& column, const string& value, const TableJson& json_table);
void DELETE(const string& command, const TableJson& json_table) ;