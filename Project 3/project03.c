#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum OPCODE {BUBBLE, SET, ADD, SUB, MUL, DIV, LOAD, STORE, BRANCH, RET} OPCODE;

#define IF          0
#define ID          1
#define IA          2
#define EX1         3
#define BR          4
#define EX2         5
#define MEM1        6
#define MEM2        7
#define WB          8

#define NORMAL      0
#define EMPTY       1
#define BUBBLE      2
#define SPECIAL     3
#define WAIT        4
#define IDLE        5

#define VALID           0
#define WRITING         1
#define SP_WRITING      2

#define STAGE_ITEMS_COUNT   9

typedef struct INS {
    OPCODE type;
    int opCode;     // OPCode
    int Rx;         // Destination R index
    int RyOrValue;  // Left R index or Value
    int RzOrValue;  // Right R index or Value
    int PC;
} INS;

int n = 0;

typedef struct PipeLineStatus
{
    int status;     //0:instruction empty 1:bubble 2:stall 3:instruction
    INS instruction;
} PipeLineStatus;


void parseINS(unsigned char *codeMap, int PC, INS *ins)
{
    memset(ins, 0, sizeof(INS));
    ins->opCode = codeMap[PC+3];
    ins->Rx = codeMap[PC+2];
    ins->RyOrValue = codeMap[PC+1];
    ins->RzOrValue = codeMap[PC+0];

    if(ins->opCode == 0x00) // set Rx, #Imm
    {
        ins->type = SET;
        ins->RyOrValue = codeMap[PC+1] * 16 * 16 + codeMap[PC+0];
    }
    else if(ins->opCode == 0x10) // add Rx, Ry, Rz
        ins->type = ADD;
    else if(ins->opCode == 0x11) // add Rx, Ry, #Imm
        ins->type = ADD;
    else if(ins->opCode == 0x20) // sub Rx, Ry, Rz
        ins->type = SUB;
    else if(ins->opCode == 0x21) // sub Rx, Ry, #Imm
        ins->type = SUB;
    else if(ins->opCode == 0x30) // mul Rx, Ry, Rz
        ins->type = MUL;
    else if(ins->opCode == 0x31) // mul Rx, Ry, #Imm
        ins->type = MUL;
    else if(ins->opCode == 0x40) // div Rx, Ry, Rz
        ins->type = DIV;
    else if(ins->opCode == 0x41) // div Rx, Ry, #Imm
        ins->type = DIV;
    else if(ins->opCode == 0x50) // ld Rx, #Addr
        ins->type = LOAD;
    else if(ins->opCode == 0x51) // ld Rx, Rz
        ins->type = LOAD;
    else if(ins->opCode == 0x60) // st Ry, #Addr
        ins->type = STORE;
    else if(ins->opCode == 0x61) // st Ry, Rz
        ins->type = STORE;
    else if(ins->opCode == 0x70) // bez Ry, #Imm
    {
        ins->type = BRANCH;
        ins->RyOrValue = codeMap[PC+1] + codeMap[PC+0];
    }
    else if(ins->opCode == 0x71) // bgez Ry, #Imm
    {
        ins->type = BRANCH;
        ins->RyOrValue = codeMap[PC+1] + codeMap[PC+0];
    }
    else if(ins->opCode == 0x72) // blez Ry, #Imm
    {
        ins->type = BRANCH;
        ins->RyOrValue = codeMap[PC+1] + codeMap[PC+0];
    }
    else if(ins->opCode == 0x73) // bgtz Ry, #Imm
    {
        ins->type = BRANCH;
        ins->RyOrValue = codeMap[PC+1] + codeMap[PC+0];
    }
    else if(ins->opCode == 0x74) // bltz Ry, #Imm
    {
        ins->type = BRANCH;
        ins->RyOrValue = codeMap[PC+1] + codeMap[PC+0];
    }
    else if(ins->opCode == 0xFF) // ret
        ins->type = RET;
    else
        ins->type = BUBBLE;
}

void printINS(int i,int PC, INS *ins)
{
    char *stage;
    switch(i)
    {
        case IF:
            stage = "IF";
            break;
        case ID:
            stage = "ID";
            break;
        case IA:
            stage = "IA";
            break;
        case EX1:
            stage = "EX1";
            break;
        case BR:
            stage = "BR";
            break;
        case EX2:
            stage = "EX2";
            break;
        case MEM1:
            stage = "MEM1";
            break;
        case MEM2:
            stage = "MEM2";
            break;
        case WB:
            stage = "WB";
            break;
    }

    if(ins->opCode == 0x00) // set Rx, #Imm
    {
        printf("%s \t\t : %04d set R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x10) // add Rx, Ry, Rz
    {
        printf("%s \t\t : %04d add R%d, R%d, R%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x11) // add Rx, Ry, #Imm
    {
        printf("%s \t\t : %04d add R%d, R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x20) // sub Rx, Ry, Rz
    {
        printf("%s \t\t : %04d sub R%d, R%d, R%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x21) // sub Rx, Ry, #Imm
    {
        printf("%s \t\t : %04d sub R%d, R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x30) // mul Rx, Ry, Rz
    {
        printf("%s \t\t : %04d mul R%d, R%d, R%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x31) // mul Rx, Ry, #Imm
    {
        printf("%s \t\t : %04d mul R%d, R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x40) // div Rx, Ry, Rz
    {
        printf("%s \t\t : %04d div R%d, R%d, R%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x41) // div Rx, Ry, #Imm
    {
        printf("%s \t\t : %04d div R%d, R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue, ins->RzOrValue);
    }
    else if(ins->opCode == 0x50) // ld Rx, #Addr
    {
        printf("%s \t\t : %04d ld R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x51) // ld Rx, Rz
    {
        printf("%s \t\t : %04d ld R%d, R%d\n", stage,PC, ins->Rx, ins->RzOrValue);
    }
    else if(ins->opCode == 0x60) // st Ry, #Addr
    {
        printf("%s \t\t : %04d st R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x61) // st Ry, Rz
    {
        printf("%s \t\t : %04d st R%d, R%d\n", stage,PC, ins->Rx, ins->RzOrValue);
    }
    else if(ins->opCode == 0x70) // bez Ry, #Imm
    {
        printf("%s \t\t : %04d bez R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x71) // bgez Ry, #Imm
    {
        printf("%s \t\t : %04d bgez R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x72) // blez Ry, #Imm
    {
        printf("%s \t\t : %04d blez R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x73) // bgtz Ry, #Imm
    {
        printf("%s \t\t : %04d bgtz R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0x74) // bltz Ry, #Imm
    {
        printf("%s \t\t : %04d bltz R%d, #%d\n", stage,PC, ins->Rx, ins->RyOrValue);
    }
    else if(ins->opCode == 0xFF) // ret
    {
        printf("%s \t\t : %04d ret\n", stage,PC);
    }
    else
    {
        printf("Parsing Error Opcode: %X\n", ins->opCode);
    }
}

int main(int argc, char const *argv[]){
    if(argc != 2)
    {
        printf("Invalid argument.\n");
        printf("Usage: sim sim1.asm\n");
        return 1;
    }

    unsigned char *codeMap = NULL;
    size_t codeMapSize = 0;
    unsigned char *memoryMap = NULL;
    size_t memoryMapSize = 0;
    FILE *fp = NULL;

    fp = fopen(argv[1], "rb");
    if(!fp)
    {
        printf("Can't open file: %s\n", argv[1]);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    codeMapSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    codeMap = (unsigned char *)malloc(codeMapSize);
    fread(codeMap, 1, codeMapSize, fp);
    fclose(fp);

    fp = fopen("./memory_map", "rb");
    if(!fp)
    {
        printf("Can't open memory_map\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    memoryMapSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    memoryMap = (unsigned char *)malloc(memoryMapSize);
    fread(memoryMap, 1, memoryMapSize, fp);
    fclose(fp);

    // parse
    INS ins;
    int PC = 0;
    PipeLineStatus pipestatus[STAGE_ITEMS_COUNT];
    memset(&pipestatus,0,sizeof(PipeLineStatus) * STAGE_ITEMS_COUNT);
    int i;
    int regs[16];
    int status[16]; // 1: valid, 0: writing
    for(i = 0; i < 16; i++){
        regs[i] = 0;
        status[i] = VALID;
    }
    for(i = 0; i < STAGE_ITEMS_COUNT;i++)
    {
        pipestatus[i].status = EMPTY;
    }
    int data_hazard_count = 1;
    int control_hazard_count = 0;
    int execution_cycles = 1;
    int completed_instructions = 0;
    while(1)
    {
        if(pipestatus[WB].status == NORMAL)
        {
            completed_instructions++;
            if(pipestatus[WB].instruction.opCode == 0x00) // set Rx, #Imm
            {
                regs[pipestatus[WB].instruction.Rx] = pipestatus[WB].instruction.RyOrValue;
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.type == SUB)
            {
                regs[pipestatus[WB].instruction.Rx] = regs[pipestatus[WB].instruction.RyOrValue] -
                regs[pipestatus[WB].instruction.RzOrValue];
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x10)    //ADD
            {
                regs[pipestatus[WB].instruction.Rx] = regs[pipestatus[WB].instruction.RyOrValue] +
                regs[pipestatus[WB].instruction.RzOrValue];
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x11)    //ADD
            {
                regs[pipestatus[WB].instruction.Rx] = regs[pipestatus[WB].instruction.RyOrValue] +
                pipestatus[WB].instruction.RzOrValue;
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x30)    //ADD
            {
                regs[pipestatus[WB].instruction.Rx] = regs[pipestatus[WB].instruction.RyOrValue] *
                regs[pipestatus[WB].instruction.RzOrValue];
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x31)    //ADD
            {
                regs[pipestatus[WB].instruction.Rx] = regs[pipestatus[WB].instruction.RyOrValue] *
                pipestatus[WB].instruction.RzOrValue;
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x51)    //ld
            {
                int mem = memoryMap[regs[pipestatus[WB].instruction.RzOrValue]];
                regs[pipestatus[WB].instruction.Rx] = mem;
                status[pipestatus[WB].instruction.Rx] = VALID;
            }
            if(pipestatus[WB].instruction.opCode == 0x61)    //st
            {
            }
            if(pipestatus[WB].instruction.opCode == 0xFF) // ret
                break;
            pipestatus[WB].status = EMPTY;
        }
        for(i = STAGE_ITEMS_COUNT - 1; i > 0; i--)
        {
            if(pipestatus[i].status == BUBBLE)
            {
                data_hazard_count++;
            }
            if(pipestatus[i].status == EMPTY && pipestatus[i - 1].status == NORMAL)
            {
                if(i == EX1)
                {
                    if(pipestatus[IA].instruction.type == SUB)
                    {
                        if(status[pipestatus[IA].instruction.Rx] == WRITING ||
                        status[pipestatus[IA].instruction.RyOrValue] == WRITING ||
                        status[pipestatus[IA].instruction.RzOrValue] == WRITING )
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.opCode == 0x10)   //ADD
                    {
                        if(status[pipestatus[IA].instruction.RyOrValue] == WRITING ||
                        status[pipestatus[IA].instruction.RzOrValue] == WRITING )
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.opCode == 0x11)   //ADD
                    {
                        if(status[pipestatus[IA].instruction.RyOrValue] == WRITING &&
                        pipestatus[IA].instruction.RyOrValue != pipestatus[IA].instruction.Rx)
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.opCode == 0x30)   //Mul
                    {
                        if(status[pipestatus[IA].instruction.RyOrValue] == WRITING &&
                        pipestatus[IA].instruction.RyOrValue != pipestatus[IA].instruction.Rx)
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.opCode == 0x31)   //Mul
                    {
                        if(status[pipestatus[IA].instruction.RyOrValue] == WRITING &&
                        pipestatus[IA].instruction.RyOrValue != pipestatus[IA].instruction.Rx)
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.opCode == 0x51)   //ld
                    {
                        if(status[pipestatus[IA].instruction.RzOrValue] == WRITING)
                        {
                            data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }

                    if(pipestatus[IA].instruction.opCode == 0x61)   //st
                    {
                        if(status[pipestatus[IA].instruction.Rx] == WRITING &&
                         status[pipestatus[IA].instruction.RzOrValue] == WRITING)
                        {
                            if(n == 0) n = 1;
                            else data_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                    if(pipestatus[IA].instruction.type == BRANCH)
                    {
                        if(status[pipestatus[IA].instruction.Rx] == WRITING)
                        {
                            data_hazard_count++;
                            control_hazard_count++;
                            pipestatus[i].status = BUBBLE;
                            continue;
                        }
                    }
                }
                pipestatus[i].status = NORMAL;
                pipestatus[i - 1].status = EMPTY;
                pipestatus[i].instruction = pipestatus[i - 1].instruction;
                if(i == BR)
                {
                    if(pipestatus[i].instruction.opCode == 0x71)
                    {
                        if(regs[pipestatus[i].instruction.Rx] >= 0)
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                            PC = pipestatus[i].instruction.RyOrValue;
                        }
                        else
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                        }
                    }
                    if(pipestatus[i].instruction.opCode == 0x72)
                    {
                        if(regs[pipestatus[i].instruction.Rx] <= 0)
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                            PC = pipestatus[i].instruction.RyOrValue;
                        }
                        else
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                        }
                    }
                    if(pipestatus[i].instruction.opCode == 0x70)        //bez
                    {
                        if(regs[pipestatus[i].instruction.Rx] == 0)
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                            PC = pipestatus[i].instruction.RyOrValue;
                        }
                        else
                        {
                            control_hazard_count++;
                            pipestatus[IF].status = EMPTY;
                        }
                    }
                }
                if(i == IA)
                {
                    if(pipestatus[EX1].status != BUBBLE)
                    {
                        if(pipestatus[IA].instruction.type == SUB)
                        {
                            status[pipestatus[IA].instruction.Rx] = SP_WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x10)    //ADD
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x11)    //ADD
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x30)    //Mul
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x31)    //Mul
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x51)    //ld
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.opCode == 0x61)    //st
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.type == SET)
                        {
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                        if(pipestatus[IA].instruction.type == BRANCH)
                        {
                            control_hazard_count++;
                            status[pipestatus[IA].instruction.Rx] = WRITING;
                        }
                    }
                }
                if(i == ID)
                {
                    if(pipestatus[i].instruction.opCode == 0x72 ||
                    pipestatus[i].instruction.opCode == 0x71 ||
                    pipestatus[i].instruction.opCode == 0x70)
                    {
                        control_hazard_count++;
                        pipestatus[IF].status = BUBBLE;
                    }
                }
            }
        }
        if(pipestatus[EX1].status == BUBBLE && pipestatus[IA].status == NORMAL)
        {
            if(pipestatus[IA].instruction.type == SUB)
            {
                if(!(status[pipestatus[IA].instruction.Rx] == WRITING) &&
                !(status[pipestatus[IA].instruction.RyOrValue] == WRITING) &&
                !(status[pipestatus[IA].instruction.RzOrValue] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x10)    //ADD
            {
                if(
                !(status[pipestatus[IA].instruction.RyOrValue] == WRITING) &&
                !(status[pipestatus[IA].instruction.RzOrValue] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x11)    //ADD
            {
                if(
                !(status[pipestatus[IA].instruction.RyOrValue] == WRITING))
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x30)    //mul
            {
                if(
                !(status[pipestatus[IA].instruction.RyOrValue] == WRITING) &&
                !(status[pipestatus[IA].instruction.RzOrValue] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x31)    //mul
            {
                if(
                !(status[pipestatus[IA].instruction.RyOrValue] == WRITING))
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x71)    //bgez
            {
                if(!(status[pipestatus[IA].instruction.Rx] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x70)    //bez
            {
                if(!(status[pipestatus[IA].instruction.Rx] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x72)    //Set
            {
                if(status[pipestatus[IA].instruction.Rx] != WRITING)
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x51)    //ld
            {
                if(status[pipestatus[IA].instruction.RzOrValue] != WRITING)
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
            if(pipestatus[IA].instruction.opCode == 0x61)   //st
            {
                if(
                !(status[pipestatus[IA].instruction.Rx] == WRITING) &&
                !(status[pipestatus[IA].instruction.RzOrValue] == WRITING) )
                {
                    pipestatus[EX1].status = EMPTY;
                }
            }
        }
        //IF Stage
        if(pipestatus[IF].status != BUBBLE && pipestatus[IF].status != NORMAL)
        {
            if(PC == codeMapSize)
            {
                 pipestatus[IF].status = WAIT;

            }
            else
            {
                parseINS(codeMap, PC, &ins);
                pipestatus[IF].status = NORMAL;
                pipestatus[IF].instruction = ins;
                pipestatus[IF].instruction.PC = PC;
                PC+=4;
            }

        }
        execution_cycles++;
    }
    execution_cycles--;
    printf("================================\n");
    printf("--------------------------------\n");
    for (int reg=0; reg<16; reg++) {
        printf("REG[%2d]   |   Value=%d     \n",reg, regs[reg]);
        printf("--------------------------------\n");
    }
    printf("================================\n\n");
    printf("Stalled cycles due to data hazard: %d \n", data_hazard_count);
    printf("Stalled cycles due to control hazard: %d \n", control_hazard_count);
    printf("\n");
    printf("Total stalls: %d \n", data_hazard_count+control_hazard_count);
    printf("Total execution cycles: %d\n", execution_cycles);
    printf("Total instruction simulated: %d\n", completed_instructions);
    printf("IPC: %f\n", ((double)completed_instructions/execution_cycles));

    return 0;
}
