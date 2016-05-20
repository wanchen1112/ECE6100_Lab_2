#include "bpred.h"
#include <string.h>

#define TAKEN   true
#define NOTTAKEN false

uint16_t g_BHR;
uint8_t g_PHT[4096];

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BPRED::BPRED(uint32_t policy) {

  
}


const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';
    
    int z;
    for (z = 8192; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    
    return b;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool BPRED::GetPrediction(uint32_t PC){
    
    uint16_t XOR_result;
    
    XOR_result = (g_BHR ^ (uint16_t)(PC & 0x0FFF)) & 0x0FFF;
    
    if((g_PHT[XOR_result] == 2) || (g_PHT[XOR_result] == 3))
        return TAKEN;
    else
        return NOTTAKEN;
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  BPRED::UpdatePredictor(uint32_t PC, bool resolveDir, bool predDir) {
    uint16_t XOR_result;
    
    XOR_result = (g_BHR ^ (uint16_t)(PC & 0x0FFF)) & 0x0FFF;
    
    if(resolveDir)
        g_PHT[XOR_result] = SatIncrement(g_PHT[XOR_result], 3);
    else
        g_PHT[XOR_result] = SatDecrement(g_PHT[XOR_result]);
    
    g_BHR = g_BHR << 1 | resolveDir;
#if 0
    printf("PC= %s,", byte_to_binary(PC));
    printf("BHR= %s\n", byte_to_binary(g_BHR));
    printf("XOR_result: %s, g_PHT[XOR_result]: %d\n", byte_to_binary(XOR_result), g_PHT[XOR_result]);
#endif
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////




