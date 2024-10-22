#define _MOTORS_C

#include "application.h"
#include "motors.h"

static void motorSetPower(unsigned char val){
    if(val > 7) val = 7;
    val = 7 - val;
    motorStruct.power = val;
    
    if(val & 0x1) uc_VSEL0_Set();
    else uc_VSEL0_Clear();
    if(val & 0x2) uc_VSEL1_Set();
    else uc_VSEL1_Clear();
    if(val & 0x4) uc_VSEL2_Set();
    else uc_VSEL2_Clear();
    
    return;
}
static void motorSetMode(MOTOR_MODE_t mode){
    motorStruct.mode = mode;
    
    switch(mode){
        case MOTORS_DISABLED:
            uc_DRIVER_ENA_Clear();
            break;
            
        case MOTOR_X_LEFT:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Set();
             uc_MOT_DIR_Clear();
            break;            
        case MOTOR_X_RIGHT:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Set();
             uc_MOT_DIR_Set();
            break;        
        case MOTOR_X_SHORT:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Clear();
            break;

        case MOTOR_Y_HOME:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Clear();
            uc_MOT_STOP_Set();
             uc_MOT_DIR_Clear();
            break;
        case MOTOR_Y_FIELD:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Clear();
            uc_MOT_STOP_Set();
             uc_MOT_DIR_Set();
            break;        
        case MOTOR_Y_SHORT:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Set();
            uc_ENABLE_A_Clear();
            uc_MOT_STOP_Clear();            
            break;
    
        case MOTOR_Z_UP:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Clear();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Set();
             uc_MOT_DIR_Clear();
            break;
        case MOTOR_Z_DOWN:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Clear();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Set();
            uc_MOT_DIR_Set();
            break;        
            
        case MOTOR_Z_SHORT:
            uc_DRIVER_ENA_Set();
            uc_ENABLE_B_Clear();
            uc_ENABLE_A_Set();
            uc_MOT_STOP_Clear();
            break;

    }
}


void motorInit(void){
    motorStruct.general_enable = true;
    motorStruct.keyboard.keyboard_enable = true;
    motorSetMode(MOTORS_DISABLED);   
    motorSetPower(0);
    
    motorLoop();
}

void motorLoop(void){
    
    
    
    // Sets the general enable status
    if(motorStruct.general_enable) uc_MOTOR_GENERAL_ENABLE_Set();
    else uc_MOTOR_GENERAL_ENABLE_Clear();
    
    // Reads the current enable status
    motorStruct.enable_feedback = uc_MOTOR_ENA_FEEDBACK_Get();
    
    // Reads the needle disable status
    motorStruct.needle_disable_feedback = uc_NEEDLE_ENA_FEEDBACK_Get();
    
    // Set the keyboard enable 
    if(motorStruct.keyboard.keyboard_enable) uc_BUTTON_ENA_Set();
    else uc_BUTTON_ENA_Clear();
    
    // Reads the status of the keyboard buttons
    motorStruct.keyboard.xp = ! uc_BUTTON_XP_Get();
    motorStruct.keyboard.xm = ! uc_BUTTON_XM_Get();
    motorStruct.keyboard.yp = ! uc_BUTTON_YP_Get();
    motorStruct.keyboard.ym = ! uc_BUTTON_YM_Get();
    motorStruct.keyboard.zp = ! uc_BUTTON_ZP_Get();
    motorStruct.keyboard.zm = ! uc_BUTTON_ZM_Get();
    
    if(motorStruct.keyboard.zm){
        motorSetMode(MOTOR_Z_DOWN);
    }else if(motorStruct.keyboard.zp){
        motorSetMode(MOTOR_Z_UP);
    }else if(motorStruct.keyboard.ym){
        motorSetMode(MOTOR_Y_HOME);
    }else if(motorStruct.keyboard.yp){
        motorSetMode(MOTOR_Y_FIELD);
    }else if(motorStruct.keyboard.xm){
        motorSetMode(MOTOR_X_RIGHT);
    }else if(motorStruct.keyboard.xp){
        motorSetMode(MOTOR_X_LEFT);
    }else{
        motorSetMode(MOTORS_DISABLED);
    }
    
}
