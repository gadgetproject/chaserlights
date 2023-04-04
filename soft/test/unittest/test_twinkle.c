/*! \file test_twinkle.c
 *
 *  \brief Coordinates Brightness unit test
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

#include "unity.h"      /* Framework */

#include "twinkle.h"    /* Module under test */

static uint8_t pwm0, pwm1;

#define PWM_STUB "pwm.h"
#include PWM_STUB

/**
 * @brief We mock our own PWM because it's easier than predicting order of setting
 * @param channel to set
 * @param duty to set
 */
extern void pwm_set(uint8_t channel, uint8_t duty)
{
    switch(channel)
    {
    case 0:
        pwm0 = duty;
        break;
    case 1:
        pwm1 = duty;
        break;
    default:
        TEST_FAIL_MESSAGE("unexpected PWM channel");
    }
}

#define CH_0_POS 0      /**< matches ../stubs/twinkle.config */
#define CH_1_POS 128    /**< matches ../stubs/twinkle.config */

void setUp(void)
{
    twinkle_set_position(0);
    twinkle_set_brightness(0);
}

void test_on(void)
{
    twinkle_set_brightness(255);
    for (uint8_t p = 0; p < 255; p++)
    {
        twinkle_set_position(p);
        TEST_ASSERT_EQUAL(255, pwm0);
        TEST_ASSERT_EQUAL(255, pwm1);
    }
}

void test_off(void)
{
    twinkle_set_brightness(0);
    for (uint8_t p = 0; p < 255; p++)
    {
        twinkle_set_position(p);
        TEST_ASSERT_EQUAL(0, pwm0);
        TEST_ASSERT_EQUAL(0, pwm1);
    }
}

void test_point(void)
{
    twinkle_set_brightness(1);
    for (uint8_t p = 0; p < 255; p++)
    {
        twinkle_set_position(p);
        if (((p+1)/2) == (CH_0_POS/2))
            TEST_ASSERT_EQUAL(255, pwm0);
        else
            TEST_ASSERT_EQUAL(0, pwm0);

        if (((p+1)/2) == (CH_1_POS/2))
            TEST_ASSERT_EQUAL(255, pwm1);
        else
            TEST_ASSERT_EQUAL(0, pwm1);
    }
}

void test_set_get(void)
{
    twinkle_set_brightness(0);
    twinkle_set_position(0);
    TEST_ASSERT_EQUAL(0, twinkle_get_brightness());
    TEST_ASSERT_EQUAL(0, twinkle_get_position());

    twinkle_set_brightness(123);
    twinkle_set_position(234);
    TEST_ASSERT_EQUAL(123, twinkle_get_brightness());
    TEST_ASSERT_EQUAL(234, twinkle_get_position());

    twinkle_set_brightness(255);
    twinkle_set_position(255);
    TEST_ASSERT_EQUAL(255, twinkle_get_brightness());
    TEST_ASSERT_EQUAL(255, twinkle_get_position());
}

void test_transition(void)
{
    twinkle_set_brightness(63);

    twinkle_set_position(0);
    TEST_ASSERT_EQUAL(255, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(31);
    TEST_ASSERT_EQUAL(255, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(47);
    TEST_ASSERT_UINT8_WITHIN(4, 128, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(64);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(80);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_UINT8_WITHIN(4, 128, pwm1);

    twinkle_set_position(100);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_EQUAL(255, pwm1);

    twinkle_set_position(128);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_EQUAL(255, pwm1);

    twinkle_set_position(150);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_EQUAL(255, pwm1);

    twinkle_set_position(175);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_UINT8_WITHIN(4, 128, pwm1);

    twinkle_set_position(192);
    TEST_ASSERT_EQUAL(  0, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(208);
    TEST_ASSERT_UINT8_WITHIN(4, 128, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(230);
    TEST_ASSERT_EQUAL(255, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);

    twinkle_set_position(255);
    TEST_ASSERT_EQUAL(255, pwm0);
    TEST_ASSERT_EQUAL(  0, pwm1);
}
