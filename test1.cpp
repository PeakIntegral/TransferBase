// Octavian Game Art Test Task
// Система передачи файлов: реализация клиента и сервера

#include <iostream> 
#include <fstream> 
#include <cstring> // Для функций работы со строками, таких как strcmp
#include <ctime> // Для получения текущей временной метки
#include <unistd.h> // Для функций POSIX API
#include <arpa/inet.h> // Для структур и функций сокетного программирования

#define PORT 8080 // Номер порта, на котором будет слушать сервер
#define BUFFER_SIZE 1024 // Размер буфера, используемого для передачи данных

// Прототипы функций
void runServer(); // Реализация логики сервера
void runClient(const char* filePath); // Реализация логики клиента
std::string getCurrentTimestamp(); // Генерации строки с временной меткой

int main(int argc, char* argv[]) {

    if (argc < 2) { 
        std::cerr << "Select run mode (-c for client, -s for server)"; 
        return 1; 
    }

    if (std::strcmp(argv[1], "-s") == 0) { 
      runServer();
    }
      
    else if (std::strcmp(argv[1], "-c") == 0) { 
      
      if (argc < 3) { 
        std::cerr << "Error: File path or server IP required for client mode not specified\n";
        return 1; 
      }
      
      runClient(argv[2]); 
    } 
    else { 
      std::cerr << "Invalid argument. Use -s for server or -c for client.\n"; 
      return 1; 
    }

    return 0; 
}

void runServer() {
    // Режим сервера: эта функция реализует серверную логику, которая ожидает соединений
    // и принимает файлы от клиентов.
    int serverSocket, newSocket; // Сокеты для прослушивания и принятия соединений
    struct sockaddr_in serverAddr, clientAddr; // Структуры для адресов сервера и клиента
    socklen_t addrLen = sizeof(clientAddr); // Длина структуры адреса клиента
    char buffer[BUFFER_SIZE] = {0}; // Буфер для хранения входящих данных

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { // Создание сокета
        perror("Socket creation failed"); // Вывод сообщения об ошибке, если сокет не удалось создать
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    serverAddr.sin_family = AF_INET; // Установка семейства адресов в IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Привязка ко всем доступным сетевым интерфейсам
    serverAddr.sin_port = htons(PORT); // Установка номера порта (преобразованного в сетевой порядок байтов)

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { // Привязка сокета к адресу
        perror("Binding failed"); // Вывод сообщения об ошибке, если привязка не удалась
        close(serverSocket); // Закрытие сокета
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    if (listen(serverSocket, 5) < 0) { // Начало прослушивания соединений (максимальная очередь 5)
        perror("Listening failed"); // Вывод сообщения об ошибке, если прослушивание не удалось
        close(serverSocket); // Закрытие сокета
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    std::cout << "Server is running and waiting for connections...\n"; // Уведомление о запуске сервера

    while (true) { // Бесконечный цикл для принятия нескольких подключений от клиентов
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen)) < 0) { // Принятие нового подключения
            perror("Connection acceptance failed"); // Вывод сообщения об ошибке, если принятие не удалось
            continue; // Переход к следующей итерации
        }

        std::cout << "Connection established with client. Receiving file...\n"; // Уведомление о новом соединении

        std::ofstream outFile(getCurrentTimestamp() + ".hex", std::ios::binary); // Открытие файла для сохранения полученных данных
        if (!outFile) { // Проверка, удалось ли открыть файл
            std::cerr << "Error creating output file.\n"; // Вывод сообщения об ошибке
            close(newSocket); // Закрытие клиентского сокета
            continue; // Переход к следующей итерации
        }

        ssize_t bytesRead; // Переменная для хранения количества прочитанных байтов
        while ((bytesRead = read(newSocket, buffer, BUFFER_SIZE)) > 0) { // Чтение данных от клиента
            outFile.write(buffer, bytesRead); // Запись данных в файл
        }

        std::cout << "File received successfully.\n"; // Уведомление об успешном приеме файла
        outFile.close(); // Закрытие файла
        close(newSocket); // Закрытие клиентского сокета
    }

    close(serverSocket); // Закрытие серверного сокета
}

void runClient(const char* filePath) {
    // Режим клиента: эта функция реализует логику клиента, который подключается к серверу
    // и отправляет указанный файл.
    int clientSocket; // Сокет для подключения к серверу
    struct sockaddr_in serverAddr; // Структура для адреса сервера
    char buffer[BUFFER_SIZE] = {0}; // Буфер для хранения данных файла

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Создание сокета
        perror("Socket creation failed"); // Вывод сообщения об ошибке, если сокет не удалось создать
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    serverAddr.sin_family = AF_INET; // Установка семейства адресов в IPv4
    serverAddr.sin_port = htons(PORT); // Установка номера порта (преобразованного в сетевой порядок байтов)

    if (inet_pton(AF_INET, "192.168.0.86", &serverAddr.sin_addr) <= 0) { // Преобразование IP-адреса в двоичный вид
        perror("Invalid address or address not supported"); // Вывод сообщения об ошибке, если преобразование не удалось
        close(clientSocket); // Закрытие сокета
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { // Подключение к серверу
        perror("Connection to server failed"); // Вывод сообщения об ошибке, если подключение не удалось
        close(clientSocket); // Закрытие сокета
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    std::cout << "Connected to server. Sending file...\n"; // Уведомление об успешном подключении

    std::ifstream inFile(filePath, std::ios::binary); // Открытие файла для отправки
    if (!inFile) { // Проверка, удалось ли открыть файл
        std::cerr << "Error: File not found.\n"; // Вывод сообщения об ошибке
        close(clientSocket); // Закрытие сокета
        exit(EXIT_FAILURE); // Выход с кодом ошибки
    }

    while (inFile.read(buffer, BUFFER_SIZE)) { // Чтение данных из файла
        send(clientSocket, buffer, inFile.gcount(), 0); // Отправка данных на сервер
    }

    // Отправка оставшихся байтов, если они есть
    if (inFile.gcount() > 0) { // Проверка на наличие оставшихся данных
        send(clientSocket, buffer, inFile.gcount(), 0); // Отправка оставшихся данных
    }

    std::cout << "File sent successfully.\n"; // Уведомление об успешной отправке файла
    inFile.close(); // Закрытие файла
    close(clientSocket); // Закрытие сокета
}

std::string getCurrentTimestamp() {
    // Формат: YYYYMMDD_HHMMSS.
    std::time_t now = std::time(nullptr); 
    char buf[20]; 
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&now)); // Форматирование временной метки
    return std::string(buf); // Возврат форматированной временной метки как строки
}
