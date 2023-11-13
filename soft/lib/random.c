/*! \file random.c
 *
 *  \brief Random Number Generator
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
 *
 * @note This random number generator is not FIPS compliant and should not
 *       be used for cryptographic purposes.
 *
 * It is based on a Galois LFSR over the prime polynomial
 *     x^32+x^28+x^19+x^18+x^16+x^14+x^11+x^10+x^9+x^6+x^5+x^1+1
 * https://www.partow.net/programming/polynomials/index.html#deg32
 *
 * This polynomial was chosen because
 * - It is prime, so the random-number sequence repeats every 2^32-1 cycles
 * - It has terms nicely spread over the 32-bit range which should distribute
 *   entropy faster.
 */

#include "random.h"

#define RANDOM_POLY3 0x10   /**< x^28 */
#define RANDOM_POLY2 0x0D   /**< x^19+x^18+x^16 */
#define RANDOM_POLY1 0x4E   /**< x^14+x^11+x^10+x^9 */
#define RANDOM_POLY0 0x63   /**< x^6+x^5+x^1+1 */

#define STATIC static

/** @note LFSR must never be zero or it will get stuck there */
STATIC uint8_t random_lfsr[4] = { 0, 0, 0, 0x80 };

/**
 * @brief Shift the LFSR
 * @note Ideally this should be in assembler, but that's harder to figure
 *       out what is going on and almost impossible to unit test.
 */
STATIC void random_pump(void)
{
    /* 32-bit shift left 1 bit */
    uint8_t carry, temp;
    carry = random_lfsr[0]>>7;
    random_lfsr[0] <<= 1;
    temp = random_lfsr[1]<<1 | carry;
    carry = random_lfsr[1]>>7;
    random_lfsr[1] = temp;
    temp = random_lfsr[2]<<1 | carry;
    carry = random_lfsr[2]>>7;
    random_lfsr[2] = temp;
    temp = random_lfsr[3]<<1 | carry;
    carry = random_lfsr[3]>>7;
    random_lfsr[3] = temp;

    /* feedback */
    if (carry)
    {
        random_lfsr[0] ^= RANDOM_POLY0;
        random_lfsr[1] ^= RANDOM_POLY1;
        random_lfsr[2] ^= RANDOM_POLY2;
        random_lfsr[3] ^= RANDOM_POLY3;
    }
}

void random_add(uint8_t seed)
{
    /* Sprinkle seed */
    uint8_t temp;
    temp = random_lfsr[0] ^= seed;
    temp |= random_lfsr[1] ^= seed;
    temp |= random_lfsr[2] ^= seed;
    temp |= random_lfsr[3] ^= seed;

    /* LFSR must never be zero or it will get stuck there */
    if (!temp)
    {
        random_lfsr[3] = 0x80;
    }

    /* Pump it through */
    for (unsigned i = 0; i < 8; i++)
    {
        random_pump();
    }
}

uint8_t random_get(uint8_t maximum)
{
    /* How many bits of entropy do we need? */
    uint8_t mask = 0;
    if (maximum)
    {
        for(mask = 0xFF; maximum <= (mask>>1); mask >>= 1);
    }

    /* Pump LFSR until a number in range is returned. On average this
     * should not take more than 2 iterations. */
    for(;;)
    {
        random_pump();

        uint8_t value = random_lfsr[0] & mask;
        if (value <= maximum)
        {
            return value;
        }
    }
}
