#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>

// Алфавіт для кодування Base64 та спецсимволи
const std::string base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';
const std::string comment_symbol = "-";

// Функція для кодування рядка в форматі Base64 з урахуванням вказаних умов
std::string base64_encode(const std::string& input) {
    std::string output;
    int val = 0, valb = -6;
    int char_count = 0;

    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(base64_alphabet[(val >> valb) & 0x3F]);
            valb -= 6;
            char_count++;
            if (char_count == 76) {
                output.push_back('\n');
                char_count = 0;
            }
        }
    }

    if (valb > -6) {
        output.push_back(base64_alphabet[((val << 8) >> (valb + 8)) & 0x3F]);
        char_count++;
    }

    if (char_count > 0) {
        while (char_count < 76) {
            output.push_back('=');
            char_count++;
        }
        output.push_back('\n');
    }

    return output;
}

// Функція для кодування вмісту файлу та збереження результату
void base64EncodeFile(const std::string& inputFileName, const std::string& outputFileName = "") {
    // Отримання вмісту файлу у бінарному режимі
    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cout << "Unable to open input file\n";
        return;
    }

    std::ostringstream contents;
    contents << inputFile.rdbuf();
    std::string encoded = base64_encode(contents.str());

    // Визначення імені вихідного файлу
    std::string finalOutputFileName;
    if (outputFileName.empty()) {
        finalOutputFileName = inputFileName + ".base64";
    }
    else {
        finalOutputFileName = outputFileName;
    }

    // Відкриття файлу для запису у текстовому режимі
    std::ofstream outputFile(finalOutputFileName);
    if (!outputFile.is_open()) {
        std::cout << "Unable to create/open output file\n";
        return;
    }

    // Запис закодованого вмісту у вихідний файл
    outputFile << encoded;
    std::cout << "Encoded successfully and saved to " << finalOutputFileName << "\n";
    outputFile.close();
}

bool isBase64(char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(const std::string& input) {
    std::string decodedText;
    size_t i = 0;
    int val = 0, valb = -8;

    for (char c : input) {
        if (c == '=') {
            break;
        }

        if (!isalnum(c) || (c == '+') || (c == '/')) {
            continue;
        }

        val = (val << 6) | base64_alphabet.find(c);
        valb += 6;

        if (valb >= 0) {
            decodedText.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decodedText;
}

// Функція для декодування вмісту файлу та збереження результату
void base64DecodeFile(const std::string& inputFileName, const std::string& outputFileName = "") {
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cout << "Unable to open input file\n";
        return;
    }

    std::ostringstream contents;
    contents << inputFile.rdbuf();
    std::string inputText = contents.str();

    const int MAX_LINE_LENGTH = 76;
    std::istringstream iss(inputText);
    std::string line;
    int line_number = 1;
    size_t total_lines = std::count(inputText.begin(), inputText.end(), '\n') + 1;
    bool found_last_line = false;

    std::string txtOutputFileName;
    if (outputFileName.empty()) {
        size_t extensionPos = inputFileName.find_last_of('.');
        if (extensionPos != std::string::npos) {
            std::string inputExtension = inputFileName.substr(extensionPos);
            if (inputExtension == ".base64") {
                txtOutputFileName = inputFileName.substr(0, extensionPos) + ".txt";
            }
            else {
                txtOutputFileName = inputFileName + ".txt";
            }
        }
        else {
            txtOutputFileName = inputFileName + ".txt";
        }
    }
    else {
        txtOutputFileName = outputFileName;
    }

    std::ofstream outputFile(txtOutputFileName);
    if (!outputFile.is_open()) {
        std::cout << "Unable to create/open output file\n";
        return;
    }

    while (std::getline(iss, line)) {
        if (line.front() == '-') {
            continue;
        }
        //перевірка довжини рядка
        if (line.length() != MAX_LINE_LENGTH) {
            std::cout << "Error: Invalid line length in line " << line_number << " (" << line.length() << " characters)\n";
            outputFile.close();
            std::remove(txtOutputFileName.c_str()); // видаляємо файл, щоб уникнути пустого результату
            return;
        }
        //перевіряємо кожен символ у кожному рядку
        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            // Перевірка чи символ належить алфавіту base64
            if (base64_alphabet.find(c) == std::string::npos && c != padding_char) {
                std::cout << "Error: Invalid character '" << c << "' in line " << line_number << " at position " << i + 1 << "\n";
                outputFile.close();
                std::remove(txtOutputFileName.c_str());
                return;
            }

        }
        //перевірка використання паддінгу
        if (line_number == total_lines) {
            size_t padding_count = std::count(line.begin(), line.end(), '=');
            if (padding_count > 0) {
                size_t last_eq_pos = line.find_last_of('=');
                if (last_eq_pos != std::string::npos && last_eq_pos != MAX_LINE_LENGTH - 1) {
                    std::cout << "Error: Improper padding usage in the last line\n";
                    outputFile.close();
                    std::remove(txtOutputFileName.c_str());
                    return;
                }
                found_last_line = true;
            }
        }

        ++line_number;
    }
    //перевірка на наявність рядків після закінчення файлу для декодування
    if (found_last_line && line_number < total_lines) {
        std::cout << "Error: Data found after the end of the message\n";
        outputFile.close();
        std::remove(txtOutputFileName.c_str());
        return;
    }

    outputFile << base64_decode(inputText);
    std::cout << "Decoded successfully and saved to " << txtOutputFileName << "\n";
    outputFile.close();
}

int main() {
    char func_choose;
    std::cout << "Choose function: [E]ncode / [D]ecode: ";
    std::cin >> func_choose;
    func_choose = std::toupper(func_choose);

    if (func_choose == 'E') {
        std::string inputFileName, outputFileName;
        std::cout << "Name of input file: ";
        std::cin >> inputFileName;

        std::cout << "Do you want to enter a custom output file name? (y/n): ";
        char choice;
        std::cin >> choice;

        if (std::tolower(choice) == 'y') {
            std::cout << "Enter the custom output file name (without extension): ";
            std::cin >> outputFileName;

            // Додати розширення .base64, якщо воно відсутнє
            if (outputFileName.find('.') == std::string::npos) {
                outputFileName += ".base64";
            }
        }
        else {
            size_t extensionPos = inputFileName.find_last_of('.');
            if (extensionPos != std::string::npos) {
                outputFileName = inputFileName.substr(0, extensionPos) + ".base64";
            }
            else {
                outputFileName = inputFileName + ".base64";
            }
        }
        base64EncodeFile(inputFileName, outputFileName);
    }
    else if (func_choose == 'D') {
        std::string inputFileName, outputFileName;
        std::cout << "Name of input file: ";
        std::cin >> inputFileName;
        std::cout << "Do you want to enter a custom output file name? (y/n): ";
        char choice;
        std::cin >> choice;

        if (std::tolower(choice) == 'y') {
            std::cout << "Enter the custom output file name (without extension): ";
            std::cin >> outputFileName;

            // Додати розширення .txt, якщо воно відсутнє
            if (outputFileName.find('.') == std::string::npos) {
                outputFileName += ".txt";
            }
        }
        else {
            size_t extensionPos = inputFileName.find_last_of('.');
            if (extensionPos != std::string::npos) {
                outputFileName = inputFileName.substr(0, extensionPos);
            }
            else {
                outputFileName = inputFileName;
            }
        }
        base64DecodeFile(inputFileName, outputFileName);
    }
    else {
        std::cout << "Input error. Please, try again.\n";
        return 1;
    }

    return 0;
}