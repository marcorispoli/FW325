
#ifndef _MAIN_H    
#define _MAIN_H

#include "definitions.h"

#undef ext
#undef ext_static

#ifdef _MAIN_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif



/// \ingroup MOTMOD
/// This is the module data structure
typedef struct{
    bool general_enable_stat; //!< Current status of the general enable switch 
    bool power_sw_stat;//!< Current status of the power switch
    bool needle_disable_stat;//!< Current status of the needle disable signal
    
    /// Sensors data structure
    struct{
        int x; //!< X position sensor
        int y; //!< Y position sensor
        int z; //!< Z position sensor
        int sh;//!< SH position sensor
        int xscroll; //!< X-SCROLL sensor
        int needle_id; //!< Needle Id sensor
        int power_supply;//!< Current motor voltage level
    }sensors;
    
    /// Keyboard data structure
    struct{
        struct{
            bool xp:1;    //!< X+ button status
            bool xm:1;    //!< X- button status
            bool yp:1;    //!< Y+ button status
            bool ym:1;    //!< Y- button status
            bool zp:1;    //!< Z+ button status
            bool zm:1;    //!< X- button status
            bool spare1:1;
            bool spare2:1;
        }hw;
        struct{
            bool keyboard_enable_stat;   //!< keyboard activation enable flag       
            bool keystep;   //!< key step mode enable flag
            bool key_present;            
        }flags;        
    }keyboard;
   
    // YUP
    bool Yup;
    
    
}DEVICE_t;

ext DEVICE_t deviceStruct; 
ext void SetKeyMode(bool enable, bool step_mode);
ext void SetPowerSwitchStat(bool stat);

#endif // _MOTLIB_H