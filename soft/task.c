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

/**
 * @brief Sleep the processor for a short period
 * @param milliseconds to sleep
 */
static void task_delay(uint8_t milliseconds)
{
    /**< TODO: proper low-power sleep */
    while(milliseconds--)
        __builtin_avr_delay_cycles(F_CPU/1000);
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
