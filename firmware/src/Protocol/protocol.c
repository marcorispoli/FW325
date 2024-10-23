#define _PROTOCOL_C

#include "application.h"
#include "protocol.h"
#include "../Motors/motors.h"

static void ApplicationProtocolCommandHandler(uint8_t cmd, uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3 ); //!< This is the Command protocol callback

/**
 * This function initializes the CAN Protocol module.
 * 
 * The function calls the library API  MET_Can_Protocol_Init() in order to:
 * + Set the Device Identifier;
 * + The number of implemented STATUS registers;
 * + The number of implemented DATA registers;
 * + The number of implemented PARAMETER registers;
 * + The Application revision 
 * + The protocol command handler;
 * 
 * The function initializes the Parameters with the default value   
 * with the library MET_Can_Protocol_SetDefaultParameter() function.
 * 
 */
void ApplicationProtocolInit ( void )
{
     
    // Initialize the Met Can Library
    MET_Can_Protocol_Init(MET_CAN_APP_DEVICE_ID, MET_CAN_STATUS_REGISTERS, MET_CAN_DATA_REGISTERS, MET_CAN_PARAM_REGISTERS, APPLICATION_MAJ_REV, APPLICATION_MIN_REV, APPLICATION_SUB_REV, ApplicationProtocolCommandHandler);
    
}
  
/**
 * This function shall be called by the MAIN loop application 
 * in order to manage the reception/transmission protocol activities.
 * 
 * The function merely call the library function  MET_Can_Protocol_Loop().
 */
void inline ApplicationProtocolLoop(void){
    
    // Handles the transmission/reception protocol
    MET_Can_Protocol_Loop();        
    
}



/// \ingroup CANPROT
/// Command handler routine
void ApplicationProtocolCommandHandler(uint8_t cmd, uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3 ){

    switch(cmd){
        
        /// \addtogroup CANPROT
        /// ## Abort Command
        /// + Command code: \ref MET_COMMAND_ABORT;
        /// + Parameters: no parameters;
        /// + Description: Abort all the activated command;
        /// + Command return mode: Immediate [0,0];
        case MET_COMMAND_ABORT:  // This is the Library mandatory 
            motorSetDisableMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
        
        /// \addtogroup CANPROT
        /// ## Disable Mode Command
        /// + Command code: \ref CMD_DISABLE_MODE
        /// + Parameters: no parameters;
        /// + Description: activates the Disable Workflow;
        /// + Command return mode: Immediate [0,0];
        case CMD_DISABLE_MODE:
            motorSetDisableMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
            
        /// \addtogroup CANPROT
        /// ## Command Mode Command
        /// + Command code: \ref CMD_COMMAND_MODE
        /// + Parameters: no parameters;
        /// + Description: activates the Command Workflow;
        /// + Command return mode: Immediate [0,0];    
        case CMD_COMMAND_MODE:
            motorSetCommandMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
        
        /// \addtogroup CANPROT
        /// ## Service Mode Command
        /// + Command code: \ref CMD_SERVICE_MODE
        /// + Parameters: no parameters;
        /// + Description: activates the Service Workflow;
        /// + Command return mode: Immediate [0,0];    
        case CMD_SERVICE_MODE: // Sets the service mode
            motorSetServiceMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
            
        /// \addtogroup CANPROT    
        /// ## Calibration Mode Command
        /// + Command code: \ref CMD_CALIB_MODE
        /// + Parameters: no parameters;
        /// + Description: activates the Calibration Workflow;
        /// + Command return mode: Immediate [0,0];    
        case CMD_CALIB_MODE: // Sets the service mode
            motorSetCalibMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
        
        /// \addtogroup CANPROT   
        /// ## Move XYZ Command
        /// + Command code: \ref CMD_MOVE_XYZ
        /// + Parameters: XL, XH, Y, Z;
        ///     + [XH,XL]: is the target X position in mm;
        ///     + [Y]: is the target Y position in mm;
        ///     + [Z]: is the target Z position in mm;
        /// + Pre Condition: COMMAND MODE active
        /// + Description: activates the XYZ axes to the target position;
        /// + Command return mode: Immediate [0,0];
        case CMD_MOVE_XYZ:
            MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_AVAILABLE);
            break;
        
        /// \addtogroup CANPROT
        /// ## Service Test Cycle  Command
        /// + Command code: \ref CMD_SERVICE_TEST_CYCLE
        /// + Parameters: no parameters;
        /// + Description: activates the Test Cycle routine;
        /// + Pre Condition: SERVICE MODE active    
        /// + Command return with success: Immediate [0,0];     
        /// + Command return with error: MET_CAN_COMMAND_NOT_ENABLED;     
        case CMD_SERVICE_TEST_CYCLE:
            if(!motorServiceTestCycle()) MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
            else MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
            
        default:
            MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_AVAILABLE);
    }
    
    return;
}

//_________________________________ PROTOCOL DATA ACCESS IMPLEMENTATION ______________________________________

void updateStatusRegister(void* reg){
    MET_Can_Protocol_SetStatusReg(((GENERIC_STATUS_t*) reg)->idx, 0, ((GENERIC_STATUS_t*) reg)->d0 );
    MET_Can_Protocol_SetStatusReg(((GENERIC_STATUS_t*) reg)->idx, 1, ((GENERIC_STATUS_t*) reg)->d1 );
    MET_Can_Protocol_SetStatusReg(((GENERIC_STATUS_t*) reg)->idx, 2, ((GENERIC_STATUS_t*) reg)->d2 );
    MET_Can_Protocol_SetStatusReg(((GENERIC_STATUS_t*) reg)->idx, 3, ((GENERIC_STATUS_t*) reg)->d3 );
}

 
        