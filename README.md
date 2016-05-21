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
- **Dependency Checking**: If the data dependency occurs (read after write => RAW), then the instruction is stalled at ID stage until the relevant intruction finished at the succeeding stages. (Please see the details in **Forwarding**)
- **Forwarding**: When a instrunction finishes its EX stage, the data will be forwarded from EX stage to ID stage. Note that there is an exception: the *LOAD* instruction. The value of a LOAD instruction is not available until it finishes the MEM stage. Therefore, the data forwarding happens from MEM stage to ID stage.
- **Branch Prediction**: Assuming this machine has a conditional branches as soon as the instruction is fetched. Therefore, what I have done is to implement the **Always Taken** and **Gshare** predictors. For the former predictor, the prediction is always "Taken" (the condition is matched), and the latter prediction is based on the Global History Register (GHR) and Pattern History Table (PHT). If the predition is correct, then the subsequent instruction is fetched, otherwise the fetch stage is stalled until the branch resolves (ID stage is finished).

### Notes  
- Please note that the code is only executable on Gatech ECE servers.  
- To validate my implementation, the instructor provided evaluation results in terms of CPI and misprediction rate for different trace files.
