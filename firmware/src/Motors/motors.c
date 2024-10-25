#define _MOTORS_C

#include "application.h"
#include "motors.h"
#include "Protocol/protocol.h"

#define TIMER_TIC_us 7800
#define TIME_us_TIC(x) (x/TIMER_TIC_us)

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

#define MOTOR_CALIB_MODE_KEEP_ALIVE_s 60 
int keep_alive_timer = 0;

// Change Working mode request from other sources
bool change_mode_request = false;
int change_mode;

#define MAX_Z_POSITION_dm 1350
#define MAX_X_POSITION_dm 2580
#define MAX_Y_POSITION_dm 700

#define abs(x) (x<0) ? (-(x)): (x)

/**
 * \ingroup MOTMOD
 * 
 * This function updates all together all the Protocol status 
 * register that depends by this module.
 * 
 * The following status are updated here:
 * + The WORKING_MODE status register;
 * + The POSITION status register;
 * 
 * This function is called in the MotorLoop() routine 
 * that is always called during the workflow after every workflow iteration.
 * 
 */
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
    StatusXYPositionRegister.XL = (unsigned char) (val & 0x00FF);
    StatusXYPositionRegister.XH = (unsigned char) ((val >> 8) & 0x00FF);
    
    val = motorStruct.sensors.y;
    if(val<0) val = 0;
    StatusXYPositionRegister.YL = (unsigned char) (val & 0x00FF);
    StatusXYPositionRegister.YH = (unsigned char) ((val >> 8) & 0x00FF);
    
    val = motorStruct.sensors.z;
    if(val<0) val = 0;
    StatusZPositionRegister.ZL = (unsigned char) (val & 0x00FF);
    StatusZPositionRegister.ZH = (unsigned char) ((val >> 8) & 0x00FF);

    updateStatusRegister((void*) &StatusZPositionRegister);
    
}

/**
 * \ingroup MOTMOD
 * 
 * This function converts the X position sensor 
 * and convert it into the position units.
 * 
 * The position shall be calibrated with the hardware trimmers on the board.
 * According with the ADC0 module setting, the routine takes about 7us to completes.
 */
static void motorGetX(){
    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN5, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());
    motorStruct.sensors.x = (int) ADC0_ConversionResultGet() - 50;
    return;
}

/**
 * \ingroup MOTMOD
 * This function converts the Y position sensor 
 * and convert it into the position units.
 * 
 * The position shall be calibrated with the hardware trimmers on the board.
 * According with the ADC0 module setting, the routine takes about 7us to completes. 
 */
static void motorGetY(){
    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN6, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());
    motorStruct.sensors.y = (int) ADC0_ConversionResultGet() - 50;
    return;
}

/**
 * \ingroup MOTMOD
 * This function converts the Z position sensor 
 * and convert it into the position units.
 * 
 * The position shall be calibrated with the hardware trimmers on the board.
 * According with the ADC0 module setting, the routine takes about 7us to completes.
 */
static void motorGetZ(){    
    ADC0_ChannelSelect( ADC_POSINPUT_AIN7, ADC_NEGINPUT_GND );
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet());    
    motorStruct.sensors.z = (int) ADC0_ConversionResultGet() - 50;
    return;
}

/**
 * \ingroup MOTMOD
 * This function calls the conversion routines to get the X,Y and Z positions.
 * According with the ADC0 module setting, the routine takes about 21us to completes.
 */
static void motorGetXYZ(){
    motorGetX();
    motorGetY();
    motorGetZ();    
    return;
}

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
 * \ingroup MOTMOD
 * 
 * This function polls the keyboard inputs and provides 
 * the current pression event.
 * 
 * @return the key pressed event.
 */
bool getKeyPressed(void){
    
    motorStruct.keyboard.xp = ! uc_BUTTON_XP_Get();
    motorStruct.keyboard.xm = ! uc_BUTTON_XM_Get();
    motorStruct.keyboard.yp = ! uc_BUTTON_YP_Get();
    motorStruct.keyboard.ym = ! uc_BUTTON_YM_Get();
    motorStruct.keyboard.zp = ! uc_BUTTON_ZP_Get();
    motorStruct.keyboard.zm = ! uc_BUTTON_ZM_Get();
    
    return motorStruct.keyboard.xp || motorStruct.keyboard.xm || motorStruct.keyboard.yp || motorStruct.keyboard.ym || motorStruct.keyboard.zp || motorStruct.keyboard.zm;
    
}

/**
 * \addtogroup MOTMOD
 * 
 * ## Disable Mode Workflow description
 * 
 * In the Disable Status workflow the motors are disabled and 
 * the safety power switch is open (both general and key enables are off).
 * 
 * From the Disable status, pressing a button for almost one second, it is possible\n
 * to enter the Calibration Mode: 
 * + the buzzer emits a sound signaling the change in the workflow status;
 * 
 * \note the disable mode workflow is the default mode after the board startup. 
 * 
 */ 

/// \ingroup MOTMOD
/// This is the Disable Mode handling routine
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
    motorDriverOutput(MOTORS_DISABLED);
    
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

/**
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
 * The Calibration Mode exits to the Disable Mode 
 * if no key button should be pressed within 60 seconds.  
 * 
 *  NOTE: the activation requires that a key button is kept pressed during 
 *  the whole travel.
 * 
 *  The Buzzer will sound with a single pulse when the target position is detected.
 *  
 *  In case of mechanical block (impact with mechanical parts) the activation terminates
 *  and a twin set of buzzer pulses will then be generated.     
 * 
 *  NOTE: When an activation terminates whether in target position, in obstacle or in the case
 * of button release, the driver shorts for 500ms the motor wires in order 
 * to stop the rotor inertia.  
 *  
 */
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
            if(motorStruct.sensors.x <= 0) motorDriverOutput(MOTOR_X_SHORT);
            return;
        }else if(motorStruct.keyboard.xp){
            if(motorStruct.sensors.x >= Xdm_To_Units(DEFAULT_BUTTON_X_TRAVEL_dm)) motorDriverOutput(MOTOR_X_SHORT);
            return;
        }else  motorDriverOutput(MOTOR_X_SHORT);
        
    }else if(activate_y){
        if(getKeyPressed()) key_release_timer = TIME_us_TIC(200000);
        if(motorStruct.keyboard.ym){
            if(motorStruct.sensors.y <= 0) motorDriverOutput(MOTOR_Y_SHORT);
            return;
        }else if(motorStruct.keyboard.yp){
            if(motorStruct.sensors.y >= Ydm_To_Units(DEFAULT_BUTTON_Y_TRAVEL_dm)) motorDriverOutput(MOTOR_Y_SHORT);
            return;
        }else motorDriverOutput(MOTOR_Y_SHORT);        
    }
    else if(activate_z){
        if(getKeyPressed()) key_release_timer = TIME_us_TIC(200000);        
        if(motorStruct.keyboard.zm){
            if(motorStruct.sensors.z <= 0) motorDriverOutput(MOTOR_Z_SHORT);
            return;
        }else if(motorStruct.keyboard.zp){
            if(motorStruct.sensors.z >= Zdm_To_Units(DEFAULT_BUTTON_Z_TRAVEL_dm)) motorDriverOutput(MOTOR_Z_SHORT);
            return;
        }else motorDriverOutput(MOTOR_Z_SHORT);
        
    }
   
    
    // Disable the motor after a time since the key release
    if(key_release_timer){
        key_release_timer--;
        if(!key_release_timer){
            motorDriverOutput(MOTORS_DISABLED);
            activate_x = false;
            activate_y = false;
            activate_z = false;
        }
        return;
    }
    
    
    if(!getKeyPressed()){
        motorDriverOutput(MOTORS_DISABLED);
        
        if(keep_alive_timer == 0){
            motorSetDisableMode();
        }
        return;
    }
    
    keep_alive_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_s;
    
    if(motorStruct.keyboard.zm){                
        if(motorStruct.sensors.z > 0){
            motorDriverOutput(MOTOR_Z_UP);
            activate_z = true;
        }
        
    }else if(motorStruct.keyboard.zp){  
        if(motorStruct.sensors.z < Zdm_To_Units(DEFAULT_BUTTON_Z_TRAVEL_dm)){
            activate_z = true;
            motorDriverOutput(MOTOR_Z_DOWN);
        }
        
        
    }else if(motorStruct.keyboard.ym){
        
        if(motorStruct.sensors.y > 0){
            motorDriverOutput(MOTOR_Y_HOME);
            activate_y = true;
        }     
        
    }else if(motorStruct.keyboard.yp){
        
         if(motorStruct.sensors.y < Ydm_To_Units(DEFAULT_BUTTON_Y_TRAVEL_dm)){
            motorDriverOutput(MOTOR_Y_FIELD);
            activate_y = true;
        }
    }else if(motorStruct.keyboard.xm){
        
         if(motorStruct.sensors.x > 0){
            motorDriverOutput(MOTOR_X_RIGHT);
            activate_x = true;
        }         
    }else if(motorStruct.keyboard.xp){
        
        if(motorStruct.sensors.x < Xdm_To_Units(DEFAULT_BUTTON_X_TRAVEL_dm)){
            motorDriverOutput(MOTOR_X_LEFT);
            activate_x = true;
        }
    }else motorDriverOutput(MOTORS_DISABLED);

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

void motorCommandModeManagement(void){
    int distance;
    int abs_dm_distance;
    static int min_power = 0;
    
    // keeps enabled the power switch
    uc_MOTOR_GENERAL_ENABLE_Set();
    motorStruct.general_enable = true;

    // No activation pending: the activation can be executed by the keyboard if enabled
    if(motorStruct.command_mode.command == MOTOR_COMMAND_NO_COMMAND){
        
        // Disables the Drivers
        //motorDriverOutput(MOTORS_DISABLED);
        
        
        return;
    }
    
     
    if(motorStruct.command_mode.command == MOTOR_COMMAND_X){
        min_power = 0;
        
        // Abort request
        if(motorStruct.abort_request){
            MET_Can_Protocol_returnCommandAborted();
            motorDriverOutput(MOTORS_DISABLED);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            return;
        }
        
        // Sets the distance and the selected power
        distance = motorStruct.command_mode.tx - motorStruct.sensors.x;
        abs_dm_distance = X_To_dm(abs(distance));
        motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
        
        // Verifies the target
        if(abs_dm_distance < 2 ){
            motorDriverOutput(MOTOR_X_SHORT);
            int pos = X_To_dm(motorStruct.sensors.x);
            unsigned char xl = (unsigned char) (pos & 0xff);
            unsigned char xh = (unsigned char) ((pos>>8) & 0xff);
            MET_Can_Protocol_returnCommandExecuted(xl,xh);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            motorStruct.command_mode.sequence =0;            
        }else if(distance < 0) motorDriverOutput(MOTOR_X_RIGHT);
        else motorDriverOutput(MOTOR_X_LEFT);
        
        return;
        
    }
    
    if(motorStruct.command_mode.command == MOTOR_COMMAND_Y){
        min_power = 0;
        
        // Abort request
        if(motorStruct.abort_request){
            MET_Can_Protocol_returnCommandAborted();
            motorDriverOutput(MOTORS_DISABLED);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            return;
        }
        
        // Sets the distance and the selected power
        distance = motorStruct.command_mode.ty - motorStruct.sensors.y;
        abs_dm_distance = Y_To_dm(abs(distance));
        motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
        
        // Verifies the target
        if(abs_dm_distance < 2 ){
            motorDriverOutput(MOTOR_Y_SHORT);
            int pos = Y_To_dm(motorStruct.sensors.y);
            unsigned char yl = (unsigned char) (pos & 0xff);
            unsigned char yh = (unsigned char) ((pos>>8) & 0xff);
            MET_Can_Protocol_returnCommandExecuted(yl,yh);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            motorStruct.command_mode.sequence =0;            
        }else if(distance > 0) motorDriverOutput(MOTOR_Y_FIELD);
        else motorDriverOutput(MOTOR_Y_HOME);
        
        return;
    }
    
    if(motorStruct.command_mode.command == MOTOR_COMMAND_Z){
        
        // Abort request
        if(motorStruct.abort_request){
            MET_Can_Protocol_returnCommandAborted();
            motorDriverOutput(MOTORS_DISABLED);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            return;
        }
        
        // Sets the distance and the selected power
        distance = motorStruct.command_mode.tz - motorStruct.sensors.z;
        abs_dm_distance = Z_To_dm(abs(distance));
        motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
        
        // Verifies the target
        if(abs_dm_distance < 2 ){
            motorDriverOutput(MOTOR_Z_SHORT);
            int pos = Z_To_dm(motorStruct.sensors.z);
            unsigned char zl = (unsigned char) (pos & 0xff);
            unsigned char zh = (unsigned char) ((pos>>8) & 0xff);
            MET_Can_Protocol_returnCommandExecuted(zl,zh);
            motorStruct.command_mode.command = MOTOR_COMMAND_NO_COMMAND;
            motorStruct.command_mode.sequence =0;            
        }else if(distance < 0){ 
            min_power = 2;
            motorDriverOutput(MOTOR_Z_UP);
        }else{ 
            min_power = 0;
            motorDriverOutput(MOTOR_Z_DOWN);
        }
        
        return;
    }
    
    
}

/**
 * This is the Service MOde management workflow routine.
 * 
 * In service mode, some service command can be executed:
 * + MOTOR_SERVICE_CYCLE_TEST: executes an infinite set of cycles 
 * moving all the axes in and out positions.\n
 * The command terminates when a key button is pressed or a protocol command
 * is received;
 * 
 * When no command is in action, the motor driver is disabled as well the 
 * safety power switch.
 *  
 */
void motorServiceModeManagement(void){
    int distance;
    int abs_dm_distance;
    int min_power;
    
    if(motorStruct.service_mode.command == MOTOR_SERVICE_CYCLE_TEST)
    {    

        // Disables the general enable 
       uc_MOTOR_GENERAL_ENABLE_Set();
       motorStruct.general_enable = true;

       // Disable the keyboard activation enable
       uc_BUTTON_ENA_Clear();
       motorStruct.keyboard.keyboard_enable = false;

        if(getKeyPressed()){
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
                
                if(motorStruct.sensors.z > Zdm_To_Units(100)){  
                    min_power = 2;
                    distance = Zdm_To_Units(100) - motorStruct.sensors.z;
                    abs_dm_distance = Z_To_dm(abs(distance));
                    motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                    motorDriverOutput(MOTOR_Z_UP);
                }else{ 
                    motorDriverOutput(MOTOR_Z_SHORT);
                    motorStruct.service_mode.sequence++;
                }             
                break;

            case 2: // Move X to 240
                if(motorStruct.sensors.x < Xdm_To_Units(2400)){  
                    min_power = 0;
                    distance = Xdm_To_Units(2400) - motorStruct.sensors.x;
                    abs_dm_distance = X_To_dm(abs(distance));
                    motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                    motorDriverOutput(MOTOR_X_LEFT);
                }else{ 
                    motorDriverOutput(MOTOR_X_SHORT);
                    motorStruct.service_mode.sequence++;
                }             
                break;

           case 3: // Move Y to 60
               if(motorStruct.sensors.y < Ydm_To_Units(600)){  
                   min_power = 0;
                   distance = Ydm_To_Units(600) - motorStruct.sensors.y;
                   abs_dm_distance = Y_To_dm(abs(distance));
                   motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                   motorDriverOutput(MOTOR_Y_FIELD);
               }else{ 
                   motorDriverOutput(MOTOR_Y_SHORT);
                   motorStruct.service_mode.sequence++;
               }             
           break;

           case 4: // Move Y to 0
               if(motorStruct.sensors.y > 0){  
                   min_power = 0;
                   distance = 0 - motorStruct.sensors.y;
                   abs_dm_distance = Y_To_dm(abs(distance));
                   motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                   motorDriverOutput(MOTOR_Y_HOME);
               }else{ 
                   motorDriverOutput(MOTOR_Y_SHORT);
                   motorStruct.service_mode.sequence++;
               }             
           break;

           case 5: // Move X to 0
           if(motorStruct.sensors.x > 0){  
                min_power = 0;
                distance = 0 - motorStruct.sensors.x;
                abs_dm_distance = X_To_dm(abs(distance));
                motorSetPower(getPowerFromDistance(abs_dm_distance,min_power));
                motorDriverOutput(MOTOR_X_RIGHT);
           }else{ 
               motorDriverOutput(MOTOR_X_SHORT);
               motorStruct.service_mode.sequence++;
           }             
           break;

           case 6: // Move Z up to 100
               if(motorStruct.sensors.z < Zdm_To_Units(1000)){  
                   min_power = 0;
                   distance = Zdm_To_Units(1000) - motorStruct.sensors.z;
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

        // Disables the general enable 
       uc_MOTOR_GENERAL_ENABLE_Clear();
       motorStruct.general_enable = false;

       // Disable the keyboard activation enable
       uc_BUTTON_ENA_Clear();
       motorStruct.keyboard.keyboard_enable = false;

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
        
        motorStruct.service_mode.sequence = 0;
        motorStruct.service_mode.command = 0;
        motorStruct.command_mode.sequence = 0;
        motorStruct.command_mode.command = 0;
        
        // Initializations
        if(motorStruct.exec_mode == CALIB_MODE){
            motorDriverOutput(MOTORS_DISABLED);
            keep_alive_timer = MOTOR_CALIB_MODE_KEEP_ALIVE_s;
        
        }else if(motorStruct.exec_mode == COMMAND_MODE){            
            // Starts with the Drivers disabled
            motorDriverOutput(MOTORS_DISABLED);
        }
    }
     
    // At loop completion updates the can protocol registers
    motorUpdateStatus();  
    
}

/**
 * This is a routine called every 1 second, used to implement long timers 
 * in the current module, as the keep-alive timers. 
 */
void motor1sLoop(void){
    
    // keep alive timer
    if(keep_alive_timer) keep_alive_timer--;
    
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
    
    // Disables the driver and set low the motor power
    motorDriverOutput(MOTORS_DISABLED);
    motorSetPower(0);
    
    // Disables the general enable 
    uc_MOTOR_GENERAL_ENABLE_Clear();
    motorStruct.general_enable = false;
    
    // Disable the keyboard activation enable
    uc_BUTTON_ENA_Clear();
    motorStruct.keyboard.keyboard_enable = false;
    
    motorStruct.abort_request = false;
    motorStruct.service_mode.command = 0;
    motorStruct.command_mode.command = 0;
    
    motorUpdateStatus();    
}

/**
 * This function requests to change the current workflow to Service Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetServiceMode(void){
    change_mode_request = true;
    change_mode = SERVICE_MODE;
    motorStruct.abort_request = false;
    
   
}

/**
 * This function requests to change the current workflow to Disable Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetDisableMode(void){
    change_mode_request = true;
    change_mode = DISABLE_MODE;
    motorStruct.abort_request = false;
}

/**
 * This function requests to change the current workflow to Command Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetCommandMode(void){
    change_mode_request = true;
    change_mode = COMMAND_MODE;
    motorStruct.abort_request = false;

}

/**
 * This function requests to change the current workflow to Calibration Mode.
 * 
 * The workflow will change at the next MotorLoop() execution.
 */
void motorSetCalibMode(void){
    change_mode_request = true;
    change_mode = CALIB_MODE;
    motorStruct.abort_request = false;
    
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

MOTOR_COMMAND_RESULTS_t  motorMoveX(int tXdm){
    if(tXdm > MAX_X_POSITION_dm) return MOTOR_ERROR_INVALID_POSITION;
    int pos = X_To_dm(motorStruct.sensors.x);    
    if( (tXdm > pos - 5) && (tXdm < pos + 5))  return MOTOR_ALREADY_IN_POSITION;
    
    
    if(motorStruct.exec_mode != COMMAND_MODE) return MOTOR_ERROR_INVALID_MODE;    
    motorStruct.enable_feedback = uc_MOTOR_ENA_FEEDBACK_Get();
    if(motorStruct.enable_feedback == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;
            
    // Command accepted
    motorStruct.command_mode.tx = Xdm_To_Units(tXdm);
    motorStruct.command_mode.command = MOTOR_COMMAND_X;
    return MOTOR_COMMAND_EXECUTING;
}
MOTOR_COMMAND_RESULTS_t  motorMoveY(int tYdm){
    if(tYdm > MAX_Y_POSITION_dm) return MOTOR_ERROR_INVALID_POSITION;
    int pos = Y_To_dm(motorStruct.sensors.y);    
    if( (tYdm > pos - 5) && (tYdm < pos + 5))  return MOTOR_ALREADY_IN_POSITION;

    if(motorStruct.exec_mode != COMMAND_MODE) return MOTOR_ERROR_INVALID_MODE;    
    motorStruct.enable_feedback = uc_MOTOR_ENA_FEEDBACK_Get();
    if(motorStruct.enable_feedback == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;
    
    // Command accepted
    motorStruct.command_mode.ty = Ydm_To_Units(tYdm);
    motorStruct.command_mode.command = MOTOR_COMMAND_Y;
    return MOTOR_COMMAND_EXECUTING;
}
MOTOR_COMMAND_RESULTS_t  motorMoveZ(int tZdm){
    if(tZdm > MAX_Z_POSITION_dm) return MOTOR_ERROR_INVALID_POSITION;
    int pos = Z_To_dm(motorStruct.sensors.z);    
    if( (tZdm > pos - 5) && (tZdm < pos + 5))  return MOTOR_ALREADY_IN_POSITION;
    
    if(motorStruct.exec_mode != COMMAND_MODE) return MOTOR_ERROR_INVALID_MODE; 
    motorStruct.enable_feedback = uc_MOTOR_ENA_FEEDBACK_Get();
    if(motorStruct.enable_feedback == false) return MOTOR_ERROR_DISABLE_CONDITION;
    if(motorStruct.command_mode.command != MOTOR_COMMAND_NO_COMMAND) return MOTOR_ERROR_BUSY;
    
    // Command accepted
    motorStruct.command_mode.tz = Zdm_To_Units(tZdm);
    motorStruct.command_mode.command = MOTOR_COMMAND_Z;
    return MOTOR_COMMAND_EXECUTING;
}

void motorAbort(void){
     motorStruct.abort_request = true;
}