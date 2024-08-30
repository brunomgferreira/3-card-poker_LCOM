#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define KBD_IRQ 1
#define KBD_HOOK_BIT 1

/* I/O port addresses */

#define KBC_STAT_REG 0x64
#define KBC_CTRL_REG 0x64
#define KBC_OUT_BUF 0x60
#define KBC_IN_BUF 0x60

/* KBC control */

#define PARITY_ERR BIT(7)
#define TIMEOUT_ERR BIT(6)
#define AUX BIT(5)
#define INH BIT(4)
#define A2 BIT(3)
#define SYS BIT(2)
#define IBF BIT(1)
#define OBF BIT(0)

/* KBC commands */

#define READ_COMMAND 0x20
#define WRITE_COMMAND 0x60
#define CHECK_KBC 0xAA
#define CHECK_KEYBOARD_INTERFACE 0xAB
#define DISABLE_KBD_INTERFACE 0xAD
#define ENABLE_KBD_INTERFACE 0xAE

/* KBC command byte */

#define DIS2 BIT(5) /**< @brief Disable mouse interface */
#define DIS BIT(4)  /**< @brief Disable keyboard interface */
#define INT2 BIT(1) /**< @brief Enable interrupt on OBF, from mouse */
#define INT BIT(0)  /**< @brief Enable interrupt on OBF, from keyboard */

/* Useful constants */

#define DELAY 20000 /**< @brief 20ms delay for KBD commands */
#define TRIES 10     /**< @brief Number of tries to read/write */

#define ESC_BREAK 0x81 /**< @brief Break code for the Escape key */
#define TWO_BYTES 0xE0

/**@}*/

#endif /* _LCOM_I8042_H */
