#include "PARCER.h"
#include "INSERT.h"
#include "DELETE.h"
#include "SELECT.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>  // Для функции waitpid
#include <pthread.h>  // Для мьютекса
#include <semaphore.h>

using namespace std;

sem_t db_mutex;  // Семофор для блокировки доступа к базе данных
int activeClients = 0;  // Счетчик активных клиентов

// Функция для обработки команд клиента
void commandClient(int clientSocket, TableJson& json_table, struct sockaddr_in clientAddr) {
    // Получаем IP-адрес и порт клиента
    string clientIp = inet_ntoa(clientAddr.sin_addr);
    int clientPort = ntohs(clientAddr.sin_port);
  

    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break; // Прерываем цикл, если клиент отключился или ошибка

        // Обработка команды
        cout << "Получена команда от клиента с IP: " << clientIp << ": " << clientPort << endl;

        string command(buffer, bytesReceived);
        istringstream iss(command);
        string word;
        iss >> word;
        string response;  // Переменная для ответа, который будет отправлен клиенту

        // Блокировка на время выполнения команды с использованием семафора
        sem_wait(&db_mutex); // Ожидание освобождения семафора
        if (word == "INSERT") {
             cout << "Обработка INSERT команды...\n";
            INSERT(command, json_table);
            response = "INSERT команда обработана.\n";  // Ответ для клиента
        } else if (word == "DELETE") {
             cout << "Обработка DELETE команды...\n";
            DELETE(command, json_table);
            response = "DELETE команда обработана.\n";  // Ответ для клиента
        } else if (word == "SELECT") {
           cout << "Обработка SELECT команды...\n";
            SELECT(command, json_table);
            response = "SELECT команда обработана.\n";  // Ответ для клиента
        } else if (word == "EXIT") {
            response = "Закрытие соединения.\n";  // Ответ перед закрытием соединения
            send(clientSocket, response.c_str(), response.length(), 0);  // Отправляем ответ клиенту
            close(clientSocket);  // Закрытие соединения
            sem_post(&db_mutex); // Освобождение семафора перед выходом
            break;
        } else {
            cerr << "Некорректная команда от клиента с IP: " << clientIp << ": " << clientPort << ": " << command << endl;  // Неизвестная команда
            response = "Ошибка: Некорректная команда.\n";  // Ответ о неправильной команде
        }
        sem_post(&db_mutex); // Освобождение семафора после выполнения команды

        send(clientSocket, response.c_str(), response.length(), 0); // Отправляем подтверждение выполнения
    }

    cout << "Клиент с IP: " << clientIp << ": " << clientPort << " отключился.\n";
    // Закрытие сокета клиента
    close(clientSocket);

    // Уменьшаем количество активных клиентов после завершения работы с клиентом
    activeClients--;

    // Если больше нет активных клиентов, завершаем сервер
    if (activeClients == 0) {
        cout << "Все клиенты отключены. Завершение работы сервера.\n";
        exit(0);
    }
}

// Основная функция - инициализация и настройка сервера
int main() {
    TableJson json_table;
    PARCER(json_table);
    string command;    
    cout << "\n\n";
    sem_init(&db_mutex, 0, 1);  // 1 - начальное значение (разрешен один процесс)

    // Создание серверного сокета
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM - используем протокол TCP
    if (serverSocket < 0) {
        cerr << "Ошибка при создании сокета.\n";
        return 1;
    }

    // Настройка адреса и порта
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // семейство адресов IPv4
    serverAddr.sin_port = htons(7432);  // Порт 7432
    serverAddr.sin_addr.s_addr = INADDR_ANY; // IPv4

    // Привязка сокета к адресу
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Не удалось выполнить привязку.\n";
        close(serverSocket);
        return 1;
    }

    // Подготовка сокета к прослушиванию входящих соединений
    if (listen(serverSocket, 2) < 0) {
        cerr << "Не удалось начать прослушивание порта.\n";
        close(serverSocket);
        return 1;
    }

    cout << "Сервер запущен с порта 7432. Ожидает клиентов...\n";

    // Ожидание подключения клиентов и создание процесса для каждого
    while (true) {

        struct sockaddr_in clientAddr;  // Объявляем здесь, чтобы использовать везде в цикле
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);  // Передаем указатель на clientAddr
        if (clientSocket < 0) {
            cerr << "Ошибка при принятии подключения клиента.\n";
            continue;
        }

        // Увеличиваем счетчик активных клиентов только в родительском процессе
        activeClients++;

        // Выводим информацию о подключении клиента
        cout << "Подключен клиент с IP: " << inet_ntoa(clientAddr.sin_addr) << ": " << ntohs(clientAddr.sin_port) << ". Активных клиентов: " << activeClients << endl;

        pid_t pid = fork();  // Создаем новый процесс

        if (pid == 0) {
            // Дочерний процесс
            close(serverSocket);  // Закрываем серверный сокет в дочернем процессе
            commandClient(clientSocket, json_table, clientAddr);  // Обрабатываем команды клиента
            close(clientSocket);  // Закрытие сокета клиента
            exit(0);  // Завершаем дочерний процесс
        } else if (pid > 0) {
            // Родительский процесс продолжает слушать входящие соединения
            close(clientSocket);  // Закрываем клиентский сокет в родительском процессе
        } else {
            cerr << "Ошибка при создании процесса.\n";
        }

        // Ожидание завершения всех дочерних процессов
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            // Ожидаем завершение процесса
        }

        // Если больше нет активных клиентов, завершаем сервер
        if (activeClients == 0) {
            cout << "Все клиенты отключены. Завершение работы сервера.\n";
            break;  // Завершаем цикл и завершаем сервер
        }
    }

    // Закрытие серверного сокета
    close(serverSocket);

    // Уничтожение семафора перед завершением
    sem_destroy(&db_mutex);

    return 0;

}