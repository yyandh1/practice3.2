#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

// Функция для работы с сервером (отправка команд и получение ответов)
void communicateWithServer(int clientSocket) {
    string command;
    
    while (true) {
        // Ввод команды от пользователя
        cout << "Введите команду (INSERT, DELETE, SELECT, EXIT): ";
        getline(cin, command);

        // Проверка команды на завершение работы
        if (command == "EXIT") {
            send(clientSocket, "EXIT", 4, 0);
            cout << "Закрытие соединения с сервером.\n";
            break;
        }
        if (command.empty()) continue;

        // Отправляем команду на сервер
        send(clientSocket, command.c_str(), command.length(), 0);

        // Получение ответа от сервера
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            cout << "Ответ сервера: " << buffer << endl;
        } else if (bytesRead == 0) {
            cout << "Соединение с сервером закрыто.\n";
            break;
        } else {
            cerr << "Ошибка при получении данных от сервера.\n";
        }
    }

    // Закрытие соединения с сервером
    close(clientSocket);
}

int main() {
    // Создание клиентского сокета
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Не удалось создать сокет.\n";
        return 1;
    }

    // Настройка адреса и порта сервера
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // протокол IPv4
    serverAddr.sin_port = htons(7432);  // Порт сервера 7432
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Адрес сервера (localhost)

    // Подключение клиента к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Ошибка подключения к серверу.\n";
        close(clientSocket);
        return 1;
    }

    pid_t pid = fork();  // Создаем новый процесс для взаимодействия с сервером

    if (pid == 0) {
        // Дочерний процесс: обрабатывает обмен данными с сервером
        communicateWithServer(clientSocket);
        exit(0);  // Завершаем дочерний процесс
    } else if (pid > 0) {
        // Родительский процесс: ожидает завершения дочернего процесса
        waitpid(pid, nullptr, 0);  // Ожидаем завершения дочернего процесса
    } else {
        cerr << "Ошибка при создании процесса.\n";
        close(clientSocket);
        return 1;
    }

    // Закрытие соединения с сервером в родительском процессе
    close(clientSocket);
    return 0;
}
