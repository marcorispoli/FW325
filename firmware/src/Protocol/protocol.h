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
  * \defgroup CANPROT CAN Communication Protocol Module
  * \ingroup applicationModule
  * 
  * This Module implements the functions of the PCB/23-325 Software Communication protocol specifications.
  * 
  */


 

/*!
 * \addtogroup CANPROT 
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
 * described in the PCB/23-325 Software Communication protocol specifications.
 * 
 */

/// \ingroup CANPROT 
/// This structure is used as a template type to cast a status register  
typedef struct {
    unsigned char idx;
    unsigned char d0;
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
}GENERIC_STATUS_t;
    
/**
 * \addtogroup CANPROT 
 * 
 * ## Protocol Setting
 * 
 * Can ID address: \ref MET_CAN_APP_DEVICE_ID  
 * Number of STATUS registers: MET_CAN_STATUS_REGISTERS
 * Number of DATA registers: MET_CAN_DATA_REGISTERS
 * Number of PARAM registers: MET_CAN_PARAM_REGISTERS
 * 
 */ 

/// \ingroup CANPROT 
/// Protocol Definition Data
typedef enum{
    MET_CAN_APP_DEVICE_ID    =  0x15,      //!< Application DEVICE CAN Id address
    MET_CAN_STATUS_REGISTERS =  3,        //!< Defines the total number of implemented STATUS registers 
    MET_CAN_DATA_REGISTERS   =  0,        //!< Defines the total number of implemented Application DATA registers 
    MET_CAN_PARAM_REGISTERS  =  0       //!< Defines the total number of implemented PARAMETER registers 
}PROTOCOL_DEFINITION_DATA_t;

/**
 * \addtogroup CANPROT
 * 
 * ## Application API
 * 
 * + ApplicationProtocolInit() : this is the module initialization;
 * + ApplicationProtocolLoop() : this is the workflow routine to be placed into the application main loop;
 * + updateStatusRegister() : this is the function to be called to update a given status register 
 * 
 */


/**
 * \ingroup CANPROT 
 * Protocol Initialization routine
 */
ext void ApplicationProtocolInit ( void);

/// \ingroup CANPROT 
/// This is the Main Loop protocol function
ext void  ApplicationProtocolLoop(void);

/// \ingroup CANPROT 
/// This is the function to update a Status register
ext void updateStatusRegister(void* reg);

/**
 * \addtogroup CANPROT
 * 
 * ## STATUS register description
 * 
 * There are the following Status registers:
 * 
 * |IDX|NAME|DESCRIPTION|
 * |:--|:--|:--|
 * |0|Mode Register|\ref STATUS_MODE_t|
 * |1|Position XY Register|\ref STATUS_XY_POSITION_t|
 * |2|Position Z Register|\ref STATUS_Z_POSITION_t|
 *   
 */

    /// \ingroup CANPROT
    /// Defines the address table for the System Registers 
    typedef enum{
      STATUS_MODE_IDX = 0, //!< Status Mode  
      STATUS_XY_POSITION_IDX = 1,//!< Status Position for the X and Y coordinate 
      STATUS_Z_POSITION_IDX = 2,//!< Status Position for the Z coordinate 
    }STATUS_INDEX_t;

    /**
     * This is the enumeration type describing the current workflow code value.
     */
    typedef enum{
        DISABLE_MODE = 0,   //!< This is the Disabled Mode 
        CALIB_MODE,         //!< This is the Calibration Mode 
        COMMAND_MODE,       //!< This is the Command Mode 
        SERVICE_MODE        //!< This is the Service Mode 
    }STATUS_WORKING_MODE_t;
        
    
    
    /**
     * \addtogroup CANPROT
     * 
     * ### MODE STATUS REGISTER
     * 
     * + Description: STATUS_MODE_t;
     * + IDX: \ref STATUS_MODE_IDX;
     * 
     * |BYTE.BIT|NAME|DESCRIPTION|
     * |:--|:--|:--|
     * |0|MODE|This is the current Working mode \ref STATUS_WORKING_MODE_t|
     * |1.0|General Enable|Status of the general enable switch|
     * |1.1|Keyboard Enable|Status of the keyboard enable switch|
     * |1.2|status of the power switch|
     * |1.3|status of the needle disable signal|
     * |2|-|-|
     * |3|-|-|
     * 
     */ 

    /// \ingroup CANPROT
    /// Status Mode description structure
    typedef struct {
        const unsigned char idx; //!< Address constant 

        unsigned char mode; //!< workflow mode code (see  STATUS_WORKING_MODE_t) at byte 0

        unsigned char general_enable:1; //!< General enable status at byte 1.0 
        unsigned char keyboard_enable:1;//!< Keyboard enable status at byte 1.1 
        unsigned char enable_feedback:1;//!< Current power switch status at byte 1.2 
        unsigned char disable_needle_feedback:1;//!< Current disable needle status at byte 1.3 
        unsigned char d1:4;//!< Spare bits at the byte 1.4 to 1.7

        unsigned char d2;//!< Spare byte 2

        unsigned char d3;//!< Spare byte 3
    }STATUS_MODE_t;
    
     /**
     * \addtogroup CANPROT
     * 
     * ### POSITION XY STATUS REGISTER
     * 
     * + Description: STATUS_XY_POSITION_t;
     * + IDX: \ref STATUS_XY_POSITION_IDX;
     * 
     * The structure of the XY POSITION register is the following:
     * 
     * |BYTE.BIT|NAME|DESCRIPTION|
     * |:--|:--|:--|
     * |0|XL|Low byte of the 16 bit X coordinate|
     * |1|XH|High byte of the 16 bit X coordinate|
     * |2|YL|Low byte of the 16 bit Y coordinate|
     * |3|YH|High byte of the 16 bit Y coordinate|
     * 
     * + Position X = XL + 256 * XH: is expressed in 0.1 mm units
     * + Position Y = YL + 256 * YH: is expressed in 0.1 mm units
     */ 
    
    /// \ingroup CANPROT
    /// Status XY Position description structure
    typedef struct {
        const unsigned char idx;
        unsigned char xl; //!< Low byte of the X Position 
        unsigned char xh; //!< High byte of the X Position 
        unsigned char yl; //!< Low byte of the Y Position
        unsigned char yh; //!< High byte of the Y Position
    }STATUS_XY_POSITION_t;
    
    /**
     * \addtogroup CANPROT
     * 
     * ### POSITION Z STATUS REGISTER
     * 
     * + Description: STATUS_Z_POSITION_t;
     * + IDX: \ref STATUS_Z_POSITION_IDX;
     * 
     * The structure of the Z POSITION register is the following:
     * 
     * |BYTE.BIT|NAME|DESCRIPTION|
     * |:--|:--|:--|
     * |0|ZL|Low byte of the 16 bit X coordinate|
     * |1|ZH|High byte of the 16 bit X coordinate|
     * |2|-|-|
     * |3|-|-|
     * 
     * + Position Z = ZL + 256 * ZH: is expressed in 0.1 mm units
     */ 
    
    
    /// \ingroup CANPROT
    /// Status Z Position description structure
    typedef struct {
        const unsigned char idx;
        unsigned char zl; //!< Low byte of the Z Position
        unsigned char zh; //!< High byte of the Z Position
        unsigned char d2;  
        unsigned char d3;  
    }STATUS_Z_POSITION_t;
    
    
    
    
    #ifdef _PROTOCOL_C
        /// \ingroup CANPROT
        /// Declaration of the Status Mode Register  global variables
        STATUS_MODE_t StatusModeRegister = {.idx=STATUS_MODE_IDX};

        /// \ingroup CANPROT
        /// Declaration of the Status XY Position Register  global variables
        STATUS_XY_POSITION_t StatusPositionRegister = {.idx=STATUS_XY_POSITION_IDX};
        
        /// \ingroup CANPROT
        /// Declaration of the Status Z Position Register  global variables
        STATUS_Z_POSITION_t StatusPositionRegister = {.idx=STATUS_Z_POSITION_IDX};
    #else
        extern STATUS_MODE_t StatusModeRegister;
        extern STATUS_XY_POSITION_t StatusPositionRegister;
        extern STATUS_Z_POSITION_t StatusPositionRegister;
    #endif  
    
    
    /**
     * \addtogroup CANPROT
     * 
     * ## PROTOCOL COMMANDS DESCRIPTION
     * 
     * See \ref PROTOCOL_COMMANDS_t
     */     

    /// \ingroup CANPROT
    /// This is the list of the implemented COMMANDS
    typedef enum{
       CMD_ABORT = 0,           //!< Abort Command
       CMD_DISABLE_MODE = 1,        //!< Disable Mode Activation Command
       CMD_COMMAND_MODE = 2,        //!< Command Mode Activation Command
       CMD_SERVICE_MODE = 3,        //!< Service Mode Activation Command
       CMD_CALIB_MODE = 4,          //!< Calibration Mode Activation Command
       CMD_MOVE_XYZ = 5,            //!< Moves to an XYZ position command
       CMD_SERVICE_TEST_CYCLE = 6   //!< Service Cycle Test activatioin command    
    }PROTOCOL_COMMANDS_t;
    
        


         

#endif 