# Computer Architecture: MIPS Pipeline & Branch Prediction Simulator

A comprehensive suite of computer architecture tools featuring a pipelined MIPS processor simulator and advanced branch prediction analysis framework.

## Project Overview

This repository contains two major components that demonstrate deep understanding of processor design, pipeline optimization, and predictive execution:

### 1. **MIPS 5-Stage Pipeline Simulator**
A cycle-accurate MIPS processor implementation with hazard detection, data forwarding, and branch prediction.

### 2. **Branch Predictor Simulator**
A configurable branch prediction analysis tool supporting multiple prediction schemes with performance metrics.

## Key Features

### Pipeline Simulator
- ✅ **5-Stage Pipeline**: IF → ID → EX → MEM → WB
- ✅ **Data Forwarding**: Automatic forwarding from EX, MEM, and WB stages
- ✅ **Hazard Detection**: Load-use hazard detection with automatic stall insertion
- ✅ **Branch Prediction**: Hybrid backward-taken/forward-not-taken prediction
- ✅ **Interactive Visualization**: Real-time web-based pipeline viewer
- ✅ **Comprehensive Instruction Support**: ADD, SUB, AND, OR, SLL, SRL, ADDI, LW, SW, BEQZ, HALT

### Branch Predictor
-  **4 Prediction Schemes**: Bimodal, GShare, Local History, Two-Level Adaptive
-  **Configurable Parameters**: Table entries, counter bits, history bits
-  **Performance Metrics**: Misprediction rate, storage overhead, prediction accuracy
-  **Trace-Based Simulation**: Process real branch instruction traces

##  Quick Start

### Pipeline Simulator

```bash
# Compile the simulator
gcc -o mips-pipe mips-small-pipe.c

# Run with test program
./mips-pipe test_program.txt

# Or open the web visualizer
open index.html
```

### Branch Predictor

```bash
# Compile the predictor
gcc -o branchsim branchsim.c branchsim_driver.c

# Run with specific configuration
./branchsim -p G -k 16 -c 2 -s 4 traces/gcc.trace
# -p: Predictor type (B=Bimodal, G=GShare, L=Local, T=Two-Level)
# -k: Number of entries
# -c: Counter bits
# -s: History bits
```

##  Sample Output

### Pipeline Execution
```
@@@
state before cycle 5 starts
	pc 20
	IFID:
		instruction lw 4 1 0
		pcPlus1 20
	IDEX:
		instruction sw 2 1 0
		readRegA 0
		readRegB 8
	EXMEM:
		instruction add 2 2 3
		aluResult 8
	MEMWB:
		instruction addi 3 0 3
		writeData 3
```

### Branch Prediction Stats
```
Branch Predictor Statistics
# Branches: 10000
# Predicted Taken: 6247
# Predicted Not Taken: 3753
# Correct: 9124
Misprediction Rate: 0.087600
Storage overhead: 144 bits
```

##  Architecture Details

### Pipeline Stages

1. **Instruction Fetch (IF)**
   - Fetches instruction from memory
   - Updates PC with hybrid branch prediction
   - Passes PC+4 to next stage

2. **Instruction Decode (ID)**
   - Decodes instruction fields
   - Reads source registers
   - Detects load-use hazards and inserts stalls

3. **Execute (EX)**
   - Performs ALU operations
   - Implements data forwarding logic
   - Calculates branch targets

4. **Memory (MEM)**
   - Handles load/store operations
   - Accesses data memory

5. **Write Back (WB)**
   - Writes results to register file
   - Completes instruction execution

### Forwarding Paths

```
EX/MEM → EX  (1-cycle forwarding)
MEM/WB → EX  (2-cycle forwarding)
WB/END → EX  (3-cycle forwarding)
```

### Branch Prediction Strategies

| Scheme | Description | Storage |
|--------|-------------|---------|
| **Bimodal** | Single pattern history table indexed by PC | N × C bits |
| **GShare** | XOR of PC and global history register | N × C + H bits |
| **Local History** | Per-branch history with separate PHT | N × H + 2^H × C bits |
| **Two-Level Adaptive** | Global PHT indexed by local histories | N × H + 2^H × C bits |

##  Interactive Visualizer Features

- 🎬 **Step-by-step execution** with play/pause controls
- 🎨 **Color-coded pipeline stages** for easy tracking
- 📈 **Live register and memory updates** with highlighting
- ⚠️ **Hazard visualization** showing stalls and forwards
- 📊 **Performance statistics** including CPI and stall count
- 🌓 **Light/Dark theme toggle** for comfortable viewing
- 🎛️ **Multiple sample programs** including hazard demonstrations

## 🧪 Testing

The repository includes several test programs:

- **Simple Program**: Basic arithmetic and load/store operations
- **Data Hazard Test**: Demonstrates load-use hazard detection
- **Branch Test**: Shows branch prediction in action
- **R-Type Suite**: Comprehensive ALU instruction testing

##  Technical Implementation Highlights

### Data Forwarding Logic
```c
// Forward from EX/MEM stage
if (r1 == field_r3(state->EXMEM.instr) && r1 != 0) {
    regA = state->EXMEM.aluResult;
}
```

### Stall Detection
```c
// Detect load-use hazard
if (opcode(state->IDEX.instr) == LW_OP &&
    (field_r2(state->IDEX.instr) == field_r1(instruction))) {
    // Insert stall
    new.IDEX.instr = NOPINSTRUCTION;
    new.pc = new.pc - 4;
}
```

### GShare Indexing
```c
// XOR PC with global history
return (pc ^ ghr) & index_mask;
```

##  Performance Insights

- **Baseline CPI**: ~1.0 (ideal pipeline)
- **With Hazards**: ~1.2-1.5 CPI depending on program
- **Branch Misprediction Impact**: 3-cycle penalty per misprediction
- **Forwarding Effectiveness**: Reduces 60-80% of potential stalls

##  Technology Stack

- **Languages**: C (simulation), JavaScript (visualization)
- **Web Technologies**: HTML5, CSS3 (responsive design)
- **Tools**: GCC compiler, browser developer tools
- **Concepts**: Pipelining, hazard detection, forwarding, branch prediction

##  Learning Outcomes

- Deep understanding of pipelined processor execution
- Practical experience with data hazard mitigation
- Implementation of multiple branch prediction algorithms
- Performance analysis and optimization techniques
- Systems programming and bit-level manipulation
- Interactive data visualization

##  Contributing

This project was developed as part of computer architecture coursework. Feel free to explore the code and use it for educational purposes.

## License

Educational project - please cite if used for academic purposes.

## Author

**Puneet Puli**

---

*Built with ❤️ for computer architecture enthusiasts*
