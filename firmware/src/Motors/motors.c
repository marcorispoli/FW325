#define _MOTORS_C

#include "application.h"
#include "motors.h"
#include "Protocol/protocol.h"

#define TIMER_TIC_us 7800

#define TIME_us_TIC(x) (x/TIMER_TIC_us)

/**
 * # X-AXES PEROFORMANCES
 * 
 * Unit conversion: 10 unit = 1mm. 
 *  
 */
#define Xmm_To_Units(mm) (int) ((mm) * 10 )
#define Xu_To_mm(u) (int) (u) / 10
#define DEFAULT_BUTTON_X_TRAVEL 250

/**
 * # X-AXES PEROFORMANCES
 * 
 * Unit conversion: 25 unit = 1mm. 
 *  
 */
#define Ymm_To_Units(mm) (int) ((mm) * 25 )
#define Yu_To_mm(u) (int) (u) / 25
#define DEFAULT_BUTTON_Y_TRAVEL 60
/**
 * # Z-AXES PEROFORMANCES
 * 
 * Unit conversion: 20 unit = 1mm. 
 *  
 */
#define Zmm_To_Units(mm) (int) ((mm) * 20 )
#define Zu_To_mm(u) (int) (u) / 20
#define DEFAULT_BUTTON_Z_TRAVEL 130

#define MOTOR_CALIB_MODE_KEEP_ALIVE 60 
int keep_alive_timer = 0;

// Change Working mode request from other sources
bool change_mode_request = false;
int change_mode;

static void motorUpdateStatus(void){
    StatusModeRegister.mode = motorStruct.exec_mode;
    StatusModeRegister.general_enable =  motorStruct.general_enable;
    StatusModeRegister.keyboard_enable = motorStruct.keyboard.keyboard_enable;
    StatusModeRegister.enable_feedback = uc_MOTOR_ENA_FEEDBACK_Get();
    StatusModeRegister.disable_needle_feedback = uc_NEEDLE_ENA_FEEDBACK_Get();

    // Updates the CAN Protocol registers
    updateStatusRegister((void*) &StatusModeRegister);
    
    int val = motorStruct.sensors.x;
    if(val<0) val = 0;
    StatusPositionRegister.xl = (unsigned char) (val & 0x00FF);
    StatusPositionRegister.xh = (unsigned char) ((val >> 8) & 0x00FF);
    
    val = motorStruct.sensors.y;
    if(val<0) val = 0;
    StatusPositionRegister.y = val;
    
    val = motorStruct.sensors.z;
    if(val<0) val = 0;
    StatusPositionRegister.z = val;

    updateStatusRegister((void*) &StatusPositionRegister);
    
}


static void motorGetXYZ(){
    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN5, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());
    motorStruct.sensors.x = (int) ADC0_ConversionResultGet() - 50;
    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN6, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());
    motorStruct.sensors.y = (int) ADC0_ConversionResultGet() - 50;
    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN7, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());    
    motorStruct.sensors.z = (int) ADC0_ConversionResultGet() - 50;
    
    
    return;
}

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
static void motorActivationMode(MOTOR_MODE_t mode){
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



bool getKeyPressed(void){
    
    motorStruct.keyboard.xp = ! uc_BUTTON_XP_Get();
    motorStruct.keyboard.xm = ! uc_BUTTON_XM_Get();
    motorStruct.keyboard.yp = ! uc_BUTTON_YP_Get();
    motorStruct.keyboard.ym = ! uc_BUTTON_YM_Get();
    motorStruct.keyboard.zp = ! uc_BUTTON_ZP_Get();
    motorStruct.keyboard.zm = ! uc_BUTTON_ZM_Get();
    
    return motorStruct.keyboard.xp || motorStruct.keyboard.xm || motorStruct.keyboard.yp || motorStruct.keyboard.ym || motorStruct.keyboard.zp || motorStruct.keyboard.zm;
    
}

void motorDisableModeManagement(void){
    static int key_pressed_timer = 0;
    static bool activated = false;
    
    // Disables the general enable 
    uc_MOTOR_GENERAL_ENABLE_Clear();
    motorStruct.general_enable = false;
    
    // Disable the keyboard activation enable
    uc_BUTTON_ENA_Clear();
    motorStruct.keyboard.keyboard_enable = false;
    
    // Motor disabled
    motorActivationMode(MOTORS_DISABLED);
    
    // Wait for the button release
    if(activated){
        BUZZER_Set();
        if(!getKeyPressed()){
            BUZZER_Clear();
            activated = false;
            motorSetCalibMode();          
            return;
        }
    }
    
    // Reads the status of the keyboard buttons: if the button is pressed
    // 1 second the working mode changes into the CALIB MODE     
    if(getKeyPressed()) key_pressed_timer++;
    else key_pressed_timer = 0;
    
    if(key_pressed_timer > TIME_us_TIC(1000000)){
        activated = true;
    }
    return;    
}


void motorCalibModeManagement(void){
    static bool activate_x = false;
    static bool activate_y = false;
    static bool activate_z = false;
    static int key_release_timer = 0;
    
    // Disables the general enable 
    uc_MOTOR_GENERAL_ENABLE_Set();
    motorStruct.general_enable = true;
    
    // Disable the keyboard activation enable
    uc_BUTTON_ENA_Set();
    motorStruct.keyboard.keyboard_enable = true;
    
    if(activate_x){
        if(getKeyPressed()) key_release_timer = TIME_us_TIC(200000);
        if(motorStruct.keyboard.xm){
            if(motorStruct.sensors.x <= 0) motorActivationMode(MOTOR_X_SHORT);
            return;
        }else if(motorStruct.keyboard.xp){
            if(motorStruct.sensors.x >= Xmm_To_Units(DEFAULT_BUTTON_X_TRAVEL)) motorActivationMode(MOTOR_X_SHORT);
            return;
        }else  motorActivationMode(MOTOR_X_SHORT);
        
    }else if(activate_y){
        if(getKeyPressed()) key_release_timer = TIME_us_TIC(200000);
        if(motorStruct.keyboard.ym){
            if(motorStruct.sensors.y <= 0) motorActivationMode(MOTOR_Y_SHORT);
            return;
        }else if(motorStruct.keyboard.yp){
            if(motorStruct.sensors.y >= Ymm_To_Units(DEFAULT_BUTTON_Y_TRAVEL)) motorActivationMode(MOTOR_Y_SHORT);
            return;
        }else motorActivationMode(MOTOR_Y_SHORT);        
    }
    else if(activate_z){
        if(getKeyPressed()) key_release_timer = TIME_us_TIC(200000);        
        if(motorStruct.keyboard.zm){
            if(motorStruct.sensors.z <= 0) motorActivationMode(MOTOR_Z_SHORT);
            return;
        }else if(motorStruct.keyboard.zp){
            if(motorStruct.sensors.z >= Zmm_To_Units(DEFAULT_BUTTON_Z_TRAVEL)) motorActivationMode(MOTOR_Z_SHORT);
            return;
        }else motorActivationMode(MOTOR_Z_SHORT);
        
    }
   
    
    // Disable the motor after a time since the key release
    if(key_release_timer){
        key_release_timer--;
        if(!key_release_timer){
            motorActivationMode(MOTORS_DISABLED);
            activate_x = false;
            activate_y = false;
            activate_z = false;
        }
        return;
    }
    
    
    if(!getKeyPressed()){
        motorActivationMode(MOTORS_DISABLED);
        
        if(keep_alive_timer == 0){
            motorSetDisableMode();
        }
        return;
    }
    
    keep_alive_timer = MOTOR_CALIB_MODE_KEEP_ALIVE;
    
    if(motorStruct.keyboard.zm){                
        if(motorStruct.sensors.z < Zmm_To_Units(DEFAULT_BUTTON_Z_TRAVEL)){
            activate_z = true;
            motorActivationMode(MOTOR_Z_DOWN);
        }
        
    }else if(motorStruct.keyboard.zp){        
        if(motorStruct.sensors.z > Zmm_To_Units(0)){
            motorActivationMode(MOTOR_Z_UP);
            activate_z = true;
        }
        
    }else if(motorStruct.keyboard.ym){
        
        if(motorStruct.sensors.y > 0){
            motorActivationMode(MOTOR_Y_HOME);
            activate_y = true;
        }     
        
    }else if(motorStruct.keyboard.yp){
        
         if(motorStruct.sensors.y < Ymm_To_Units(DEFAULT_BUTTON_Y_TRAVEL)){
            motorActivationMode(MOTOR_Y_FIELD);
            activate_y = true;
        }
    }else if(motorStruct.keyboard.xm){
        
         if(motorStruct.sensors.x > Xmm_To_Units(0)){
            motorActivationMode(MOTOR_X_RIGHT);
            activate_x = true;
        }         
    }else if(motorStruct.keyboard.xp){
        
        if(motorStruct.sensors.x < Xmm_To_Units(DEFAULT_BUTTON_X_TRAVEL)){
            motorActivationMode(MOTOR_X_LEFT);
            activate_x = true;
        }
    }else motorActivationMode(MOTORS_DISABLED);

}

void motorCommandModeManagement(void){
    
}

void motorServiceModeManagement(void){
    
    if(motorStruct.service.command == MOTOR_SERVICE_CYCLE_TEST)
    {    

        // Disables the general enable 
       uc_MOTOR_GENERAL_ENABLE_Set();
       motorStruct.general_enable = true;

       // Disable the keyboard activation enable
       uc_BUTTON_ENA_Clear();
       motorStruct.keyboard.keyboard_enable = false;

        if(getKeyPressed()){
            motorStruct.service.command = MOTOR_SERVICE_NO_COMMAND;
            motorActivationMode(MOTORS_DISABLED);
            return;
        }

        switch(motorStruct.service.sequence){
            case 0: 
                motorSetPower(0);
                motorStruct.service.sequence++;
                break;

            case 1: // Move Z up to 10
                if(motorStruct.sensors.z > Zmm_To_Units(10))  motorActivationMode(MOTOR_Z_UP);
                else{ 
                    motorActivationMode(MOTOR_Z_SHORT);
                    motorStruct.service.sequence++;
                }             
                break;

            case 2: // Move X to 240
                if(motorStruct.sensors.x < Xmm_To_Units(240))  motorActivationMode(MOTOR_X_LEFT);
                else{ 
                    motorActivationMode(MOTOR_X_SHORT);
                    motorStruct.service.sequence++;
                }             
                break;

           case 3: // Move Y to 60
               if(motorStruct.sensors.y < Ymm_To_Units(60))  motorActivationMode(MOTOR_Y_FIELD);
               else{ 
                   motorActivationMode(MOTOR_Y_SHORT);
                   motorStruct.service.sequence++;
               }             
           break;

           case 4: // Move Y to 0
               if(motorStruct.sensors.y > 0)  motorActivationMode(MOTOR_Y_HOME);
               else{ 
                   motorActivationMode(MOTOR_Y_SHORT);
                   motorStruct.service.sequence++;
               }             
           break;

           case 5: // Move X to 0
           if(motorStruct.sensors.x > 0)  motorActivationMode(MOTOR_X_RIGHT);
           else{ 
               motorActivationMode(MOTOR_X_SHORT);
               motorStruct.service.sequence++;
           }             
           break;

           case 6: // Move Z up to 100
               if(motorStruct.sensors.z < Zmm_To_Units(100))  motorActivationMode(MOTOR_Z_DOWN);
               else{ 
                   motorActivationMode(MOTOR_Z_SHORT);
                   motorStruct.service.sequence++;
               }             
               break;

            default: motorStruct.service.sequence = 0;

        }


    }else{

        // Disables the general enable 
       uc_MOTOR_GENERAL_ENABLE_Clear();
       motorStruct.general_enable = false;

       // Disable the keyboard activation enable
       uc_BUTTON_ENA_Clear();
       motorStruct.keyboard.keyboard_enable = false;

       // Disables the motor driver
       motorActivationMode(MOTORS_DISABLED);
    }
 
}

void motorLoop(void){
    
    // Always gets the XYZ
    motorGetXYZ(); 
    
    // Handles the execution mode
    switch(motorStruct.exec_mode){
        case DISABLE_MODE: motorDisableModeManagement(); break;
        case CALIB_MODE: motorCalibModeManagement(); break;
        case COMMAND_MODE: motorCommandModeManagement(); break;
        case SERVICE_MODE: motorServiceModeManagement(); break;
    }  
    
    if(change_mode_request){
        change_mode_request = false;
        motorStruct.exec_mode = change_mode;
        
        motorStruct.service.sequence = 0;
        motorStruct.service.command = 0;
        
        // Initializations
        if(motorStruct.exec_mode == CALIB_MODE){
            keep_alive_timer = MOTOR_CALIB_MODE_KEEP_ALIVE;
        }
    }
     
    // At loop completion updates the can protocol registers
    motorUpdateStatus();  
    
}

void motor1sLoop(void){
    
    // keep alive timer
    if(keep_alive_timer) keep_alive_timer--;
    
}

void motorInit(void){
    
    // Sets the initial operating mode
    motorStruct.exec_mode = (int) DISABLE_MODE;
    
    // Disables the driver and set low the motor power
    motorActivationMode(MOTORS_DISABLED);
    motorSetPower(0);
    
    // Disables the general enable 
    uc_MOTOR_GENERAL_ENABLE_Clear();
    motorStruct.general_enable = false;
    
    // Disable the keyboard activation enable
    uc_BUTTON_ENA_Clear();
    motorStruct.keyboard.keyboard_enable = false;
    
    motorUpdateStatus();    
}

void motorSetServiceMode(void){
    change_mode_request = true;
    change_mode = SERVICE_MODE;
}
void motorSetDisableMode(void){
    change_mode_request = true;
    change_mode = DISABLE_MODE;
    
}
void motorSetCommandMode(void){
    change_mode_request = true;
    change_mode = COMMAND_MODE;

}
void motorSetCalibMode(void){
    change_mode_request = true;
    change_mode = CALIB_MODE;
}

bool motorServiceTestCycle(void){
    if(motorStruct.exec_mode != SERVICE_MODE) return false;
    
    // Stops the cycle if it is running
    if(motorStruct.service.command != 0){
        motorStruct.service.command = 0;
        return true;
    }
    
    motorStruct.service.sequence = 0;
    motorStruct.service.command = MOTOR_SERVICE_CYCLE_TEST;
    return true;
}