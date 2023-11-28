/*! \file drip.c
 *
 *  \brief Drip effect on LEDs
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
#include "random.h"

#include <stdint.h>
#include <avr/pgmspace.h>

#define DRIP_TICK 100
#define DRIP_DISPERSE 100
#define DRIP_PROBABILITY 20

/* Get fade configuration */
#ifndef FADE_CONFIG
# define FADE_CONFIG "fade.config"
#endif
#include FADE_CONFIG

/* Make a list of PWMs to drip through */
static const PROGMEM uint8_t drip_led[] =
{
#define DRIP_LED(pwm_) pwm_,
FADE_PWMS(DRIP_LED)
#undef DRIP_LED
};


static uint8_t drip_task(uint8_t ms_later)
{
    static uint8_t wait;
    static uint8_t state;

    switch(ms_later)
    {
    case TASK_STARTUP:
        wait = 0;
        state = 0xFF;
        /* Fade to black */
        fade_set_brightness(0);
        fade_set_update(DRIP_DISPERSE);
        fade_set_rate_linear(16);
        break;
    case TASK_SHUTDOWN:
        break;
    default:
        wait += ms_later;
        if (wait < DRIP_TICK)
        {
            break;
        }
        wait = 0;

        /* Start a new drip? */
        if (state == 0xFF)
        {
            if (random_get(DRIP_PROBABILITY) == 0)
            {
                state = 0;
            }
            break;
        }

        /* Avoid using precious RAM for this constant table */
        uint8_t led = pgm_read_byte_near(&drip_led[state]);
        pwm_set(led, 255);
        state++;
        if (state >= sizeof(drip_led))
        {
            /* Drip complete */
            state = 0xFF;
        }
        break;
    }

    return DRIP_TICK-wait;
}

TASK_DECLARE(drip_task);
