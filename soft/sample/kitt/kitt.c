/*! \file kitt.c
 *
 *  \brief LED bar sample, possibly useful on a black 1982 Pontiac Trans Am
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
#include "twinkle.h"

#include <stdint.h>

#define KITT_TICK 8
#define KITT_ENDSTOP 30

#if KITT_ENDSTOP & 1
# error "KITT_ENDSTOP needs to be an even number"
#endif

static uint8_t kitt_task(uint8_t ms_later)
{
    static uint8_t wait;

    switch(ms_later)
    {
    case TASK_STARTUP:
        wait = 0;
        twinkle_set_position(0);
        twinkle_set_brightness(40);
        break;
    case TASK_SHUTDOWN:
        twinkle_set_brightness(0);
        break;
    default:
        wait += ms_later;
        if (wait < KITT_TICK)
            break;
        wait -= KITT_TICK;
        /* 256x8ms ~ 1s cycle */
        uint8_t was = twinkle_get_position();
        if (was < KITT_ENDSTOP)
        {
            /* Left endstop reached: scan right */
            twinkle_set_position(KITT_ENDSTOP);
        }
        else if (was > 255-KITT_ENDSTOP)
        {
            /* Right endstop reached; scan left */
            twinkle_set_position(255-KITT_ENDSTOP);
        }
        else if ((was & 1) == 0)
        {
            /* Scan right on even positions */
            twinkle_set_position(was+2);
        }
        else
        {
            /* Scan left on odd positions */
            twinkle_set_position(was-2);
        }
        break;
    }

    return KITT_TICK-wait;
}

TASK_DECLARE(kitt_task);
