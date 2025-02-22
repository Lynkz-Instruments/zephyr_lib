/**
 * @file lcz_led.c
 * @brief LED control
 *
 * Copyright (c) 2020-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(lcz_led, CONFIG_LCZ_LED_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <drivers/gpio.h>
#include <kernel.h>

#include "lcz_led.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define TAKE_MUTEX(m) k_mutex_lock(&m, K_FOREVER)
#define GIVE_MUTEX(m) k_mutex_unlock(&m)

#define MINIMUM_ON_TIME_MSEC 1
#define MINIMUM_OFF_TIME_MSEC 1

enum led_state {
	ON = true,
	OFF = false,
};

enum led_blink_state {
	BLINK = true,
	DONT_BLINK = false,
};

struct led {
	bool initialized;
	led_index_t index;
	enum led_state state;
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
	void (*on)(void);
	void (*off)(void);
#else
	const struct device *device_handle;
	uint32_t pin;
	gpio_flags_t flags;
#endif
	bool pattern_busy;
	struct lcz_led_blink_pattern pattern;
	struct k_timer timer;
#ifndef CONFIG_MCUBOOT
	struct k_work work;
#endif
#ifdef CONFIG_LCZ_LED_DRIVER_ATOMIC
	atomic_t locked;
#endif
	void (*pattern_complete_function)(void);
};

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#ifdef CONFIG_LCZ_LED_DRIVER_MUTEX
static struct k_mutex led_mutex;
#endif

static struct led led[CONFIG_LCZ_NUMBER_OF_LEDS];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void led_timer_callback(struct k_timer *timer_id);
static void turn_on(struct led *pLed);
static void turn_off(struct led *pLed);
static void set_pattern(struct led *pLed,
			struct lcz_led_blink_pattern const *pPattern);
static void change_state(struct led *pLed, bool state, bool blink);

#ifdef CONFIG_MCUBOOT
static void led_timer_handler(struct led *pLed);
#else
static void system_workq_led_timer_handler(struct k_work *item);
#endif

#ifndef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void led_bind_and_configure(struct lcz_led_configuration *pConfig);
static void led_bind_device(led_index_t index, const struct device *dev);
static void led_configure_pin(led_index_t index, uint32_t pin);
#endif

static bool valid_index(led_index_t index);
static int led_lock(led_index_t index);
static void led_unlock(led_index_t index);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void lcz_led_init(struct lcz_led_configuration *pConfig, size_t size)
{
	size_t i;
	struct lcz_led_configuration *pc = pConfig;

#ifdef CONFIG_LCZ_LED_DRIVER_MUTEX
	k_mutex_init(&led_mutex);
	TAKE_MUTEX(led_mutex);
#endif

	for (i = 0; i < MIN(size, CONFIG_LCZ_NUMBER_OF_LEDS); i++) {
		k_timer_init(&led[i].timer, led_timer_callback, NULL);
		k_timer_user_data_set(&led[i].timer, &led[i]);

#ifndef CONFIG_MCUBOOT
		k_work_init(&led[i].work, system_workq_led_timer_handler);
#endif

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
		led[i].on = pc->on;
		led[i].off = pc->off;
		turn_off(&led[i]);
#else
		led_bind_and_configure(pc);
#endif
		led[i].initialized = true;
#ifdef CONFIG_LCZ_LED_DRIVER_ATOMIC
		led[i].locked = false;
#endif
		led[i].index = i;
		pc += 1;
	}

#ifdef CONFIG_LCZ_LED_DRIVER_MUTEX
	GIVE_MUTEX(led_mutex);
#endif
}

int lcz_led_turn_on(led_index_t index)
{
	int r = led_lock(index);
	if (r == 0) {
		change_state(&led[index], ON, DONT_BLINK);
		led_unlock(index);
	}

	return r;
}

int lcz_led_turn_off(led_index_t index)
{
	int r = led_lock(index);
	if (r == 0) {
		change_state(&led[index], OFF, DONT_BLINK);
		led_unlock(index);
	}

	return r;
}

int lcz_led_blink(led_index_t index,
		  struct lcz_led_blink_pattern const *pPattern, bool force)
{
	int r = -EINVAL;

	if (pPattern != NULL) {
		if (led[index].state == ON && !force) {
			return -EBUSY;
		}
		r = led_lock(index);
		if (r == 0) {
			led[index].pattern_busy = true;
			set_pattern(&led[index], pPattern);
			change_state(&led[index], ON, BLINK);
			led_unlock(index);
		}
	} else {
		__ASSERT(false, "NULL LED pattern");
	}

	return r;
}

int lcz_led_register_pattern_complete_function(led_index_t index,
					       void (*function)(void))
{
	int r = led_lock(index);
	if (r == 0) {
		led[index].pattern_complete_function = function;
		led_unlock(index);
	}

	return r;
}

bool lcz_led_pattern_busy(led_index_t index)
{
	if (!valid_index(index)) {
		return false;
	}

	return led[index].pattern_busy;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
#ifndef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void led_bind_and_configure(struct lcz_led_configuration *pConfig)
{
	if (pConfig->index >= CONFIG_LCZ_NUMBER_OF_LEDS) {
		__ASSERT(false, "Invalid LED Index");
		return;
	}
	led_bind_device(pConfig->index, pConfig->dev);
	led[pConfig->index].flags = pConfig->flags;
	led_configure_pin(pConfig->index, pConfig->pin);
}

static void led_bind_device(led_index_t index, const struct device *dev)
{
	led[index].device_handle = dev;
	if (!led[index].device_handle) {
		LOG_ERR("Cannot find %s!", dev->name);
	}
}

static void led_configure_pin(led_index_t index, uint32_t pin)
{
	int ret;
	led[index].pin = pin;
	ret = gpio_pin_configure(led[index].device_handle, led[index].pin,
				 (GPIO_OUTPUT_INACTIVE | led[index].flags));
	if (ret) {
		LOG_ERR("Error configuring GPIO");
	}
	led[index].state = OFF;
}
#endif

#ifdef CONFIG_MCUBOOT
static void led_timer_handler(struct led *pLed)
{
	int r = led_lock(pLed->index);
	if (r == 0) {
		if (pLed->pattern.repeat_count == 0) {
			change_state(pLed, OFF, DONT_BLINK);
			if (pLed->pattern_complete_function != NULL) {
				pLed->pattern_busy = false;
				pLed->pattern_complete_function();
			}
		} else {
			/* Blink patterns start with the LED on, so check the repeat count
			 * after the first on->off cycle has completed (when the repeat
			 * count is non-zero).
			 */
			if (pLed->state == ON) {
				change_state(pLed, OFF, BLINK);
			} else {
				if (pLed->pattern.repeat_count !=
				    REPEAT_INDEFINITELY) {
					pLed->pattern.repeat_count -= 1;
				}
				change_state(pLed, ON, BLINK);
			}
		}
		led_unlock(pLed->index);
	}
}
#else
static void system_workq_led_timer_handler(struct k_work *item)
{
	struct led *pLed = CONTAINER_OF(item, struct led, work);

	int r = led_lock(pLed->index);
	if (r == 0) {
		if (pLed->pattern.repeat_count == 0) {
			change_state(pLed, OFF, DONT_BLINK);
			if (pLed->pattern_complete_function != NULL) {
				pLed->pattern_busy = false;
				pLed->pattern_complete_function();
			}
		} else {
			/* Blink patterns start with the LED on, so check the repeat count
			 * after the first on->off cycle has completed (when the repeat
			 * count is non-zero).
			 */
			if (pLed->state == ON) {
				change_state(pLed, OFF, BLINK);
			} else {
				if (pLed->pattern.repeat_count !=
				    REPEAT_INDEFINITELY) {
					pLed->pattern.repeat_count -= 1;
				}
				change_state(pLed, ON, BLINK);
			}
		}
		led_unlock(pLed->index);
	}
}
#endif

static void change_state(struct led *pLed, bool state, bool blink)
{
	if (state == ON) {
		turn_on(pLed);
	} else {
		turn_off(pLed);
	}

	if (!blink) {
		pLed->pattern.repeat_count = 0;
		k_timer_stop(&pLed->timer);
	} else {
		if (state == ON) {
			k_timer_start(&pLed->timer,
				      K_MSEC(pLed->pattern.on_time), K_NO_WAIT);
		} else {
			k_timer_start(&pLed->timer,
				      K_MSEC(pLed->pattern.off_time),
				      K_NO_WAIT);
		}
	}

	LOG_DBG("%s %s", state ? "On" : "Off", blink ? "blink" : "Don't blink");
}

static void set_pattern(struct led *pLed,
			struct lcz_led_blink_pattern const *pPattern)
{
	memcpy(&pLed->pattern, pPattern, sizeof(struct lcz_led_blink_pattern));
	pLed->pattern.on_time =
		MAX(pLed->pattern.on_time, MINIMUM_ON_TIME_MSEC);
	pLed->pattern.off_time =
		MAX(pLed->pattern.off_time, MINIMUM_OFF_TIME_MSEC);
}

static void turn_on(struct led *pLed)
{
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
	if (pLed->on != NULL) {
		pLed->on();
	}
#else
	gpio_pin_set(pLed->device_handle, pLed->pin, ON);
#endif
	pLed->state = ON;
}

static void turn_off(struct led *pLed)
{
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
	if (pLed->off != NULL) {
		pLed->off();
	}
#else
	gpio_pin_set(pLed->device_handle, pLed->pin, OFF);
#endif
	pLed->state = OFF;
}

static bool valid_index(led_index_t index)
{
	if (index >= 0 && index < CONFIG_LCZ_NUMBER_OF_LEDS) {
		return led[index].initialized;
	} else {
		LOG_DBG("Invalid LED index %d", index);
		return false;
	}
}

static int led_lock(led_index_t index)
{
	if (!valid_index(index)) {
		return -EINVAL;
	}

#ifdef CONFIG_LCZ_LED_DRIVER_ATOMIC
	if (atomic_cas(&led[index].locked, 0, 1)) {
		return 0;
	}
#else
	TAKE_MUTEX(led_mutex);
	return 0;
#endif

	LOG_WRN("Unable to lock led %d", index);
	return -ENOLCK;
}

static void led_unlock(led_index_t index)
{
#ifdef CONFIG_LCZ_LED_DRIVER_ATOMIC
	atomic_clear(&led[index].locked);
#else
	GIVE_MUTEX(led_mutex);
#endif
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void led_timer_callback(struct k_timer *timer_id)
{
	/* Add item to system work queue so that it can be handled in task
	 * context because LEDs cannot be handed in interrupt context (mutex).
	 *
	 * This doesn't apply for atomic version of driver, but the same
	 * flow is kept.
	 */
	struct led *pLed = (struct led *)k_timer_user_data_get(timer_id);

#ifdef CONFIG_MCUBOOT
	led_timer_handler(pLed);
#else
	k_work_submit(&pLed->work);
#endif
}
