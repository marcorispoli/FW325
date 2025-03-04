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
 */

/// \ingroup CANPROT 
/// This structure of a generic protocol register 
typedef struct {
    unsigned char idx;
    unsigned char d0;
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
}REGISTER_STRUCT_t;
    
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
    MET_CAN_DATA_REGISTERS   =  1,        //!< Defines the total number of implemented Application DATA registers 
    MET_CAN_PARAM_REGISTERS  =  0       //!< Defines the total number of implemented PARAMETER registers 
}PROTOCOL_DEFINITION_DATA_t;

/**
 * \addtogroup CANPROT
 * 
 * ## Application API
 * 
 * + ApplicationProtocolInit() : this is the module initialization;
 * + ApplicationProtocolLoop() : this is the workflow routine to be placed into the application main loop;
 * + updateStatusRegister() : this is the function to be called to update a given STATUS register 
 * + updateDataRegister() : this is the function to be called to update a given DATA register 
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

/// \ingroup CANPROT 
/// This is the function to update a Data register
ext void updateDataRegister(void* reg);


//________________________________________ STATUS REGISTER DEFINITION SECTION _

/**
 * \addtogroup CANPROT
 * 
 * ## STATUS register description
 * 
 * There are the following Status registers:\n
 * (See \ref STATUS_INDEX_t enum table)
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
 * \ingroup CANPROT
 * This is the enumeration type describing the current workflow code value.
 */
typedef enum{
    DISABLE_MODE = 0,   //!< This is the Disabled Mode 
    CALIB_MODE = 1,         //!< This is the Calibration Mode 
    COMMAND_MODE = 2,       //!< This is the Command Mode 
    SERVICE_MODE = 3        //!< This is the Service Mode 
}STATUS_WORKING_MODE_t;
        
 /**
 * \ingroup CANPROT
 * This is the enumeration of the X-SCROLL position
 */
typedef enum{
    XSCROLL_UNDETECTED = 0,     //!< No detected position
    XSCROLL_RIGHT = 1,          //!< Right position detected
    XSCROLL_CENTER = 2,         //!< Center position detected
    XSCROLL_LEFT = 3,           //!< Left position detected
}STATUS_XSCROLL_t;

/**
 * \ingroup CANPROT
 * This is the enumeration of the NEEDLE ADAPTER IDENTIFIER
 */
typedef enum{
    NEEDLE_UNDETECTED = 0,     //!< No detected position
    NEEDLE_A = 1,                  //!< Type A adapter
    NEEDLE_B = 2,                  //!< Type B Adapter
    NEEDLE_C = 3,                  //!< Type C adapter
}STATUS_NEEDLE_t;
    
    /**
     * \addtogroup CANPROT
     * 
     * ### [MODE] STATUS REGISTER
     * 
     * + Description: STATUS_MODE_t;
     * + IDX: \ref STATUS_MODE_IDX;
     * 
     * |BYTE.BIT|NAME|DESCRIPTION|
     * |:--|:--|:--|
     * |0|MODE|This is the current Working mode \ref STATUS_WORKING_MODE_t|
     * |1.0|power swith status|this is the actual safety power switch status|
     * |1.1|General enable|this is the current general enable status|
     * |1.2|Keyboard enable|this is the current keyboard enable status|
     * |1.3|Needle Status|status of the needle disable signal|
     * |2.0|Key Step mode activation bit|This is the current status of the Key Step mode|
     * |2.1|Y Up position detected|This is the current detected Y-UP status|
     * |2..|-|-|
     * |3|-|-|
     * 
     */ 

    /// \ingroup CANPROT
    /// Status Mode description structure
    typedef struct {
        const unsigned char idx;                    //!< Address constant 
        unsigned char mode;                         //!< workflow mode code (see  STATUS_WORKING_MODE_t) at byte 0

        unsigned char power_sw_status:1;            //!< Actual power switch status
        unsigned char power_sw_general_enable:1;    //!< General enable bit
        unsigned char power_sw_keyboard_enable:1;   //!< Keyboard enable bit
        unsigned char power_sw_needle_disable:1;    //!< Needle disable bit
        unsigned char d1:4;                         //!< Spare bits at the byte 1.4 to 1.7

        unsigned char keystep_mode_enabled:1;       //!< Key Step enable bit
        unsigned char y_up_detected:1;              //!< Status of the Y UP position detection
        unsigned char xscroll_code:2;               //!< Status of the X-Scroll position (see STATUS_XSCROLL_t) 
        unsigned char needle_code:4;                //!< Detected code of the needle identification

        unsigned char d3;                           //!< Spare byte 3
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
        unsigned char XL; //!< Low byte of the X Position 
        unsigned char XH; //!< High byte of the X Position 
        unsigned char YL; //!< Low byte of the Y Position
        unsigned char YH; //!< High byte of the Y Position
    }STATUS_XY_POSITION_t;
    
    /**
     * \addtogroup CANPROT
     * 
     * ### Z AND SLIDER POSITION STATUS REGISTER
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
     * |2|SL|Low byte of the Slider |
     * |3|SH|High byte of the Slider|
     * 
     * + Position Z = ZL + 256 * ZH: is expressed in 0.1 mm units
     * + Slider = SL + 256 * SH: is expressed in 0.1 mm units
     * 
     */ 
    
    
    /// \ingroup CANPROT
    /// Status Z Position description structure
    typedef struct {
        const unsigned char idx;
        unsigned char ZL; //!< Low byte of the Z Position
        unsigned char ZH; //!< High byte of the Z Position
        unsigned char SL; //!< Low byte of the Slider Position 
        unsigned char SH; //!< High byte of the Slider Position 
    }STATUS_Z_POSITION_t;
    
    
    #ifdef _PROTOCOL_C
        /// \ingroup CANPROT
        /// Declaration of the Status Mode Register  global variables
        STATUS_MODE_t StatusModeRegister = {.idx=STATUS_MODE_IDX};

        /// \ingroup CANPROT
        /// Declaration of the Status XY Position Register  global variables
        STATUS_XY_POSITION_t StatusXYPositionRegister = {.idx=STATUS_XY_POSITION_IDX};
        
        /// \ingroup CANPROT
        /// Declaration of the Status Z Position Register  global variables
        STATUS_Z_POSITION_t StatusZPositionRegister = {.idx=STATUS_Z_POSITION_IDX};
    #else
        extern STATUS_MODE_t StatusModeRegister;
        extern STATUS_XY_POSITION_t StatusXYPositionRegister;
        extern STATUS_Z_POSITION_t StatusZPositionRegister;
    #endif  
    
//________________________________________ DATA REGISTER DEFINITION SECTION _   

/**
* \addtogroup CANPROT
* 
* ## DATA register description
* 
*  No DATA registers are implemented. 
*
*   
*/

    
//_______________________________________ PROTOCOL COMMANDS DEFINITION SECTION _        
        
/**
 * \addtogroup CANPROT
 * 
 * ## PROTOCOL COMMANDS DESCRIPTION
 * 
 * The Firmware implements the following Commands:
 * + [0] CMD_ABORT: immediate abort of any motor activation;
 * + [1] CMD_DISABLE_MODE: motor disable mode activation request;
 * + [2] CMD_COMMAND_MODE: motor command mode activation request;
 * + [3] CMD_SERVICE_MODE: motor service mode activation request;
 * + [4] CMD_CALIB_MODE: motor calibration mode activation request;
 * + [5] CMD_MOVE_X: X motor activation;
 * + [6] CMD_MOVE_Y: Y motor activation;
 * + [7] CMD_MOVE_Z: Z motor activation;
 * + [8] CMD_ENABLE_KEYSTEP: KeyStep enable command;
 * + [9] CMD_SERVICE_TEST_CYCLE: cycle test command;
 * 
 */     

/// \ingroup CANPROT
/// This is the list of the implemented COMMANDS
typedef enum{
   CMD_ABORT = 0,           //!< Abort Command
   CMD_DISABLE_MODE = 1,        //!< Disable Mode Activation Command
   CMD_COMMAND_MODE = 2,        //!< Command Mode Activation Command
   CMD_SERVICE_MODE = 3,        //!< Service Mode Activation Command
   CMD_CALIB_MODE = 4,          //!< Calibration Mode Activation Command
   CMD_MOVE_X = 5,              //!< Moves the X position command
   CMD_MOVE_Y = 6,              //!< Moves the Y position command
   CMD_MOVE_Z = 7,              //!< Moves the Z position command
   CMD_ENABLE_KEYSTEP = 8,       //!< Enable/Disable the Key Step mode (only in COMMAND mode)
   CMD_SERVICE_TEST_CYCLE = 9   //!< Service Cycle Test activatioin command    
}PROTOCOL_COMMANDS_t;
    
        


         

#endif 