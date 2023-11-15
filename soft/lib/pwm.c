/*! \file pwm.c
 *
 *  \brief Software Pulse Width Modulation implementation
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

#include "pwm.h"
#include "gpio.h"
#include "task.h"

/* Select configuration */
#ifndef PWM_COMFIG
# define PWM_CONFIG "pwm.config"
#endif

#define PWM_CYCLE_MILLISECONDS 16     /**< 67Hz cycle */

/* This is metaprogramming - more common in languages like C++ - where
 * a template causes specialised code to be generated as opposed to
 * data generated that is operated on by generic code.
 *
 * It's extremely efficient on a small processor with specialised I/O
 * instructions and not many ports.
 */
#include PWM_CONFIG

#if defined(PWM_GPIOS)

/**
 * Array of duty factors, initialised to the number of PWM GPIOs configured
 */
static uint8_t pwm_duty[] =
{
#define PWM_GPIO_DUTY(port_,pin_) 0,    /**< initialise all OFF */
PWM_GPIOS(PWM_GPIO_DUTY)
#undef PWM_GPIO_DUTY
};

void pwm_set(uint8_t channel, uint8_t duty)
{
    if (channel < sizeof(pwm_duty))
        pwm_duty[channel] = duty;
}

uint8_t pwm_get(uint8_t channel)
{
    return (channel < sizeof(pwm_duty)) ? pwm_duty[channel] : 0;
}

static uint8_t pwm_task(uint8_t ms_later)
{
    static uint8_t tick;

    switch(ms_later)
    {
    case TASK_STARTUP:
        tick = ~0;
        /* Enable outputs */
        PWM_GPIOS(GPIO_CONFIGURE_DIGITAL_OUTPUT);
        return 1;

    case TASK_SHUTDOWN:
        /* Disable outputs */
        PWM_GPIOS(GPIO_CONFIGURE_UNUSED);
        return 1;

    default:
        {
            /* Keep track of tick within cycle */
            tick += ms_later;
            tick %= PWM_CYCLE_MILLISECONDS;

            /* Work out corresponding duty */
#if (PWM_CYCLE_MILLISECONDS&(PWM_CYCLE_MILLISECONDS-1)) != 0
# warning "For efficiency, PWM_CYCLE_MILLISECONDS ought to be a power of 2"
#endif
            /* NOTE: looks complicated but gcc optimises to ROTATE and AND */
            uint8_t duty = (256u*(uint16_t)tick)
                           /(uint8_t)PWM_CYCLE_MILLISECONDS;

            /* Switch GPIOs ON/OFF according to duty setting vs tick count */
            uint8_t channel = 0;
            uint8_t next_duty = 255;
#define PWM_GPIO_PORT_SWITCH(port_,pin_)                \
            if (pwm_duty[channel] > duty)               \
            {                                           \
                GPIO_OUTPUT_Vcc(port_, pin_); /* ON */  \
                if (pwm_duty[channel] < next_duty)      \
                    next_duty = pwm_duty[channel];      \
            }                                           \
            else                                        \
                GPIO_OUTPUT_GND(port_, pin_); /* OFF */ \
            channel++;
PWM_GPIOS(PWM_GPIO_PORT_SWITCH)
#undef PWM_GPIO_PORT_SWITCH

            /* Find out when the next transition will occur */
            uint8_t next_tick = (PWM_CYCLE_MILLISECONDS*(uint16_t)(next_duty+1))
                                /256u;
            return (tick == next_tick) ? 1 : next_tick-tick;
        }
    }
}

TASK_DECLARE(pwm_task);

#endif /* defined(PWM_GPIOS) */
