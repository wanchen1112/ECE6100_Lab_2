# ECE6100_Lab_2
## 5-stage Superscalar Pipeline

### Objective
This project is to evaluate the pipelined machine. In particular, the data dependencies between the pipeline stages was checked and the forwarding mechanism was implemented. Besides, the width of the superscalar can be up to 8 (Part A). Moreover, in the second phase of this project (Part B), the branch prediction was implemented as well.

### Background
- The 5-stage pipeline consists of Fetch (FE), Instruction Decode (ID), Execute (EX), Memory (MEM), and Writeback (WB).
- This simulation is trace-driven based, which is used for timing evaluation. 
- As mentioned in Objective, the evaluation focuses on how many clock cycles it takes to execute the given trace files. Therefore, there is no functional simulation; in other words, there is no exact data values in the registers, memory, and PC.
- The baseline implementation contains a sample superscalar pipelined machine and a trace parser, and I implemented some further functions which are listed in the next section.

### Implementation

The following functions were implemented in the 5-stage superscalar pipeline:  
- Dependency Checking:  
- Forwarding:  
- Branch Prediction:  

