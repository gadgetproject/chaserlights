/*! \file task.c
 *
 *  \brief Scheduler implementation
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "task.h"

#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

static volatile uint8_t task_ticks; /**< increments each millisecond */

/**
 * @brief Calculate the number of F_CPU/256 ticks in 2ms, rounding up
 */
#define TIMER_SYSCLK_256_2ms ((F_CPU+64000)/128000)

/**
 * @brief Timer0 comparison interrupt handler
 */
ISR (TIMER0_COMPA_vect)
{
    /* Count ticks of timer comparison */
    TCNT0 = 0;

    /* Modulate comparison to give /128 resolution */
#if TIMER_SYSCLK_256_2ms % 2
    OCR0A ^= 1;
#endif

    task_ticks++;
}

/**
 * @brief Sleep the processor for a short period
 * @param milliseconds to sleep
 */
static void task_delay(uint8_t milliseconds)
{
    while (task_ticks < milliseconds)
    {
        sleep_cpu();
    }

    /* Reset timer on exit so next delay is aligned to this time */
    task_ticks = 0;
}

/**
 * @brief Call task_cycle for each task, collating next wake times
 * @param [in] ms_later how many milliseconds elapsed since last call
 * @return time to sleep before next call or TASK_SHUTDOWN
 */
static uint8_t task_cycle_all(uint8_t ms_later)
{
    uint8_t all_wake = UINT8_MAX;

    /* See TASK_DECLARE() which builds an array of task function pointers */
    extern const task_cycle __task_list_start;
    extern const task_cycle __task_list_end;
    for (const task_cycle* pTask = &__task_list_start; pTask != &__task_list_end; pTask++)
    {
        /* Array is stored in flash so we need to explicitly read */
        task_cycle task = (task_cycle)pgm_read_word_near(pTask);

        /* Give the task a chance to run and tell us how long it can sleep for */
        uint8_t wake = task(ms_later);

        /* Collate the soonest waking task */
        if (wake < all_wake)
            all_wake = wake;
    }

    return all_wake;
}

/**
 * @brief Round-robin scheduler calls each task function in turn
 */
int main(void)
{
    /* Power management */
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    sleep_enable();

    /* We use TIMER0 with Counter A and /256 prescaler */
    TCCR0A = 0x02;  /* OCRA */
    TCCR0B = 0x04;  /* /256 */

    /* Reset counter and set ~1ms compare */
    TCNT0 = 0;
    OCR0A = (uint8_t)(TIMER_SYSCLK_256_2ms/2);

    /* Raise interrupt on Compare Match A */
    TIMSK = 0x10;   /* OCIE0A */

    /* Initialise and run */
    sei();
    for(uint8_t sleep = TASK_STARTUP; sleep != TASK_SHUTDOWN; task_delay(sleep))
    {
        sleep = task_cycle_all(sleep);
        if (sleep == TASK_STARTUP)
            sleep--;
    }
    cli();

    /* Shutdown */
    (void)task_cycle_all(TASK_SHUTDOWN);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_cpu();
}
