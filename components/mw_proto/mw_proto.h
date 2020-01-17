#ifndef __MW_PROTO_H__
#define __MW_PROTO_H__

#include <stdint.h>

#define FRAME_LEN 16

/* -------------------------------------------------------------------------- */
/*                                  TYPEDEFS                                  */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t data[FRAME_LEN];
    uint8_t len;
} mw_frame_t;


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize and return a starting frame
 * 
 * @return  Return initial frame
 */
mw_frame_t get_new_mw_frame( void );

/**
 * @brief Change roll bytes in provided frame to the provided value
 * 
 * @param value Roll value which should be included in the frame
 * @param frame Frame which should be modified
 */
void mw_set_roll( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change pitch bytes in provided frame to the provided value
 * 
 * @param value Pitch value which should be included in the frame
 * @param frame Frame which should be modified
 */
void mw_set_pitch( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change throttle bytes in provided frame to the provided value
 * 
 * @param value Throttle value which should be included in the frame
 * @param frame Frame which should be modified
 */
void mw_set_throttle( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change yaw bytes in provided frame to the provided value
 * 
 * @param value Yaw value which should be included in the frame
 * @param frame Frame which should be modified
 */
void mw_set_yaw( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change ARM bytes in provided frame to the provided value
 * 
 * @param value ARM value which should be included in the frame
 * @param frame Frame which should be modified 
 */
void mw_set_arm( uint16_t value, mw_frame_t *frame );

/**
 * @brief Initialize the peripherals to which the joystick is connected
 * 
 * @param joystick Structure which holds the joystick information
 */
void mw_ser_crc( mw_frame_t *frame );


#endif // __MW_PROTO_H__
