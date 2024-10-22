
#ifndef _MOTORS_H    
#define _MOTORS_H

#include "definitions.h"

#undef ext
#undef ext_static

#ifdef _MOTORS_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

typedef enum{
    MOTORS_DISABLED = 0,
    MOTOR_X_LEFT,
    MOTOR_X_RIGHT,
    MOTOR_X_SHORT,
    MOTOR_Y_HOME,
    MOTOR_Y_FIELD,
    MOTOR_Y_SHORT,
    MOTOR_Z_UP,
    MOTOR_Z_DOWN,
    MOTOR_Z_SHORT,            
}MOTOR_MODE_t;

typedef struct{
    bool general_enable;
    bool enable_feedback;
    bool needle_disable_feedback;
    MOTOR_MODE_t mode;
    unsigned char power;
    
    struct{
        unsigned short x;
        unsigned short y;
        unsigned short z;
    }sensors;
    
    struct{
        bool xp;
        bool xm;
        bool yp;
        bool ym;
        bool zp;
        bool zm;
        bool keyboard_enable;        
    }keyboard;
   
}MOTORS_t;

ext MOTORS_t motorStruct; 
ext void motorLoop(void);
ext void motorInit(void);

#endif // _MOTLIB_H