/*! \file task.h
 *
 *  \brief Scheduler API stub for unit testing
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

#include "../../inc/task.h"

#undef TASK_DECLARE
/**
 * @brief Override task declaration so we can unit tests tasks
 */
#define TASK_DECLARE(task_cycle_) \
const task_cycle task_cycle_##_fn = task_cycle_

/**
 * @brief Access the task function declared above
 */
#define TASK_IMPORT(task_cycle_) extern const task_cycle task_cycle_##_fn

/**
 * @brief Execute the task function defined above
 */
#define TASK_CYCLE(task_cycle_) task_cycle_##_fn

/**
 * @brief Task loop
 */
void task_main(void);
