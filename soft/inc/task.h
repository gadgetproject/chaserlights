/*! \file task.h
 *
 *  \brief Scheduler API
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

#include <stdint.h>

#define TASK_STARTUP ((uint8_t)255) /**< special value to initialise */
#define TASK_SHUTDOWN ((uint8_t)0)  /**< special value to shutdown */

/**
 * @brief Task cycle prototype
 *
 * @param [in] ms_later number of millisecond elapsed since last call or
 *             - @ref TASK_STARTUP on first call since boot
 *             - @ref TASK_SHUTDOWN on last call before reset.
 *
 * @returns requested maximum millisecond to call task again or @ref TASK_SHUTDOWN
 *          to request shutdown.
 *
 * @note the task runner may be called more frequently than the requested maximum
 *       and it is up to the task implementer to deal with this via the @ref ms_later
 *       value passed. It is guaranteed that @ref TASK_STARTUP and @ref TASK_SHUTDOWN
 *       values will only be passed once per session.
 */

typedef uint8_t (*task_cycle)(uint8_t ms_later);

/**
 * @brief Preprocessor and Linker magic to insert task cycle pointer into global task list
 * @param [in] task_cycle_ function pointer
 * @note see linker.ld for usage of .task_list section
 */
#define TASK_DECLARE(task_cycle_) TASK_DECLARE2(task_cycle_, __LINE__)
#define TASK_DECLARE2(task_cycle_, line_) TASK_DECLARE3(task_cycle_, line_)
#define TASK_DECLARE3(task_cycle_, line_) \
static volatile const task_cycle task_cycle_##line_ __attribute__((section(".task_list"))) = task_cycle_
