#include <iostream>
#include <string>

using ts = std::string; // ts (ToString) is much shorter

// private functions:
static void print(const std::string &text);
static void print_begin();
static void print_end();
static void print_line(const std::string &text);
static void print_line_begin(const std::string &text);
static void print_line_end(const std::string &text);

// function implementations:
int main(int argc, char **argv)
{
    print_begin();
    print_line("Asymptotical Regularization in Computer Tomographie");
    print_line("Example: Schlieren Imaging");
    print_end();

    return 0;
}

void print_begin()
{
    std::cout << "****" << std::endl;
}

void print_end()
{
    std::cout << "****" << std::endl << std::endl;
}

void print_line(const std::string &text)
{
    print_line_begin(text);
    std::cout << std::endl;
}

void print_line_begin(const std::string &text)
{
    std::cout << "** " << text;
}

void print_line_end(const std::string &text)
{
    std::cout << " " << text << std::endl;
}
