#include <iostream>
#include <fstream>
#include <string>

#include "asymreg.h"

#define DATA_FILE  "../data/data-11x11.csv"

using namespace Eigen;
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

    print_begin();
    print_line_begin(ts("Loading data from file: \"") + DATA_FILE);

    MatrixXd zMat(ASYMREG_DATSRC_SIZE, ASYMREG_DATSRC_SIZE);
    std::fstream fs;
    fs.open(DATA_FILE, std::fstream::in);
    if (fs.is_open()) {
        fs >> zMat;
        print_line_end("\" ok!");
    } else {
        zMat.setZero();
        print_line_end("\" failed!");
    }

    print_line("source data: ");
    std::cout << zMat << std::endl;
    print_end();

    AsymReg::createSourceFunction(zMat);

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
