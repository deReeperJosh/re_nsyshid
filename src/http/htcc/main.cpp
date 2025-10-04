
#include <cstring>
#include <fstream>

#include "TemplateProcessor.h"

int main(int argc, char const *argv[]) {
    if (argc != 2 && argc != 3) {
        puts("Usage: htcc <input file> [output file]");
        exit(EXIT_FAILURE);
    }

    std::ifstream infile(argv[1]);

    if (!infile.is_open()) {
        perror("Could not open input file");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(argv[1]);

    std::string className;
    className.reserve(len);

    size_t start = 0;

    for (size_t i = 0; i < len; i++)
        if (argv[1][i] == '/')
            start = i + 1;

    if (start >= len) {
        puts("Invalid input file name");
        exit(EXIT_FAILURE);
    }

    bool uppercaseFlag = true;
    for (size_t i = start; i < len; i++) {
        char c = argv[1][i];

        if (c == '.') {
            if (strstr(&argv[1][i + 1], ".") == NULL)
                break;

            uppercaseFlag = true;
            continue;
        }

        if (isspace(c)) {
            uppercaseFlag = true;
            continue;
        }

        if (!isalnum(c) && c != '_')
            continue;

        if (i == 0 && isdigit(c))
            className += '_';

        if (isalpha(c) && uppercaseFlag) {
            uppercaseFlag = false;
            className += (char) toupper(c);
            continue;
        }

        className += c;
    }

    className += "Template";

    if (argc == 3) {
        std::ofstream outfile(argv[2]);

        if (!outfile.is_open()) {
            perror("Could not open input file");
            exit(EXIT_FAILURE);
        }

        TemplateProcessor tp(infile, outfile, className);
        tp.process();
        return 0;
    }

    TemplateProcessor tp(infile, std::cout, className);
    tp.process();

    return 0;
}
