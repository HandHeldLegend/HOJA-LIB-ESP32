#include "main.h"

void app_main()
{
    const char* TAG = "app_main";

    rb_err_t err;

    err = rb_api_init();
    err = rb_api_setCore(CORE_NINTENDOSWITCH);
    err = rb_api_startController();
}
