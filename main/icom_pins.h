/* 

    SIP Intercom system

*/

#ifndef _ICOM_PINS_H_
#define _ICOM_PINS_H_


/**
 * @brief LED pins
 */

#define ICOM_LED_WIFI   GPIO_NUM_2
#define ICOM_LED_VOIP   GPIO_NUM_22
#define ICOM_LED_CALL   GPIO_NUM_19
#define ICOM_LED_ANSW   GPIO_NUM_4
#define ICOM_LED_OPEN   GPIO_NUM_15


/**
 * @brief IO pins
 */

#define ICOM_IN_CALL    GPIO_NUM_33
#define ICOM_OUT_ANSW   GPIO_NUM_21
#define ICOM_OUT_OPEN   GPIO_NUM_12


/**
 * @brief Button pins
 */


/**
 * @brief Button Function Definition
 */

#define FUNC_BUTTON_EN  (1)
#define INPUT_KEY_NUM   2
#define BTN_REC_ID      GPIO_NUM_36
#define BTN_MODE_ID     GPIO_NUM_39

#define INPUT_KEY_DEFAULT_INFO() {              \
    {                                           \
        .type = PERIPH_ID_BUTTON,               \
        .user_id = INPUT_KEY_USER_ID_REC,       \
        .act_id = BTN_REC_ID,                   \
    },                                          \
    {                                           \
        .type = PERIPH_ID_BUTTON,               \
        .user_id = INPUT_KEY_USER_ID_MODE,      \
        .act_id = BTN_MODE_ID,                  \
    }                                           \
}



#endif  //  _ICOM_PINS_H_