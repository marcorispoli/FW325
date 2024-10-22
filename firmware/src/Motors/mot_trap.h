
#ifndef _MOT_TRAP_H    
#define _MOT_TRAP_H

#include "definitions.h"
#include "motlib.h"

#undef ext
#undef ext_static

#ifdef _MOT_TRAP_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/*!
 * \defgroup trapMotorModule Back-Trap Motor Common Routines module
 *
 * \ingroup applicationModule
 * 
 * 
 * 
 * # Dependencies
 * 
 * 
 *  @{
 * 
 */

    /**
     * \defgroup moduleConstants Constants module definition
     * 
     * This section describes the module constants
     * 
     *  @{
     */
       

     /// @}   moduleConstants

     /**
     * \defgroup moduleStructures Module Data and Structure definition
     * 
     * This section describes the module structures
     * 
     *  @{
     */

       ext volatile MOTOR_STRUCT_t trapMotor; //!< Motor main structure variable declaration
        
     /// @}   moduleStructures  


     /**
     * \defgroup publicModuleApi Public API Module definition
     * 
     * This section describes the API of the module
     * 
     *  @{
     */
        ext void motorTrapInit(void); //!< Module initialization function
        ext bool activateTrapCollimation(unsigned short target);//!< Activates the collimation
     
        
        
    /// @}   publicModuleApi 


/** @}*/ // trapMotorModule

#endif // _MOT_TRAP_H