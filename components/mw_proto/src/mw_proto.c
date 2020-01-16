#include "mw_proto.h"


/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

void mw_set_roll( uint16_t value, mw_frame_t *frame )
{
    frame->data[5] = (uint8_t) value;
    frame->data[6] = (uint8_t) (value >> 8);
    mw_ser_crc(frame);
}

void mw_set_pitch( uint16_t value, mw_frame_t *frame )
{
    frame->data[7] = (uint8_t) value;
    frame->data[8] = (uint8_t) (value >> 8);
    mw_ser_crc(frame);
}

void mw_set_throttle( uint16_t value, mw_frame_t *frame )
{
    frame->data[9] = (uint8_t) value;
    frame->data[10] = (uint8_t) (value >> 8);
    mw_ser_crc(frame);
}

void mw_set_yaw( uint16_t value, mw_frame_t *frame )
{
    frame->data[11] = (uint8_t) value;
    frame->data[12] = (uint8_t) (value >> 8);
    mw_ser_crc(frame);
}

void mw_set_arm( uint16_t value, mw_frame_t *frame )
{
    frame->data[13] = (uint8_t) value;
    frame->data[14] = (uint8_t) (value >> 8);
    mw_ser_crc(frame);
}

void mw_ser_crc( mw_frame_t *frame )
{   
    uint8_t crc = 0;
    for(int i = 3; i < 15; i++){
        crc = crc ^ frame->data[i];
    }
    frame->data[15] = crc;
}

 mw_frame_t get_new_mw_frame( void )
 {
    mw_frame_t frame;

    //Preamble
    frame.data[0] = (uint8_t) '$';
    frame.data[1] = (uint8_t) 'M';
    frame.data[2] = (uint8_t) '<';
        
    //Size and Type
    frame.data[3] = 10U;     // length
    frame.data[4] = 200U;    // type

    //Payload init values
    mw_set_roll(1500, &frame);
    mw_set_pitch(1500, &frame);
    mw_set_throttle(1000, &frame);
    mw_set_yaw(1500, &frame);
    mw_set_arm(1000, &frame);

    frame.len = FRAME_LEN;
    
    return frame
 }