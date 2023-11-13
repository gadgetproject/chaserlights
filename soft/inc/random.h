/*! \file random.h
 *
 *  \brief Random Number Generator API
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
 */

#include <stdint.h>

/**
 * @brief Add up to 8 bits of entropy to the RNG
 * @param seed random bit(s)
 * @note This adds to any existing entopy, rather than resetting the pseudo-
 *       random sequence. random_set(A) followed by B=random_get(C) won't
 *       produce repeatable results.
 */
void random_add(uint8_t seed);

/**
 * @brief Return a pseudo-random number
 * @param maximum number than should be returned
 * @returns a number in the range 0..maximum
 * @note random_get(0) pumps the random number generator (potentially adding
 *       some entropy) but always returns zero.
 */
uint8_t random_get(uint8_t maximum);
