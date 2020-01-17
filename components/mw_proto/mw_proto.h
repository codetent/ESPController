#ifndef __MW_PROTO_H__
#define __MW_PROTO_H__

#include <stdint.h>
#include "esp_err.h"

#define MW_PROTO_FRAME_LEN 16

/* -------------------------------------------------------------------------- */
/*                                  TYPEDEFS                                  */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t data[MW_PROTO_FRAME_LEN];
    uint8_t len;
} mw_frame_t;


/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize frame structure of the given frame pointer
 * 
 * @param value Frame wich should be initialized
 * @return esp_err_t 
 */
esp_err_t init_mw_frame( mw_frame_t *frame);

/**
 * @brief Change roll bytes in provided frame to the provided value
 * 
 * @param value Roll value which should be included in the frame
 * @param frame Frame which should be modified
 * @return esp_err_t 
 */
esp_err_t mw_set_roll( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change pitch bytes in provided frame to the provided value
 * 
 * @param value Pitch value which should be included in the frame
 * @param frame Frame which should be modified
 * @return esp_err_t 
 */
esp_err_t mw_set_pitch( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change throttle bytes in provided frame to the provided value
 * 
 * @param value Throttle value which should be included in the frame
 * @param frame Frame which should be modified
 * @return esp_err_t 
 */
esp_err_t mw_set_throttle( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change yaw bytes in provided frame to the provided value
 * 
 * @param value Yaw value which should be included in the frame
 * @param frame Frame which should be modified
 * @return esp_err_t 
 */
esp_err_t mw_set_yaw( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change ARM bytes in provided frame to the provided value
 * 
 * @param value ARM value which should be included in the frame
 * @param frame Frame which should be modified 
 * @return esp_err_t 
 */
esp_err_t mw_set_arm( uint16_t value, mw_frame_t *frame );

/**
 * @brief Change ARM bytes in provided frame to the provided value
 * 
 * @param frame Toggle arm bytes depending on current state
 * @return esp_err_t 
 */
esp_err_t mw_toggle_arm( mw_frame_t *frame );

/**
 * @brief Sets the crc of the frame
 * 
 * @param frame Structure which holds the frame information
 * @return esp_err_t 
 */
esp_err_t mw_set_crc( mw_frame_t *frame );


#endif // __MW_PROTO_H__
