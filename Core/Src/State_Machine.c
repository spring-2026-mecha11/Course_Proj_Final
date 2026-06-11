/*
 * State_Machine.c
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */


#include "State_Machine.h"

#include "Mode_Select.h"
#include "System_Config.h"
#include "Stepper_Motion.h"
#include "pressure_system.h"
#include "servo_system.h"

static SystemState_t system_state = SYS_STATE_BOOT;
static SystemFlags_t system_flags;


/*
 * Debug/watch variables.
*/
volatile uint8_t debug_system_state = 0;
volatile uint8_t debug_active_mode = 0;
volatile uint8_t debug_stepper_homed = 0;
volatile uint8_t debug_fault_active = 0;
volatile uint8_t debug_error_code = 0;

volatile uint8_t debug_stepper_cal_enable = 0;
volatile uint8_t debug_stepper_cal_apply = 0;
volatile uint8_t debug_stepper_cal_stop = 0;

volatile int32_t debug_stepper_cal_target_steps = 0;
volatile int32_t debug_stepper_cal_last_commanded_steps = 0;
volatile int32_t debug_stepper_cal_actual_steps = 0;

volatile uint8_t debug_stepper_cal_status = 0;



/*
 * Internal helper declarations.
 */
static void State_Machine_ClearFlags(void);

static void Hardware_Init_Request(void);
static bool Stepper_Home_Request(void);
static void Stepper_Stop_Request(void);


static void Servo_SetMuted_Request(bool muted);
static void Fan_Enable_Request(bool enable);

static void Live_Harmonizer_Enable_Request(bool enable);
static void Song_Player_Enable_Request(bool enable);
static void Song_Player_Start_Request(void);
static bool Song_Player_IsDone_Request(void);

static void Stepper_DebugCal_Task(void);

//External CUBEMX Handles


void State_Machine_Init(void)
{
    system_state = SYS_STATE_BOOT;

    State_Machine_ClearFlags();

    debug_system_state = (uint8_t)system_state;
    debug_active_mode = (uint8_t)APP_MODE_NONE;
    debug_stepper_homed = 0;
    debug_fault_active = 0;
    debug_error_code = (uint8_t)SYS_ERROR_NONE;
}

void State_Machine_Task(uint32_t now_ms)
{
    (void)now_ms;

    debug_system_state = (uint8_t)system_state;
    debug_active_mode = (uint8_t)system_flags.active_mode;
    debug_stepper_homed = system_flags.stepper_homed ? 1u : 0u;
    debug_fault_active = system_flags.fault_active ? 1u : 0u;
    debug_error_code = (uint8_t)system_flags.error_code;

    switch (system_state)
    {
        case SYS_STATE_BOOT:
        {
            system_state = SYS_STATE_INIT_HARDWARE;
            break;
        }

        case SYS_STATE_INIT_HARDWARE:
        {
            /*
             * Startup behavior:
             *
             * 1. Keep the servo muted.
             * 2. Keep the fan pressure control OFF.
             * 3. Let PressureSystem_Update() finish settling/calibration.
             * 4. Only after pressure is ready, initialize stepper software.
             * 5. Then move to stepper homing.
             */

            Servo_SetMuted_Request(true);
            Fan_Enable_Request(false);

            Live_Harmonizer_Enable_Request(false);
            Song_Player_Enable_Request(false);

            system_flags.active_mode = APP_MODE_NONE;

            if (!PressureSystem_IsReady())
            {
                /*
                 * Stay here while pressure system does:
                 * PRESSURE_STATE_SETTLING
                 * PRESSURE_STATE_CALIBRATING
                 *
                 * PressureSystem_Update() is called from App_Task().
                 */
                break;
            }

            if (!system_flags.hardware_initialized)
            {
                Hardware_Init_Request();
                system_flags.hardware_initialized = true;
            }

            system_state = SYS_STATE_HOME_STEPPER;
            break;
        }

        case SYS_STATE_HOME_STEPPER:
        {
            bool home_ok = Stepper_Home_Request();

            if (home_ok)
            {
                system_flags.stepper_homed = true;
                system_state = SYS_STATE_MODE_SELECT;
            }
            else
            {
                system_flags.fault_active = true;
                system_flags.error_code = SYS_ERROR_STEPPER_HOME_FAILED;
                system_state = SYS_STATE_ERROR;
            }

            break;
        }

        case SYS_STATE_MODE_SELECT:
        {
            /*
             * Mode select behavior:
             *
             * Servo stays muted.
             * Fan stays OFF.
             * Live/song modules are disabled.
             * No one commands the stepper.
             *
             * Fan pressure control starts only after a valid mode is selected.
             */

            Servo_SetMuted_Request(true);
            Fan_Enable_Request(false);

            Live_Harmonizer_Enable_Request(false);
            Song_Player_Enable_Request(false);

            system_flags.active_mode = APP_MODE_NONE;

            if (Mode_Select_HasSelection())
            {
                AppMode_t requested_mode = Mode_Select_GetSelectedMode();

                system_flags.mode_selected = true;
                system_flags.requested_mode = requested_mode;

                if (requested_mode == APP_MODE_LIVE_HARMONIZER)
                {
                    system_flags.active_mode = APP_MODE_LIVE_HARMONIZER;
                    system_state = SYS_STATE_LIVE_HARMONIZER;
                }
                else if (requested_mode == APP_MODE_SONG_PLAYER)
                {
                    system_flags.active_mode = APP_MODE_SONG_PLAYER;

                    Song_Player_Start_Request();

                    system_state = SYS_STATE_SONG_PLAYER;
                }
                else
                {
                    system_flags.fault_active = true;
                    system_flags.error_code = SYS_ERROR_INVALID_MODE;
                    system_state = SYS_STATE_ERROR;
                }
            }

            break;
        }

        case SYS_STATE_LIVE_HARMONIZER:
        {
            /*
             * Live harmonizer mode:
             *
             * Fan stays running.
             * Audio keeps listening.
             * Live harmonizer logic controls:
             *      - servo mute/open behavior
             *      - stepper percent target
             *
             * No separate idle-muted top-level state is needed.
             */
            Fan_Enable_Request(true);
            Servo_SetMuted_Request(true);

            Live_Harmonizer_Enable_Request(true);
            Song_Player_Enable_Request(false);

            system_flags.active_mode = APP_MODE_LIVE_HARMONIZER;

            Stepper_DebugCal_Test();

            /*
             * Future:
             *      - check for long button press to return to mode select
             *      - check for hardware faults
             */

            break;
        }

        case SYS_STATE_SONG_PLAYER:
        {
            /*
             * Song player mode:
             *
             * Fan stays running.
             * Song player logic controls:
             *      - servo for notes/rests
             *      - stepper percent target
             */
            Fan_Enable_Request(true);
            Servo_SetMuted_Request(true);

            Live_Harmonizer_Enable_Request(false);
            Song_Player_Enable_Request(true);

            system_flags.active_mode = APP_MODE_SONG_PLAYER;

            if (Song_Player_IsDone_Request())
            {
                Song_Player_Enable_Request(false);
                Mode_Select_Reset();

                system_flags.song_done = true;
                system_flags.active_mode = APP_MODE_NONE;

                system_state = SYS_STATE_MODE_SELECT;
            }

            break;
        }

        case SYS_STATE_ERROR:
        default:
        {
            /*
             * Safe fault behavior.
             */
            Live_Harmonizer_Enable_Request(false);
            Song_Player_Enable_Request(false);

            Servo_SetMuted_Request(true);
            Stepper_Stop_Request();

            /*
             * In error, turn fan off for now.
             * You can change this later if safe idle airflow is preferred.
             */
            Fan_Enable_Request(false);

            system_flags.active_mode = APP_MODE_NONE;
            system_flags.fault_active = true;

            break;
        }
    }
}

SystemState_t State_Machine_GetState(void)
{
    return system_state;
}

SystemFlags_t *State_Machine_GetFlags(void)
{
    return &system_flags;
}

static void State_Machine_ClearFlags(void)
{
    system_flags.hardware_initialized = false;
    system_flags.stepper_homed = false;

    system_flags.mode_selected = false;
    system_flags.requested_mode = APP_MODE_NONE;
    system_flags.active_mode = APP_MODE_NONE;

    system_flags.live_harmonizer_enabled = false;
    system_flags.song_player_enabled = false;

    system_flags.fan_enabled = false;
    system_flags.fan_pressure_stable = false;

    system_flags.mute_closed = true;

    system_flags.audio_data_valid = false;
    system_flags.user_audio_active = false;
    system_flags.pitch_error_valid = false;

    system_flags.usb_song_loaded = false;
    system_flags.song_playing = false;
    system_flags.song_done = false;

    system_flags.fault_active = false;
    system_flags.error_code = SYS_ERROR_NONE;
}

/*
 * Hardware request functions.
 *
 * These are intentionally blank or fake for now.
 * Add real driver calls later.
 */

static void Hardware_Init_Request(void)
{

	Stepper_Init();
	/*
	 * Software modules are initialized in App_Init().
	 * This state is for startup commands/safe outputs if needed.
	 */
}

static bool Stepper_Home_Request(void)
{
    StepperStatus_t status = Stepper_Home();

    if (status == STEPPER_OK)
    {
        return true;
    }

    return false;
}

static void Stepper_Stop_Request(void)
{
    Stepper_Stop();
}


static void Servo_SetMuted_Request(bool muted)
{
	if (muted)
	    {
	        ServoSystem_Down();
	    }
	    else
	    {
	        ServoSystem_Up();
	    }

    system_flags.mute_closed = muted;
}

static void Fan_Enable_Request(bool enable)
{
    if (enable)
    {
        if (!system_flags.fan_enabled)
        {
            float target_kpa = LIVE_PRESSURE_TARGET_KPA;

            if (system_flags.active_mode == APP_MODE_SONG_PLAYER)
            {
                target_kpa = SONG_PRESSURE_TARGET_KPA;
            }

            PressureSystem_SetTarget(target_kpa);
        }

        system_flags.fan_enabled = true;
        system_flags.fan_pressure_stable = PressureSystem_IsStable();
    }
    else
    {
        if (system_flags.fan_enabled)
        {
            PressureSystem_Stop();
        }

        system_flags.fan_enabled = false;
        system_flags.fan_pressure_stable = false;
    }
}

static void Live_Harmonizer_Enable_Request(bool enable)
{
    /*
     * Future real code:
     *
     * Live_Harmonizer_Enable(enable);
     */

    system_flags.live_harmonizer_enabled = enable;
}

static void Song_Player_Enable_Request(bool enable)
{
    /*
     * Future real code:
     *
     * Song_Player_Enable(enable);
     */

    system_flags.song_player_enabled = enable;

    if (!enable)
    {
        system_flags.song_playing = false;
    }
}

static void Song_Player_Start_Request(void)
{
    /*
     * Future real code:
     *
     * Song_Player_Start();
     */

    system_flags.song_playing = true;
    system_flags.song_done = false;
}

static bool Song_Player_IsDone_Request(void)
{
    /*
     * Future real code:
     *
     * return Song_Player_IsDone();
     */

    /*
     * Temporary:
     * Always return false until the song player exists.
     *
     * When you want to test returning from SONG_PLAYER to MODE_SELECT,
     * temporarily change this to return true.
     */
    return false;
}



static void Stepper_DebugCal_Task(void)
{
    debug_stepper_cal_actual_steps = Stepper_GetPositionSteps();

    if (!debug_stepper_cal_enable)
    {
        return;
    }

    if (debug_stepper_cal_stop)
    {
        Stepper_Stop();

        debug_stepper_cal_apply = 0;
        debug_stepper_cal_stop = 0;
        debug_stepper_cal_status = 100; // stopped by debug request

        return;
    }

    if (debug_stepper_cal_apply)
    {
        StepperStatus_t status;

        status = Stepper_MoveToAbsSteps(debug_stepper_cal_target_steps);

        debug_stepper_cal_last_commanded_steps = debug_stepper_cal_target_steps;
        debug_stepper_cal_status = (uint8_t)status;

        debug_stepper_cal_apply = 0;
    }
}
