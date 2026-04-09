/*! \file heartbeat.c
 *
 *  \brief Heartbeat effect on LEDs
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
#include "fade.h"
#include "pwm.h"

#include <stdint.h>

#define HEART_BPM 70

#define HEART_RHYTHM    (60000U/HEART_BPM)
#define HEART_SYSTOLIC  (HEART_RHYTHM/10)
#define HEART_ASYSTOLIC (HEART_RHYTHM/5)

static uint8_t heartbeat_task(uint8_t ms_later)
{
    static uint16_t cycle;

    switch(ms_later)
    {
    case TASK_STARTUP:
        cycle = 0;
        /* Fade to black in 80ms */
        fade_set_brightness(0);
        fade_set_update(10);
        fade_set_rate_linear(32);
        break;
    case TASK_SHUTDOWN:
        break;
    default:
        cycle += ms_later;
        if (cycle == HEART_SYSTOLIC)
        {
            /* Systolic */
            pwm_set(0, 255);
            pwm_set(1, 255);
        }
        else if (cycle == HEART_ASYSTOLIC)
        {
            /* Asystolic */
            pwm_set(1, 128);
            pwm_set(2, 255);
        }
        else if (cycle >= HEART_RHYTHM)
        {
            /* Rhythmic */
            cycle -= HEART_RHYTHM;
        }
        break;
    }

    /* Calculate when the next event will be */
    uint16_t next;
    if (cycle < HEART_SYSTOLIC)
    {
        next = HEART_SYSTOLIC-cycle;
    }
    else if (cycle < HEART_ASYSTOLIC)
    {
        next = HEART_ASYSTOLIC-cycle;
    }
    else
    {
        next = HEART_RHYTHM-cycle;
    }

    /* Tell scheduler when we'd like to be woken */
    return (next < 250) ? (uint8_t)next : 250;
}

TASK_DECLARE(heartbeat_task);
