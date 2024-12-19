#include "INSERT.h"
//функция EXISTTAB, которая проверяет, существует ли таблица с заданным именем в связном списке таблиц.
bool EXISTTAB(const string& tableName, Node* tableHead) {
    Node* current = tableHead;
    while (current) {
        if (current->table == tableName) {
            return true;
        }
        current = current->next;
    }
    return false;
}
//Функция которая проверяет состояние таблицы
bool ISLOCK(const string& tableName, const string& schemeName) {
    string baseDir = "/home/yyandh1/localrepos1/practice3.1/" + schemeName + "/" + tableName;
    string lockFile = baseDir + "/" + (tableName + "_lock.txt");

    if (!fs::exists(lockFile)) {
        cerr << "Ошибка: файл блокировки не существует: " << lockFile << ".\n";
        return false;
    }

    ifstream file(lockFile);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл блокировки: " << lockFile << ".\n";
        return false;
    }

    string current;
    file >> current;
    file.close();
    return current == "locked";
}
//Функция, которая запирает или отпирает таблицу в зависимости от ее состояния
void LOCK(const string& tableName, const string& schemeName) {
    string baseDir = "/home/yyandh1/localrepos1/practice3.1/" + schemeName + "/" + tableName;
    string lockFile = baseDir + "/" + (tableName + "_lock.txt");

    if (!fs::exists(lockFile)) {
        cerr << "Ошибка: файл блокировки не существует: " << lockFile << "\n";
        return;
    }

    ifstream fileIn(lockFile);
    if (!fileIn.is_open()) {
        cerr << "Не удалось открыть файл блокировки: " << lockFile << "\n";
        return;
    }

    string current;
    fileIn >> current;
    fileIn.close();

    ofstream fileOut(lockFile);
    if (!fileOut.is_open()) {
        cerr << "Не удалось открыть файл для записи блокировки: " << lockFile << "\n";
        return;
    }

    fileOut << (current == "locked" ? "unlocked" : "locked");
    fileOut.close();
}

// Функция для копирования названий колонок из одного файла в другой
void COPYNAMECOL(const string& from_file, const string& to_file) {
    string columns;
    ifstream fileF(from_file); // открываем файл для чтения колонок
    if (!fileF.is_open()) {
        cerr << "Не удалось открыть файл: " << from_file << "\n";
        return;
    }
    fileF >> columns;
    fileF.close();

    ofstream fileT(to_file); // открываем файл для записи колонок
    if (!fileT.is_open()) {
        cerr << "Не удалось открыть файл: " << to_file << "\n";
        return;
    }
    fileT << columns << endl;
    fileT.close();
}
//Эта функция подсчитывает количество существующих CSV файлов для заданной таблицы.
int CSVCOUNT(const TableJson& json_table, const string& tableName) {
    int csvCount = 0;
    int csvNumber = 1;

    while (true) {
        string csvFile = "/home/yyandh1/localrepos1/practice3.1/" + json_table.Name + "/" + tableName + "/" + (to_string(csvNumber) + ".csv");

        // Проверяем, существует ли файл
        ifstream fileIn(csvFile);
        if (!fileIn.is_open()) {
            // Файл не существует, выходим из цикла, так как дальше файлов нет
            break;
        }
        fileIn.close();  // Закрываем файл после проверки

        // Увеличиваем счётчик найденных файлов
        csvCount++;
        
        // Переходим к следующему файлу
        csvNumber++;
    }

    // Возвращаем общее количество существующих файлов
    return csvCount;
}
//Эта функция создает новый CSV файл, если текущий достиг максимального количества строк.
void CSVNEW(const string& baseDir, const string& tableName, int& csvNumber, const TableJson& tableJson) {
    // Получаем максимальное количество строк на файл из структуры TableJson
    int maxRowsPerFile = tableJson.TableSize;
    
    // Формируем путь к текущему CSV файлу
    string csvFile = baseDir + "/" + tableName + "/" + to_string(csvNumber) + ".csv";
    
    // Проверяем количество строк в текущем файле
    rapidcsv::Document doc(csvFile);
    if (doc.GetRowCount() >= maxRowsPerFile) {
        // Если достигнут лимит строк, увеличиваем номер файла
        csvNumber++;
        csvFile = baseDir + "/" + tableName + "/" + to_string(csvNumber) + ".csv";
    }

    // Если файла нет, создаём его
    if (!fs::exists(csvFile)) {
        // Создаём новый файл и копируем в него названия колонок
        string csvFirst = baseDir + "/" + tableName + "/1.csv";
        COPYNAMECOL(csvFirst, csvFile);
    }
}
//Функция обрабатывающая SQL-запросы и заполняющая таблицы в форма CSV
void INSERT(const string& command, TableJson json_table) {
    istringstream iss(command);
    string slovo;
    iss >> slovo >> slovo;

    if (slovo != "INTO") {
        cerr << "Некорректная команда.\n";
        return;
    }

    string tableName;
    iss >> tableName;
    if (!EXISTTAB(tableName, json_table.Tablehead)) {
        cerr << "Такой таблицы нет.\n";
        return;
    }

    iss >> slovo;
    if (slovo != "VALUES") {
        cerr << "Некорректная команда.\n";
        return;
    }

    string values;
    while (iss >> slovo) {
        values += slovo;
    }

    if (values.front() != '(' || values.back() != ')') {
        cerr << "Некорректная команда.\n";
        return;
    }

    if (ISLOCK(tableName, json_table.Name)) {
        cerr << "Таблица заблокирована.\n";
        return;
    }

    LOCK(tableName, json_table.Name);

    int currentPK;
    string PKFile = "/home/yyandh1/localrepos1/practice3.1/" + json_table.Name + "/" + tableName + "/" + (tableName + "_pk_sequence.txt");
    ifstream fileIn(PKFile);
    if (!fileIn.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }

    fileIn >> currentPK;
    fileIn.close();

    ofstream fileOut(PKFile);
    if (!fileOut.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    currentPK++;
    fileOut << currentPK;
    fileOut.close();

    // Логика для определения количества существующих файлов
    int csvNumber = CSVCOUNT(json_table, tableName);

    string baseDir = "/home/yyandh1/localrepos1/practice3.1/" + json_table.Name;

    // Используем новую функцию для создания нового CSV файла, если нужно
    CSVNEW(baseDir, tableName, csvNumber, json_table);

    string csvSecond = baseDir + "/" + tableName + "/" + to_string(csvNumber) + ".csv";

    // Открываем CSV файл для записи
    ofstream csv(csvSecond, ios::app);
    if (!csv.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }

    // Записываем данные в CSV файл
    csv << currentPK << ",";
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == '\'') {
            i++;
            while (i < values.size() && values[i] != '\'') {
                csv << values[i++];
            }
            if (i + 1 < values.size() && values[i + 1] != ')') {
                csv << ",";
            } else {
                csv << endl;
            }
        }
    }

    csv.close();
    LOCK(tableName, json_table.Name);
}