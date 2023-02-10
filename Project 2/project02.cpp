#include "project02.h"

int main(int argc, char *argv[]){
    if (argc != 2)
    {
        std::cout << "Usage:\n    " << argv[0] << " trace_file" << std::endl;
        return EXIT_SUCCESS;
    }
    // open the input file as binary
    std::ifstream ifile(argv[1], std::ios::in | std::ios::binary);
    if (!ifile.is_open())
    {
        std::cerr << "Error opening file \"" << argv[1] << "\"" << std::endl;
        return EXIT_FAILURE;
    }

    Simple_Pipe simple_pipe;
    simple_pipe.simulate(ifile);
    simple_pipe.print_result();

    ifile.close();
    return 0;
}

/* fetch stage reads the instruction from given file,
   if the pipeline is stalled it does nothing and
   if the end of file is found it returns true */
bool Simple_Pipe::fetch(std::ifstream &ifile, bool stalled)
{
    // every instruction is 4 bytes
    char instruction[4];

    fetch_inst = INVALID_INST;    // initialize to invalid instruction
    if (!ifile.eof() && !stalled)
    {
        ifile.read(instruction, 4);      // read instruction
        if (ifile)
        {
            // save it as a 32 bit integer
            fetch_inst = ((unsigned)(instruction[3]) << 24) |
                            (((unsigned)(instruction[2]) & 0xFF) << 16) |
                            (((unsigned)(instruction[1]) & 0xFF) << 8) |
                            ((unsigned)(instruction[0]) & 0xFF);
            log << "[1] fetching: " << std::hex << fetch_inst << std::endl;
            log << std::dec;
            request_get++;
        }
        else 
            return true;    // eof
    }
    else if (!stalled)
        return true;    // return true to indicate eof
    return false;
}

/* decode stage, if the pipeline is stalled it does nothing
   else decodes the instruction and returns true if not error,
   and returns false otherwise */
bool Simple_Pipe::decode(bool stalled)
{
    if (decode_inst == INVALID_INST)     // if the pipeline is empty
        return false;
    else if (stalled)
    {
        log << "[2] decode stall " << std::endl;
        return false;
    }
    log << "[2] decoding: " << std::hex << decode_inst << std::endl;
    log << std::dec;

    decode_opcode = decode_inst >> 24;  // top byte is opcode
    switch (decode_opcode)
    {
    case SET: case ADDI: case SUBI: case MULI: case DIVI:
    case ADD: case SUB: case MUL: case DIV:
        break;
    default:
        std::cerr << "ERROR: Invalid instruction" << std::endl;
        return false;
    }
    decode_dest = (decode_inst >> 16) & 0xFF;  // next byte is destination
    if (decode_dest >= REG_COUNT)
    {
        std::cerr << "ERROR: Invalid destination register" << std::endl;
        return true;
    }

    // next byte is register or immediate operand
    unsigned char op1 = decode_inst >> 8;
    if (decode_opcode == SET)
        decode_op1 = op1;
    else if (op1 >= 0 && op1 < REG_COUNT)
        decode_op1 = registers[op1];
    else
    {
        std::cerr << "ERROR: Invalid left operand register" << std::endl;
        return true;
    }

    // lowest byte can be register or immediate
    unsigned char op2 = decode_inst;
    switch (decode_opcode)
    {
    // if immediate
    case SET:
        decode_op2 = 0;
        break;
    case ADDI: case SUBI: case MULI: case DIVI:
        decode_op2 = op2;
        break;
    // if register
    default:
        if (op2 >= 0 && op2 < REG_COUNT)
            decode_op2 = registers[op2];
        else
        {
            std::cerr << "ERROR: Invalid right operand register" << std::endl;
            return true;
        }
    }
    return false; // no error
}

/* execute stage, if the pipeline is stalled it updates the current cycle for the stalling instruction,
   if the instruction needs more cycles then returns true in order to stall the pipeline,
   otherwise executes instruction and returns false to indicate no stall */
bool Simple_Pipe::execute(bool stalled)
{
    // execution
    if (execute_inst == INVALID_INST)     // if not empty
        return false;
    if (!stalled)
    {
        execute_curr_cycle = 1;
        // determine cycles required for instruction
        switch (execute_opcode)
        {
        case SET: case ADD: case ADDI: case SUB: case SUBI:
            execute_cycles = 1;
            break;
        case MUL: case MULI:
            execute_cycles = 2;
            break;
        case DIV: case DIVI:
            execute_cycles = 4;
            break;
        }
    }
    else
        execute_curr_cycle++;   // if stalled, increment cycles used
    log << "[3] executing: " << std::hex << execute_inst;
    log << " current cycle: " << std::dec << execute_curr_cycle;
    log << " cycle needed: " << std::dec << execute_cycles << std::endl;
    // if waited number of cycles, execute instruction
    if (execute_curr_cycle == execute_cycles)
    {
        switch (execute_opcode)
        {
        case SET:
            registers[execute_dest] = execute_op1;
            break;
        case ADD: case ADDI:
            registers[execute_dest] = execute_op1 + execute_op2;
            break;
        case SUB: case SUBI:
            registers[execute_dest] = execute_op1 - execute_op2;
            break;
        case MUL: case MULI:
            registers[execute_dest] = execute_op1 * execute_op2;
            break;
        case DIV: case DIVI:
            registers[execute_dest] = execute_op1 / execute_op2;
            break;
        }
        request_done++;
        return false;   // not stalled anymore, instruction has executed
    }
    else
        return true;    // instruction is executing, stall for another cycle
}

// simulate the pipeline execution using the instructions loaded from the given file
void Simple_Pipe::simulate(std::ifstream &ifile) {
    bool halt = false;
    bool stalled = false;
    bool error;

    // clear all registers
    for(int i = 0; i < REG_COUNT; i++){
        registers[i] = 0;
    }
    // invalidate all stages
    fetch_inst = INVALID_INST;
    decode_inst = INVALID_INST;
    execute_inst = INVALID_INST;

    execution_time = 0;
    request_get = 0;
    request_done = 0;

    log.open("logfile.txt", std::ios::out);
    if (!log.is_open())
    {
        std::cerr << "Error creating log file" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!halt)   // repeat until halted
    {
        log << "\nexecution time: " << std::dec << execution_time << std::endl;

        // execute
        stalled = execute(stalled);

        // decode
        error = decode(stalled);

        // fetch instruction
        fetch(ifile, stalled);

        // advance pipeline
        if (!stalled)
        {
            execute_inst = decode_inst;
            execute_opcode = decode_opcode;
            execute_dest = decode_dest;
            execute_op1 = decode_op1;
            execute_op2 = decode_op2;
            decode_inst = fetch_inst;
        }

        // if all stages are empty, halt simulation
        if (fetch_inst == INVALID_INST && decode_inst == INVALID_INST &&
                execute_inst == INVALID_INST)
            halt = true;
        if (error)
            halt = true;
        execution_time++;
    }
    log << "\nTotal instruction received: " << request_get << std::endl;
    log << "Total instruction handled: " << request_done << std::endl;
    print_result(log);
    log.close();
}

// print registers
void Simple_Pipe::print_regs(std::ostream &out){
    out << "\nRegisters: \n";
    out << "----------------------------------------" << std::endl;
    for(int i = 0; i < REG_COUNT; i+=2){
        std::string regl("R");
        regl.append(std::to_string(i));
        regl.append(": ");

        std::string regr("R");
        regr.append(std::to_string(i+1));
        regr.append(": ");
        if(i < 15){
            out << "  " << std::setiosflags(std::ios::left)
            << std::setw(5) << regl  << std::setw(10) << registers[i] << " |   "
            << std::setw(5) << regr << std::setw(10) << registers[i+1] << std::endl;
            out << "----------------------------------------" << std::endl;
        }else{
            out << "  " << std::setiosflags(std::ios::left)
            << std::setw(5) << regl << std::setw(10) << registers[i] << " |   " << std::endl;
            out << "----------------------------------------" << std::endl;
        }
    }
    out << std::endl;
}

// print result
void Simple_Pipe::print_result(std::ostream &out) {
    print_regs(out);
    out << "Total execution cycles: " << execution_time << std::endl;
    out << "\nIPC: " << (request_done/(double)execution_time) << std::endl << std::endl;
}

// print the output results to the screen
void Simple_Pipe::print_result() {
    print_result(std::cout);
}
