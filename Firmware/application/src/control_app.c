#include "../inc/control_app.h"
#include "../inc/pid_control.h"
#include "../../platform/inc/actuator_hal.h" 

static PIDController_t heater_pid;

static float target_temperature = 40.0f; 

static SystemMode_t current_app_mode = MODE_AUTO; 

void ControlApp_Init(void) {
    
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
}

void ControlApp_Update(GreenhouseData_t *data) {
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
                ActuatorHAL_Set(heater_cmd);
                
                fan_cmd.value = 0.0f;
                ActuatorHAL_Set(fan_cmd);
                break; 
            }

            
           float pid_output = PID_Compute(&heater_pid, data->temperature, data->temp_setpoint);
						
            data->heater_pwm = (uint16_t)pid_output;
            data->heater_state = (data->heater_pwm > 0) ? ACT_ON : ACT_OFF;

            heater_cmd.value = (float)data->heater_pwm;
            ActuatorHAL_Set(heater_cmd);

            if (data->temperature > (data->temp_setpoint + 5.0f)) {
                data->fan_pwm = 100; 
                data->fan_state = ACT_ON;
            } 
            else if (data->temperature < (data->temp_setpoint + 3.0f)) {
                data->fan_pwm = 0;
                data->fan_state = ACT_OFF;
            }
            
            fan_cmd.value = (float)data->fan_pwm;
            ActuatorHAL_Set(fan_cmd);
            
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

void ControlApp_SetTempSetpoint(float temp) {
    target_temperature = temp;
}

void ControlApp_SetPIDTune(float p, float i, float d) {
    heater_pid.Kp = p;
    heater_pid.Ki = i;
    heater_pid.Kd = d;
}
