
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

/*!
  * \defgroup MOTMOD Motor Activation Module
  * \ingroup applicationModule
  * 
  * This module controls all the modalities of the motor activation.
  *  
  * ## Dependencies
  * 
  * This module depends by the following applicatione modules
  * - Protocol/protocol.c
  * - Protocol/protocol.h
  * 
  * ## Module Function Description
  *  
  * ## Module API
  * 
  * + motorInit() : this is the module initialization routine. It shall be called at the application beginning;
  * + motorLoop() : this is the routine to be called every 7.8 ms in the main loop;
  * + motor1sLoop() : this is the routine to be called every 1 second from the main loop;
  * + motorSetServiceMode() : activates the Service workflow;
  * + motorSetDisableMode() : activates the Disable workflow;  
  * + motorSetCommandMode() : activates the Command  workflow;  
  * + motorSetCalibMode() : activates the Calibration  workflow;  
  * + motorServiceTestCycle() : activates the service test cycle routine
  * + motorMoveX() : move the X axe to a position;
  * + motorMoveY() : move the Y axe to a position;
  * + motorMoveZ() : move the Z axe to a position;
  * + motorAbort() : aborts a pending command;
  */

/// \ingroup MOTMOD
/// Table of the Motor activation returned codes 
typedef enum{    
    MOTOR_ALREADY_IN_POSITION = 0, //!< The current position is already at the target position
    MOTOR_COMMAND_EXECUTING = 1,//!< The command is executing
    MOTOR_ERROR_INVALID_POSITION = 2,//!< The requested target is invalid
    MOTOR_ERROR_INVALID_MODE = 3,//!< the workflow is invalid for this command
    MOTOR_ERROR_DISABLE_CONDITION = 4,//!< The power switch is not enabled
    MOTOR_ERROR_BUSY = 5,//!< A command is already running
            
}MOTOR_COMMAND_RESULTS_t;

/// \ingroup MOTMOD
/// this is the routine to be called every 7.8 ms in the main loop 
ext void motorLoop(void);


/// \ingroup MOTMOD
/// this is the module initialization routine
ext void motorInit(void);

/// \ingroup MOTMOD
/// activates the Service workflow
ext void motorSetServiceMode(void);

/// \ingroup MOTMOD
/// activates the Disable workflow
ext void motorSetDisableMode(void);

/// \ingroup MOTMOD
/// activates the Command workflow
ext void motorSetCommandMode(void);

/// \ingroup MOTMOD
/// activates the Calibration workflow
ext void motorSetCalibMode(void);

/// \ingroup MOTMOD
/// activates the Cycle Test routine
ext bool motorServiceTestCycle(void);

/// \ingroup MOTMOD
/// activates the X to a target
ext MOTOR_COMMAND_RESULTS_t  motorMoveX(int tXdm, bool protocol, bool key_request);

/// \ingroup MOTMOD
/// activates the Y to a target
ext MOTOR_COMMAND_RESULTS_t  motorMoveY(int tYdm, bool protocol, bool key_request);

/// \ingroup MOTMOD
/// activates the Z to a target
ext MOTOR_COMMAND_RESULTS_t  motorMoveZ(int tZdm, bool protocol, bool key_request);

/// \ingroup MOTMOD
/// Enables/Disables the KeyStep mode
ext bool  motorEnableKeyStepMode(unsigned char par);

/// \ingroup MOTMOD
/// aborts a pending command
ext void motorAbort(void);

/// \ingroup MOTMOD
/// Table of the driver activation mode
typedef enum{
    MOTORS_DISABLED = 0, //!< All drivers are set in HZ
    MOTOR_X_LEFT,   //!< The X driver is set to Left direction mode
    MOTOR_X_RIGHT,  //!< The X driver is set to Right direction mode
    MOTOR_X_SHORT,  //!< The X driver is shorted to ground (keeps position)
    MOTOR_Y_HOME,   //!< The Y driver is set to Home direction mode
    MOTOR_Y_FIELD,  //!< The Y driver is set to Field direction mode
    MOTOR_Y_SHORT,  //!< The Y driver is shorted to ground (keeps position)
    MOTOR_Z_UP,     //!< The Z driver is set to Up direction mode
    MOTOR_Z_DOWN,   //!< The Z driver is set to Down direction mode
    MOTOR_Z_SHORT,  //!< The Z driver is shorted to ground (keeps position)          
}MOTOR_MODE_t;

/// \ingroup MOTMOD
/// Table of the Service Mode commands
typedef enum{    
    MOTOR_SERVICE_NO_COMMAND = 0,//!< no service mode commands
    MOTOR_SERVICE_CYCLE_TEST,//!< service cycle test 
}MOTOR_SERVICE_t;

/// \ingroup MOTMOD
/// Table of the Command Mode commands
typedef enum{    
    MOTOR_COMMAND_NO_COMMAND = 0,//!< no command mode commands    
    MOTOR_COMMAND_X = 1,//!< move X command execution
    MOTOR_COMMAND_Y = 2,//!< move Y command execution
    MOTOR_COMMAND_Z = 3,//!< move Z command execution                
}MOTOR_COMMAND_t;



/// \ingroup MOTMOD
/// This is the module data structure
typedef struct{    
    MOTOR_MODE_t mode; //!< Current status of the motor driver mode
    unsigned power;//!< Current motor voltage level    
    int exec_mode;//!< Current workflow
    
    int key_timer; //!< Generic Timer to handle the keyboard chage status
    
    
    /// data structure for the service workflow 
    struct{
      MOTOR_SERVICE_t command;  
      int sequence;
    }service_mode;
    
    /// data structure for the command workflow 
    struct{
      MOTOR_COMMAND_t command;  
      int  sequence;
      int  target;              //!< Target position for the pending activaion
      
      bool protocol_activation; //!< The command is initiated by the CAN protocol
      bool key_requested;       //!< The activation requires the button pressed
      bool abort_request;       //!< abort command request flag
      int min_power;            //!< Minimum value of the power usable during the activation
      int activation_timer;     //!< Time since the command beginning
      int activation_timeout;   //!< Sets the whole activation timeout
      bool termination_fase;    //!< Command Termination management
      int termination_timer;    //!< Time to let the motor to hold the position
      bool termination_success; //!< The completion result of the last activation command
      unsigned char termination_error;//!< In case of error this is the error code
    }command_mode;
    
}MOTORS_t;

ext MOTORS_t motorStruct; 



#endif // _MOTLIB_H