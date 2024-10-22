#ifndef _MET_CAN_PROTOCOL_H
    #define _MET_CAN_PROTOCOL_H

#include "definitions.h"                // SYS function prototypes



#undef ext
#undef ext_static

#ifdef _MET_CAN_PROTOCOL_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/*!
 * \defgroup metProtocolModule  CAN Communication Engine Module
 *
 * \ingroup libraryModules
 * 
 * # Overview
 *
 * This module implements the engine communication functionalities 
 * of the Z190 Device Communication protocol specifications in the 
 * CAN Channel 0.
 * 
 * 
 * 
 * # Communication settings
 * 
 * The CAN communication is based on the following 
 * characteristics:
 * 
 * - Standard (not Flexible) communication;
 * - Standard frame (11 bit ID);
 * - Baude Rate: 1Mb/s;
 * - Application Can ID reception address: 0x140 + deviceID;
 * - Bootloader Can ID reception address: 0x100 + deviceID;
 * 
 * The deviceID is a decimal value from 1 to 0x3F.
 * 
 * # Harmony 3 Configurator Settings
 * 
 * This Module makes use of the following Processor modules:
 * - CAN0 peripheral module;
 * - CAN1 peripheral module (optional);
 * - Peripheral Can0 clock;
 * - Peripheral Can1 clock (optional);
 * - PIN Assignment;
 * - SYSTEM Memory;
 * 
 * ## SYSTEM Peripheral module setup
 * 
 * This Module makes use of the Smart EEPROM feature of the microcontroller.
 * 
 * The module shall provides 256 32-bit word to be assigned to parameters.
 * 
 * According with this requisite the SYSTEM peripheral module shall be set as follow:
 * ```text
 * 
 * Device & Project Configuration
 *      - ATSAME51J20A Device Configuration/Fuse settings:
 *              - Number Of Phisical NVM Blocks Composing a SmartEEPROM Sector = 1;
 *              - Size Of SmartEEPROM Page = 1;
 * 
 * ```
 * 
 * ## Pin Assignement
 * 
 * + The CAN TX Input shall be configured with CAN0-TX 
 * + The CAN RX Input shall be configured with CAN0-TX 
 * + The Bridge CAN TX Input shall be configured with CAN1-TX (optional)
 * + The Bridge CAN RX Input shall be configured with CAN1-TX (optional) 
 * 
 * ## Peripheral Clock Settings 
 * 
 *+ Activate the GLK4 Clock:
 *   + Set the DFLL (48MHz) as the Input clock;
 *   + Set the divisor to 2 to obtain a final 24MHz of GLK4 Clock;
 *   + The CAN0 Peripheral clock shall be configured with the GLK4 clock at 24 MHz.
 * 
 * ## CAN0 configuration
 * 
 * ```text
 * 
 * + CAN Operational Mode = NORMAL;
 * + Interrupt Mode: Yes;
 * + Bit Timing Calculation
 *  + Nominal Bit Timing
 *      + Automatic Nominal Bit Timing: Yes;
 *      + BIt Rate: 1000
 * 
 * + Use RX FIFO 0: Yes
 *   + RX FIFO 0 Setting
 *      + Number of element: 1
 * 
 * + Use RX FIFO 1: Yes
 *   + RX FIFO 1 Setting
 *      + Number of element: 16
 * 
 * + Use TX FIFO: Yes
 *   + TX FIFO Setting
 *      + Number of element: 16 
 * 
 * + Standard Filters 
 *  + Number Of STandard Filters: 4
 * 
 *  + Standard Filter 1
 *      + Type: Range;
 *      + ID1: 0x140 + Device ID 
 *      + ID2: 0x140 + Device ID 
 *      + Element Configuration: Store in RX FIFO 0
 * 
 *  + Standard Filter 2
 *      + Type: Range;
 *      + ID1: 0x100 + Device ID  
 *      + ID2: 0x100 + Device ID 
 *      + Element Configuration: Store in RX FIFO 0
 * 
 *  + Standard Filter 3
 *      + Type: Range;
 *      + ID1: 0  
 *      + ID2: 0 
 *      + Element Configuration: Store in RX FIFO 1
 * 
 *  + Standard Filter 4
 *      + Type: Range;
 *      + ID1: 0x603  
 *      + ID2: 0x607
 *      + Element Configuration: Store in RX FIFO 1

 *  + Reject Standard Remote Frames: YES
 * 
 *  + Timestamp Enable: YES 
 * 
 * ```
 *
 * 
 *  * ## CAN1 configuration (for Motor Bridge only)
 * 
 * ```text
 * 
 * + CAN Operational Mode = NORMAL;
 * + Interrupt Mode: Yes;
 * + Bit Timing Calculation
 *  + Nominal Bit Timing
 *      + Automatic Nominal Bit Timing: Yes;
 *      + BIt Rate: 1000
 * 
 * + Use RX FIFO 0: Yes
 *   + RX FIFO 0 Setting
 *      + Number of element: 1
 * 
 * + Use RX FIFO 1: No
 * 
 * + Use TX FIFO: Yes
 *   + TX FIFO Setting
 *      + Number of element: 16 
 *
 * + Standard Filters 
 *  + Number Of STandard Filters: 2
 * 
 *  + Standard Filter 1
 *      + Type: Range;
 *      + ID1: 0  
 *      + ID2: 0 
 *      + Element Configuration: Store in RX FIFO 0
 * 
 *  + Standard Filter 2
 *      + Type: Range;
 *      + ID1: 0x603  
 *      + ID2: 0x607
 *      + Element Configuration: Store in RX FIFO 0

 *  + Reject Standard Remote Frames: YES
 * 
 *  + Timestamp Enable: YES 
 * 
 * ```
 *  # Application Usage
 * 
 *  The Application shall define the structure of the Protocol registers:
 * + shall define the number of STATUS registers;
 * + shall define the number of DATA registers;
 * + shall define the number of PARAMETER registers;
 * + Shall instantiate the Command callback function;
 * 
 * + In case the implementation makes use of the Motor Bridge from Can0 to Can1 
 *   the application shall define #define _MET_MOTOR_BRIDGE_ in top of the headers 
 * + The Application initializes the module with the MET_Can_Protocol_Init() function;
 * + The Application shall call the MET_Can_Protocol_Loop() function in the main loop;
 *   
 *  The Application shall use the following routines to handle with the Registers:
 * 
 *  + Functions to Get/Set the Application STATUS registers:
 *      + MET_Can_Protocol_SetStatusBit(): sets ON/OFF all the bits of a mask;
 *      + MET_Can_Protocol_SetStatusReg(): sets a byte of a STATUS register;
 *      + MET_Can_Protocol_GetStatus(): returns a byte value of a STATUS register;
 *      + MET_Can_Protocol_TestStatus(): test a condition on a STATUS register mask;
 * 
 *  + Functions to Get/Set the Application ERRORS register:
 *      + MET_Can_Protocol_SetErrorsBit(): sets ON/OFF all the bits of a mask into the ERRORS register;
 *      + MET_Can_Protocol_SetErrorsReg(): sets a byte of a ERRORS register;
 *      + MET_Can_Protocol_GetErrors(): returns a byte value of a ERRORS register;
 *      + MET_Can_Protocol_TestErrors(): test a condition on a ERRORS register mask;
 *
 *  
 *  + Functions to Test the Application DATA registers:
 *      + MET_Can_Protocol_GetData(): returns a byte value of a DATA register;
 *      + MET_Can_Protocol_TestData(): test a condition on a DATA register mask;
 *
 *  + Functions to Test the Application PARAMETER registers:
 *      + MET_Can_Protocol_GetParameter(): returns a byte value of a PARAMETER register;
 *      + MET_Can_Protocol_TestParameter(): test a condition on a PARAMETER register mask;
 * 
 * 
 *  + Functions to handle the Command Execution:
 *      + MET_Can_Protocol_getCommandCode(): return the requested command code;
 *      + MET_Can_Protocol_getCommandParam0(): requests for the command parameter 0;
 *      + MET_Can_Protocol_getCommandParam1(): requests for the command parameter 1;
 *      + MET_Can_Protocol_getCommandParam2(): requests for the command parameter 2;
 *      + MET_Can_Protocol_getCommandParam3(): requests for the command parameter 3;
 *      + MET_Can_Protocol_setReturnCommand(): set the return code should be returned after the command handling.
 * 
 *
 *    @{
 * 
 */

    /** 
     * \defgroup metCanConstants  Module Constants  definition
     * 
     * This section implements the data structures of the module
     *  @{
     */
        // Defines the EEPROM size to 1024 byte
        #define MET_EEPROM_BLK  1 //!< NUMBER OF SMART EEPROM BLOCKS 
        #define MET_EEPROM_PSZ  1 //!< SIZE OF SMART EEPROM BLOCK 

        #define MAX_STATUS_REG      200 //!< MAX NUMBER OF STATUS REGISTERS
        #define MAX_DATA_REG        200 //!< MAX NUMBER OF DATA REGISTERS
        #define MAX_PARAM_REG       200 //!< MAX NUMBER OF PARAM REGISTERS

        #define _CAN_ID_BASE_ADDRESS 0x140 //!< This is the base address for the communication point to point
        #define _CAN_ID_BOOTLOADER_ADDRESS 0x100 //!< This is the base address for the Loader frames
        #define _BOOTLOADER_SHARED_RAM   0x20000000 //!< RAM shared start address


        #define _BOOT_ACTIVATION_CODE_PRESENCE0  0x11 //!< Code 0 Bootloader presence
        #define _BOOT_ACTIVATION_CODE_PRESENCE1  0x82 //!< Code 1 Bootloader presence
        #define _BOOT_ACTIVATION_CODE_PRESENCE2  0x13 //!< Code 2 Bootloader presence
        #define _BOOT_ACTIVATION_CODE_PRESENCE3  0x84 //!< Code 3 Bootloader presence

        #define _BOOT_ACTIVATION_CODE_START0  0x1 //!< Code 0 Bootloader start
        #define _BOOT_ACTIVATION_CODE_START1  0x2 //!< Code 1 Bootloader start
        #define _BOOT_ACTIVATION_CODE_START2  0x3 //!< Code 2 Bootloader start
        #define _BOOT_ACTIVATION_CODE_START3  0x4 //!< Code 3 Bootloader start

        #define MET_COMMAND_ABORT 0 //!< This is the reserved command code for the ABORT command

    /** @}*/  // metCanConstants

    /** 
     * \defgroup metCanEnumeration  Module Enumeration types
     * 
     * This section describes the Module enumeration type definitions
     *  @{
     */

        /// Bootloader Command enumeration
        typedef enum{
            BOOTLOADER_GET_INFO = 1,            //!< Read the bootloader and app info
            BOOTLOADER_START,                   //!< Request to start the bootloader
        }BOOTLOADER_FRAME_CODES;
        
        /**
         * @brief This is the enumeration class for the Frame Command codes
         */
        typedef enum{
            MET_CAN_PROTOCOL_READ_REVISION = 1, //!< Read Application Special Revision registers frame type
            MET_CAN_PROTOCOL_READ_ERRORS,       //!< Read Application Special Errors registers frame type
            MET_CAN_PROTOCOL_READ_COMMAND,      //!< Read Application Special Command registers frame type
            MET_CAN_PROTOCOL_READ_STATUS,       //!< Read Application Status registers frame type
            MET_CAN_PROTOCOL_READ_DATA,         //!< Read DATA registers frame type
            MET_CAN_PROTOCOL_READ_PARAM,        //!< Read PARAMETER registers frame type
            MET_CAN_PROTOCOL_WRITE_DATA,        //!< Write Data Registers frame type
            MET_CAN_PROTOCOL_WRITE_PARAM,       //!< Write Parameter Registers frame type
            MET_CAN_PROTOCOL_STORE_PARAMS,      //!< Store Parameters command frame 
            MET_CAN_PROTOCOL_COMMAND_EXEC,      //!< Command Execution frame
            MET_CAN_PROTOCOL_RESET_CODE         //!< Reset Code 
        }MET_FRAME_CODES;
        
         /** 
         * @brief Can Protocol Error codes
         * 
         * Those errors are generated into the module in case
         * of some function should fail. When a error is detected, 
         * the error is provided to an internal error handler: MET_CanOpen_Error_Callback()
         * 
         */ 
        typedef enum
        {    
            MET_CAN_PROTOCOL_ERROR_OK=0, //!< No error
            MET_CAN_PROTOCOL_ERROR_RECEPTION_ACTIVATION, //!< Error in activating the reception
            MET_CAN_PROTOCOL_ERROR_TRANSMISSION, //!> Error during data transmission       
            MET_CAN_PROTOCOL_ERROR_INVALID_CRC, //!> Frame with invalid CRC received        
            MET_CAN_PROTOCOL_ERROR_INVALID_LENGHT, //!> Frame with invalid Lenght

        } MET_CAN_PROTOCOL_ERROR_DEFS;
        
         /**
         * @brief This is the Command Error enumeration
         */
        typedef enum{
            MET_CAN_COMMAND_NO_ERROR = 0,       //!< No Error 
            MET_CAN_COMMAND_BUSY = 1,           //!< A command is executing
            MET_CAN_COMMAND_INVALID_DATA = 2,   //!< The Command data are invalid
            MET_CAN_COMMAND_NOT_ENABLED = 3,    //!< The Command cannot be momentary executed 
            MET_CAN_COMMAND_NOT_AVAILABLE = 4,  //!< The Command cannot be executed in this revision      
            MET_CAN_COMMAND_WRONG_RETURN_CODE = 5,  //!< The Command returned a non valid status code
            MET_CAN_COMMAND_ABORT_CODE = 6,         //!< The Command has been Aborted
            MET_CAN_COMMAND_APPLICATION_ERRORS  //!< Starting code for applicaiton specific errors                    
        }MET_CommandErrors_t;
        
          /**
         * @brief This type defines the possible Command Execution status
         * 
         */
        typedef enum{
            MET_CAN_COMMAND_EXECUTING = 1,       //!< The Command is processing
            MET_CAN_COMMAND_EXECUTED,            //!< The Command has been processed successfully
            MET_CAN_COMMAND_ERROR,               //!< The Command has been terminated with error condition
            MET_CAN_COMMAND_STATUS_UNASSIGNED,   //!< The Application forgot to assign a correct return code
        }MET_CommandExecStatus_t;
        
        /**
        * This is the enumeration to access the content of the ERRORS register
        */
       typedef enum{
            MET_CAN_ERROR_MOM0 = 0, //!< byte index of the Momentary error 0
            MET_CAN_ERROR_MOM1,     //!< byte index of the Momentary error 1
            MET_CAN_ERROR_PERSISTENT0, //!< byte index of the Persisten error 0
            MET_CAN_ERROR_PERSISTENT1, //!< byte index of the Persisten error 1
        }MET_CAN_ERROR_BYTE_t;
        
    /** @}*/  // metCanEnumeration

    /** 
     * \defgroup metCanData  Module data structures
     * 
     * This section implements the data structures of the module
     *  @{
     */

        /**
         * This is the structure of the Shared ram area
         */
        typedef struct{
            uint8_t activation_code0; //!< Activation code byte 0
            uint8_t activation_code1; //!< Activation code byte 1
            uint8_t activation_code2; //!< Activation code byte 2
            uint8_t activation_code3; //!< Activation code byte 3

            uint8_t boot_maj;   //!< Bootloader Major revision code
            uint8_t boot_min;   //!< Bootloader Minor revision code
            uint8_t boot_sub;   //!< Bootloader Sub revision code
            uint8_t app_maj;    //!< Application Major revision code
            uint8_t app_min;    //!< Application Mino revision code
            uint8_t app_sub;    //!< Application Sub revision code

            uint8_t sp0;
            uint8_t sp1;
            uint8_t sp2;
            uint8_t sp3;
            uint8_t sp4;
            uint8_t sp5;

        }_BOOTLOADER_SHARED_t;
        

        /**
         * This is the Protocol frame data content
         */
        typedef struct {
            uint8_t seq;        //!< Frame sequence number
            uint8_t frame_cmd;  //!< Frame command code  
            uint8_t idx;        //!< This is the IDX field
            uint8_t d[4];       //!< Frame data content
            uint8_t crc;        //!< Frame CRC
        }MET_Can_Frame_t;
        
        /**
         * Register Data type defining the content of a register
         *  
         */
        typedef struct {
            uint8_t d[4]; //!< Register data content
        }MET_Register_t;
        
        
       
        /**
         * @brief This is the type definition for the Command callback function
         * 
         * @Param cmd is the requested command code;
         * @Param d0 is the optional command d0;
         * @Param d1 is the optional command d1;
         * @Param d2 is the optional command d2;
         * @Param d3 is the optional command d3;
         */       
        typedef void (*MET_commandHandler_t)(uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
        
        
        /** 
        * ***REVISION STATUS REGISTER***
        * 
        * This is the structure of the Revision register content
        * 
        * The register data content is:
        * 
        * |BYTE|DESCRIPTION|
        * |:---:|:---|
        * |D0|Revision Major number|
        * |D1|Revision Minor number|
        * |D2|Revision Sub number|
        * |D3|-|
        * 
        * 
        */  
       typedef struct {
         uint8_t maj;  //!< D0 - This is the Major revision number
         uint8_t min;  //!< D1 - This is the minor revision number
         uint8_t sub;  //!< D2 - This is the Sub-minor revision number
         uint8_t d3;   //!< D3 - NA
       }MET_Revision_Register_t;
       
       /** 
        * ***ERRORS STATUS REGISTER***
        * 
        * This is the structure of the Errors register content
        * 
        * The register data content is:
        * 
        * |BYTE|DESCRIPTION|
        * |:---:|:---|
        * |D0|momentary-0|
        * |D1|momentary-1|
        * |D2|Persistent-0|
        * |D3|Persistent-1|
        * 
        * 
        */  
       typedef struct {
         uint8_t mom0;  //!< D0 - This is the Momentary error byte 0
         uint8_t mom1;  //!< D1 - This is the Momentary error byte 1
         uint8_t pers0;  //!< D2 - This is the Persisten error byte 0
         uint8_t pers1;   //!< D3 - This is the Persisten error byte 1
       }MET_Errors_Register_t;

        
        /** 
        * ***COMMAND STATUS REGISTER***
        * 
        * This is the structure of the Command register content.
        * 
        */  
       typedef struct {
            uint8_t command;                //!< Command Code
            MET_CommandExecStatus_t status; //!< Command execution status code
            uint8_t result[2];              //!< Command result data
            uint8_t error;                  //!< Command error code in case of unsuccess; 
       }MET_Command_Register_t;
           
       
            
    /** @}*/  // metCanData

/** @}*/  // metProtocolModule
        


/******************************************************************************/
/*                      API DI ISTALLAZIONE PROTOCOLLO                        */
/******************************************************************************/
        

        
     /** 
     * \defgroup metCanApi  Module Installation API
     * 
     * \ingroup metProtocolModule
     * This section providides the API for the CAN protocol integration.
     * 
     *  @{
     */
        
        /// Can protocol initialization
        ext void MET_Can_Protocol_Init(uint8_t devId, uint8_t statReg, uint8_t dataReg, uint8_t paramReg, uint8_t appMaj, uint8_t appMin, uint8_t appSub, MET_commandHandler_t pCommandHandler);
        
        /**
         * If _MET_MOTOR_BRIDGE_ define is present 
         * this function activates the communication bridge from 
         * CAN0-FIFO1 reception and CAN1-FIFO0 reception 
         */
        ext void MET_InitCanBridge(void);
        
        /// Application Main Loop function handler
        void MET_Can_Protocol_Loop(void);  
        
     /** @}*/  // metCanApi
        
    /** 
     * \defgroup metCanRegApi  Module Register Access API
     * 
     * \ingroup metProtocolModule
     * This section providides the API for the CAN protocol integration.
     * 
     *  @{
     */
      
        /// Sets the bit-wise content of a STATUS register
        ext void  MET_Can_Protocol_SetStatusBit(uint8_t idx, uint8_t data_index, uint8_t mask, bool stat);
    
        /// Sets the whole content of a STATUS register
        ext void  MET_Can_Protocol_SetStatusReg(uint8_t idx, uint8_t data_index, uint8_t val );
        
        /// Returns the content of the STATUS register array
        uint8_t  MET_Can_Protocol_GetStatus(uint8_t idx, uint8_t data_index);
    
        // The function tests the content of a STATUS register with a mask byte
        bool  MET_Can_Protocol_TestStatus(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        /// This function set a error condition
        ext void  MET_Can_Protocol_SetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1);
        
        /// This function get the current error condition        
        ext void  MET_Can_Protocol_GetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1);
        
        
        /// Returns the pointer to the Application Data register array
        ext uint8_t  MET_Can_Protocol_GetData(uint8_t idx, uint8_t data_index);
    
        // The function tests the content of a Data register with a mask byte
        ext bool  MET_Can_Protocol_TestData(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        /// Returns the pointer to the Application PARAMETER register array
        ext uint8_t  MET_Can_Protocol_GetParameter(uint8_t idx, uint8_t data_index);
    
        /// The function tests the content of a PARAMETER register with a mask byte
        ext bool  MET_Can_Protocol_TestParameter(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        /// This function shall be used to set the Default parameter value 
        ext void  MET_Can_Protocol_SetDefaultParameter(uint8_t idx, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);        

        /// Set the COMMAND EXECUTION return code
        ext void MET_Can_Protocol_returnCommandExecuting(void);
        
        /// Set the COMMAND EXECUTED return code
        ext void MET_Can_Protocol_returnCommandExecuted(uint8_t ris0, uint8_t ris1);
        
        /// Set the COMMAND ERROR return code
        ext void MET_Can_Protocol_returnCommandError(uint8_t err);
        
        /// Set the COMMAND ABORTED  return code
        ext void MET_Can_Protocol_returnCommandAborted(void);
        
        /** @}*/  // metCanRegApi



#endif /* _MET_CAN_PROTOCOL_H */

