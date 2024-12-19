#pragma once
#include <iostream>
#include <filesystem>
#include "rapidcsv.h" 
#include "NODE.h"

using namespace std;
namespace fs = filesystem;

bool EXISTTAB(const string& tableName, Node* tableHead);
bool ISLOCK(const string& tableName, const string& schemeName);
void COPYNAMECOL(const string& from_file, const string& to_file);
void LOCK(const string& tableName, const string& schemeName);
int CSVCOUNT(const TableJson& json_table, const string& tableName);
void CSVNEW(const std::string& baseDir, const std::string& tableName, int& csvNumber, const TableJson& tableJson);
void INSERT(const string& command, TableJson json_table);