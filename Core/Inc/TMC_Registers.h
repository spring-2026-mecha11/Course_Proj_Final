/**
 * @file TMC_Registers.h
 * @brief TMC5240 register address constants used by the motor driver.
 *
 * @defgroup TMCRegisters TMC5240 register map
 * @brief Register addresses used by the project-specific TMC5240 driver.
 * @{
 */

#ifndef INC_TMC_REGISTERS_H_
#define INC_TMC_REGISTERS_H_

/** General configuration register. */
#define TMC5240_GCONF        0x00
/** Global status and reset flag register. */
#define TMC5240_GSTAT        0x01
/** SPI interface transfer counter. */
#define TMC5240_IFCNT        0x02
/** Node configuration register, unused by normal motion control. */
#define TMC5240_NODECONF     0x03
/** Driver input-status register. */
#define TMC5240_IOIN         0x04
/** Comparator position register. */
#define TMC5240_X_COMP       0x05

/** Hold/run current and hold-delay register. */
#define TMC5240_IHOLD_IRUN   0x10
/** Standstill power-down delay register. */
#define TMC5240_TPOWERDOWN   0x11
/** Measured step-time register. */
#define TMC5240_TSTEP        0x12
/** StealthChop threshold register. */
#define TMC5240_TPWMTHRS     0x13
/** CoolStep threshold register. */
#define TMC5240_TCOOLTHRS    0x14
/** High-velocity threshold register. */
#define TMC5240_THIGH        0x15

/** Ramp mode selector for position or velocity control. */
#define TMC5240_RAMPMODE     0x20
/** Actual motor position counter. */
#define TMC5240_XACTUAL      0x21
/** Actual motor velocity register. */
#define TMC5240_VACTUAL      0x22
/** Ramp generator start velocity. */
#define TMC5240_VSTART       0x23
/** First acceleration segment. */
#define TMC5240_A1           0x24
/** Velocity threshold between acceleration segments. */
#define TMC5240_V1           0x25
/** Maximum acceleration. */
#define TMC5240_AMAX         0x26
/** Maximum velocity. */
#define TMC5240_VMAX         0x27
/** Maximum deceleration. */
#define TMC5240_DMAX         0x28
/** Optional time at maximum velocity. */
#define TMC5240_TVMAX        0x29
/** Final deceleration segment. */
#define TMC5240_D1           0x2A
/** Stop velocity. */
#define TMC5240_VSTOP        0x2B
/** Delay after reaching zero velocity. */
#define TMC5240_TZEROWAIT    0x2C
/** Target motor position used for slide commands. */
#define TMC5240_XTARGET      0x2D
/** Second velocity threshold, unused in this project. */
#define TMC5240_V2           0x2E
/** Second acceleration segment, unused in this project. */
#define TMC5240_A2           0x2F
/** Second deceleration segment, unused in this project. */
#define TMC5240_D2           0x30

/** Minimum DC velocity register. */
#define TMC5240_VDCMIN       0x33
/** Limit-switch behavior configuration. */
#define TMC5240_SWMODE       0x34
/** Ramp and limit-switch status register. */
#define TMC5240_RAMP_STAT    0x35
/** Latched position at switch event. */
#define TMC5240_XLATCH       0x36

/** Chopper configuration register. */
#define TMC5240_CHOPCONF     0x6C
/** CoolStep current-control configuration. */
#define TMC5240_COOLCONF     0x6D
/** DCStep control configuration. */
#define TMC5240_DCCTRL       0x6E
/** Driver status and diagnostic register. */
#define TMC5240_DRV_STATUS   0x6F
/** PWM mode configuration register. */
#define TMC5240_PWMCONF      0x70
/** PWM amplitude scaling status. */
#define TMC5240_PWM_SCALE    0x71
/** Automatic PWM tuning status. */
#define TMC5240_PWM_AUTO     0x72
/** Driver configuration register. */
#define TMC5240_DRV_CONF     0x0A
/** Global motor-current scale register. */
#define TMC5240_GLOBALSCALER 0x0B


/** @} */
#endif /* INC_TMC_REGISTERS_H_ */
