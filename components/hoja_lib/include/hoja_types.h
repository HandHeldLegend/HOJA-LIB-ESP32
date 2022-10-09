#ifndef HOJA_TYPES_H
#define HOJA_TYPES_H

/**
 * Error handling
 */
typedef enum
{
    HOJA_OK     = 0,
    HOJA_FAIL   = 1,
    HOJA_USB_NODET  = 2, // USB Core not detected on attempt.
} hoja_err_t;



#endif