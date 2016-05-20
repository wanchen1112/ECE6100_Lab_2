/***********************************************************************
 * File         : pipeline.cpp
 * Author       : Soham J. Desai 
 * Date         : 14th January 2014
 * Description  : Superscalar Pipeline for Lab2 ECE 6100
 **********************************************************************/

#include "pipeline.h"
#include <cstdlib>

extern int32_t PIPE_WIDTH;
extern int32_t ENABLE_MEM_FWD;
extern int32_t ENABLE_EXE_FWD;
extern int32_t BPRED_POLICY;

/**********************************************************************
 * Support Function: Read 1 Trace Record From File and populate Fetch Op
 **********************************************************************/

void pipe_get_fetch_op(Pipeline *p, Pipeline_Latch* fetch_op){
    uint8_t bytes_read = 0;
    bytes_read = fread(&fetch_op->tr_entry, 1, sizeof(Trace_Rec), p->tr_file);

    
    
    // check for end of trace
    if( bytes_read < sizeof(Trace_Rec)) {
      fetch_op->valid=false;
      p->halt_op_id=p->op_id_tracker;
      return;
    }

    // got an instruction ... hooray!
    fetch_op->valid=true;
    fetch_op->stall=false;
    fetch_op->is_mispred_cbr=false;
    p->op_id_tracker++;
    fetch_op->op_id=p->op_id_tracker;
  
    return; 
}


/**********************************************************************
 * Pipeline Class Member Functions 
 **********************************************************************/

Pipeline * pipe_init(FILE *tr_file_in){
    printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);

    // Initialize Pipeline Internals
    Pipeline *p = (Pipeline *) calloc (1, sizeof (Pipeline));

    p->tr_file = tr_file_in;
    p->halt_op_id = ((uint64_t)-1) - 3;           

    // Allocated Branch Predictor
    if(BPRED_POLICY){
      p->b_pred = new BPRED(BPRED_POLICY);
    }

    return p;
}


/**********************************************************************
 * Print the pipeline state (useful for debugging)
 **********************************************************************/

void pipe_print_state(Pipeline *p){
    std::cout << "--------------------------------------------" << std::endl;
    std::cout <<"cycle count : " << p->stat_num_cycle << " retired_instruction : " << p->stat_retired_inst << std::endl;

    uint8_t latch_type_i = 0;   // Iterates over Latch Types
    uint8_t width_i      = 0;   // Iterates over Pipeline Width
    for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
        switch(latch_type_i) {
            case 0:
                printf(" FE: ");
                break;
            case 1:
                printf(" ID: ");
                break;
            case 2:
                printf(" EX: ");
                break;
            case 3:
                printf(" MEM: ");
                break;
            default:
                printf(" ---- ");
        }
    }
    printf("\n");
    for(width_i = 0; width_i < PIPE_WIDTH; width_i++) {
        for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
            if(p->pipe_latch[latch_type_i][width_i].valid == true) {
	      printf(" %6u ",(uint32_t)( p->pipe_latch[latch_type_i][width_i].op_id));
            } else {
                printf(" ------ ");
            }
        }
        printf("\n");
    }
    printf("\n");

}


/**********************************************************************
 * Pipeline Main Function: Every cycle, cycle the stage 
 **********************************************************************/


/**********************************************************************
 * -----------  DO NOT MODIFY THE CODE ABOVE THIS LINE ----------------
 **********************************************************************/

//#define PRINT_DEBUG

const char* get_OP_type(uint8_t t)
{
    switch(t)
    {
        case 0: return "ALU";
        case 1: return "LOAD";
        case 2: return "STORE";
        case 3: return "CBR";
        case 4: return "OTHERS";
    }
}
void debug_message(Pipeline *p)
{
    int ii;
    for(ii=0; ii<PIPE_WIDTH; ii++)
    {
        printf("\n-------------------------------\nOP type: %s, seq: %d\n",get_OP_type(p->pipe_latch[0][ii].tr_entry.op_type),p->pipe_latch[0][ii].op_id);
        printf("Fetch src1: %d, ",p->pipe_latch[0][ii].tr_entry.src1_reg);
        printf("Fetch src2: %d, ",p->pipe_latch[0][ii].tr_entry.src2_reg);
        printf("Fetch dest: %d\n",p->pipe_latch[0][ii].tr_entry.dest);
        
        printf("Fetch src1 valid: %d, ",p->pipe_latch[0][ii].tr_entry.src1_needed);
        printf("Fetch src2 valid: %d, ",p->pipe_latch[0][ii].tr_entry.src2_needed);
        printf("Fetch dest valid: %d\n",p->pipe_latch[0][ii].tr_entry.dest_needed);
        printf("Mem addr: %d, ",p->pipe_latch[0][ii].tr_entry.mem_addr);
        printf("Mem write: %d, ",p->pipe_latch[0][ii].tr_entry.mem_write);
        printf("Mem read: %d\n,",p->pipe_latch[0][ii].tr_entry.mem_read);
        printf("CC read: %d, ",p->pipe_latch[0][ii].tr_entry.cc_read);
        printf("CC write: %d\n ",p->pipe_latch[0][ii].tr_entry.cc_write);
        
        printf("inst_addr: %d, ",p->pipe_latch[0][ii].tr_entry.inst_addr);

        printf("br_dir: %d, ",p->pipe_latch[0][ii].tr_entry.br_dir);
        printf("br_target: %d\n ",p->pipe_latch[0][ii].tr_entry.br_target);
        
    }
}


void pipe_cycle(Pipeline *p)
{
    p->stat_num_cycle++;
    
    pipe_cycle_WB(p);
    pipe_cycle_MEM(p);
    pipe_cycle_EX(p);
    pipe_cycle_ID(p);
    pipe_cycle_FE(p);
    
    //debug_message(p);
    //pipe_print_state(p);
}


void release_dependency(Pipeline_Latch* id_latch, uint8_t dest)
{
    int ii;
    uint8_t src1_t, src2_t;
    
    
    for(ii = 0; ii < PIPE_WIDTH; ii++)
    {
                
        if((id_latch[ii].src1_rd_denied != 0) || (id_latch[ii].src2_rd_denied != 0))
        {
            src1_t = id_latch[ii].tr_entry.src1_reg;
            src2_t = id_latch[ii].tr_entry.src2_reg;
            
            if(dest == src1_t)
              id_latch[ii].src1_rd_denied -= 1;
            if(dest == src2_t)
              id_latch[ii].src2_rd_denied -= 1;

        }

        if(id_latch[ii].cc_rd_denied != 0)
        {
            id_latch[ii].cc_rd_denied -= 1;
        }

        if((id_latch[ii].src1_rd_denied == 0) && (id_latch[ii].src2_rd_denied == 0) && (id_latch[ii].cc_rd_denied == 0)){
            id_latch[ii].stall = false;
            break;
        }
#ifdef PRINT_DEBUG
                  printf("seq no.: %d, cc_rd_denied= %d\n", id_latch[ii].op_id, id_latch[ii].cc_rd_denied);
#endif
    
    } // end of for

} // end of release

void pipe_cycle_WB(Pipeline *p){
  
  int ii, jj;
  bool dep1, dep2, cc_dep;
  uint8_t dest_t;
    
  for(ii=0; ii<PIPE_WIDTH; ii++){
      
      if(p->pipe_latch[MEM_LATCH][ii].valid){
          p->stat_retired_inst++;
          p->pipe_latch[MEM_LATCH][ii].valid = false;
#ifdef PRINT_DEBUG
          printf("-----------------------------\nWB retired: %d\n", p->stat_retired_inst);
#endif
          dep1 = p->pipe_latch[MEM_LATCH][ii].dest_src1_dep;
          dep2 = p->pipe_latch[MEM_LATCH][ii].dest_src2_dep;
          cc_dep = p->pipe_latch[MEM_LATCH][ii].cc_wr_dep;
          dest_t = p->pipe_latch[MEM_LATCH][ii].tr_entry.dest;
          
          if(!(ENABLE_EXE_FWD) && !(ENABLE_MEM_FWD))
          {
              if(dep1 == true || dep2 == true || cc_dep == true)
                  release_dependency(p->pipe_latch[FE_LATCH],dest_t);
          }
          
          if(p->pipe_latch[MEM_LATCH][ii].op_id >= p->halt_op_id){
              p->halt=true;
          }
      }
      else{
#ifdef PRINT_DEBUG
          printf("-----------------------------\nWB invalid\n");
#endif
      }
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_MEM(Pipeline *p){
  int ii;
  bool dep1, dep2, cc_dep;
  uint8_t dest_t;
  
    for(ii=0; ii<PIPE_WIDTH; ii++){
        if(p->pipe_latch[EX_LATCH][ii].valid == true){
            p->pipe_latch[MEM_LATCH][ii]=p->pipe_latch[EX_LATCH][ii];
            p->pipe_latch[EX_LATCH][ii].valid = false;
            
            dep1 = p->pipe_latch[MEM_LATCH][ii].dest_src1_dep;
            dep2 = p->pipe_latch[MEM_LATCH][ii].dest_src2_dep;
            cc_dep = p->pipe_latch[MEM_LATCH][ii].cc_wr_dep;
            dest_t = p->pipe_latch[MEM_LATCH][ii].tr_entry.dest;
            
            if(ENABLE_EXE_FWD && ENABLE_MEM_FWD)
            {
                if(dep1 == true || dep2 == true || cc_dep == true)
                    release_dependency(p->pipe_latch[FE_LATCH],dest_t);
            }
            
        }
        else
            p->pipe_latch[MEM_LATCH][ii].valid = false;
        
#ifdef PRINT_DEBUG
    printf("MEM valid: %d, src1: %d, src2: %d, dst: %d, seq: %d\n", p->pipe_latch[MEM_LATCH][ii].valid, p->pipe_latch[MEM_LATCH][ii].tr_entry.src1_reg, p->pipe_latch[MEM_LATCH][ii].tr_entry.src2_reg, p->pipe_latch[MEM_LATCH][ii].tr_entry.dest, p->pipe_latch[MEM_LATCH][ii].op_id);
#endif
    }
}

//--------------------------------------------------------------------//

void pipe_cycle_EX(Pipeline *p){
  int ii;
  bool dep1, dep2, cc_dep;
  uint8_t dest_t;
    
    
  for(ii=0; ii<PIPE_WIDTH; ii++){
      
      if(p->pipe_latch[ID_LATCH][ii].valid == true){
        p->pipe_latch[EX_LATCH][ii]=p->pipe_latch[ID_LATCH][ii];
        p->pipe_latch[ID_LATCH][ii].valid = false;
          
        dep1 = p->pipe_latch[EX_LATCH][ii].dest_src1_dep;
        dep2 = p->pipe_latch[EX_LATCH][ii].dest_src2_dep;
        cc_dep = p->pipe_latch[EX_LATCH][ii].cc_wr_dep;
        dest_t = p->pipe_latch[EX_LATCH][ii].tr_entry.dest;
          
        if(p->pipe_latch[EX_LATCH][ii].ex_release == true)
        {
            if(dep1 == true || dep2 == true || cc_dep == true)
                release_dependency(p->pipe_latch[FE_LATCH],dest_t);
        }
          
      }
    else
        p->pipe_latch[EX_LATCH][ii].valid = false;

#ifdef PRINT_DEBUG
      printf("EX valid: %d, src1: %d, src2: %d, dst:%d, seq: %d\n", p->pipe_latch[EX_LATCH][ii].valid, p->pipe_latch[EX_LATCH][ii].tr_entry.src1_reg, p->pipe_latch[EX_LATCH][ii].tr_entry.src2_reg, p->pipe_latch[EX_LATCH][ii].tr_entry.dest, p->pipe_latch[EX_LATCH][ii].op_id);
#endif
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_ID(Pipeline *p){

  int ii, jj, src1_n, src2_n, cc_r;
  uint8_t src1_t,src2_t;
  bool EX_mem_rd, ID_mem_rd;
  uint32_t ID_valid_count;
   
  ID_valid_count = 0;
    
  for(ii=0; ii<PIPE_WIDTH; ii++)
  {
      if(p->pipe_latch[FE_LATCH][ii].stall == true){
#ifdef PRINT_DEBUG
          printf("remain stall, seq: %d\n",p->pipe_latch[FE_LATCH][ii].op_id);
#endif
          if((ID_valid_count != 0) && (p->pipe_latch[FE_LATCH][ii-ID_valid_count].valid == true))
              printf("Bug!!, seq: %d\n", p->pipe_latch[FE_LATCH][ii-ID_valid_count].op_id);
          
          p->pipe_latch[FE_LATCH][ii-ID_valid_count] =  p->pipe_latch[FE_LATCH][ii];
          
          if(ID_valid_count != 0){
              p->pipe_latch[FE_LATCH][ii].stall = false;
              p->pipe_latch[FE_LATCH][ii].valid = false;
          }
          
          if(ii != (PIPE_WIDTH-1))
              p->pipe_latch[FE_LATCH][ii+1].stall = true;
          
          continue;
      }

// check normal
      
      src1_n = p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed;
      src2_n = p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed;
      src1_t = p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg;
      src2_t = p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg;
      cc_r = p->pipe_latch[FE_LATCH][ii].tr_entry.cc_read;
      
      
      if((src1_n != 0) || (src2_n !=0) || (cc_r == 1)) // there are src registers -> check dependency
      {
          for(jj = 0; jj < PIPE_WIDTH; jj++) // check for older instructions
          {
              
            if(ENABLE_EXE_FWD)
            {
                if(p->pipe_latch[EX_LATCH][jj].valid == true)
                    EX_mem_rd = p->pipe_latch[EX_LATCH][jj].tr_entry.mem_read;
                
                ID_mem_rd = p->pipe_latch[ID_LATCH][jj].tr_entry.mem_read;
#ifdef PRINT_DEBUG
                printf("seq: %d, mem_rd= %d\n",p->pipe_latch[EX_LATCH][jj].op_id, EX_mem_rd);
#endif
            }
            else
            {
               EX_mem_rd = 0;
               ID_mem_rd = 0;
            }

          if(src1_n == 1) // check src1
          {
              // only check instr. in older ID stages
              if(jj < ii){
                  if((src1_t == p->pipe_latch[ID_LATCH][jj].tr_entry.dest) && (p->pipe_latch[ID_LATCH][jj].valid == true)){
                      
                      p->pipe_latch[FE_LATCH][ii].src1_rd_denied++;
                      p->pipe_latch[ID_LATCH][jj].dest_src1_dep = true;
                      
                      if((ID_mem_rd == false) && (ENABLE_EXE_FWD))
                          p->pipe_latch[ID_LATCH][jj].ex_release = true;
                      
                  }
              } // check last ID stages
              
              if((src1_t == p->pipe_latch[EX_LATCH][jj].tr_entry.dest) && (p->pipe_latch[EX_LATCH][jj].valid == true)){
                  
                  if(!(ENABLE_EXE_FWD) || (EX_mem_rd == true)){
                      p->pipe_latch[FE_LATCH][ii].src1_rd_denied++;
                      p->pipe_latch[EX_LATCH][jj].dest_src1_dep = true;
                      
                  }
                  
              }
              if((src1_t == p->pipe_latch[MEM_LATCH][jj].tr_entry.dest) && (p->pipe_latch[MEM_LATCH][jj].valid == true)){
                  if(!(ENABLE_MEM_FWD)){
                      p->pipe_latch[FE_LATCH][ii].src1_rd_denied++;
                      p->pipe_latch[MEM_LATCH][jj].dest_src1_dep = true;
                  }
                  
              }
              
          }
  
          if(src2_n == 1) // check src2
          {
              // only check instr. in older ID stages
              if(jj < ii){
                  if((src2_t == p->pipe_latch[ID_LATCH][jj].tr_entry.dest) && (p->pipe_latch[ID_LATCH][jj].valid == true)){
                      
                      p->pipe_latch[FE_LATCH][ii].src2_rd_denied++;
                      p->pipe_latch[ID_LATCH][jj].dest_src2_dep = true;
                      
                      if((ID_mem_rd == false) && (ENABLE_EXE_FWD))
                          p->pipe_latch[ID_LATCH][jj].ex_release = true;
                      
                  }
              } // check last ID stages
              
              if((src2_t == p->pipe_latch[EX_LATCH][jj].tr_entry.dest) && (p->pipe_latch[EX_LATCH][jj].valid == true)){
                  
                  if(!(ENABLE_EXE_FWD) || (EX_mem_rd == true)){
                      p->pipe_latch[FE_LATCH][ii].src2_rd_denied++;
                      p->pipe_latch[EX_LATCH][jj].dest_src2_dep = true;
                      
                  }
                  
              }
              if((src2_t == p->pipe_latch[MEM_LATCH][jj].tr_entry.dest) && (p->pipe_latch[MEM_LATCH][jj].valid == true)){
                  if(!(ENABLE_MEM_FWD)){
                      p->pipe_latch[FE_LATCH][ii].src2_rd_denied++;
                      p->pipe_latch[MEM_LATCH][jj].dest_src2_dep = true;
                  }
                  
              }
              
          } // end of src2
              
              
        // conditional read/write
        if(cc_r == 1)
        {
            if(jj < ii){
                if(p->pipe_latch[ID_LATCH][jj].tr_entry.cc_write == 1 && p->pipe_latch[ID_LATCH][jj].valid == true){

                    p->pipe_latch[FE_LATCH][ii].cc_rd_denied++;
                    p->pipe_latch[ID_LATCH][jj].cc_wr_dep = true;
                    
                    if((ID_mem_rd == false) && (ENABLE_EXE_FWD))
                        p->pipe_latch[ID_LATCH][jj].ex_release = true;

                }
                
            } // check last ID stages
            
            if(p->pipe_latch[EX_LATCH][jj].tr_entry.cc_write == 1 && p->pipe_latch[EX_LATCH][jj].valid == true){
                
                
                if(!(ENABLE_EXE_FWD) || (EX_mem_rd == true)){
                    p->pipe_latch[EX_LATCH][jj].cc_wr_dep = 1;
                    p->pipe_latch[FE_LATCH][ii].cc_rd_denied++;
                }
                
            }
            
            if(p->pipe_latch[MEM_LATCH][jj].tr_entry.cc_write == 1 && p->pipe_latch[MEM_LATCH][jj].valid == true){
                
                if(!(ENABLE_MEM_FWD)){
                    p->pipe_latch[MEM_LATCH][jj].cc_wr_dep = 1;
                    p->pipe_latch[FE_LATCH][ii].cc_rd_denied++;
                    
                }
            }
            
            
        } // end of if cc_r ==1

     } // end of for jj
          
  } // end if (src1 || src2 || cc_r)

      
      // after checking normal/cc wr dependency, update stall
      if(p->pipe_latch[FE_LATCH][ii].src1_rd_denied != 0 || p->pipe_latch[FE_LATCH][ii].src2_rd_denied != 0)
      {
          p->pipe_latch[FE_LATCH][ii].stall = true;
          p->pipe_latch[ID_LATCH][ii].valid = false;


#ifdef PRINT_DEBUG
          printf("ID stage: instruction %d is stall (normal read)\n", p->pipe_latch[FE_LATCH][ii].op_id);
          printf("src1_rd: %d, src2_rd: %d\n",p->pipe_latch[FE_LATCH][ii].src1_rd_denied, p->pipe_latch[FE_LATCH][ii].src2_rd_denied);
          printf("src1_n: %d, src2_n: %d\n",p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed, p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed);
#endif
      }

      if(p->pipe_latch[FE_LATCH][ii].cc_rd_denied != 0)
      {
          p->pipe_latch[FE_LATCH][ii].stall = true;
          p->pipe_latch[ID_LATCH][ii].valid = false;

#ifdef PRINT_DEBUG
          printf("ID stage: instruction %d is stall (cc read), denied: %d\n", p->pipe_latch[FE_LATCH][ii].op_id, p->pipe_latch[FE_LATCH][ii].cc_rd_denied);
#endif
      }
 
      
/**** pass the dependency test, copy data to ID stage, and update valid/stall for current instr. ****/
      
      if(p->pipe_latch[FE_LATCH][ii].stall == false)
      {
          p->pipe_latch[ID_LATCH][ii] = p->pipe_latch[FE_LATCH][ii];
          p->pipe_latch[FE_LATCH][ii].valid = false;
          ID_valid_count++;
          if(ii != (PIPE_WIDTH-1))
            p->pipe_latch[FE_LATCH][ii+1].stall = false;
      }
      else  // dependency found in this stage: stall
      {
          // move the instruction to top
          p->pipe_latch[FE_LATCH][ii-ID_valid_count] =  p->pipe_latch[FE_LATCH][ii];
#ifdef PRINT_DEBUG         
          printf("seq: %d moves to upper space %d\n",p->pipe_latch[FE_LATCH][ii-ID_valid_count].op_id,ii-ID_valid_count);
#endif
          if(ID_valid_count != 0){
              p->pipe_latch[FE_LATCH][ii].stall = false;
              p->pipe_latch[FE_LATCH][ii].valid = false;
          }
          
          if(ii != (PIPE_WIDTH-1))
             p->pipe_latch[FE_LATCH][ii+1].stall = true;
      }

#ifdef PRINT_DEBUG
      printf("ID valid: %d, src1: %d, src2: %d, dst: %d, seq: %d, ID_valid_count: %d\n", p->pipe_latch[ID_LATCH][ii].valid, p->pipe_latch[ID_LATCH][ii].tr_entry.src1_reg, p->pipe_latch[ID_LATCH][ii].tr_entry.src2_reg, p->pipe_latch[ID_LATCH][ii].tr_entry.dest, p->pipe_latch[ID_LATCH][ii].op_id, ID_valid_count);
#endif
  } // end of for loop (ii)
    
} // end of ID stage

//--------------------------------------------------------------------//

void pipe_cycle_FE(Pipeline *p){
  int ii;
  Pipeline_Latch fetch_op;
  bool tr_read_success;
    
  for(ii=0; ii<PIPE_WIDTH; ii++){
    if((p->pipe_latch[FE_LATCH][ii].stall == false) && (p->pipe_latch[FE_LATCH][ii].valid == false))// no valid data, read data from traces
    {
        pipe_get_fetch_op(p, &fetch_op);
        
        fetch_op.src1_rd_denied = 0;
        fetch_op.src2_rd_denied = 0;
        fetch_op.dest_src1_dep = 0;
        fetch_op.dest_src2_dep = 0;
        fetch_op.cc_rd_denied = 0;
        fetch_op.cc_wr_dep = 0;
        fetch_op.ex_release = 0;
        
        if(BPRED_POLICY){
          pipe_check_bpred(p, &fetch_op);
        }
        // copy the op in FE LATCH
        p->pipe_latch[FE_LATCH][ii]=fetch_op;
#ifdef PRINT_DEBUG
      printf("FE valid: %d, src1: %d, src2: %d, dst: %d, seq: %d\n", p->pipe_latch[FE_LATCH][ii].valid, p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg, p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg, p->pipe_latch[MEM_LATCH][ii].tr_entry.dest, p->pipe_latch[FE_LATCH][ii].op_id);
#endif
    }
#ifdef PRINT_DEBUG
    else
        printf("FE stall: %d, seq: %d, p->halt: %d\n", p->pipe_latch[FE_LATCH][ii].stall, p->pipe_latch[FE_LATCH][ii].op_id,p->halt);
#endif
      
  }
  
}


//--------------------------------------------------------------------//

void pipe_check_bpred(Pipeline *p, Pipeline_Latch *fetch_op){
  // call branch predictor here, if mispred then mark in fetch_op
  // update the predictor instantly
  // stall fetch using the flag p->fetch_cbr_stall
}


//--------------------------------------------------------------------//

