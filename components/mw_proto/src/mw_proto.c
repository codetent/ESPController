#include "mw_proto.h"


/* -------------------------------------------------------------------------- */
/*                                   DEFINES                                  */
/* -------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------- */
/*                        FUNCTIONS (EXTERNAL LINKAGE)                        */
/* -------------------------------------------------------------------------- */

esp_err_t mw_set_roll( uint16_t value, mw_frame_t *frame )
{   
    esp_err_t status = ESP_FAIL;

    //Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
       if(value < MW_MIN_VALUE){
            value = MW_MIN_VALUE;
        }else if(value > MW_MAX_VALUE){
            value = MW_MAX_VALUE;
        }

        frame->data[5] = (uint8_t) value;
        frame->data[6] = (uint8_t) (value >> 8U);
        mw_set_crc(frame);
        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_set_pitch( uint16_t value, mw_frame_t *frame )
{
    esp_err_t status = ESP_FAIL;

    //Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
       if(value < MW_MIN_VALUE){
            value = MW_MIN_VALUE;
        }else if(value > MW_MAX_VALUE){
            value = MW_MAX_VALUE;
        }

        frame->data[7] = (uint8_t) value;
        frame->data[8] = (uint8_t) (value >> 8U);
        mw_set_crc(frame);
        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_set_throttle( uint16_t value, mw_frame_t *frame )
{
    esp_err_t status = ESP_FAIL;
    
    //Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
       if(value < MW_MIN_VALUE){
            value = MW_MIN_VALUE;
        }else if(value > MW_MAX_VALUE){
            value = MW_MAX_VALUE;
        }

        frame->data[9] = (uint8_t) value;
        frame->data[10] = (uint8_t) (value >> 8U);
        mw_set_crc(frame);
        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_set_yaw( uint16_t value, mw_frame_t *frame )
{
    esp_err_t status = ESP_FAIL;
    
    //Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
       if(value < MW_MIN_VALUE){
            value = MW_MIN_VALUE;
        }else if(value > MW_MAX_VALUE){
            value = MW_MAX_VALUE;
        }

        frame->data[11] = (uint8_t) value;
        frame->data[12] = (uint8_t) (value >> 8U);
        mw_set_crc(frame);
        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_set_arm( uint16_t value, mw_frame_t *frame )
{
    esp_err_t status = ESP_FAIL;
    
    //Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
       if(value < MW_MIN_VALUE){
            value = MW_MIN_VALUE;
        }else if(value > MW_MAX_VALUE){
            value = MW_MAX_VALUE;
        }

        frame->data[13] = (uint8_t) value;
        frame->data[14] = (uint8_t) (value >> 8U);
        mw_set_crc(frame);
        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_set_crc( mw_frame_t *frame )
{   
    esp_err_t status = ESP_FAIL;

    // Check input
    if(frame == NULL){
        status = ESP_ERR_INVALID_ARG;
    }else{
        // Set bytes
        uint8_t crc = 0;
        for(int i = 3; i < 15; i++){
            crc = crc ^ frame->data[i];
        }
        frame->data[15] = crc;

        status = ESP_OK;
    }
    
    return status;
}

esp_err_t mw_toggle_arm( mw_frame_t *frame )
{   
    esp_err_t status = ESP_FAIL;

    // Check input
    if(frame == NULL){
        return ESP_ERR_INVALID_ARG;
    }else{
        // If disarmed -> arm
        if( (frame->data[13] == (uint8_t) MW_MIN_VALUE) && 
            (frame->data[14] == (uint8_t) (MW_MIN_VALUE >> 8U)) ){
            
            mw_set_arm(MW_MAX_VALUE, frame);

        }else{ // Safe state -> disarm
            mw_set_arm(MW_MIN_VALUE, frame);
        }
        mw_set_crc(frame);
        status = ESP_OK;
    }

    return status;
}

 esp_err_t init_mw_frame( mw_frame_t *frame )
 {
     esp_err_t status = ESP_FAIL;

    //Check input
    if(frame == NULL){
        return ESP_ERR_INVALID_ARG;
    }else{
        //Preamble
        frame->data[0] = (uint8_t) '$';
        frame->data[1] = (uint8_t) 'M';
        frame->data[2] = (uint8_t) '<';
            
        //Size and Type
        frame->data[3] = MW_MSP_SET_RAW_RC_LEN;     // length
        frame->data[4] = MW_MSP_SET_RAW_RC_TYPE;    // type

        //Payload init values
        mw_set_roll(MW_MID_VALUE, frame);
        mw_set_pitch(MW_MID_VALUE, frame);
        mw_set_yaw(MW_MID_VALUE, frame);
        mw_set_throttle(MW_MIN_VALUE, frame);
        mw_set_arm(MW_MIN_VALUE, frame);

        frame->len = MW_PROTO_FRAME_LEN;
        status = ESP_OK;
    }

    return status;
 }