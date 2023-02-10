#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#define REG_COUNT 16
#define INVALID_REG 17

// instruction opcodes

#define SET  0x00
#define ADD  0x10
#define ADDI 0x11
#define SUB  0x20
#define SUBI 0x21
#define MUL  0x30
#define MULI 0x31
#define DIV  0x40
#define DIVI 0x41
#define INVALID_INST 0xFFFFFFFF

unsigned long long execution_time;
unsigned long long request_get;
unsigned long long request_done;
unsigned long long line_num;

class Simple_Pipe{
    std::ofstream log;
    // fetch stage data
    unsigned int fetch_inst;
    // decode stage data
    unsigned int decode_inst;
    unsigned int decode_opcode;
    unsigned int decode_dest;
    int decode_op1;
    int decode_op2;
    // execute stage data
    unsigned int execute_inst;
    unsigned int execute_opcode;
    unsigned int execute_dest;
    int execute_op1;
    int execute_op2;
    int execute_cycles;
    int execute_curr_cycle;

    bool fetch(std::ifstream &ifile, bool stalled);
    bool decode(bool stalled);
    bool execute(bool stalled);
    
    void print_regs(std::ostream &out);
    void print_result(std::ostream &out);
public:
    int registers[16];
    void simulate(std::ifstream &ifile);
    void print_result();
};
