#include "config_fw.h"
#include "mmp_lib.h"

void dump_pll_reg(void)
{
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;
    MMP_USHORT i;
       
    MMP_USHORT dpll0_range[2] = { 0x6910 , 0x691f } ;
    MMP_USHORT dpll1_range[2] = { 0x69f4 , 0x69f9 } ;
    for(i=dpll0_range[0];i<=dpll0_range[1];i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
    for(i=dpll1_range[0];i<=dpll1_range[1];i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}


#if DUMP_REG_EN==1
void dump_h264_reg(void)
{
    MMP_USHORT i ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;   
    dbg_printf(3,"400~626:\r\n");
    for(i=0x400;i<=0x660;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}


void dump_vif_reg(void)
{
    MMP_USHORT i ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;  
    dbg_printf(3,"vif:\r\n"); 
    dbg_printf(3,"6000~61b6:\r\n");
    for(i=0x6000;i<=0x61b6;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}

void dump_ibc_reg(void)
{
    MMP_USHORT i ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;   
    dbg_printf(3,"ibc:\r\n");
    dbg_printf(3,"6500~65ff:\r\n");
    for(i=0x6500;i<=0x65ff;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}

void dump_scaler_reg(void)
{
    MMP_USHORT i ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;   
    dbg_printf(3,"scaler:\r\n");
    dbg_printf(3,"6400~65ff:\r\n");
    for(i=0x6400;i<=0x65ff;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
    dbg_printf(3,"6f00~6fff:\r\n");
    for(i=0x6f00;i<=0x6fff;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}

void dump_jpeg_reg(void)
{
    MMP_USHORT i ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;   
    dbg_printf(3,"jpeg:\r\n");
    dbg_printf(3,"6200~62ff:\r\n");
    for(i=0x6200;i<=0x62ff;i++) {
         dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]); 
    }
}

void dump_gbl_dram_reg(void)
{
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;   
    MMP_USHORT i ;
    dbg_printf(3,"gbl:\r\n");
    dbg_printf(3,"6900~69ff:\r\n");
    for(i=0x6900;i<=0x69ff;i++) {
        dbg_printf(3,"%x=%x\r\n",i,REG_BASE_B[i]);    
    }

}
#endif