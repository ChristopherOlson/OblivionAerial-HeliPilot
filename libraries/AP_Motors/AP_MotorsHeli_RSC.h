#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>            // ArduPilot Mega Vector/Matrix math Library
#include <RC_Channel/RC_Channel.h>
#include <SRV_Channel/SRV_Channel.h>

// rotor controller states
enum RotorControlState {
    ROTOR_CONTROL_STOP = 0,
    ROTOR_CONTROL_IDLE,
    ROTOR_CONTROL_ACTIVE
};

// rotor control modes
enum RotorControlMode {
    ROTOR_CONTROL_MODE_DISABLED = 0,
    ROTOR_CONTROL_MODE_THROTTLE_CURVE,
    ROTOR_CONTROL_MODE_GOVERNOR
};

class AP_MotorsHeli_RSC {
public:
    friend class AP_MotorsHeli_Single;

    AP_MotorsHeli_RSC(SRV_Channel::Aux_servo_function_t aux_fn,
                      uint8_t default_channel) :
        _aux_fn(aux_fn),
        _default_channel(default_channel)
    {};

    // init_servo - servo initialization on start-up
    void        init_servo();

    // set_control_mode - sets control mode
    void        set_control_mode(RotorControlMode mode) { _control_mode = mode; }

    // set_critical_speed
    void        set_critical_speed(float critical_speed) { _critical_speed = critical_speed; }

    // get_critical_speed
    float       get_critical_speed() const { return _critical_speed; }

    // set_idle_output
    float       get_idle_output() { return _idle_output; }
    void        set_idle_output(float idle_output) { _idle_output = idle_output; }

    // set rotor speed governor parameters
    void        set_governor_disengage(float governor_disengage) {_governor_disengage = governor_disengage; }
    void        set_governor_droop_response(float governor_droop_response) { _governor_droop_response = governor_droop_response; }
    void        set_governor_output(float governor_output) {_governor_output = governor_output; }
    void        set_governor_reference(float governor_reference) { _governor_reference = governor_reference; }
    void        set_governor_range(float governor_range) { _governor_range = governor_range; }
    void        set_governor_tcgain(float governor_tcgain) {_governor_tcgain = governor_tcgain; }

    // get_desired_speed
    float       get_desired_speed() const { return _desired_speed; }

    // set_desired_speed
    void        set_desired_speed(float desired_speed) { _desired_speed = desired_speed; }

    // get_control_speed
    float       get_control_output() const { return _control_output; }

    // get_rotor_speed - estimated rotor speed when no governor or rpm sensor is used
    float       get_rotor_speed() const;
    
    // set_rotor_rpm - when speed sensor is available for governor
    void        set_rotor_rpm(float rotor_rpm) {_rotor_rpm = (float)rotor_rpm; }
    
    // get_governor_output
    float       get_governor_output() const { return _governor_output; }

    // is_runup_complete
    bool        is_runup_complete() const { return _runup_complete; }

    // set_ramp_time
    void        set_ramp_time(int8_t ramp_time) { _ramp_time = ramp_time; }

    // set_runup_time
    void        set_runup_time(int8_t runup_time) { _runup_time = runup_time; }

    // set_throttle_curve
    void        set_throttle_curve(float thrcrv[5], uint16_t slewrate);

    // set_collective. collective for throttle curve calculation
    void        set_collective(float collective) { _collective_in = collective; }

    // output - update value to send to ESC/Servo
    void        output(RotorControlState state);

private:
    uint64_t        _last_update_us;

    // channel setup for aux function
    SRV_Channel::Aux_servo_function_t _aux_fn;
    uint8_t         _default_channel;

    // internal variables
    RotorControlMode _control_mode = ROTOR_CONTROL_MODE_DISABLED;   // throttle control mode
    float           _critical_speed;              // rotor speed below which flight is not possible
    float           _idle_output;                 // motor output idle speed
    float           _desired_speed;               // latest desired rotor speed from pilot
    float           _control_output;              // latest logic controlled output
    float           _rotor_ramp_output;           // scalar used to ramp rotor speed between _rsc_idle_output and full speed (0.0-1.0f)
    float           _rotor_runup_output;          // scalar used to store status of rotor run-up time (0.0-1.0f)
    bool            _rpm_sensor;                  // flag to determine if rpm sensor is used
    int8_t          _ramp_time;                   // time in seconds for the output to the main rotor's ESC to reach full speed
    int8_t          _runup_time;                  // time in seconds for the main rotor to reach full speed.  Must be longer than _rsc_ramp_time
    bool            _runup_complete;              // flag for determining if runup is complete
    float           _thrcrv_poly[4][4];           // spline polynomials for throttle curve interpolation
    uint16_t        _power_slewrate;              // slewrate for throttle (percentage per second)
    float           _collective_in;               // collective in for throttle curve calculation, range 0-1.0f
    float           _rotor_rpm;                   // rotor rpm from speed sensor for governor
    float           _governor_disengage;          // throttle percentage where governor disenages to allow return to flight idle
    float           _governor_output;             // governor output for rotor speed control
    float           _governor_range;              // RPM range +/- governor rpm reference setting where governor is operational
    float           _governor_reference;          // sets rotor speed for governor
    float           _governor_droop_response;     // governor response to droop under load
    bool            _governor_engage;             // RSC governor status flag for soft-start
    float           _governor_tcgain;             // governor throttle curve gain, range 50-100%

    // update_rotor_ramp - slews rotor output scalar between 0 and 1, outputs float scalar to _rotor_ramp_output
    void            update_rotor_ramp(float rotor_ramp_input, float dt);

    // update_rotor_runup - function to slew rotor runup scalar, outputs float scalar to _rotor_runup_ouptut
    void            update_rotor_runup(float dt);

    // write_rsc - outputs pwm onto output rsc channel. servo_out parameter is of the range 0 ~ 1
    void            write_rsc(float servo_out);

    // calculate_desired_throttle - uses throttle curve and collective input to determine throttle setting
    float           calculate_desired_throttle(float collective_in);
};
