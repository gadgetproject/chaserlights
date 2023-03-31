/*! \file test_pwm.c
 *
 *  \brief Software Pulse Width Modulation unit test
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

#include "unity.h"  /* Framework */

#include "pwm.h"    /* Module under test */

#define TASK_STUB "../stubs/task.h"
#include TASK_STUB
#include "../stubs/avr/io.h"

/** task.c mock */
TASK_IMPORT(pwm_task);

/** avr/io.h mock */
unsigned char PORTA, DDRA, PORTB, DDRB, PORTC, DDRC;


void setUp(void)
{
    DDRA=DDRB=DDRC=0;

    TASK_CYCLE(pwm_task)(TASK_STARTUP);

    TEST_ASSERT_EQUAL(1<<1, DDRA);
    TEST_ASSERT_EQUAL(1<<2, DDRB);
    TEST_ASSERT_EQUAL(1<<3, DDRC);
}

void tearDown(void)
{
    DDRA=DDRB=DDRC=0xFF;

    TASK_CYCLE(pwm_task)(TASK_SHUTDOWN);

    TEST_ASSERT_EQUAL(0xFF^(1<<1), DDRA);
    TEST_ASSERT_EQUAL(0xFF^(1<<2), DDRB);
    TEST_ASSERT_EQUAL(0xFF^(1<<3), DDRC);
}

/**
 * @brief Run PWM task for 2 simulated seconds
 * @param channel0 returns observed duty on channel 0
 * @param channel1 returns observed duty on channel 0
 * @param channel2 returns observed duty on channel 0
 */
static void run_2s(uint8_t* channel0, uint8_t* channel1, uint8_t* channel2)
{
    unsigned ch0_ms=0, ch1_ms=0, ch2_ms=0;
    PORTA = PORTB = PORTC = 0;

    for (unsigned time_ms = 0, sleep_ms = 1; time_ms < 2000; time_ms += sleep_ms)
    {
        /* Run PWM task */
        sleep_ms = TASK_CYCLE(pwm_task)(sleep_ms);
        TEST_ASSERT_TRUE(sleep_ms != TASK_SHUTDOWN);

        /* Tests are OFF, HALF, QUARTER, ON and should never need to run
         * faster than 250Hz
         */
        TEST_ASSERT_TRUE(sleep_ms >= 4);

        /* Observe PWM duty - note channels are mapped (0,1,2)->(B,A,C) */
        if (PORTA == 1<<1)
            ch1_ms += sleep_ms;
        else
            TEST_ASSERT_EQUAL(0, PORTA);
        if (PORTB == 1<<2)
            ch0_ms += sleep_ms;
        else
            TEST_ASSERT_EQUAL(0, PORTB);
        if (PORTC == 1<<3)
            ch2_ms += sleep_ms;
        else
            TEST_ASSERT_EQUAL(0, PORTC);
    }

    /* Calculate and write results */
    if (channel0)
        *channel0 = (uint8_t)((255*ch0_ms)/2000);
    if (channel1)
        *channel1 = (uint8_t)((255*ch1_ms)/2000);
    if (channel2)
        *channel2 = (uint8_t)((255*ch2_ms)/2000);
}

void test_set_get(void)
{
    /* Only 3 PWM channels defined */
    pwm_set(0, 0x11);
    pwm_set(1, 0x72);
    pwm_set(2, 0xF3);
    /* Setting others is No-Op */
    for (unsigned i = 3; i < 256; i++)
        pwm_set(i, 44);

    TEST_ASSERT_EQUAL(0x11, pwm_get(0));
    TEST_ASSERT_EQUAL(0x72, pwm_get(1));
    TEST_ASSERT_EQUAL(0xF3, pwm_get(2));
    /* Fetching non-existent channels */
    for (unsigned i = 3; i < 256; i++)
        TEST_ASSERT_EQUAL(0, pwm_get(i));
}

void test_off_duty(void)
{
    pwm_set(0, 0);
    pwm_set(1, 0);
    pwm_set(2, 0);

    uint8_t ch0, ch1, ch2;
    run_2s(&ch0, &ch1, &ch2);

    TEST_ASSERT_EQUAL(0, ch0);
    TEST_ASSERT_EQUAL(0, ch1);
    TEST_ASSERT_EQUAL(0, ch2);
}

void test_quarter_duty(void)
{
    pwm_set(0, 0);
    pwm_set(1, 0x40);
    pwm_set(2, 0xC0);

    uint8_t ch0, ch1, ch2;
    run_2s(&ch0, &ch1, &ch2);

    TEST_ASSERT_EQUAL(0, ch0);
    TEST_ASSERT_UINT8_WITHIN(1, 0x40, ch1);
    TEST_ASSERT_UINT8_WITHIN(1, 0xC0, ch2);
}

void test_half_duty(void)
{
    pwm_set(0, 0x80);
    pwm_set(1, 0);
    pwm_set(2, 0x7F);

    uint8_t ch0, ch1, ch2;
    run_2s(&ch0, &ch1, &ch2);

    TEST_ASSERT_UINT8_WITHIN(1, 0x80, ch0);
    TEST_ASSERT_EQUAL(0, ch1);
    TEST_ASSERT_UINT8_WITHIN(1, 0x7F, ch2);
}

void test_on_duty(void)
{
    pwm_set(0, 0xFF);
    pwm_set(1, 0xFF);
    pwm_set(2, 0xFF);

    uint8_t ch0, ch1, ch2;
    run_2s(&ch0, &ch1, &ch2);

    TEST_ASSERT_EQUAL(0xFF, ch0);
    TEST_ASSERT_EQUAL(0xFF, ch1);
    TEST_ASSERT_EQUAL(0xFF, ch2);
}
