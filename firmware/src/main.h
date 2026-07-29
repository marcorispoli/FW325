
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

/**
 * # X-AXES PEROFORMANCES
 * 
 * Unit conversion: 1 unit = 0.1 mm. 
 *  
 */
#define Xdm_To_Units(dm) (int) ((dm) * 1 )
#define X_To_dm(u) (int) (u) 
#define DEFAULT_BUTTON_X_TRAVEL_dm 2500

/**
 * # X-AXES PEROFORMANCES
 * 
 * Unit conversion: 2.5 unit = 0.1 mm. 
 *  
 */
#define Ydm_To_Units(dm) (int) (((dm) * 25) / 10 )
#define Y_To_dm(u) (int) (u) * 10 / 25
#define DEFAULT_BUTTON_Y_TRAVEL_dm 600

/**
 * # Z-AXES PEROFORMANCES
 * 
 * Unit conversion: 2 unit = 0.1 mm. 
 *  
 */
#define Zdm_To_Units(dm) (int) ((dm) * 2 )
#define Z_To_dm(u) (int) (u) / 2
#define DEFAULT_BUTTON_Z_TRAVEL_dm 1000


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
    
    struct{
        int xdm; //!< X position sensor
        int ydm; //!< Y position sensor
        int zdm; //!< Z position sensor
        int pos; //!< This is the last coordinate read
    }pointer;
    
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

typedef struct{
    bool status;
    int timer;
    int ton;
    int toff;
    int num_pulses;
    
}BUZZER_t;

ext DEVICE_t deviceStruct; 
ext void SetKeyMode(bool enable, bool step_mode);
ext void SetPowerSwitchStat(bool stat);
ext void BuzzerSet(int pulses, int ton, int toff);
ext void GetX(void);
ext void GetY(void);
ext void GetZ(void);

#endif // _MOTLIB_H