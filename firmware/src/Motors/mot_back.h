
#ifndef _MOT_BACK_H    
#define _MOT_BACK_H

#include "definitions.h"
#include "motlib.h"

#undef ext
#undef ext_static

#ifdef _MOT_BACK_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/**
 * \defgroup backMotorModule Backward Collimation Module
 * \ingroup applicationModule
 * 
 * This module implements the requirements of the back blade collimation control.
 


 /**
 * \defgroup backMotorGeneralDescription General Description
 * \ingroup backMotorModule
 *
 * # Module Function Description
 * 
 * This module controls the position of the back collimation blade.
 * The module controls a linear stepper motor with an A3984 driver
 * capable to micro-stepping the activation, up to 16 u-steps.
 * 
 * The position is determined counting the steps from a zero detected position.
 * The zero position is detected with a photocell when obscured.
 * 
 * There are two activation mode:
 * + The static collimation mode, for 2D exposures;
 * + The dynamic collimation mode, for Tomography;
 * 
 * ## Static Collimation mode
 * 
 * In static collimation mode, the positioning is achieved 
 * in two steps:
 * + Zero positioning first: the blade is moved until the zero photocell is detected;
 * + In Field positioning: form the zero position as reference, the blade is moved of a 
 * predefined number of steps (calibrated) to find the final target position.
 * 
 * The number of the steps required to find the target position shall be calibrated.
 * 
 * ## Dynamic Collimation mode
 * 
 * The dynamic collimation mode is used during a tomograpy scan,
 * in order to keep the back collimation when the Tube Arm change the current angle.
 * This kind of adjustment cannot make use of the zero photocell but need
 * to "remember" the number of steps from the initial zero position, implementing such
 * a local absolute encoder.
 * 
 * The current blade position is then received from the Control Unit of the Gantry,
 * during the Tube rotation. The Module will adjust the steps in order to get the 
 * requested position.
 * 
 * # Driver performance description
 * 
 * + Motor Model: VSM20L-048S-0508-32-01;
 * + Power: 3.4W;
 * + Phase Current: 385mA;
 * + 7.5° per step;
 * + Step Travel: 0.0508mm; 
 * + Stroke: 32mm;
 * + 20N at 300 step/s;
 * 
 * # Blade Performances
 * 
 * 
 * 
 *  
 *  
 * 
 * 
 * 
 */



/**
 * \defgroup backModuleConstants Constants definition
 * \ingroup backMotorModule
 * 
 * This section describes the constants used in the module
 * 
 *  @{
 */
      

 /// @}   moduleConstants

/**
* \defgroup backModuleStructures Structures definition
* \ingroup backMotorModule
 * 
* This section describes the module structures
* 
*  @{
*/

        ext volatile MOTOR_STRUCT_t backMotor; //!< Motor main structure variable declaration

/// @}   moduleStructures  


/**
* \defgroup backModulePublicApi Public API 
* \ingroup backMotorModule
* 
* This section describes the API of the module
* 
*  @{
*/
        ext void motorBackInit(void); //!< Module initialization function
        ext bool activateBackCollimation(unsigned short target);//!< Activates the collimation

        
        
/// @}   publicModuleApi 




#endif // _MOT_BACK_H