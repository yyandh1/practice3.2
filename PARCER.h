#pragma once
#include "NODE.h" // структура таблиц
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem> // директории
#include "json.hpp" // json

using namespace std;
using json = nlohmann::json; 
namespace fs = filesystem;


void DELDIR(const fs::path& directoryPath); // удаление директории
void CREATEDIR(const fs::path& SchemePath, const json& structure, TableJson& json_table); // создание полной директории и файлов
void PARCER(TableJson& json_table); // парсинг схемы