/*! \file twinkle.c
 *
 *  \brief Coordinate the brightness of several LEDs
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

#include "twinkle.h"
#include "pwm.h"

/* Select configuration */
#ifndef TWINKLE_COMFIG
# define TWINKLE_CONFIG "twinkle.config"
#endif

/* This is metaprogramming - more common in languages like C++ - where
 * a template causes specialised code to be generated as opposed to
 * data generated that is operated on by generic code.
 *
 * It's extremely efficient on a small processor with specialised I/O
 * instructions and not many ports.
 */
#include TWINKLE_CONFIG

static uint8_t twinkle_position;    /**< current position of light source, wraps around 255-0 */
static uint8_t twinkle_brightness;  /**< current brightness of light source: 0 = All OFF, 255 = All ON */

/**
 * @brief Reassess light position and brightness vs LED positions
 */
static void twinkle_set_pwms(void)
{
#define TWINKLE_SET_PWM(channel_,position_)                                             \
    {                                                                                   \
        uint8_t distance = twinkle_position - position_;                                \
        if (distance & 0x80)                                                            \
            distance = ~distance;                                                       \
        if (distance >= twinkle_brightness)                                             \
            pwm_set(channel_, 0);                                                       \
        else if ((distance*2) < twinkle_brightness)                                     \
            pwm_set(channel_, 255);                                                     \
        else                                                                            \
        {                                                                               \
            distance -= twinkle_brightness/2;                                           \
            pwm_set(channel_, 255-((256u*(uint16_t)(distance*2))/twinkle_brightness));  \
        }                                                                               \
    }
TWINKLE_PWMS(TWINKLE_SET_PWM)
#undef TWINKLE_SET_PWM
}

void twinkle_set_position(uint8_t position)
{
    twinkle_position = position;
    twinkle_set_pwms();
}

uint8_t twinkle_get_position(void)
{
    return twinkle_position;
}

void twinkle_set_brightness(uint8_t brightness)
{
    twinkle_brightness = brightness;
    twinkle_set_pwms();
}

uint8_t twinkle_get_brightness(void)
{
    return twinkle_brightness;
}
