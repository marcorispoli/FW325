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
        
        /**
         * 
         * \addtogroup CANPROT 
         * ## ABORT COMMAND
         * 
         * This command aborts any pending command.
         * 
         * @param cmd = \ref MET_COMMAND_ABORT;
         * @param d0 = not used
         * @param d1 = not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         * + ImmediateError(ABORT)
         * 
         */   
        case MET_COMMAND_ABORT:  // This is the Library mandatory 
            motorStruct.abort_request = true;
            MET_Can_Protocol_returnCommandAborted();              
            break;
        
        /**
         * 
         * \addtogroup CANPROT 
         * ## ACTIVATE DISABLE MODE WORKFLOW
         * 
         * This command activates the Disable Mode Workflow.
         * 
         * @param cmd = \ref CMD_DISABLE_MODE;
         * @param d0 = not used
         * @param d1 = not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         * + ImmediateExecuted(0,0)
         * 
         */   
        case CMD_DISABLE_MODE:
            motorSetDisableMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
            
        /**
         * 
         * \addtogroup CANPROT 
         * ## ACTIVATE COMMAND MODE WORKFLOW
         * 
         * This command activates the Command Mode Workflow.
         * 
         * @param cmd = \ref CMD_COMMAND_MODE;
         * @param d0 = not used
         * @param d1 = not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         * + ImmediateExecuted(0,0)
         * 
         */   
        case CMD_COMMAND_MODE:
            motorSetCommandMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
        
        /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         * ## ACTIVATE SERVICE MODE WORKFLOW
         * 
         * This command activates the Service Mode Workflow.
         * 
         * @param cmd = \ref CMD_SERVICE_MODE;
         * @param d0 = not used
         * @param d1 = not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         * + ImmediateExecuted(0,0)
         * 
         */   
        case CMD_SERVICE_MODE: // Sets the service mode
            motorSetServiceMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
            
         /**
         * 
         * \addtogroup CANPROT 
         * ## ACTIVATE CALIBRATION MODE WORKFLOW
         * 
         * This command activates the Calibration Mode Workflow.
         * 
         * @param cmd = \ref CMD_CALIB_MODE;
         * @param d0 = not used
         * @param d1 = not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         * + ImmediateExecuted(0,0)
         * 
         */   
        case CMD_CALIB_MODE: // Sets the service mode
            motorSetCalibMode();
            MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
        
        /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         * ## MOVE-X COMMAND
         * 
         * This command activates the X to the target position.\n
         * The target position is passed to the command in 0.1mm/units
         * 
         * @param cmd = \ref CMD_MOVE_X;
         * @param d0 = XL: low byte of the target X position (0.1mm/units)
         * @param d1 = XH: high byte of the target X position (0.1mm/units)
         * @param d2: not used
         * @param d3: not used
         * @return
         * 
         */
        case CMD_MOVE_X:
            switch(motorMoveX((int) d0 + (int) d1 * 256)){
                
                /// \addtogroup CANPROT
                /// \test Already-in-target: ImmediateExecuted(XL,XH)
                case MOTOR_ALREADY_IN_POSITION:
                    MET_Can_Protocol_returnCommandExecuted(d0,d1);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Command-started: CommandExecuting
                case MOTOR_COMMAND_EXECUTING:
                    MET_Can_Protocol_returnCommandExecuting();
                    break;
                
                /// \addtogroup CANPROT
                /// \test Invalid-Position (exceeding the maximum position): ImmediateError(\ref MET_CAN_COMMAND_INVALID_DATA)
                case MOTOR_ERROR_INVALID_POSITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_INVALID_DATA);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Invalid-Working-Mode : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_INVALID_MODE:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Safety-Switch-Disabled (needle detected) : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_DISABLE_CONDITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Busy : ImmediateError(\ref MET_CAN_COMMAND_BUSY)       
                case MOTOR_ERROR_BUSY:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_BUSY);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test invalid-return_code (software bug): ImmediateError(\ref MET_CAN_COMMAND_WRONG_RETURN_CODE)       
                default:    
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_WRONG_RETURN_CODE);
            }
            break;
        
         /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         * ## MOVE-Y COMMAND
         * 
         * This command activates the Y to the target position.\n
         * The target position is passed to the command in 0.1mm/units
         * 
         * @param cmd = \ref CMD_MOVE_Y;
         * @param d0 = YL: low byte of the target Y position (0.1mm/units)
         * @param d1 = YH: high byte of the target Y position (0.1mm/units)
         * @param d2: not used
         * @param d3: not used
         * @return
         * 
         */
        case CMD_MOVE_Y:
            switch(motorMoveY((int) d0 + (int) d1 * 256)){
                
                /// \addtogroup CANPROT
                /// \test Already-in-target: ImmediateExecuted(YL,YH)
                case MOTOR_ALREADY_IN_POSITION:
                    MET_Can_Protocol_returnCommandExecuted(d0,d1);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Command-started: CommandExecuting
                case MOTOR_COMMAND_EXECUTING:
                    MET_Can_Protocol_returnCommandExecuting();
                    break;
                
                /// \addtogroup CANPROT
                /// \test Invalid-Position (exceeding the maximum position): ImmediateError(\ref MET_CAN_COMMAND_INVALID_DATA)
                case MOTOR_ERROR_INVALID_POSITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_INVALID_DATA);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Invalid-Working-Mode : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_INVALID_MODE:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Safety-Switch-Disabled (needle detected) : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_DISABLE_CONDITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Busy : ImmediateError(\ref MET_CAN_COMMAND_BUSY)       
                case MOTOR_ERROR_BUSY:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_BUSY);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test invalid-return_code (software bug): ImmediateError(\ref MET_CAN_COMMAND_WRONG_RETURN_CODE)       
                default:    
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_WRONG_RETURN_CODE);
            }
            break;
            
         /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         * ## MOVE-Z COMMAND
         * 
         * This command activates the Z to the target position.\n
         * The target position is passed to the command in 0.1mm/units
         * 
         * @param cmd = \ref CMD_MOVE_Z;
         * @param d0 = ZL: low byte of the target Z position (0.1mm/units)
         * @param d1 = ZH: high byte of the target Z position (0.1mm/units)
         * @param d2: not used
         * @param d3: not used
         * @return
         * 
         */
        case CMD_MOVE_Z:
            switch(motorMoveZ((int) d0 + (int) d1 * 256)){
                
                /// \addtogroup CANPROT
                /// \test Already-in-target: ImmediateExecuted(ZL,ZH)
                case MOTOR_ALREADY_IN_POSITION:
                    MET_Can_Protocol_returnCommandExecuted(d0,d1);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Command-started: CommandExecuting
                case MOTOR_COMMAND_EXECUTING:
                    MET_Can_Protocol_returnCommandExecuting();
                    break;
                
                /// \addtogroup CANPROT
                /// \test Invalid-Position (exceeding the maximum position): ImmediateError(\ref MET_CAN_COMMAND_INVALID_DATA)
                case MOTOR_ERROR_INVALID_POSITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_INVALID_DATA);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Invalid-Working-Mode : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_INVALID_MODE:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Safety-Switch-Disabled (needle detected) : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED)    
                case MOTOR_ERROR_DISABLE_CONDITION:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test Busy : ImmediateError(\ref MET_CAN_COMMAND_BUSY)       
                case MOTOR_ERROR_BUSY:
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_BUSY);
                    break;
                    
                /// \addtogroup CANPROT
                /// \test invalid-return_code (software bug): ImmediateError(\ref MET_CAN_COMMAND_WRONG_RETURN_CODE)       
                default:    
                    MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_WRONG_RETURN_CODE);
            }
            break;
            
        
        /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         * ## TEST CYCLE SERVICE COMMAND
         * 
         * This command activates a test cycle routine.
         * 
         * The test executes the following steps:
         * + Moves Z to 10mm;
         * + Moves X to 240 mm;
         * + Moves Y to 60 mm;
         * + Moves Y to 0 mm;
         * + Moves X to 0 mm;
         * + Moves Z to 10 mm;
         * 
         * The cycle is repeated until a key is pressed or the same command is received.
         * 
         * @param cmd = \ref CMD_SERVICE_TEST_CYCLE;
         * @param d0:  not used
         * @param d1: not used
         * @param d2: not used
         * @param d3: not used
         * 
         * @return
         * 
         */
        case CMD_SERVICE_TEST_CYCLE:
            
            /// \addtogroup CANPROT
            /// \test not-in-service-mode : ImmediateError(\ref MET_CAN_COMMAND_NOT_ENABLED) 
            if(!motorServiceTestCycle()) MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_ENABLED);
            
            /// \addtogroup CANPROT
            /// \test test-started : ImmediateSuccess(0,0) 
            else MET_Can_Protocol_returnCommandExecuted(0,0);
            break;
          
         /**
         * <div style="page-break-after: always;"></div>
         * \addtogroup CANPROT 
         */ 
        default:
            MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_AVAILABLE);
    }
    
    return;
}

//_________________________________ PROTOCOL DATA ACCESS IMPLEMENTATION ______________________________________

void updateStatusRegister(void* reg){
    MET_Can_Protocol_SetStatusReg(((REGISTER_STRUCT_t*) reg)->idx, 0, ((REGISTER_STRUCT_t*) reg)->d0 );
    MET_Can_Protocol_SetStatusReg(((REGISTER_STRUCT_t*) reg)->idx, 1, ((REGISTER_STRUCT_t*) reg)->d1 );
    MET_Can_Protocol_SetStatusReg(((REGISTER_STRUCT_t*) reg)->idx, 2, ((REGISTER_STRUCT_t*) reg)->d2 );
    MET_Can_Protocol_SetStatusReg(((REGISTER_STRUCT_t*) reg)->idx, 3, ((REGISTER_STRUCT_t*) reg)->d3 );
}

void updateDataRegister(void* reg){
    ((REGISTER_STRUCT_t*) reg)->d0 = MET_Can_Protocol_GetData(((REGISTER_STRUCT_t*) reg)->idx, 0);
    ((REGISTER_STRUCT_t*) reg)->d1 = MET_Can_Protocol_GetData(((REGISTER_STRUCT_t*) reg)->idx, 1);
    ((REGISTER_STRUCT_t*) reg)->d2 = MET_Can_Protocol_GetData(((REGISTER_STRUCT_t*) reg)->idx, 2);
    ((REGISTER_STRUCT_t*) reg)->d3 = MET_Can_Protocol_GetData(((REGISTER_STRUCT_t*) reg)->idx, 3);
    
}