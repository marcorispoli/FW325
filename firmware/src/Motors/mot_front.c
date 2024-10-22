#define _MOT_FRONT_C

#include "application.h"
#include "mot_front.h"
#include "motlib.h"


static void motorFrontCallback(TC_COMPARE_STATUS status, uintptr_t context); //!< Callback every STEP pin changes

// Every full step is 0.1 mm translation
#define uSTEP 16
#define umToSteps(val) ( ((uint32_t) val * (uint32_t) uSTEP) / ((uint32_t) 100) ) //!< Macro conversion from um to u-pulse
#define stepsToum(val) ( ((uint32_t) val * (uint32_t) 100) / ((uint32_t) uSTEP) ) //!< Macro conversion from step to um

#define MOTOR_ID MOTOR_FRONT_ID
#define MOTOR_STEP_SET uC_STEP_FRONT_Set()
#define MOTOR_STEP_CLEAR uC_STEP_FRONT_Clear()
#define OPTO_GET uC_OPTO_FRONT_Get()
#define MOTOR_DATA frontMotor

#define HOME_EXTRA_STEPS 10 //!< Macro setting the extra steps executed entering hte home position
#define MOTOR_HOME MOT_DIRCCW
#define MOTOR_FIELD MOT_DIRCW



/**
 * Motor initialization initialization
 */
void motorFrontInit(void){
    
    MOTOR_DATA.command_activated = false;
    MOTOR_DATA.position_valid = false;
    MOTOR_DATA.command_sequence = 0;
    MOTOR_DATA.running = false; 
    
    MOTOR_DATA.target_position = umToSteps(100);
    motorDisable(MOTOR_ID);
    
    // Registers the working callback
    motorRegisterTcCallback(MOTOR_ID,motorFrontCallback );
    

}

bool activateFrontCollimation(unsigned short target){
    if(MOTOR_DATA.command_activated) return false;    
    MOTOR_DATA.command_activated = true;
    MOTOR_DATA.command_sequence = 0;
    MOTOR_DATA.target_position = umToSteps(target);
    setTcPeriod(MOTOR_ID, 1000);    
    return true;
}

void motorFrontCallback(TC_COMPARE_STATUS status, uintptr_t context){
   static uint32_t stp=0;
   static bool step = false;
   bool error = false;
   bool completed = false;
   
   // Updates the opto current status
   MOTOR_DATA.opto_status = OPTO_GET;
   
   switch(MOTOR_DATA.command_sequence){
       case 0:
           error = false;
           completed = false;
           step = false;
           
           // Speed regulation
           MOTOR_DATA.final_period = 2000;
           MOTOR_DATA.init_period = 10000;
           MOTOR_DATA.ramp_rate = 100;
           MOTOR_DATA.running = true;
           setTcPeriod(MOTOR_ID, MOTOR_DATA.init_period);                      
           motorOn(MOTOR_ID, MOT_TORQUE_HIGH, MOT_uSTEP_16, MOTOR_HOME );
                   
           // Activates the motor driver to move to Home position
           MOTOR_DATA.command_sequence++;
           stp = 0;
           return;
           
       case 1:
           if(!isLatched(MOTOR_ID)) return;
           MOTOR_DATA.command_sequence++;
           return;
           
       case 2: // Wait for the opto = ENGAGED           
           stp++;
          //  if(stp > 5 * MAX_STEPS_BETWEEN_SLOTS ){ error = true; break;}            
           if(!MOTOR_DATA.opto_status) break;
           
           stp = 0;
           MOTOR_DATA.command_sequence++;
           return;
           
       case 3:
           if(stp >= HOME_EXTRA_STEPS ){
                // Changes the direction to enter into field at the requsted number of steps
                motorOn(MOTOR_ID, MOT_TORQUE_HIGH, MOT_uSTEP_16, MOTOR_FIELD );
                
                // Speed regulation
                MOTOR_DATA.final_period = 2000;                 
                MOTOR_DATA.init_period = 10000;
                MOTOR_DATA.ramp_rate = 50;    
                setTcPeriod(MOTOR_ID, MOTOR_DATA.init_period);
                MOTOR_DATA.command_sequence++;                 
                return;
           }
                
           break;
           
       case 4:
           if(!isLatched(MOTOR_ID)) return;           
           stp = 0;
           MOTOR_DATA.command_sequence++;
           return;
           
       case 5:
            // Waits for the sensor free
            if(MOTOR_DATA.opto_status) break;

            // Starts counting 
            stp = 0;
            
            MOTOR_DATA.command_sequence++;
            break;
            
       case 6:
           if(stp >= MOTOR_DATA.target_position ){
                completed = true;           
                break;              
           }
           
           break;
                                      
   }
   
   // If an error condition was detected, the activation ends in 
   if(error){
       MOTOR_STEP_CLEAR;
       step = false;
       error = false;
       motorDisable(MOTOR_ID);
       stopTc(MOTOR_ID);
       MOTOR_DATA.command_activated = false;            
       MOTOR_DATA.position_valid = false;
       return;
   }
   
   // If the complete activation has been detected, the activation ends
   if(completed){
       MOTOR_STEP_CLEAR;
       step = false;
       motorDisable(MOTOR_ID);
       stopTc(MOTOR_ID);
       MOTOR_DATA.command_activated = false;            
       MOTOR_DATA.position_valid = true;
       return;
   }
   
   // Pulse the step pin to provide a motor step
   if(step){ 
       MOTOR_STEP_SET;
       stp++;
   } else{ 
       MOTOR_STEP_CLEAR;
   }
   step = !step;
   
   setRampPeriod(MOTOR_ID, MOTOR_DATA.final_period, MOTOR_DATA.ramp_rate);
    
}