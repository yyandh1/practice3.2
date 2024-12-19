#include "PARCER.h"


void DELDIR(const fs::path& directoryPath) { // удаление директории
    if (fs::exists(directoryPath)) { // проверка, существует ли она вообще
        fs::remove_all(directoryPath); // удаление
    }
}
//Ваш код для функции CREAREDIR создает директории и файлы на основе структуры данных, представленной в формате JSON
void CREATEDIR(const fs::path& SchemePath, const json& structure, TableJson& json_table) {
    Node* tableHead = nullptr;
    Node* tableTail = nullptr;

    for (const auto& table : structure.items()) {
        fs::path tablePath = SchemePath / table.key();
        if (!fs::create_directory(tablePath)) {
            cerr << "Не удалось создать директорию: " << tablePath << endl;
            return;
        }
        cout << "Создана директория: " << tablePath << endl;

        // Создаем таблицу
        Node* newTable = new Node{table.key(), nullptr, nullptr};

        // Создаем файл блокировки
        string lockFileName = table.key() + "_lock.txt";
        ofstream lockFile(tablePath / lockFileName);
        if (!lockFile.is_open()) {
            cerr << "Не удалось открыть файл: " << lockFileName << endl;
            return;
        }
        lockFile << "unlocked";
        lockFile.close();

        // Добавляем таблицу в список
        if (tableHead == nullptr) {
            tableHead = newTable;
            tableTail = newTable;
        } else {
            tableTail->next = newTable;
            tableTail = newTable;
        }

        // Создаем колонку pk
        string keyColumn = table.key() + "_pk";
        ListNode* column_pk = new ListNode{keyColumn, nullptr};
        newTable->column = column_pk;

        // Создаем CSV файл
        fs::path csvFilePath = tablePath / "1.csv";
        ofstream csvFile(csvFilePath);
        if (!csvFile.is_open()) {
            cerr << "Не удалось создать файл: " << csvFilePath << endl;
            return;
        }
        csvFile << keyColumn << ",";
        
        // Записываем названия колонок в CSV и добавляем их в список
        const auto& columns = table.value();
        for (size_t i = 0; i < columns.size(); ++i) {
            string columnName = columns[i].get<string>();
            csvFile << columnName;

            // Добавляем колонку в список
            ListNode* newColumn = new ListNode{columnName, nullptr};

            if (newTable->column == nullptr) {
                newTable->column = newColumn;
            } else {
                ListNode* lastColumn = newTable->column;
                while (lastColumn->next != nullptr) {
                    lastColumn = lastColumn->next;
                }
                lastColumn->next = newColumn;
            }

            // Не ставим запятую после последней колонки
            if (i < columns.size() - 1) {
                csvFile << ",";
            }
        }

        csvFile << endl;
        csvFile.close();
        cout << "Создан файл: " << csvFilePath << endl;

        // Создаем файл для первичного ключа
        string pkFileName = keyColumn + "_sequence.txt";
        ofstream pkFile(tablePath / pkFileName);
        if (!pkFile.is_open()) {
            cerr << "Не удалось открыть файл: " << pkFileName << endl;
            return;
        }
        pkFile << "0"; // начальный первичный ключ
        pkFile.close();
    }

    json_table.Tablehead = tableHead;
}

//код для функции PARSER отвечает за чтение JSON-файла, извлечение информации и создание структуры данных на основе этого файла.
void PARCER(TableJson& json_table) {
    string fileName = "SHOP.json";
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл " << fileName << endl;
        return;
    }

    string json_include;
    string line;
    while (getline(file, line)) {
        json_include += line;
    }
    file.close();

    json parser_Json = json::parse(json_include);

    if (parser_Json.contains("name")) {
        json_table.Name = parser_Json["name"];
    } else {
        cerr << "Ошибка: 'name' не найден в схеме.\n";
        return;
    }

    fs::path schemePath = fs::current_path() / json_table.Name; // Путь к директории схемы
    cout << "Текущая директория: " << fs::current_path() << endl;
    cout << "Имя схемы: " << json_table.Name << endl;

    // Если схема существует, удаляем её
    if (fs::exists(schemePath)) {
        cout << "Удаляем старую директорию: " << schemePath << endl;
        DELDIR(schemePath);
    }

    // Создаем директорию для схемы
    if (!fs::create_directory(schemePath)) {
        cerr << "Не удалось создать директорию: " << schemePath << endl;
        cout << "Ошибка: " << strerror(errno) << endl;
        return;
    }
    cout << "Создана директория: " << schemePath << endl;

    if (parser_Json.contains("structure")) {
        CREATEDIR(schemePath, parser_Json["structure"], json_table);
    }

    if (parser_Json.contains("tuples_limit")) {
        json_table.TableSize = parser_Json["tuples_limit"];
    } else {
        cerr << "Ошибка: 'tuples_limit' не найден в схеме.\n";
        return;
    }
}