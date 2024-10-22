#ifndef _PROTOCOL_H    
#define _PROTOCOL_H

#include "definitions.h"  
#include "application.h"  
#include "Shared/CAN/MET_can_protocol.h"

#undef ext
#undef ext_static

#ifdef _PROTOCOL_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif



/*!
 * \defgroup protocolModule CAN Communication Protocol Module
 *
 * \ingroup applicationModule
 * 
 * 
 * This Module implements the functions of the PCB/22-303 Software Communication protocol specifications.
 * 
 * ## Dependencies
 * 
 * This module requires the following library modules:
 * - Shared/CAN/MET_Can_Protocol.c
 * - Shared/CAN/MET_Can_Protocol.h
 * 
 * 
 * ## Protocol Communication setting
 *  
 * The Application implements the communication protocol  
 * described in the PCB/22-303 Software Communication protocol specifications.
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
        // Can Module Definitions
        static const unsigned char   MET_CAN_APP_DEVICE_ID    =  0x15 ;     //!< Application DEVICE CAN Id address
        static const unsigned char   MET_CAN_STATUS_REGISTERS =  2 ;        //!< Defines the total number of implemented STATUS registers 
        static const unsigned char   MET_CAN_DATA_REGISTERS   =  0 ;        //!< Defines the total number of implemented Application DATA registers 
        static const unsigned char   MET_CAN_PARAM_REGISTERS  =  0 ;       //!< Defines the total number of implemented PARAMETER registers 

     /// @}   moduleConstants

    /**
     * \defgroup moduleApiInterface CAN Protocol API interface
     * 
     * This section describes the functions implementing the Protocol module.
     * 
     *  @{
     */
        /// This is the Protocol initialization function
        ext void ApplicationProtocolInit ( void);

        /// This is the Main Loop protocol function
        ext void  ApplicationProtocolLoop(void);

     /// @}   moduleApiInterface
    

     /** \defgroup ErrorRegisterGroup ERROR REGISTER Definition
     *  
     *  This section describes the implementation of the Protocol Error Register
     *  @{
     */

       


        
    /// @}   ErrorRegisterGroup
    
       
    /** \defgroup StatusRegisterGroup STATUS REGISTERS Definition
     *  
     *  This section describes the implementation of the Application STATUS Registers 
     *  @{
     */
        

    typedef struct {
        unsigned char idx;
        unsigned char d0;
        unsigned char d1;
        unsigned char d2;
        unsigned char d3;

    }GENERIC_STATUS_t;
    
    typedef enum{
        DISABLE_MODE = 0,
        CALIB_MODE,
        COMMAND_MODE,    
        SERVICE_MODE
    }STATUS_WORKING_MODE_t;
        
    /// This is the list of the implemented DATA REGISTERS    
    typedef struct {
        const unsigned char idx;

        unsigned char mode;

        unsigned char general_enable:1;
        unsigned char keyboard_enable:1;
        unsigned char enable_feedback:1;
        unsigned char disable_needle_feedback:1;
        unsigned char d1:4;

        unsigned char d2;

        unsigned char d3;    
    }STATUS_MODE_t;
    
    /// This is the list of the implemented DATA REGISTERS    
    typedef struct {
        const unsigned char idx;
        unsigned char xl;
        unsigned char xh;                
        unsigned char y;
        unsigned char z;    
    }STATUS_POSITION_t;
    ext void updateStatusRegister(void* reg);
    
    // Declaration of the status registers global data   
    #ifdef _PROTOCOL_C
        STATUS_MODE_t StatusModeRegister = {.idx=0};
        STATUS_POSITION_t StatusPositionRegister = {.idx=1};
    #else
        extern STATUS_MODE_t StatusModeRegister;
        extern STATUS_POSITION_t StatusPositionRegister;
    #endif  
    
    
     /// @}   StatusRegisterGroup

     
    /** \defgroup DataRegisterGroup DATA REGISTERS Definition
     *  
     *  This section describes the implementation of the Protocol DATA Registers 
     *  @{
     */
        
        
    /// @}   DataRegisterGroup

    /** \defgroup ParamRegisterGroup PARAMETER Registers Definition
     *  
     *  This section describes the implementation of the Application PARAMETER Registers 
     *  @{
     */
             
        
         
    /// @}   ParamRegisterGroup
        
        
     /** \defgroup CommandGroup COMMAND Execution Definition
     *  
     *  This section describes the Application Command Execution codes
     *  @{
     */
        
    /// This is the list of the implemented COMMANDS
    
    
    /// This is the list of the implemented ERRORS
    typedef enum{
       CMD_ABORT = 0,           //!< Abort Command
       CMD_DISABLE_MODE,
       CMD_COMMAND_MODE,
       CMD_SERVICE_MODE,
       CMD_CALIB_MODE,
       CMD_MOVE_XYZ,                //!< Moves to an XYZ position
       CMD_SERVICE_TEST_CYCLE   //!< Cycle Test     
    }PROTOCOL_COMMANDS_t;
    
     /// @}   CommandGroup

        


         
/** @}*/ // protocolModule
#endif 