#define _MOTORS_C

#include "application.h"
#include "motors.h"
#include "Protocol/protocol.h"
#include "../main.h"

#define TIMER_TIC_us 7800
#define TIME_us_TIC(x) (x/TIMER_TIC_us)

#define MOTOR_HOLD_TIME 80


#define MOTOR_CALIB_MODE_KEEP_ALIVE_7ms 8561 


// Change Working mode request from other sources
bool change_mode_request = false;
int change_mode;

#define MAX_Z_POSITION_dm 1350
#define MAX_X_POSITION_dm 2580
#define MAX_Y_POSITION_dm 700

// #define abs(x) (x<0) ? (-(x)): (x)

static int abs(int val){
    if(val < 0) return -1*val;
    else return val;
}

static void motorActivationHandler(void);



/**
 * \ingroup MOTMOD
 * This function allows to select up to 7 power levels:
 * 
 * |Level|Voltage (respect 24V) |
 * |:--|:--|
 * |0|38%|
 * |1|42%|
 * |2|46%|
 * |3|50%|
 * |4|65%|
 * |5|71%|
 * |6|85%|
 * 
 * @param val: this is the requested power level
 * 
 */
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

/**
 * \ingroup MOTMOD
 * This function sets the current driver activation mode.
 * 
 * The driver can be configured to the following modes:
 * +  MOTORS_DISABLED: all the motors are set to high impedance;
 * +  MOTOR_X_LEFT: the X motor is enabled and the direction is set to Left direction;
 * +  MOTOR_X_RIGHT: the X motor is enabled and the direction is set to Right direction;
 * +  MOTOR_X_SHORT: the X motor is enabled and the output are closed to ground;
 * +  MOTOR_Y_HOME: the Y motor is enabled and the direction is set to Home direction;
 * +  MOTOR_Y_FIELD: the Y motor is enabled and the direction is set to In Field direction;
 * +  MOTOR_Y_SHORT: the Y motor is enabled and the output are closed to ground;
 * +  MOTOR_Z_UP: the Z motor is enabled and the direction is set to Up direction;
 * +  MOTOR_Z_DOWN: the Z motor is enabled and the direction is set to Down direction;
 * +  MOTOR_Z_SHORT: the Z motor is enabled and the output are closed to ground;  
 * 
 * @param mode: this is the requested driver output mode
 */
static void motorDriverOutput(MOTOR_MODE_t mode){
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



/**
 * \addtogroup MOTMOD
 * 
 * ## MOTOR ACTIVATION MODE DESCRIPTION
 * 
 * The firmware implements for possible modes to handle the motor activation:
 * + DISABLE MODE: this working mode disables the usage of the motors at all;
 * + CALIBRATION MODE: this working mode allows to use the external (and internal) keyboard to
 * activate the motor for the position calibration purpose;
 * + COMMAND MODE: this is the working mode dedicated to the operative usage;
 * + SERVICE MODE: this is the working mode reserved for test procedures.
 * 
 * The following chapters describe in detail every working mode. 
 */ 
 
/**
 * \addtogroup MOTMOD
 * 
 * ### DISABLE WORKING MODE DESCRIPTION
 * 
 * In the Disable working mode, the motors are disabled and 
 * the safety power switch is kept open (both general and key enables are off).
 * 
 * The firmware activates this working mode after the system startup.
 * 
 * From the Disable status, pressing a button for almost one second, it is possible\n
 * to enter the Calibration Mode: 
 * + the buzzer emits a sound signaling the change in the working mode status;
 * 
 */ 

/// \ingroup MOTMOD
/// This is the Disable Mode handling routine
void motorDisableModeManagement(void){
    
    if(motorStruct.key_timer > TIME_us_TIC(1000000)){
        // Wait for the key release before to change the current status
        if(deviceStruct.keyboard.flags.key_present) return;        
        motorSetCalibMode();          
        return;
    }
    
    // Reads the status of the keyboard buttons: if the button is pressed
    // 1 second the working mode changes into the CALIB MODE     
    if(deviceStruct.keyboard.flags.key_present) motorStruct.key_timer++;
    else motorStruct.key_timer = 0;
    
    if(motorStruct.key_timer > TIME_us_TIC(1000000)) BuzzerSet(2,5,5);
    
    return;    
}

/**
 * \addtogroup MOTMOD
 * 
 * ### MOTOR CALIBRATION MODE DESCRIPTION
 * 
 * The calibration mode handles the hardware position calibration.
 * 
 * The calibration process involves the setting of the Zero position trimmer
 * and the Travel trimmer for all the axes position sensors.
 * 
 * The zero position trimmer sets the mechanical position where the 
 * position value is recognized as 0.
 * 
 * The Travel trimmer regulates the unit scale: the trimmer shall be adjusted 
 * so that the measured distance equals to the expected.
 * 
 * There are two push buttons for any axe to help the trimmer adjustment.
 * 
 * The X adjustment buttons:
 * + The X- button activates the X axes to the current 0 position:\n
 *  the trimmer shall be changed in order to get the expected mechanical zero position.
 * + The X+ button activates the X axes to the position 250mm: the trimmer shall be adjusted \n
 * so that the actual travel distance matches with the 250mm.
 *  
 * The Y adjustment buttons:
 * + The Y- button activates the Y axes to the current 0 position:\n
 *  the trimmer shall be changed in order to get the expected mechanical zero position.
 * + The Y+ button activates the Y axes to the position 60mm: the trimmer shall be adjusted \n
 * so that the actual travel distance matches with the 60mm.
 *  
 * The Z adjustment buttons:
 * + The Z- button activates the Z axes to the current 0 position:\n
 *  the trimmer shall be changed in order to get the expected mechanical zero position.
 * + The Z+ button activates the Z axes to the position 130mm: the trimmer shall be adjusted \n
 * so that the actual travel distance matches with the 130mm.
 * 
 * \important The Calibration Mode exits to the Disable Mode 
 * if no keys are pressed within 60 seconds.  
 * 
 *  NOTE: the activation requires that a key button is kept pressed during 
 *  the whole travel.
 * 
 *  The Buzzer will sound with a single pulse when the target position is detected.
 *  
 *  In case of mechanical block (impact with mechanical parts) the activation terminates
 *  and a twin set of buzzer pulses will then be generated.     
 * 
 * \note When an activation terminates whether in target position, in obstacle or in the case
 * of button release, the driver shorts for 500ms the motor wires in order 
 * to stop the rotor inertia.  
 *  
 */
void motorCalibModeManagement(void){
    
   
    // Motor activation 
    if(deviceStruct.keyboard.hw.zm){      
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveZ(0,false,true); // target, no protocol, need key pressed        
    }else if(deviceStruct.keyboard.hw.zp){  
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveZ(DEFAULT_BUTTON_Z_TRAVEL_dm,false,true); // target, no protocol, need key pressed                        
    }else if(deviceStruct.keyboard.hw.ym){
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveY(0,false,true); // target, no protocol, need key pressed        
    }else if(deviceStruct.keyboard.hw.yp){
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveY(DEFAULT_BUTTON_Y_TRAVEL_dm,false,true); // target, no protocol, need key pressed        
    }else if(deviceStruct.keyboard.hw.xm){
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveX(0,false,true); // target, no protocol, need key pressed                
    }else if(deviceStruct.keyboard.hw.xp){
        motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        motorMoveX(DEFAULT_BUTTON_X_TRAVEL_dm,false,true); // target, no protocol, need key pressed        
    }else{
        motorDriverOutput(MOTORS_DISABLED);
        
        motorStruct.key_timer--;
        
        // When no key is pressed, after a timeout the calibration mode expires
        if(motorStruct.key_timer == 0){
            BuzzerSet(1,20,5);
            motorSetDisableMode();
        }
    }

}


unsigned char getPowerFromDistance(int distance, int min_power){
   
    int val;
    if(distance > 1000) val = 7;
    else if(distance > 500)  val = 6;
    else if(distance > 400)  val = 5;
    else if(distance > 300)  val = 4;
    else if(distance > 200)  val = 3;
    else if(distance > 100)  val = 2;
    else if(distance > 50)   val = 1;
    else val = 0;
    
    if(val < min_power) val = min_power;
    
    return val;
    
}

/**
 * \addtogroup MOTMOD
 * 
 * ### SERVICE MODE MANAGEMENT
 * 
 * This is the Service worning mode management.
 * 
 * In service mode, some service command can be executed:
 * + MOTOR_SERVICE_CYCLE_TEST: executes an infinite set of cycles  moving all the axes in and out positions.
 * The command terminates when a key button is pressed or a protocol command is received;
 * When no command is in action, the motor driver is disabled as well the  safety power switch.
 * 
 *  
 */


void motorServiceModeManagement(void){
    int distance;
    int abs_dm_distance;
    int min_power;
    
    if(motorStruct.service_mode.command == MOTOR_SERVICE_CYCLE_TEST)
    {    

        // Enable The Power Switch
        SetPowerSwitchStat(true);

        // Disable the keyboard activation enable
        SetKeyMode(false,false);

        if(deviceStruct.keyboard.flags.key_present){
            motorStruct.service_mode.command = MOTOR_SERVICE_NO_COMMAND;
            motorDriverOutput(MOTORS_DISABLED);
            return;
        }

        switch(motorStruct.service_mode.sequence){
            case 0: 
                motorSetPower(0);
                motorStruct.service_mode.sequence++;
                break;

            case 1: // Move Z up to 10
                
                if(deviceStruct.sensors.z > Zdm_To_Units(100)){  
                    min_power = 2;
                    distance = Zdm_To_Units(100) - deviceStruct.sensors.z;
                    abs_dm_distance = Z_To_dm(abs(distance));
                    motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                    motorDriverOutput(MOTOR_Z_UP);
                }else{ 
                    motorDriverOutput(MOTOR_Z_SHORT);
                    motorStruct.service_mode.sequence++;
                }             
                break;

            case 2: // Move X to 240
                if(deviceStruct.sensors.x < Xdm_To_Units(2400)){  
                    min_power = 0;
                    distance = Xdm_To_Units(2400) - deviceStruct.sensors.x;
                    abs_dm_distance = X_To_dm(abs(distance));
                    motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                    motorDriverOutput(MOTOR_X_LEFT);
                }else{ 
                    motorDriverOutput(MOTOR_X_SHORT);
                    motorStruct.service_mode.sequence++;
                }             
                break;

           case 3: // Move Y to 60
               if(deviceStruct.sensors.y < Ydm_To_Units(600)){  
                   min_power = 0;
                   distance = Ydm_To_Units(600) - deviceStruct.sensors.y;
                   abs_dm_distance = Y_To_dm(abs(distance));
                   motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                   motorDriverOutput(MOTOR_Y_FIELD);
               }else{ 
                   motorDriverOutput(MOTOR_Y_SHORT);
                   motorStruct.service_mode.sequence++;
               }             
           break;

           case 4: // Move Y to 0
               if(deviceStruct.sensors.y > 0){  
                   min_power = 0;
                   distance = 0 - deviceStruct.sensors.y;
                   abs_dm_distance = Y_To_dm(abs(distance));
                   motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                   motorDriverOutput(MOTOR_Y_HOME);
               }else{ 
                   motorDriverOutput(MOTOR_Y_SHORT);
                   motorStruct.service_mode.sequence++;
               }             
           break;

           case 5: // Move X to 0
           if(deviceStruct.sensors.x > 0){  
                min_power = 0;
                distance = 0 - deviceStruct.sensors.x;
                abs_dm_distance = X_To_dm(abs(distance));
                motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                motorDriverOutput(MOTOR_X_RIGHT);
           }else{ 
               motorDriverOutput(MOTOR_X_SHORT);
               motorStruct.service_mode.sequence++;
           }             
           break;

           case 6: // Move Z up to 100
               if(deviceStruct.sensors.z < Zdm_To_Units(1000)){  
                   min_power = 0;
                   distance = Zdm_To_Units(1000) - deviceStruct.sensors.z;
                   abs_dm_distance = Z_To_dm(abs(distance));
                   motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                   motorDriverOutput(MOTOR_Z_UP);
                   motorDriverOutput(MOTOR_Z_DOWN);
               }else{ 
                   motorDriverOutput(MOTOR_Z_SHORT);
                   motorStruct.service_mode.sequence++;
               }             
               break;

            default: motorStruct.service_mode.sequence = 0;

        }


    }else{

       // Disables the Power switch
       SetPowerSwitchStat(false);

       // Disable the keyboard activation enable
       SetKeyMode(false,false);

       // Disables the motor driver
       motorDriverOutput(MOTORS_DISABLED);
    }
 
}

/**
 * This is the Main Workflow management routine.
 * 
 * This function handles the current workflow status, 
 * calling the proper workflow handling routine.
 * 
 * At the end of every workflow routine sequence, 
 * the protocol status registers are updated.
 * 
 */
void motorLoop(void){

    
    // A motor is activated: handle the activation
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND){
        motorActivationHandler();
        return;
    }
      
    
    // Handles the execution mode
    switch(motorStruct.exec_mode){
        case DISABLE_MODE: motorDisableModeManagement(); break;
        case CALIB_MODE: motorCalibModeManagement(); break;
        case COMMAND_MODE: ; break;
        case SERVICE_MODE: motorServiceModeManagement(); break;
    }  
    
    if(change_mode_request){
        change_mode_request = false;
        StatusModeRegister.mode = motorStruct.exec_mode = change_mode;
        
        motorStruct.service_mode.sequence = 0;
        motorStruct.service_mode.command = 0;
        motorStruct.command_mode.sequence = 0;
        motorStruct.command_mode.command = 0;
        
        // Disables The Keyboard
        SetKeyMode(false,false);
        
        // Initializations
        if(motorStruct.exec_mode == CALIB_MODE){
            motorDriverOutput(MOTORS_DISABLED);
            // Enable the power switch
            SetPowerSwitchStat(true);
            // Enable the keyboard
            SetKeyMode(true,false);
            
            // Reset the  keep alive timer
            motorStruct.key_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_7ms;
        
        }else if(motorStruct.exec_mode == COMMAND_MODE){            
            // Starts with the Drivers disabled
            motorDriverOutput(MOTORS_DISABLED);
            // Enable the power switch
            SetPowerSwitchStat(true);
            // Enable the keyboard
            SetKeyMode(true,false);
        }
        else if(motorStruct.exec_mode == DISABLE_MODE){            
            // Starts with the Drivers disabled
            motorDriverOutput(MOTORS_DISABLED);
            // Enable the power switch
            SetPowerSwitchStat(false);
            // Enable the keyboard
            SetKeyMode(false,false);
            
            motorStruct.key_timer = 0;
        }
    }
    
    // Gets the current outputs and update the motor stat
    StatusModeRegister.power_sw_status = deviceStruct.power_sw_stat = uc_MOTOR_ENA_FEEDBACK_Get();
    
}


/**
 * This function shall be called at the beginning of the application
 * in order to initialize the motor module.
 * 
 * At the end of the initialization the module is set in the Disabled Mode.
 * 
 */
void motorInit(void){
    
    // Sets the initial operating mode
    motorStruct.exec_mode = (int) DISABLE_MODE;
    StatusModeRegister.mode = motorStruct.exec_mode;
    
    // Disables the driver and set low the motor power
    motorDriverOutput(MOTORS_DISABLED);
    motorSetPower(0);
    
    // Disables the general enable 
    SetPowerSwitchStat(false);
    
    motorStruct.service_mode.command = 0;
    motorStruct.command_mode.command = 0;
    motorStruct.command_mode.sequence = 0;    
    motorStruct.command_mode.abort_request = false;
}

/**
 * This function requests to change the current workflow to Service Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetServiceMode(void){
    
    // Only in Idle can be changed the current mode
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return;
    
    unsigned char target_status = SERVICE_MODE;
    
    // Already in the target status or already in changing mode to target status
    if(motorStruct.exec_mode == target_status) return;
    if((change_mode_request) && (change_mode == target_status)) return;    
    change_mode_request = true;
    change_mode = target_status;
    
   
    
}

/**
 * This function requests to change the current workflow to Disable Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetDisableMode(void){
     // Only in Idle can be changed the current mode
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return;
    
    unsigned char target_status = DISABLE_MODE;
    
    // Already in the target status or already in changing mode to target status
    if(motorStruct.exec_mode == target_status) return;
    if((change_mode_request) && (change_mode == target_status)) return;    
    change_mode_request = true;
    change_mode = target_status;

   
  
}

/**
 * This function requests to change the current workflow to Command Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetCommandMode(void){
     // Only in Idle can be changed the current mode
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return;
    
     unsigned char target_status = COMMAND_MODE;
    
    // Already in the target status or already in changing mode to target status
    if(motorStruct.exec_mode == target_status) return;
    if((change_mode_request) && (change_mode == target_status)) return;    
    change_mode_request = true;
    change_mode = target_status;
    
  
}

/**
 * This function requests to change the current workflow to Calibration Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetCalibMode(void){
     // Only in Idle can be changed the current mode
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return;
    
    unsigned char target_status = CALIB_MODE;
    
    // Already in the target status or already in changing mode to target status
    if(motorStruct.exec_mode == target_status) return;
    if((change_mode_request) && (change_mode == target_status)) return;    
    change_mode_request = true;
    change_mode = target_status;
     
}

/**
 * This function requests to activate the Service cycle test.
 * 
 * The test will start at the next MotorLoop() execution.
 */
bool motorServiceTestCycle(void){
    if(motorStruct.exec_mode != SERVICE_MODE) return false;
    
    // Stops the cycle if it is running
    if(motorStruct.service_mode.command != 0){
        motorStruct.service_mode.command = 0;
        return true;
    }
    
    motorStruct.service_mode.sequence = 0;
    motorStruct.service_mode.command = MOTOR_SERVICE_CYCLE_TEST;
    return true;
}

MOTOR_COMMAND_RESULTS_t  motorMoveX(int tXdm, bool protocol, bool key_request){

    // If the command is generated by the external device, the current mode shall be COMMAND_MODE
    if((protocol) && (motorStruct.exec_mode != COMMAND_MODE)) return MOTOR_ERROR_INVALID_MODE; 
    if(deviceStruct.power_sw_stat == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;

   // Upgrade the position and checks if already in position
    GetX();
    int distance = abs(deviceStruct.pointer.xdm - tXdm);
    if(distance < 5) return MOTOR_ALREADY_IN_POSITION;
    
    // Command accepted
    motorStruct.command_mode.command = MOTOR_COMMAND_X;
    motorStruct.command_mode.sequence = 0;    
    motorStruct.command_mode.target = tXdm;
    motorStruct.command_mode.protocol_activation = protocol;
    motorStruct.command_mode.key_requested = key_request;
    motorStruct.command_mode.abort_request = false;
    motorStruct.command_mode.activation_timer = 0;
    motorStruct.command_mode.min_power = 0;
    motorStruct.command_mode.termination_fase = false;
    motorStruct.command_mode.termination_timer = MOTOR_HOLD_TIME;
  
    // 1dm every 7ms + 350ms
    motorStruct.command_mode.activation_timeout = distance + 50;
    return MOTOR_COMMAND_EXECUTING;
}

MOTOR_COMMAND_RESULTS_t  motorMoveY(int tYdm, bool protocol, bool key_request){
    
    // If the command is generated by the external device, the current mode shall be COMMAND_MODE
    if((protocol) && (motorStruct.exec_mode != COMMAND_MODE)) return MOTOR_ERROR_INVALID_MODE; 
    if(deviceStruct.power_sw_stat == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;
    
    // Upgrade the position and checks if already in position
    GetY();    
    int distance = abs(deviceStruct.pointer.ydm - tYdm);
    if(distance < 5) return MOTOR_ALREADY_IN_POSITION;
    
    // Command accepted
    motorStruct.command_mode.command = MOTOR_COMMAND_Y;
    motorStruct.command_mode.sequence = 0;    
    motorStruct.command_mode.target = tYdm;
    motorStruct.command_mode.protocol_activation = protocol;
    motorStruct.command_mode.key_requested = key_request;
    motorStruct.command_mode.abort_request = false;
    motorStruct.command_mode.activation_timer = 0;
    motorStruct.command_mode.min_power = 0;
    motorStruct.command_mode.termination_fase = false;
    motorStruct.command_mode.termination_timer = MOTOR_HOLD_TIME;
   
    // 1dm every 7ms + 350ms
    motorStruct.command_mode.activation_timeout = distance + 50;
    return MOTOR_COMMAND_EXECUTING;
}

MOTOR_COMMAND_RESULTS_t  motorMoveZ(int tZdm, bool protocol, bool key_request){
     // If the command is generated by the external device, the current mode shall be COMMAND_MODE
    if((protocol) && (motorStruct.exec_mode != COMMAND_MODE)) return MOTOR_ERROR_INVALID_MODE; 
    if(deviceStruct.power_sw_stat == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;
    
    if(tZdm > MAX_Z_POSITION_dm) return MOTOR_ERROR_INVALID_POSITION;

    // Upgrade the position and checks if already in position
    GetZ();
    int distance = abs(deviceStruct.pointer.zdm - tZdm);
    if(distance < 5) return MOTOR_ALREADY_IN_POSITION;
    
  // Command accepted
    motorStruct.command_mode.command = MOTOR_COMMAND_Z;
    motorStruct.command_mode.sequence = 0;    
    motorStruct.command_mode.target = tZdm;
    motorStruct.command_mode.protocol_activation = protocol;
    motorStruct.command_mode.key_requested = key_request;
    motorStruct.command_mode.abort_request = false;
    motorStruct.command_mode.activation_timer = 0;
    motorStruct.command_mode.min_power = 0;
    motorStruct.command_mode.termination_fase = false;
    motorStruct.command_mode.termination_timer = MOTOR_HOLD_TIME;
  
    // 1dm every 7ms + 350ms
    motorStruct.command_mode.activation_timeout = distance * 2 + 50;
    return MOTOR_COMMAND_EXECUTING;
}

void motorAbort(void){
     motorStruct.command_mode.abort_request = true;
}


bool  motorEnableKeyStepMode(unsigned char par){
    if(par == 0) {
        SetKeyMode(false,false);
        return true;
    }
    
    if(motorStruct.exec_mode != COMMAND_MODE) return false;
    SetKeyMode(true,true);
    return true;
}

static void MotorCommandPositionHold(void){
    if(motorStruct.command_mode.command == MOTOR_COMMAND_X){                
        motorDriverOutput(MOTOR_X_SHORT);
    }else if(motorStruct.command_mode.command == MOTOR_COMMAND_Y){
        motorDriverOutput(MOTOR_Y_SHORT);
    }else{
        motorDriverOutput(MOTOR_Z_SHORT);
    }
    return;
}
#define BLANK_OBSTACLE_INITIAL_TIME 50
#define OBSTACLE_DELAY_TIME_TEST 15
#define OBSTACLE_DELTA_POSITION 2
void motorActivationHandler(void){
        
        int distance;        
        static int delay_timer = 0;
        static int obstacle_position = 0;
        
        
        if(motorStruct.command_mode.termination_fase){
            
            // Waits for the key release
            if(motorStruct.command_mode.key_requested){
                // Waits for the keyboard release
                if(deviceStruct.keyboard.flags.key_present) return;
            }
            
            // Waits for the termination timer
            if(motorStruct.command_mode.termination_timer){
                motorStruct.command_mode.termination_timer--;
                if(!motorStruct.command_mode.termination_timer){
                    motorDriverOutput(MOTORS_DISABLED);
                    // Command termination here
                    motorStruct.command_mode.termination_fase = false;
                    motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;            
                }
            }
            
            return;
        }
        
        // Sets the current positions data
        if(motorStruct.command_mode.command == MOTOR_COMMAND_X){
            GetX();                      
        }else if(motorStruct.command_mode.command == MOTOR_COMMAND_Y){
            GetY();                                 
        }else{
            GetZ();            
        }     
        distance = motorStruct.command_mode.target - deviceStruct.pointer.pos;
        
        
        motorStruct.command_mode.activation_timer++;
        delay_timer++;
        
       
        // External Abort Request
        if(motorStruct.command_mode.abort_request){
            motorStruct.command_mode.termination_fase = true;
            if(motorStruct.command_mode.protocol_activation) MET_Can_Protocol_returnCommandAborted();
            BuzzerSet(5,5,5);
            MotorCommandPositionHold();
            return;
        }
            
        // Checks if the button is pressed (if required))
        if(motorStruct.command_mode.key_requested){
            if(!deviceStruct.keyboard.flags.key_present){ 
                motorStruct.command_mode.termination_fase = true;
                if(motorStruct.command_mode.protocol_activation) MET_Can_Protocol_returnCommandAborted();
                BuzzerSet(2,5,5);
                MotorCommandPositionHold();
                return;
            }
        }
    
        // Test for the obstacle detection
        // The obstacle is not tested in the initial fase of the activation
        if(motorStruct.command_mode.activation_timer > BLANK_OBSTACLE_INITIAL_TIME){
            if(delay_timer > OBSTACLE_DELAY_TIME_TEST){
                if(abs(deviceStruct.pointer.pos-obstacle_position) > OBSTACLE_DELTA_POSITION){
                    // OK
                    delay_timer = 0;
                    obstacle_position = deviceStruct.pointer.pos;                    
                }else{
                    // Obstacle Detected
                    motorStruct.command_mode.termination_fase = true;
                    if(motorStruct.command_mode.protocol_activation) MET_Can_Protocol_returnCommandAborted();
                    BuzzerSet(3,5,5);
                    MotorCommandPositionHold();
                    return;
                }
            }
        }else{
            delay_timer = 0;
            obstacle_position = deviceStruct.pointer.pos; // Store the current position
        }
        
        // Test the Activation timeout
        if(motorStruct.command_mode.activation_timer > motorStruct.command_mode.activation_timeout) {
           motorStruct.command_mode.termination_fase = true;
           if(motorStruct.command_mode.protocol_activation) MET_Can_Protocol_returnCommandAborted();
           BuzzerSet(4,5,5);
           MotorCommandPositionHold();
           return;
        }

        
        // Sets the power based on the distance
        motorSetPower(getPowerFromDistance(abs(distance),motorStruct.command_mode.min_power));        
        
        // Verifies the target
        if(abs(distance) < 2 ){
         
            MotorCommandPositionHold();
            
            motorStruct.command_mode.termination_fase = true;                    
            if(motorStruct.command_mode.protocol_activation){
                unsigned char L = (unsigned char) (deviceStruct.pointer.pos & 0xff);
                unsigned char H = (unsigned char) ((deviceStruct.pointer.pos >>8) & 0xff);
                MET_Can_Protocol_returnCommandExecuted(L,H);                
            }
            BuzzerSet(1,10,10);
            return;
                      
        }else if(distance < 0){
            if(motorStruct.command_mode.command == MOTOR_COMMAND_X){   
                motorStruct.command_mode.min_power = 0;
                motorDriverOutput(MOTOR_X_RIGHT);
            }else if(motorStruct.command_mode.command == MOTOR_COMMAND_Y){
                motorStruct.command_mode.min_power = 0;
                motorDriverOutput(MOTOR_Y_HOME);
            }else{
                motorStruct.command_mode.min_power = 2; // When activated upward the power cannot be too low
                motorDriverOutput(MOTOR_Z_UP);
            }            
        }else{
            if(motorStruct.command_mode.command == MOTOR_COMMAND_X){                
                motorStruct.command_mode.min_power = 0;
                motorDriverOutput(MOTOR_X_LEFT);
            }else if(motorStruct.command_mode.command == MOTOR_COMMAND_Y){
                motorStruct.command_mode.min_power = 0;
                motorDriverOutput(MOTOR_Y_FIELD);
            }else{
                motorStruct.command_mode.min_power = 0;
                motorDriverOutput(MOTOR_Z_DOWN);
            }            
        }
        return;
        
}
