#define _MOT_BACK_C

#include "application.h"
#include "mot_back.h"
#include "motlib.h"

// The following macros define the motor identification structures in the code
#define MOTOR_ID MOTOR_BACK_ID  //!< This is the iddentifier of the motor
#define MOTOR_STEP_SET uC_STEP_BACK_Set() //!< This is the call to set the STEP output
#define MOTOR_STEP_CLEAR uC_STEP_BACK_Clear() //!< This is the call to clear the STEP output
#define OPTO_GET uC_OPTO_BACK_Get()//!< This is the call to get the zero setting photocell
#define MOTOR_DATA backMotor//!< This is the pointer to the motor data structure


static void motorBackCallback(TC_COMPARE_STATUS status, uintptr_t context); //!< Callback every STEP pin changes

/**
 * \ingroup backModuleConstants 
 *  @{
 */
 const uint32_t _nm_per_step            =   50800;          //!< linear displacement for any step in nanometers   
 const uint32_t _max_blade_travel       =   30000;          //!< maximum blade travel from the zero reference in micron
 const uint32_t _max_step_per_second    =   300;            //!< maximum speed of the motor 
 const MOT_MICROSTEP_t _ustep_control   =   MOT_uSTEP_16;   //!< microstepping used in the control

 /// @}   moduleConstants

#define umToUsteps(val) ( ((uint32_t) val * (uint32_t) MICROSTEP(_ustep_control)) * 1000 / ((uint32_t) _nm_per_step) ) //!< Macro conversion from um to u-step
#define UstepsToum(val) ( ((uint32_t) val * (uint32_t) _nm_per_step) / ((uint32_t) MICROSTEP(_ustep_control) * 1000) ) //!< Macro conversion from u-step to um


#define HOME_EXTRA_STEPS 100 //!< Macro setting the extra steps executed entering hte home position
#define MOTOR_HOME MOT_DIRCCW
#define MOTOR_FIELD MOT_DIRCW



/**
 * Motor initialization initialization
 */
void motorBackInit(void){
    
    MOTOR_DATA.command_activated = false;
    MOTOR_DATA.position_valid = false;
    MOTOR_DATA.command_sequence = 0;
    MOTOR_DATA.running = false; 
    
    MOTOR_DATA.target_position = umToSteps(100);
    motorDisable(MOTOR_ID);
    
    // Registers the working callback
    motorRegisterTcCallback(MOTOR_ID,motorBackCallback );
    

}


bool activateBackCollimation(unsigned short target){
    if(MOTOR_DATA.command_activated) return false;    
    MOTOR_DATA.command_activated = true;
    MOTOR_DATA.command_sequence = 0;
    MOTOR_DATA.target_position =  umToSteps(target);
    setTcPeriod(MOTOR_ID, 1000);
    return true;
}

void motorBackCallback(TC_COMPARE_STATUS status, uintptr_t context){
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