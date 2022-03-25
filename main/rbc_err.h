#ifndef RBC_ERR_H
#define RBC_ERR_H
/**
 * Error handling
 */
typedef enum
{
    RB_OK               = 0,
    RB_ERR_FAILURE      = 1,
    RB_ERR_UNSUPPORTED  = 2,
    RB_ERR_BADWRITE     = 3,
    RB_ERR_OVERFLOW     = 4,
    RB_ERR_TOOSHORT     = 5,
    RB_ERR_RUNNING      = 6,
    RB_ERR_UNSPECIFIEDCORE = 7
} rb_err_t;



#endif