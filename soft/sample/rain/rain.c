/*! \file rain.c
 *
 *  \brief Rain effect on LEDs
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

#define RAIN_TICK 100
#define RAIN_DISPERSE 80
#define RAIN_SPREAD 50

static uint8_t rain_task(uint8_t ms_later)
{
    static uint8_t wait;

    switch(ms_later)
    {
    case TASK_STARTUP:
        wait = 0;
        /* Fade to black */
        fade_set_brightness(0);
        fade_set_update(RAIN_DISPERSE);
        fade_set_rate_linear(8);
        break;
    case TASK_SHUTDOWN:
        break;
    default:
        wait += ms_later;
        if (wait >= RAIN_TICK)
        {
            /* pwm_set() ignores addressing a non-existent channel */
            pwm_set(random_get(RAIN_SPREAD), 255);
            wait = 0;
        }        
        break;
    }

    return RAIN_TICK-wait;
}

TASK_DECLARE(rain_task);
