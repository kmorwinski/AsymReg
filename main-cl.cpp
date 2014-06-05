#include <iostream>
#include <fstream>
#include <string>

#include "asymreg.h"
#include "duration.h"

#define DATA_FILE  "../data/recang-testdata-10x10.csv" // TODO: read from QSettings? or from argv?

using ts = std::string; // ts (ToString) is much shorter

// private functions:
static inline void print(const std::string &text = "");
static inline void print_begin();
static inline void print_end();
static inline void print_line(const std::string &text = "");
static inline void print_line_begin(const std::string &text = "");
static inline void print_line_end(const std::string &text = "");
static void set_fpu (unsigned int mode);

// function implementations:
int main(int argc, char **argv)
{
    //set_fpu(0x270); // use double-precision rounding
    std::cout << std::fixed; // write floating-point values in fixed-point notation

    print_begin();
    print_line("Asymptotical Regularization in Computer Tomographie");
    print_line("Example: Schlieren Imaging");
    print_end();
/* -------------------------------------------------------------------- */
    print_begin();
    print_line_begin(ts("Importing data from file: \"") + DATA_FILE);

    MatrixXd zMat(ASYMREG_DATSRC_SIZE, ASYMREG_DATSRC_SIZE);
    std::fstream fs;
    fs.open(DATA_FILE, std::fstream::in);
    if (fs.is_open()) {
        fs >> zMat.format2(EIGEN_IOFMT_CSV);
        print_line_end("\" ok!");
    } else {
        zMat.setZero();
        print_line_end("\" failed!");
    }

    std::cout << "source data =" << std::endl
              << zMat << std::endl << std::endl;
    print_end();

    AsymReg::createSourceFunction(zMat);
/* -------------------------------------------------------------------- */
    print_begin();
    print_line("Generating Schlieren Data Sets...");

    Duration dt;
    AsymReg::generateDataSet(&dt);

    print_line_begin("...done (time used: ");
    std::cout << dt.value() << dt.unit() << ").";
    print_line_end();
    print_end();
/* -------------------------------------------------------------------- */
    print_begin();
    print_line("Running Asymptotical Regularization...");

    Duration dt2;
    AsymReg::regularize(&dt2);

    print_line_begin("...done (time used: ");
    std::cout << dt2.value() << dt2.unit() << ").";
    print_line_end();
    print_end();
/* -------------------------------------------------------------------- */
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
    print_line_end();
}

void print_line_begin(const std::string &text)
{
    std::cout << "** " << text;
}

void print_line_end(const std::string &text)
{
    if (!text.empty())
        std::cout << " " << text;
    std::cout << std::endl;
}

void set_fpu (unsigned int mode)
{
    asm("fldcw %0" : : "m" (*&mode));
}
