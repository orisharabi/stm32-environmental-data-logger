/**
 * @file scheduler.c
 * @brief Interrupt-driven periodic measurement scheduling.
 */

#include "scheduler.h"
#include "tim.h"

/* Shared between the timer ISR and the main loop. */
static volatile uint8_t measurement_due = 0;

/**
 * @brief Starts TIM3 in interrupt mode.
 * @return true on success, false on failure.
 */
bool Scheduler_Start(void)
{
    return HAL_TIM_Base_Start_IT(&htim3) == HAL_OK;
}

/**
 * @brief Atomically consumes the pending measurement flag.
 * @return true if a measurement is pending, otherwise false.
 * @note Multiple pending timer events are coalesced into one.
 */
bool Scheduler_ConsumeMeasurementFlag(void)
{
    uint32_t previous_state = __get_PRIMASK();

    /* Protect the flag's read-and-clear operation. */
    __disable_irq();

    uint8_t due = measurement_due;
    measurement_due = 0;

    /* Restore the previous interrupt state. */
    __set_PRIMASK(previous_state);

    return due;
}

/**
 * @brief Handles timer period elapsed events.
 * @param htim Pointer to the timer handle.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        /* Signal the main loop and keep the ISR short. */
        measurement_due = 1;
    }
}
