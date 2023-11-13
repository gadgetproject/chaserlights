/*! \file test_random.c
 *
 *  \brief Random Number Generator unit test
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

#include "unity.h"     /* Framework */

#include "random.h"    /* Module under test */

#include <stdlib.h>    /* calloc, free */

void do_distribution(uint8_t maximum, unsigned iterations, float tolerance)
{
	unsigned* histogram = calloc((maximum+1), sizeof(unsigned));

	for (unsigned i = 0; i < iterations*(maximum+1); i++)
	{
		uint8_t value = random_get(maximum);
		TEST_ASSERT_LESS_OR_EQUAL_UINT(maximum, value);
		histogram[value]++;
	}

	/* All values within X of an even distribution */
	for (unsigned i = 0; i <= maximum; i++)
	{
		TEST_ASSERT_UINT_WITHIN((unsigned)(iterations*tolerance), iterations, histogram[i]);
	}

	free(histogram);
}

/**
 * @brief Test whole 8-bit entropy
 */
void test_distribution(void)
{
	do_distribution(255, 10000, 0.05);
}

/**
 * @brief Test clamping methodology doesn't skew distribution
 *        e.g. modulus would fail here
 */
void test_clamp(void)
{
	do_distribution(200, 10000, 0.05);
}

/**
 * @brief Test a die roll
 */
void test_die(void)
{
	do_distribution(5, 100000, 0.01);
}

/**
 * @brief Test a coin flip
 */
void test_coin(void)
{
	do_distribution(1, 1000000, 0.001);
}
