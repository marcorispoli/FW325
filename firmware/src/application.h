/*!
 * \defgroup  applicationModule Application Modules section
 * 
 */


/*!
 * \defgroup  applicationConstants Application Constants section
 * 
 * \ingroup applicationModule
 */


/*!
 * \defgroup  libraryModules Library Modules Used in the Application.
 */


/*!
 * \mainpage
 *
 * # Overview
 *
 * This Firmware implements the functions of the PCB/22-303 device  software specification.
 * 
 * 
 * # CONFIGURATION SETTING
 * 
 * ## BOOTLOADER object link
 * 
 * This application links the FW303_BOOT.xx.yy.hex as a loadable object;
 * 
 * ## SHARED RAM AREA DEFINITION
 * 
 * The Application reserves 16 byte of RAM for the Bootloader shared area
 * 
 * ```text
 * Menu-Production
 * - Set Project Configuration
 *    - Customize ..
 *        - XC32 Global Options
 *            - XC32-ld
 *                - Additional options: -DRAM_ORIGIN=0x20000010,-DRAM_LENGTH=0x3FFF0 
 * 
 *   ```
 * 
 * 
 * ## HARMONY 3 REVISION PACK 
 * 
 * The Application has been developed with the use of the Microchip Harmony 3
 * libraries with the following revision codes: (see config/default/harmony-manifest-success.yml)
 * 
 *  - mcc_version: v5.3.0
 *  - mcc_core_version: v5.5.0
 *  - mplabx_version: v6.05        # if MPLAB X plugin only
 *  - harmony_version: v1.3.0
 *  - compiler: XC32 (v3.01) 
 *
 *  - modules:
 *       - {name: "csp", version: "v3.9.1"}
 *       - {name: "dev_packs", version: "v3.9.0"}
 *
 * ## PERIPHERAL CONFIGURATION
 * 
 * ### CLOCK GENERATOR CONFIGURATION
 * 
 * + DFLL: set to 48MHz Open Loop Mode;
 * + GLK1: input = DFLL, output = 48MHz;
 * + FDPLL0: input = GLK1, output = 120MHz;
 * + GLK0: input = FDPLL0, output = 120MHz;
 * + GLK2: input = DFLL, output = 8MHz;
 * + GLK4: input = DFLL, output = 24MHz;
 * 
 * ### PERIPHERAL CLOCK CONFIGURATION
 * 
 * + CPU : input = GLK0, output = 120MHz;
 * + CAN0: input = GLK4; output = 24MHz;
 * + TCO,TC1 : input = GLK2, output = 8MHz;
 * 
 *  ### MCC SYSTEM configuration
 * 
 * ```text
 * Device & Project Configuration
 *    - Project Configuration
 *        - Tool ChainSelections
 *            - XC32 Global Options
 *                - Linker
 *                    - Symbols & Macros
 *                        - Application Start Address (Hex) = 0x2000
 * 
 * Device & Project Configuration
 *      - ATSAME51J20A Device Configuration/Fuse settings:
 *              - Number Of Phisical NVM Blocks Composing a SmartEEPROM Sector = 1;
 *              - Size Of SmartEEPROM Page = 1;
 * 
 * ```
 * 
 * ### MCC CAN0 configuration
 * 
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
 *      + Number of element: 1
 * 
 * + Standard Filters 
 *  + Number Of STandard Filters: 2
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
 *      + Element Configuration: Store in RX FIFO 1
 * 
 *  + Reject Standard Remote Frames: YES
 * 
 *  + Timestamp Enable: YES 
 * 
 * ### TC0 CONFIGURATION
 * 
 * + Counter mode: 16bit mode;
 * + Select Prescaler: GLK_TC;
 * + Operating mode: Timer;
 * + Enable One-Shot mode: yes;
 * + Timer Period Unit: microsecond;
 * + Time: 1 
 * + Enable Timer Period Interrupt: yes;
 *  
 * 
 * # Licensing
 *
 * The Application has been developed with the Microchip Harmony 3 libraries 
 * with the Microchip License Agreement conditions as described in:
 * - @ref licenseAgreement 
 *
 * # Application documentation
 * 
 * This documentation has been produced with Doxygen 
 */



#ifndef _APPLICATION_H    /* Guard against multiple inclusion */
#define _APPLICATION_H

// Defines the Application revision code

/** \addtogroup applicationConstants
 *  @{
*/

// Application Revision 
static const unsigned char  APPLICATION_MAJ_REV =  1 ;  //!< Revision Major Number
static const unsigned char  APPLICATION_MIN_REV =  0 ;  //!< Revision Minor Number
static const unsigned char  APPLICATION_SUB_REV =  1 ;  //!< Revision build Number

/** @}*/
        
#endif 