/*! \file fade.c
 *
 *  \brief Coordinate the up/down fading of several LEDs
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

#include "fade.h"
#include "pwm.h"
#include "task.h"

#include <stdbool.h>

/* Select configuration */
#ifndef FADE_CONFIG
# define FADE_CONFIG "fade.config"
#endif

/* This is metaprogramming - more common in languages like C++ - where
 * a template causes specialised code to be generated as opposed to
 * data generated that is operated on by generic code.
 *
 * It's extremely efficient on a small processor with very little RAM.
 */
#include FADE_CONFIG

#if defined(FADE_PWMS)

#ifdef TEST
# define STATIC /* extern */
#else
# define STATIC static
#endif

STATIC uint8_t fade_target;
STATIC uint8_t fade_cycle_ms;
STATIC uint8_t fade_rate;

void fade_set_brightness(uint8_t target)
{
    fade_target = target;
}

void fade_set_rate_linear(uint8_t delta)
{
    fade_rate = delta;
}

void fade_set_update(uint8_t milliseconds)
{
    fade_cycle_ms = milliseconds;
}

/**
 * @brief Apply fade_target, fade_rate to adjust brightness linearly
 * @param previous brightness
 * @return next brightness
 */
STATIC uint8_t fade_adjust_linear(uint8_t previous)
{
    if (fade_target == previous)
    {
        /* Target achieved */
        return fade_target;
    }
    else if (previous > fade_target)
    {
        /* Fade down */
        if ((previous-fade_target) > fade_rate)
        {
            /* Tracking */
            return previous-fade_rate;
        }
        else
        {
            /* Achieved */
            return fade_target;
        }
    }
    else
    {
        /* Fade up */
        if ((fade_target-previous) > fade_rate)
        {
            /* Tracking */
            return previous+fade_rate;
        }
        else
        {
            /* Achieved */
            return fade_target;
        }
    }
}

static uint8_t fade_task(uint8_t ms_later)
{
    static uint8_t tick;

    switch(ms_later)
    {
    case TASK_STARTUP:
        tick = 0;
        return 1;

    case TASK_SHUTDOWN:
        return 1;

    default:
        if (!fade_cycle_ms)
        {
            /* Achieve target immediately */
#define FADE_SET_TARGET(channel_) pwm_set(channel_, fade_target);
            FADE_PWMS(FADE_SET_TARGET);
#undef FADE_SET_TARGET

            /* And now idle */
            tick = 0;
            return 255;
        }
        else
        {
            /* Ready for another cycle? */
            tick += ms_later;
            if (tick < fade_cycle_ms)
            {
                return fade_cycle_ms-tick;
            }
            tick = 0;
            
            /* Iterate over PWM channels, adjusting each */
            bool idle = true;
#define FADE_TO_TARGET(channel_)                                \
            {                                                   \
                uint8_t previous = pwm_get(channel_);           \
                uint8_t next = fade_adjust_linear(previous);    \
                if (next != previous)                           \
                {                                               \
                    idle = false;                               \
                    pwm_set(channel_, next);                    \
                }                                               \
            }
            FADE_PWMS(FADE_TO_TARGET);
#undef FADE_TO_TARGET

            return idle ? 255 : fade_cycle_ms;
        }
    }
}

TASK_DECLARE(fade_task);

#endif /* defined(FADE_PWMS) */
