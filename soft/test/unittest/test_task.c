/*! \file test_task.c
 *
 *  \brief Scheduler unit test
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

#include "task.h"    /* Module under test */

#include "../stubs/avr/interrupt.h"
#include "../stubs/avr/pgmspace.h"
#include "../stubs/avr/sleep.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

ISR(TIMER0_COMPA_vect);

unsigned char TCCR0A;
unsigned char TCCR0B;
unsigned char TCNT0;
unsigned char OCR0A;
unsigned char TIMSK;

unsigned F_CPU;

static bool interrupts_enabled;
static bool sleep_enabled;
static enum sleep_mode sleep_mode_selected;
static bool powered_down;

/**
 * Pointers to test tasks
 */
static task_cycle test_task[3];

static uint8_t dummy_task(uint8_t later_ms)
{
    return TASK_SHUTDOWN;
}

/**
 * Incrementing simulated milliseconds
 */
static unsigned milliseconds_timer;

void setUp(void)
{
    TCCR0A = 0;
    TCCR0B = 0;
    TCNT0 = 0;
    OCR0A = 0;
    TIMSK = 0;

    F_CPU = 16500000;

    interrupts_enabled = false;
    sleep_enabled = false;
    sleep_mode_selected = 0;
    powered_down = false;
    test_task[0] = dummy_task;
    test_task[1] = dummy_task;
    test_task[2] = dummy_task;
}

void tearDown(void)
{
    TEST_ASSERT_TRUE_MESSAGE(powered_down, "chip not powered down");
}

void test_timer_configuration(void)
{
    task_main();

    /* Check timer configuration */
    TEST_ASSERT_EQUAL(0x02, TCCR0A);    /* Reset on Compare Match */
    TEST_ASSERT_EQUAL(0x10, TIMSK);     /* Interrupt on Compare Match */

    /* Calculate timer period */
    int prescale = TCCR0B & 7;
    uint32_t scale[8] = { 0, 1, 8, 64, 256, 1024, 0, 0 };
    TEST_ASSERT_TRUE_MESSAGE(scale[prescale] != 0, "bad clock");
    uint32_t ns = (1000000000 / (F_CPU / scale[prescale])) * (OCR0A+1);

    /* 1% tolerance */
    TEST_ASSERT_UINT32_WITHIN(10000, 1000000, ns);
}

void test_startup_shutdown(void)
{
    static bool startup[3];
    static bool shutdown[3];

    /* Callbacks */
    uint8_t task(unsigned index, uint8_t ms_later)
    {
        switch(ms_later)
        {
        case TASK_STARTUP:
            TEST_ASSERT_FALSE_MESSAGE(startup[index], "startup twice");
            startup[index] = true;
            break;
        case TASK_SHUTDOWN:
            TEST_ASSERT_TRUE_MESSAGE(startup[index], "not started");
            TEST_ASSERT_FALSE_MESSAGE(shutdown[index], "shutdown twice");
            shutdown[index] = true;
            break;
        default:
            TEST_FAIL_MESSAGE("unexpected ms_later");
            break;
        }
        return TASK_SHUTDOWN;
    }
    uint8_t task1(uint8_t ms_later) { return task(0, ms_later); }
    uint8_t task2(uint8_t ms_later) { return task(1, ms_later); }
    uint8_t task3(uint8_t ms_later) { return task(2, ms_later); }

    /* Initial state */
    (void)memset(startup, 0, sizeof(startup));
    (void)memset(shutdown, 0, sizeof(shutdown));

    /* Run tasks */
    test_task[0] = task1;
    test_task[1] = task2;
    test_task[2] = task3;
    task_main();

    /* Final state */
    TEST_ASSERT_TRUE(startup[0]);
    TEST_ASSERT_TRUE(shutdown[0]);
    TEST_ASSERT_TRUE(startup[1]);
    TEST_ASSERT_TRUE(shutdown[1]);
    TEST_ASSERT_TRUE(startup[2]);
    TEST_ASSERT_TRUE(shutdown[2]);
}

void test_schedule_requests(void)
{
    /* Callbacks */
    uint8_t task1(uint8_t ms_later)
    {
        switch(ms_later)
        {
        case 1:
            TEST_ASSERT_EQUAL(1, milliseconds_timer);
            milliseconds_timer = 0;
            return 2;       /* trigger task 2 */

        case TASK_STARTUP:
        case 2:
        case 3:
        case TASK_SHUTDOWN:
            return 100;     /* idle */

        default:
            TEST_FAIL_MESSAGE("unexpected ms_later");
        }
    }
    uint8_t task2(uint8_t ms_later)
    {
        switch(ms_later)
        {
        case 2:
            TEST_ASSERT_EQUAL(2, milliseconds_timer);
            milliseconds_timer = 0;
            return 3;       /* trigger task 3 */

        case TASK_STARTUP:
        case 1:
        case 3:
        case TASK_SHUTDOWN:
            return 200;     /* idle */

        default:
            TEST_FAIL_MESSAGE("unexpected ms_later");
        }
    }
    uint8_t task3(uint8_t ms_later)
    {
        switch(ms_later)
        {
        case TASK_STARTUP:
            TEST_ASSERT_EQUAL(0, milliseconds_timer);
            return 1;       /* Trigger task 1 */

        case 3:             /* test complete */
            TEST_ASSERT_EQUAL(3, milliseconds_timer);
            milliseconds_timer = 0;
            return TASK_SHUTDOWN;

        case TASK_SHUTDOWN:
            TEST_ASSERT_EQUAL(0, milliseconds_timer);
            /*FALLTHRU*/
        case 1:
        case 2:
            return 250;     /* idle */

        default:
            TEST_FAIL_MESSAGE("unexpected ms_later");
        }
    }

    /* Run tasks */
    test_task[0] = task1;
    test_task[1] = task2;
    test_task[2] = task3;
    milliseconds_timer = 0;
    task_main();
}

void test_startup_once(void)
{
    static bool startup[3];

    /* Callbacks */
    uint8_t task(unsigned index, uint8_t ms_later)
    {
        switch(ms_later)
        {
        case TASK_STARTUP:
            TEST_ASSERT_FALSE_MESSAGE(startup[index], "startup twice");
            startup[index] = true;
            return 100;
        case 100:
            /* Try to startup again! */
            return TASK_STARTUP;
        default:
            return TASK_SHUTDOWN;
        }
    }
    uint8_t task1(uint8_t ms_later) { return task(0, ms_later); }
    uint8_t task2(uint8_t ms_later) { return task(1, ms_later); }
    uint8_t task3(uint8_t ms_later) { return task(2, ms_later); }

    /* Initial state */
    (void)memset(startup, 0, sizeof(startup));

    /* Run tasks */
    test_task[0] = task1;
    test_task[1] = task2;
    test_task[2] = task3;
    task_main();

    /* Final state */
    TEST_ASSERT_TRUE(startup[0]);
    TEST_ASSERT_TRUE(startup[1]);
    TEST_ASSERT_TRUE(startup[2]);
}

void mock_sleep_cpu(void)
{
    if (interrupts_enabled)
    {
        /* Low power sleep */
        TEST_ASSERT_EQUAL(SLEEP_MODE_IDLE, sleep_mode_selected);

        /* Fake interrupts to wake CPU */
        milliseconds_timer++;
        MOCK_IRQ(TIMER0_COMPA_vect)();
    }
    else
    {
        /* Powering down */
        TEST_ASSERT_EQUAL(SLEEP_MODE_PWR_DOWN, sleep_mode_selected);
        powered_down = true;
        /* TEST ENDS */
    }
}

void mock_cli(void)
{
    interrupts_enabled = false;
}

void mock_sei(void)
{
    interrupts_enabled = true;
}

void mock_sleep_enable(void)
{
    sleep_enabled = true;
}

void mock_set_sleep_mode(enum sleep_mode mode)
{
    sleep_mode_selected = mode;
}

/*
 * We need to arrange to store 3 "tasks" in memory such that they are surrounded by
 * global symbols - see etc/linker.ld for details.
 * There's no way to reliably do this in 'C' so we use a little bit of hopefully
 * portable assembler.
 */
#if UINTPTR_MAX == 0xFFFFFFFFu
__asm__(
    "   .global __task_list_start   \n"
    "   .global __task_list_end     \n"
    "__task_list_start:             \n"
    "   .int    0x1DEFACED          \n" /* Task 1 'pointer' */
    "   .int    0x2DEFACED          \n" /* Task 2 'pointer' */
    "   .int    0x3DEFACED          \n" /* Task 3 'pointer' */
    "__task_list_end:               \n"
);
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
__asm__(
    "   .global __task_list_start   \n"
    "   .global __task_list_end     \n"
    "__task_list_start:             \n"
    "   .quad   0x1DEFACED          \n" /* Task 1 'pointer' */
    "   .quad   0x2DEFACED          \n" /* Task 2 'pointer' */
    "   .quad   0x3DEFACED          \n" /* Task 3 'pointer' */
    "__task_list_end:               \n"
);
#else
# error "Can't figure out unittest architecture"
#endif

/*
 * Task pointers between __task_list_start and __task_list_end are directed through
 * pgm_read_word_near() because AVR8 is a Harvard architecture. Here we turn them
 * into real pointers
 */
void* mock_pgm_read_word_near(const void* ptr)
{
    switch (*(uintptr_t*)ptr)
    {
    case 0x1DEFACED:
        return test_task[0];
    case 0x2DEFACED:
        return test_task[1];
    case 0x3DEFACED:
        return test_task[2];
    default:
        TEST_FAIL_MESSAGE("unexpected trampoline");
        return NULL;
    }
}
