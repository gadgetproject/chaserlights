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

#include <util/delay.h>
#define task_delay _delay_ms        /**< TODO: proper low-power sleep */

#define TASK_BLINKY

#ifdef TASK_BLINKY
#include <avr/io.h>

static uint8_t task_test(uint8_t ms_later)
{
    switch(ms_later)
    {
    case TASK_STARTUP:
        DDRB |= 2;  /* PB1 */
        break;
    case TASK_SHUTDOWN:
        DDRB &= ~2; /* PB1 */
        break;
    default:
        PORTB ^= 2; /* PB1 */
    }
    
    /* Minimum 2Hz blink but actually follows most wakeful task */
    return 250;
}
#endif

static const task_cycle tasks[] = {
#ifdef TASK_BLINKY
    task_test,
#endif
};

/**
 * @brief Call task_cycle for each task, collating next wake times
 * @param [in] ms_later how many milliseconds elapsed since last call
 * @return time to sleep before next call or TASK_SHUTDOWN
 */
static uint8_t task_cycle_all(uint8_t ms_later)
{
    uint8_t all_wake = UINT8_MAX;
    
    for (uint8_t i = 0; i < sizeof(tasks)/sizeof(*tasks); i++)
    {
        uint8_t task_wake = tasks[i](ms_later);
        if (task_wake < all_wake)
            all_wake = task_wake;
    }

    return all_wake;
}

/**
 * @brief Round-robin scheduler calls each task cycle function in turn
 */
int main(void)
{
    /* Initialise and run */
    for(uint8_t sleep = TASK_STARTUP; sleep != TASK_SHUTDOWN; task_delay(sleep))
    {
        sleep = task_cycle_all(sleep);
        if (sleep == TASK_STARTUP)
            sleep--;
    }

    /* Shutdown */
    (void)task_cycle_all(TASK_SHUTDOWN);
    for(;;);
}
