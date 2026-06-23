#include "../inc/control_app.h"
#include "../inc/pid_control.h"
#include "../../platform/inc/actuator_hal.h" 
#include "../../platform/inc/soft_timer.h"
#include "../../platform/inc/sensor_hal.h"
#include "../inc/protocol_handler.h"
#include "../../common/inc/frame_common.h"
#include "../../common/inc/queue_manager.h"

// Greenhouse data
static GreenhouseData_t g_data = {
	.temperature = 0.0f,
	.temp_setpoint = 40.0f,
	.fan_state = ACT_OFF,
	.heater_state = ACT_OFF,
	.fan_pwm = 0,
	.heater_pwm = 0,
	.mode = MODE_AUTO,
	.report_mode = REPORT_MODE_STREAM,
	.report_once_pending = 0
};

static uint8_t actuator_error_to_mid(errorCode code)
{
    switch (code)
    {
        case ACTUATOR_ERROR_INDEX: return MID_ACT_ERR_INDEX;
        case ACTUATOR_ERROR_MODE:  return MID_ACT_ERR_MODE;
        case ACTUATOR_ERROR_UNIT:  return MID_ACT_ERR_UNIT;
        case ACTUATOR_ERROR_VALUE: return MID_ACT_ERR_VALUE;
        default:                   return MID_ACT_ERR_VALUE; // fallback, không nên x?y ra
    }
}

static void actuator_push_error(errorCode code, uint8_t   mid, ActuatorDevice_t index)
{
    FrameUnion_t err_frame;
    err_frame.data.sof  = FRAME_SOF;
    err_frame.data.type = FRAME_TYPE_RSP;
    err_frame.data.id   = Frame_MakeId(GID_ERROR, mid);
    Frame_SetLength(&err_frame.data, 3);
    err_frame.data.payload[0] = (uint8_t)code;
    err_frame.data.payload[1] = (uint8_t)index;
    err_frame.data.payload[2] = mid;
 
    TX_Queue_Push(&err_frame);
}

#define ACT_ERROR(code, mid, index)                          \
    do {                                                      \
        actuator_push_error((code), (mid), (index));          \
    } while (0)

static SoftTimer_t s_sensor_timer;
static SoftTimer_t s_report_timer;

static volatile uint8_t s_sensor_due = 0;
static volatile uint8_t s_report_due = 0;

static void on_sensor_timer(void) {
	s_sensor_due = 1;
}

static void on_report_timer(void) {
	s_report_due = 1;
}

static PIDController_t heater_pid;

static float target_temperature = 40.0f; 

static SystemMode_t current_app_mode = MODE_AUTO; 

void ControlApp_Init(void) {
	SystickInit();
	ActuatorHAL_Init();
	Protocol_Init();
	
    
	heater_pid.Kp = 2.5f;
	heater_pid.Ki = 0.1f;
	heater_pid.Kd = 0.4f;

	heater_pid.T   = 0.1f;          
	heater_pid.tau = 0.05f;         

	heater_pid.limMin = 0.0f;
	heater_pid.limMax = 100.0f;      
	heater_pid.limMinInt = -40.0f;  
	heater_pid.limMaxInt = 40.0f;   

	PID_Init(&heater_pid);
	
	DHT22_ReadData();
	g_data.temperature = Temperature;
	ControlApp_Update(&g_data);
	
	SoftTimer_Start(&s_sensor_timer, 2000, on_sensor_timer);
	SoftTimer_Start(&s_report_timer, 2000, on_report_timer);
}

void ControlApp_SystickUpdate(void)
{
    SoftTimer_Update(&s_sensor_timer);
    SoftTimer_Update(&s_report_timer);
    Protocol_SystickUpdate();
}

void ControlApp_Run(void)
{
	SoftTimer_Dispatch(&s_sensor_timer);
	SoftTimer_Dispatch(&s_report_timer);

	Protocol_Run(&g_data);

	if (s_sensor_due)
	{
		s_sensor_due = 0;

		DHT22_ReadData();
		g_data.temperature = Temperature;

		if (g_data.mode == MODE_AUTO)
		{
				ControlApp_Update(&g_data);
		}
	}

	if (s_report_due)
	{
		s_report_due = 0;

		if (g_data.report_mode == REPORT_MODE_STREAM)
		{
				Protocol_SendStatus(&g_data);
		}
	}

	if (g_data.report_once_pending)
	{
		g_data.report_once_pending = 0;
		Protocol_SendStatus(&g_data);
	}
}



void ControlApp_Update(GreenhouseData_t *data) {
	
	errorCode error;
	
	if (data == 0) return;

	data->temp_setpoint = target_temperature;
	
	data->mode = current_app_mode;
	
	ActuatorPrams_struct_t heater_cmd;
	heater_cmd.index = ACTUATOR_HEATER; 
	heater_cmd.mode  = ACT_MODE_PWM;               
	heater_cmd.unit  = UNIT_PERCENT;

	ActuatorPrams_struct_t fan_cmd;
	fan_cmd.index = ACTUATOR_FAN;
	fan_cmd.mode  = ACT_MODE_ONOFF;    
	fan_cmd.unit  = UNIT_PERCENT;

	switch (data->mode) {
			case MODE_AUTO: {
					
				if (data->temperature < -40.0f || data->temperature > 80.0f) {
						data->heater_pwm = 0;
						data->heater_state = ACT_OFF;
						data->fan_pwm = 0;
						data->fan_state = ACT_OFF;
						
						heater_cmd.value = 0.0f;
						error = ActuatorHAL_Set(heater_cmd);
                if (error != ACTUATOR_OK) {
                    ACT_ERROR(error, actuator_error_to_mid(error), heater_cmd.index);
                    data->heater_pwm = 0;
                    data->heater_state = ACT_OFF;
									  data->fan_pwm = 0;
                    data->fan_state = ACT_OFF;
                    break;
                }
						
						fan_cmd.value = 0.0f;
						error = ActuatorHAL_Set(fan_cmd);
                if (error != ACTUATOR_OK) {
                    ACT_ERROR(error, actuator_error_to_mid(error), fan_cmd.index);
									  data->heater_pwm = 0;
                    data->heater_state = ACT_OFF;
                    data->fan_pwm = 0;
                    data->fan_state = ACT_OFF;
                    break;
                }
						break; 
				}

					
				 float pid_output = PID_Compute(&heater_pid, data->temp_setpoint, data->temperature);
					
					data->heater_pwm = (uint16_t)pid_output;
					data->heater_state = (data->heater_pwm > 0) ? ACT_ON : ACT_OFF;

					heater_cmd.value = (float)data->heater_pwm;
					error = ActuatorHAL_Set(heater_cmd);
            if (error != ACTUATOR_OK) {
                ACT_ERROR(error, actuator_error_to_mid(error), heater_cmd.index);
                data->heater_pwm = 0;
                data->heater_state = ACT_OFF;
							  data->fan_pwm = 0;
                data->fan_state = ACT_OFF;
                break;
            }

					if (data->temperature > (data->temp_setpoint + 5.0f)) {
							data->fan_pwm = 100; 
							data->fan_state = ACT_ON;
					} 
					else if (data->temperature < (data->temp_setpoint + 3.0f)) {
							data->fan_pwm = 0;
							data->fan_state = ACT_OFF;
					}
					
					fan_cmd.value = (float)data->fan_pwm;
					error = ActuatorHAL_Set(fan_cmd);
            if (error != ACTUATOR_OK) {
                ACT_ERROR(error, actuator_error_to_mid(error), fan_cmd.index);
							  data->heater_pwm = 0;
                data->heater_state = ACT_OFF;
                data->fan_pwm = 0;
                data->fan_state = ACT_OFF;
                break;
            }
					
					break;
			}

			case MODE_MANUAL:
					break;

			default:
					data->mode = MODE_MANUAL;
					current_app_mode = MODE_MANUAL;
					break;
	}

}

void ControlApp_SetMode(SystemMode_t mode) {
    
    current_app_mode = mode;
    
    PID_Init(&heater_pid);
}

void ControlApp_SetTempSetPoint(float temp) {
    target_temperature = temp;
}

void ControlApp_SetPIDTune(float p, float i, float d) {
    heater_pid.Kp = p;
    heater_pid.Ki = i;
    heater_pid.Kd = d;
}
