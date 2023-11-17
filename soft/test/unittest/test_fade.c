/*! \file test_fade.c
 *
 *  \brief Coordinated fading unit test
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

#include "fade.h"       /* Module under test */

/** private fade.c interface */
extern uint8_t fade_adjust_linear(uint8_t previous);

/** task.c mock */
#define TASK_STUB "../stubs/task.h"
#include TASK_STUB
TASK_IMPORT(fade_task);

/** pwm.c mock */
#define PWM_STUB "pwm.h"
#include PWM_STUB

static uint8_t pwm0, pwm1;

/**
 * @brief We mock our own PWM because it's easier than predicting order of setting
 * @param channel to set
 * @param duty to set
 */
extern uint8_t pwm_get(uint8_t channel)
{
    switch(channel)
    {
    case 0:
        return pwm0;
    case 1:
        return pwm1;
    default:
        TEST_FAIL_MESSAGE("unexpected PWM channel");
    }
}

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

void setUp(void)
{
    /* Synchronise fade task update */
    fade_set_update(1);
    TASK_CYCLE(fade_task)(1);
}

void test_fade_up(void)
{
    fade_set_rate_linear(10);

    /* Fade up from 0 to 255, skipping uninteresting iterations */
    fade_set_brightness(255);
    TEST_ASSERT_EQUAL(10, fade_adjust_linear(0));
    TEST_ASSERT_EQUAL(20, fade_adjust_linear(10));
    TEST_ASSERT_EQUAL(130, fade_adjust_linear(120));
    TEST_ASSERT_EQUAL(250, fade_adjust_linear(240));
    TEST_ASSERT_EQUAL(255, fade_adjust_linear(250));

    /* Perfect hit target */
    TEST_ASSERT_EQUAL(255, fade_adjust_linear(245));

    /* Track target */
    TEST_ASSERT_EQUAL(255, fade_adjust_linear(255));

    /* Don't overshoot target */
    fade_set_brightness(253);
    TEST_ASSERT_EQUAL(253, fade_adjust_linear(250));
}

void test_fade_down(void)
{
    fade_set_rate_linear(10);

    /* Fade down from 255 to 0, skipping uninteresting iterations */
    fade_set_brightness(0);
    TEST_ASSERT_EQUAL(245, fade_adjust_linear(255));
    TEST_ASSERT_EQUAL(235, fade_adjust_linear(245));
    TEST_ASSERT_EQUAL(125, fade_adjust_linear(135));
    TEST_ASSERT_EQUAL(5, fade_adjust_linear(15));
    TEST_ASSERT_EQUAL(0, fade_adjust_linear(5));

    /* Perfect hit target */
    TEST_ASSERT_EQUAL(0, fade_adjust_linear(10));

    /* Track target */
    TEST_ASSERT_EQUAL(0, fade_adjust_linear(0));

    /* Don't overshoot target */
    fade_set_brightness(2);
    TEST_ASSERT_EQUAL(2, fade_adjust_linear(5));
}

void test_fade_middle(void)
{
    fade_set_rate_linear(30);

    fade_set_brightness(100);
    TEST_ASSERT_EQUAL(90, fade_adjust_linear(60));
    TEST_ASSERT_EQUAL(100, fade_adjust_linear(90));
    TEST_ASSERT_EQUAL(100, fade_adjust_linear(110));
    TEST_ASSERT_EQUAL(110, fade_adjust_linear(140));

    /* Perfect hit target */
    TEST_ASSERT_EQUAL(100, fade_adjust_linear(70));
    TEST_ASSERT_EQUAL(100, fade_adjust_linear(130));

    /* Track target */
    TEST_ASSERT_EQUAL(100, fade_adjust_linear(100));
}

void test_fade_task_sleep(void)
{
    pwm0 = pwm1 = 0;
    fade_set_rate_linear(5);
    fade_set_brightness(200);
    fade_set_update(100);
    TEST_ASSERT_EQUAL(99, TASK_CYCLE(fade_task)(1));
    TEST_ASSERT_EQUAL(9, TASK_CYCLE(fade_task)(90));
    TEST_ASSERT_EQUAL(100, TASK_CYCLE(fade_task)(9));
}

void test_fade_task_update(void)
{
    pwm0 = 20;
    pwm1 = 250;
    fade_set_rate_linear(5);
    fade_set_brightness(200);
    fade_set_update(100);
    TEST_ASSERT_EQUAL(100, TASK_CYCLE(fade_task)(100));
    TEST_ASSERT_EQUAL(25, pwm0);
    TEST_ASSERT_EQUAL(245, pwm1);
}

void test_fade_task_idle(void)
{
    pwm0 = 197;
    pwm1 = 203;
    fade_set_rate_linear(5);
    fade_set_brightness(200);
    fade_set_update(100);
    TEST_ASSERT_EQUAL(100, TASK_CYCLE(fade_task)(100));
    TEST_ASSERT_EQUAL(200, pwm0);
    TEST_ASSERT_EQUAL(200, pwm1);
    TEST_ASSERT_EQUAL(255, TASK_CYCLE(fade_task)(100));

    /* Bump just one of the PWMs */
    pwm1 = 205;
    TEST_ASSERT_EQUAL(100, TASK_CYCLE(fade_task)(100));
    TEST_ASSERT_EQUAL(200, pwm0);
    TEST_ASSERT_EQUAL(200, pwm1);
    TEST_ASSERT_EQUAL(255, TASK_CYCLE(fade_task)(100));
}

void test_fade_task_instantaneous(void)
{
    pwm0 = 20;
    pwm1 = 250;
    fade_set_rate_linear(5);
    fade_set_brightness(200);
    fade_set_update(0);
    TEST_ASSERT_EQUAL(255, TASK_CYCLE(fade_task)(100));
    TEST_ASSERT_EQUAL(200, pwm0);
    TEST_ASSERT_EQUAL(200, pwm1);
}
