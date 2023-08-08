/*! \file blinky.c
 *
 *  \brief LED flashing task
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

#define BLINKY_TICK 10

static uint8_t blinky_task(uint8_t ms_later)
{
    static uint8_t wait;

    switch(ms_later)
    {
    case TASK_STARTUP:
        wait = 0;
        /* Split position range into thirds: ON, OFF, FADE up/down */
        twinkle_set_position(0);
        twinkle_set_brightness(85);
        break;
    case TASK_SHUTDOWN:
        twinkle_set_brightness(0);
        break;
    default:
        wait += ms_later;
        if (wait < BLINKY_TICK)
            break;
        wait -= BLINKY_TICK;
        /* 256x10ms ~ 2.6s cycle */
        twinkle_set_position(twinkle_get_position()+1);
        break;
    }

    return BLINKY_TICK-wait;
}

TASK_DECLARE(blinky_task);
