/*
 * TMC_Registers.h
 *
 *  Created on: Jun 9, 2026
 *      Author: dreed
 */

#ifndef INC_TMC_REGISTERS_H_
#define INC_TMC_REGISTERS_H_

#define TMC5240_GCONF        0x00
#define TMC5240_GSTAT        0x01
#define TMC5240_IFCNT        0x02
#define TMC5240_NODECONF     0x03
#define TMC5240_IOIN         0x04
#define TMC5240_X_COMP       0x05

#define TMC5240_IHOLD_IRUN   0x10
#define TMC5240_TPOWERDOWN   0x11
#define TMC5240_TSTEP        0x12
#define TMC5240_TPWMTHRS     0x13
#define TMC5240_TCOOLTHRS    0x14
#define TMC5240_THIGH        0x15

#define TMC5240_RAMPMODE     0x20
#define TMC5240_XACTUAL      0x21
#define TMC5240_VACTUAL      0x22
#define TMC5240_VSTART       0x23
#define TMC5240_A1           0x24
#define TMC5240_V1           0x25
#define TMC5240_AMAX         0x26
#define TMC5240_VMAX         0x27
#define TMC5240_DMAX         0x28
#define TMC5240_TVMAX        0x29
#define TMC5240_D1           0x2A
#define TMC5240_VSTOP        0x2B
#define TMC5240_TZEROWAIT    0x2C
#define TMC5240_XTARGET      0x2D
#define TMC5240_V2           0x2E
#define TMC5240_A2           0x2F
#define TMC5240_D2           0x30

#define TMC5240_VDCMIN       0x33
#define TMC5240_SWMODE       0x34
#define TMC5240_RAMP_STAT    0x35
#define TMC5240_XLATCH       0x36

#define TMC5240_CHOPCONF     0x6C
#define TMC5240_COOLCONF     0x6D
#define TMC5240_DCCTRL       0x6E
#define TMC5240_DRV_STATUS   0x6F
#define TMC5240_PWMCONF      0x70
#define TMC5240_PWM_SCALE    0x71
#define TMC5240_PWM_AUTO     0x72
#define TMC5240_DRV_CONF     0x0A
#define TMC5240_GLOBALSCALER 0x0B


#endif /* INC_TMC_REGISTERS_H_ */
