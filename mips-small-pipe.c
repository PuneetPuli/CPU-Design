/* Ouneet Puli
    1188936195*/
#include "mips-small-pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************************/
int main(int argc, char *argv[])
{
  short i;
  char line[MAXLINELENGTH];
  state_t state;
  FILE *filePtr;

  if (argc != 2)
  {
    printf("error: usage: %s <machine-code file>\n", argv[0]);
    return 1;
  }

  memset(&state, 0, sizeof(state_t));

  state.pc = state.cycles = 0;
  state.IFID.instr = state.IDEX.instr = state.EXMEM.instr = state.MEMWB.instr =
      state.WBEND.instr = NOPINSTRUCTION; /* nop */

  /* read machine-code file into instruction/data memory (starting at address 0) */

  filePtr = fopen(argv[1], "r");
  if (filePtr == NULL)
  {
    printf("error: can't open file %s\n", argv[1]);
    perror("fopen");
    exit(1);
  }

  for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
       state.numMemory++)
  {
    if (sscanf(line, "%x", &state.dataMem[state.numMemory]) != 1)
    {
      printf("error in reading address %d\n", state.numMemory);
      exit(1);
    }
    state.instrMem[state.numMemory] = state.dataMem[state.numMemory];
    printf("memory[%d]=%x\n", state.numMemory, state.dataMem[state.numMemory]);
  }

  printf("%d memory words\n", state.numMemory);

  printf("\tinstruction memory:\n");
  for (i = 0; i < state.numMemory; i++)
  {
    printf("\t\tinstrMem[ %d ] = ", i);
    printInstruction(state.instrMem[i]);
  }

  run(&state);

  return 0;
}
/************************************************************/

/************************************************************/
void run(Pstate state)
{
  int instruction = 0;
  int r1 = -1;
  int r2 = -1;
  int regA = 0;
  int regB = 0;
  /*Variables to look 3 instructions ahead*/
  int previous1 = 0;
  int previous2 = 0;
  int previous3 = 0;
  state_t new;
  memset(&new, 0, sizeof(state_t));

  while (1)
  {

    printState(state);

    /* copy everything so all we have to do is make changes.
       (this is primarily for the memory and reg arrays) */
    memcpy(&new, state, sizeof(state_t));

    new.cycles++;

    /* --------------------- IF stage --------------------- */
    new.IFID.instr = state->instrMem[new.pc / 4];
    new.pc += 4;
    /* Hybrid branch prediction implementation */
    if (opcode(new.IFID.instr) == BEQZ_OP && offset(new.IFID.instr) < 0)
    {
      new.IFID.pcPlus1 = new.pc;
      new.pc = new.pc + offset(new.IFID.instr);
    }
    else
    {
      new.IFID.pcPlus1 = new.pc;
    }

    /* --------------------- ID stage --------------------- */
    new.IDEX.instr = state->IFID.instr;
    instruction = new.IDEX.instr;
    new.IDEX.readRegA = new.reg[field_r1(instruction)];
    new.IDEX.readRegB = new.reg[field_r2(instruction)];
    new.IDEX.offset = offset(instruction);
    new.IDEX.pcPlus1 = state->IFID.pcPlus1;
    /* Stall checking for load instructions */
    if ((field_r2(state->IDEX.instr) == field_r2(instruction) ||
         field_r2(state->IDEX.instr) == field_r1(instruction)) &&
        opcode(instruction) != HALT_OP &&
        opcode(instruction) == REG_REG_OP &&
        opcode(state->IDEX.instr) == LW_OP)
    {

      new.IDEX.instr = NOPINSTRUCTION;
      new.IDEX.offset = offset(NOPINSTRUCTION);
      new.IDEX.pcPlus1 = 0;
      new.IDEX.readRegA = 0;
      new.IDEX.readRegB = 0;
      new.IFID.instr = state->IFID.instr;

      new.pc = new.pc - 4;
      new.IFID.pcPlus1 = new.pc;
    }
    else if (opcode(state->IDEX.instr) == LW_OP &&
             (field_r2(state->IDEX.instr) == field_r1(instruction)) && opcode(instruction) != HALT_OP)
    {

      new.IDEX.instr = NOPINSTRUCTION;
      new.IDEX.offset = offset(NOPINSTRUCTION);
      new.IDEX.pcPlus1 = 0;
      new.IDEX.readRegA = 0;
      new.IDEX.readRegB = 0;
      new.IFID.instr = state->IFID.instr;

      new.pc = new.pc - 4;
      new.IFID.pcPlus1 = new.pc;
    }
    /* --------------------- EX stage --------------------- */
    new.EXMEM.aluResult = 0;
    new.EXMEM.instr = state->IDEX.instr;
    instruction = new.EXMEM.instr;
    r1 = field_r1(instruction);
    r2 = field_r2(instruction);
    regA = new.reg[field_r1(instruction)];
    regB = new.reg[field_r2(instruction)];
    previous1 = state->EXMEM.instr;
    previous2 = state->MEMWB.instr;
    previous3 = state->WBEND.instr;
    new.EXMEM.aluResult = 0;
    new.EXMEM.readRegB = 0;
    /* Forwarding checking */
    if (opcode(previous3) == REG_REG_OP)
    {
      if (previous3 != NOPINSTRUCTION)
      {
        if (r1 == field_r3(state->WBEND.instr) && r1 != 0)
        {
          regA = state->WBEND.writeData;
        }
        if (r1 == field_r3(state->WBEND.instr && r2 != 0))
        {
          regB = state->WBEND.writeData;
        }
      }
    }
    else if (opcode(previous3) == BEQZ_OP)
    {
      if (previous3 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->WBEND.instr) && r1 != 0)
        {
          regA = state->WBEND.writeData;
        }
        if (r2 == field_r2(state->WBEND.instr) && r2 != 0)
        {
          regB = state->WBEND.writeData;
        }
      }
    }
    else if (opcode(previous3) == SW_OP)
    {
      if (previous3 != NOPINSTRUCTION)
      {
        if (r2 == field_r2(state->WBEND.instr) && r2 != 0)
        {
          regB = state->EXMEM.aluResult;
        }
      }
    }
    else if (opcode(previous3) == LW_OP)
    {
      if (previous3 != NOPINSTRUCTION)
      {
        if ((r1 == field_r2(state->WBEND.instr)) &&
            (r1 != 0))
        {
          regA = state->WBEND.writeData;
        }
        if (r2 == field_r2(state->WBEND.instr) && (r2 != 0))
        {
          regB = state->WBEND.writeData;
        }
      }
    }
    
    if (opcode(previous2) == ADDI_OP)
    {
      if (previous2 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->MEMWB.instr) && r1 != 0)
        {
          regA = state->MEMWB.writeData;
        }
        if (r2 == field_r2(state->MEMWB.instr) && r2 != 0)
        {
          regB = state->MEMWB.writeData;
        }
      }
    }
    else if (opcode(previous2) == LW_OP)
    {
      if (previous2 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->MEMWB.instr) &&
            (r2 != 0))
        {
          regA = state->MEMWB.writeData;
        }
        if (r2 == field_r2(state->MEMWB.instr) && r2 != 0)
        {
          regB = state->MEMWB.writeData;
        }
      }
    }
    else if (opcode(previous2) == REG_REG_OP)
    {
      if (previous2 != NOPINSTRUCTION)
      {
        if (r1 == field_r3(state->MEMWB.instr) && r1 != 0)
        {
          regA = state->MEMWB.writeData;
        }
        if (r2 == field_r3(state->MEMWB.instr) && r2 != 0)
        {
          regB = state->MEMWB.writeData;
        }
      }
    }
    else if (opcode(previous2) == BEQZ_OP)
    {
      if (previous2 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->MEMWB.instr) && r1 != 0)
        {
          regA = state->MEMWB.writeData;
        }
        if (r2 == field_r2(state->MEMWB.instr) && (r1 != 0))
        {
          regB = state->MEMWB.writeData;
        }
      }
    }

    if (opcode(previous1) == ADDI_OP)
    {
      if (previous1 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->EXMEM.instr) && r1 != 0)
        {
          regA = state->EXMEM.aluResult;
        }
        if (r2 == field_r2(state->EXMEM.instr) && r2 != 0)
        {
          regB = state->EXMEM.aluResult;
        }
      }
    }
    else if (opcode(previous1) == LW_OP)
    {
      if (previous1 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->EXMEM.instr) && r1 != 0)
        {
          regA = state->EXMEM.aluResult;
        }
        if (r2 == field_r2(state->EXMEM.instr) && r2 != 0)
        {
          /* code */
          regB = state->EXMEM.aluResult;
        }
      }
    }
    else if (opcode(previous1) == REG_REG_OP)
    {
      if (r1 == field_r3(state->EXMEM.instr) && r1 != 0)
      {
        regA = state->EXMEM.aluResult;
      }
      if (r2 == field_r3(state->EXMEM.instr) && r2)
      {
        regB = state->EXMEM.aluResult;
      }
    }
    else if (opcode(previous1) == BEQZ_OP)
    {
      if (previous1 != NOPINSTRUCTION)
      {
        if (r1 == field_r2(state->EXMEM.instr) && r1 != 0)
        {
          /* code */
          regA = state->EXMEM.aluResult;
        }
        if (r2 == field_r2(state->EXMEM.instr) && r2 != 0)
        {
          regB = state->EXMEM.aluResult;
        }
      }
    }
    /* End Forwarding checking */
    /* Execution of instructions with forwarded values */
    if (opcode(instruction) == ADDI_OP && field_r2(instruction))
    {
      new.EXMEM.aluResult = regA + offset(instruction);
      new.EXMEM.readRegB = state->IDEX.readRegB;
    }
    else if (opcode(instruction) == LW_OP && field_r2(instruction) > 0)
    {
      /* printf("Offest: %d\n", offset(instruction));
      printf("Immediate: %d\n", field_imm(instruction));
      printf("register A: %d\n", regA);
      printf("EXMEM RESULT: %d\n",  new.reg[field_r1(instruction)] + offset(instruction)); */
      new.EXMEM.aluResult = regA + offset(instruction);
      new.EXMEM.readRegB = new.reg[field_r2(instruction)];
    }
    else if (opcode(instruction) == SW_OP && field_r2(instruction) > 0)
    {
      new.EXMEM.readRegB = regB;
      new.EXMEM.aluResult = regA + field_imm(instruction);
    }

    if (instruction == NOPINSTRUCTION)
    {
      new.EXMEM.aluResult = 0;
      new.EXMEM.readRegB = 0;
    }
    if (opcode(instruction) == REG_REG_OP)
    {
      if (func(instruction) == ADD_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA + regB;
        new.EXMEM.readRegB = regB;
      }
      else if (func(instruction) == SUB_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA - regB;
        new.EXMEM.readRegB = regB;
      }
      else if (func(instruction) == AND_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA &regB;
        new.EXMEM.readRegB = regB;
      }
      else if (func(instruction) == OR_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA | regB;
        new.EXMEM.readRegB = regB;
      }
      else if (func(instruction) == SRL_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA >> regB;
        new.EXMEM.readRegB = regB;
      }
      else if (func(instruction) == SLL_FUNC && field_r3(instruction) > 0)
      {
        new.EXMEM.aluResult = regA << regB;
        new.EXMEM.readRegB = regB;
      }
    }

    if (opcode(instruction) == BEQZ_OP)
    {
      /*printf("new instr: ");
      printInstruction(new.IDEX.instr);
      printf("old instr: ");
      printInstruction(state->IDEX.instr);
      printf("IDEX OFFSET: %d\n", state->IDEX.offset);
      printf("REG %d VALUE %d\n", field_r1(instruction), new.reg[field_r1(instruction)]); */
      /* Hybrid branch execution */
      if ((state->IDEX.offset < 0 && regA != 0) ||
          (state->IDEX.offset > 0 && regA == 0))
      {
        new.IFID.instr = NOPINSTRUCTION;
        new.IDEX.instr = NOPINSTRUCTION;
        new.EXMEM.aluResult = state->IDEX.pcPlus1 + offset(instruction);
        new.IFID.pcPlus1 = 0;
        new.IDEX.readRegA = 0;
        new.IDEX.pcPlus1 = 0;
        new.IDEX.offset = 32;
        new.IDEX.readRegB = 0;
        new.EXMEM.readRegB = new.reg[field_r2(instruction)];
        new.pc = state->IDEX.pcPlus1 + state->IDEX.offset;
      }
      else
      {
        new.EXMEM.aluResult = state->IDEX.pcPlus1 + offset(instruction);
        new.EXMEM.readRegB = new.reg[field_r2(instruction)];
      }
    }
    if (instruction == NOPINSTRUCTION)
    {
      new.EXMEM.aluResult = 0;
      new.EXMEM.readRegB = 0;
    }
    
    /* --------------------- MEM stage --------------------- */
    new.MEMWB.instr = state->EXMEM.instr;
    instruction = new.MEMWB.instr;
    new.MEMWB.writeData = 0;
    /* Write back only if not writing back to $0 */
    if (instruction != NOPINSTRUCTION && opcode(instruction) != HALT_OP)
    {
      if (opcode(instruction) == REG_REG_OP)
      {
        if (field_r3(instruction) != 0)
        {
          new.MEMWB.writeData = state->EXMEM.aluResult;
        }
        else
        {
          new.MEMWB.writeData = 0;
        }
      }
      else if (opcode(instruction) == LW_OP)
      {
        if (field_r2(instruction) != 0)
        {
          new.MEMWB.writeData = state->EXMEM.aluResult;
        }
        else
        {
          new.MEMWB.writeData = 0;
        }
        new.MEMWB.writeData = state->dataMem[state->EXMEM.aluResult / 4];
      }
      else if (opcode(instruction) == SW_OP)
      {
        if (field_r2(instruction) != 0)
        {
          new.MEMWB.writeData = state->EXMEM.readRegB;
        }
        else
        {
          new.MEMWB.writeData = 0;
        }
        new.dataMem[(field_imm(instruction) + new.reg[field_r1(instruction)]) / 4] =
            new.MEMWB.writeData;
      }
      else if (opcode(instruction) == ADDI_OP)
      {
        if (field_r2(instruction) != 0)
        {
          new.MEMWB.writeData = state->EXMEM.aluResult;
        }
        else
        {
          new.MEMWB.writeData = 0;
        }
      }
      else if (opcode(instruction) == BEQZ_OP)
      {
        new.MEMWB.writeData = state->EXMEM.aluResult;
      }
    }

    /* --------------------- WB stage --------------------- */
    new.WBEND.instr = state->MEMWB.instr;
    new.WBEND.writeData = state->MEMWB.writeData;
    instruction = new.WBEND.instr;
    if (opcode(instruction) != HALT_OP && instruction != NOPINSTRUCTION)
    {
      /*  printInstruction(instruction);
printf("Instruction opcode: %d, R2: %d, WriteData: %d\n", opcode(instruction), field_r2(instruction), state->MEMWB.writeData); */
      if (opcode(instruction) == ADDI_OP || opcode(instruction) == LW_OP)
      {
        if (field_r2(instruction) != 0)
        {
          new.reg[field_r2(instruction)] = state->MEMWB.writeData;
        }
      }
      if (opcode(instruction) == REG_REG_OP)
      {
        if (field_r3(instruction) != 0)
        {
          new.reg[field_r3(instruction)] = state->MEMWB.writeData;
        }
      }
      if (opcode(instruction) == SW_OP)
      {
        if (field_r2(instruction) != 0)
        {
          new.WBEND.writeData = state->MEMWB.writeData;
        }
      }
      if (opcode(instruction) == BEQZ_OP)
      {
        new.WBEND.writeData = state->MEMWB.writeData;
      }
      /* if (opcode(instruction) == LW_OP) {
           new.reg[field_r2(instruction)] = state->MEMWB.writeData;
       } */
    }
    /* Resetting $0, and temperory registers reg a and reg b */
    new.reg[0] = 0;
    regA = 0;
    regB = 0;
    /* --------------------- end stage --------------------- */
    if (opcode(new.WBEND.instr) == HALT_OP)
    {
      new.cycles--;
      printf("machine halted\n");
      printf("total of %d cycles executed\n", new.cycles);
      break;
    }

    /* transfer new state into current state */
    memcpy(state, &new, sizeof(state_t));
  }
}
/************************************************************/

/************************************************************/
int opcode(int instruction) { return (instruction >> OP_SHIFT) & OP_MASK; }
/************************************************************/

/************************************************************/
int func(int instruction) { return (instruction & FUNC_MASK); }
/************************************************************/

/************************************************************/
int field_r1(int instruction) { return (instruction >> R1_SHIFT) & REG_MASK; }
/************************************************************/

/************************************************************/
int field_r2(int instruction) { return (instruction >> R2_SHIFT) & REG_MASK; }
/************************************************************/

/************************************************************/
int field_r3(int instruction) { return (instruction >> R3_SHIFT) & REG_MASK; }
/************************************************************/

/************************************************************/
int field_imm(int instruction) { return (instruction & IMMEDIATE_MASK); }
/************************************************************/

/************************************************************/
int offset(int instruction)
{
  /* only used for lw, sw, beqz */
  return convertNum(field_imm(instruction));
}
/************************************************************/

/************************************************************/
int convertNum(int num)
{
  /* convert a 16 bit number into a 32-bit Sun number */
  if (num & 0x8000)
  {
    num -= 65536;
  }
  return (num);
}
/************************************************************/

/************************************************************/
void printState(Pstate state)
{
  short i;
  printf("@@@\nstate before cycle %d starts\n", state->cycles);
  printf("\tpc %d\n", state->pc);

  printf("\tdata memory:\n");
  for (i = 0; i < state->numMemory; i++)
  {
    printf("\t\tdataMem[ %d ] %d\n", i, state->dataMem[i]);
  }
  printf("\tregisters:\n");
  for (i = 0; i < NUMREGS; i++)
  {
    printf("\t\treg[ %d ] %d\n", i, state->reg[i]);
  }
  printf("\tIFID:\n");
  printf("\t\tinstruction ");
  printInstruction(state->IFID.instr);
  printf("\t\tpcPlus1 %d\n", state->IFID.pcPlus1);
  printf("\tIDEX:\n");
  printf("\t\tinstruction ");
  printInstruction(state->IDEX.instr);
  printf("\t\tpcPlus1 %d\n", state->IDEX.pcPlus1);
  printf("\t\treadRegA %d\n", state->IDEX.readRegA);
  printf("\t\treadRegB %d\n", state->IDEX.readRegB);
  printf("\t\toffset %d\n", state->IDEX.offset);
  printf("\tEXMEM:\n");
  printf("\t\tinstruction ");
  printInstruction(state->EXMEM.instr);
  printf("\t\taluResult %d\n", state->EXMEM.aluResult);
  printf("\t\treadRegB %d\n", state->EXMEM.readRegB);
  printf("\tMEMWB:\n");
  printf("\t\tinstruction ");
  printInstruction(state->MEMWB.instr);
  printf("\t\twriteData %d\n", state->MEMWB.writeData);
  printf("\tWBEND:\n");
  printf("\t\tinstruction ");
  printInstruction(state->WBEND.instr);
  printf("\t\twriteData %d\n", state->WBEND.writeData);
}
/************************************************************/

/************************************************************/
void printInstruction(int instr)
{

  if (opcode(instr) == REG_REG_OP)
  {

    if (func(instr) == ADD_FUNC)
    {
      print_rtype(instr, "add");
    }
    else if (func(instr) == SLL_FUNC)
    {
      print_rtype(instr, "sll");
    }
    else if (func(instr) == SRL_FUNC)
    {
      print_rtype(instr, "srl");
    }
    else if (func(instr) == SUB_FUNC)
    {
      print_rtype(instr, "sub");
    }
    else if (func(instr) == AND_FUNC)
    {
      print_rtype(instr, "and");
    }
    else if (func(instr) == OR_FUNC)
    {
      print_rtype(instr, "or");
    }
    else
    {
      printf("data: %d\n", instr);
    }
  }
  else if (opcode(instr) == ADDI_OP)
  {
    print_itype(instr, "addi");
  }
  else if (opcode(instr) == LW_OP)
  {
    print_itype(instr, "lw");
  }
  else if (opcode(instr) == SW_OP)
  {
    print_itype(instr, "sw");
  }
  else if (opcode(instr) == BEQZ_OP)
  {
    print_itype(instr, "beqz");
  }
  else if (opcode(instr) == HALT_OP)
  {
    printf("halt\n");
  }
  else
  {
    printf("data: %d\n", instr);
  }
}
/************************************************************/

/************************************************************/
void print_rtype(int instr, const char *name)
{
  printf("%s %d %d %d\n", name, field_r3(instr), field_r1(instr),
         field_r2(instr));
}
/************************************************************/

/************************************************************/
void print_itype(int instr, const char *name)
{
  printf("%s %d %d %d\n", name, field_r2(instr), field_r1(instr),
         offset(instr));
}
/************************************************************/