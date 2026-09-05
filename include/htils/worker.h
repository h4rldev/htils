#ifndef HTILS_WORKER_H
#define HTILS_WORKER_H

#ifndef HTILS_THREAD_SAFE
#error "htils workers are only available on thread-safe builds of htils"
#else

/***********************************/

#include <stdatomic.h>
#include <threads.h>

#include <htils/arena.h>
#include <htils/atomic_types.h>
#include <htils/basictypes.h>

/***********************************/

/**
 * @brief Result of one task slice.
 */
typedef enum {
  HTILS_WORKER_CONTINUE, // Run another slice.
  HTILS_WORKER_DONE,     // Finished; end the task.
} htils_worker_step_t;

/**
 * @brief Single-shot worker body.
 * @details Runs once on its own thread. @c userdata carries the task's inputs
 * and result (the caller pre-allocates result storage and points a field at
 * it). @c scratch is private working memory, cleared when the task returns.
 * For a long task, poll @ref htils_worker_should_stop and return early.
 *
 * @param userdata The caller's task context (input + result).
 * @param scratch Worker's scratch arena (cleared at task end).
 *
 * @pre
 * - @c userdata must be pre-allocated for both writing and reading, unless the
 * worker single-shot is just being used for something context-less.
 * - @c scratch must be valid and cannot be `null`.
 */
typedef void (*htils_worker_fn_t)(void *userdata, arena_t *scratch);

/**
 * @brief Sliced task body.
 * @details Run by a library-owned loop that calls this repeatedly and checks
 * stop/pause between slices, so the body never polls. Do one bounded unit of
 * work per call and return. @c scratch persists across slices and is cleared
 * only when the task ends.
 *
 * @param userdata The caller's task context (input + result).
 * @param scratch Worker's scratch arena, kept between slices.
 *
 * @pre
 * - @c userdata must be pre-allocated for both writing and reading, unless the
 * task is context-less.
 * - @c scratch must be valid and cannot be `null`.
 *
 * @return @ref HTILS_WORKER_CONTINUE for another slice,
 *         @ref HTILS_WORKER_DONE to finish.
 */
typedef htils_worker_step_t (*htils_worker_task_t)(void *userdata,
                                                   arena_t *scratch);

/**
 * @brief Worker options.
 * @param detach Fire-and-forget; skip join, poll running 'til false.
 */
typedef struct htils_worker_config {
  b32 detach;
} htils_worker_config_t;

/**
 * @brief A reusable worker slot.
 * @details Caller-owned, one per concurrent task. Each slot owns one scratch
 * arena, created lazily and cleared at the end of every task. A slot must not
 * be respawned while still running. It runs either a single-shot @c fn or a
 * sliced @c task (exactly one is set).
 *
 * @param thread The worker thread.
 * @param scratch Private temp arena; cleared each task, reused on respawn.
 * @param running Cleared when the task finishes or is stopped.
 * @param pause Pauses a sliced task between slices.
 * @param detached Whether the worker is fire-and-forget.
 * @param fn The active single-shot body, or null.
 * @param task The active sliced body, or null.
 * @param userdata The active task context.
 */
typedef struct htils_worker {
  thrd_t thread;
  arena_t *scratch;
  atomic_b32 running;
  atomic_b32 pause;
  b32 detached;
  htils_worker_fn_t fn;
  htils_worker_task_t task;
  void *userdata;
  mtx_t pause_mtx; // guards pause awaits
  cnd_t pause_cnd; // signals on resume/stop
} htils_worker_t;

//
//
//

/**
 * @brief Default worker options (Detachable).
 */
htils_worker_config_t htils_worker_config_default(void);

//
//
//

/**
 * @brief Spawns a single-shot worker.
 * @details Runs @c fn once on a new thread. The slot must not already be
 * running.
 *
 * @param worker The slot to initialize.
 * @param config Options, or null for defaults.
 * @param fn The single-shot function to run.
 * @param userdata The task context passed to @c fn.
 *
 * @pre
 * - @c worker is a valid, non-running slot.
 * - @c fn is not null.
 *
 * @return true on success, false if the thread couldn't be created.
 */
b32 htils_worker_spawn(htils_worker_t *worker,
                       const htils_worker_config_t *config,
                       htils_worker_fn_t fn, void *userdata);

//
//
//

/**
 * @brief Spawns a sliced worker task.
 * @details Runs @c task in a library-owned loop that checks stop/pause between
 * slices, so the body never polls. @c task returns @ref HTILS_WORKER_CONTINUE
 * to keep going or @ref HTILS_WORKER_DONE to finish.
 *
 * @param worker The slot to initialize.
 * @param config Options, or null for defaults.
 * @param task The sliced task to run.
 * @param userdata The task context passed to @c task.
 *
 * @pre
 * - @c worker is a valid, non-running slot.
 * - @c task is not null.
 *
 * @return true on success, false if the thread couldn't be created.
 */
b32 htils_worker_spawn_task(htils_worker_t *worker,
                            const htils_worker_config_t *config,
                            htils_worker_task_t task, void *userdata);

//
//
//

/**
 * @brief Ask a worker to stop.
 * @details Clears the running flag. A sliced task stops at the next slice
 * boundary; a single-shot body observes it via @ref htils_worker_should_stop.
 * Non-blocking.
 *
 * @param worker The worker to signal.
 */
void htils_worker_request_stop(htils_worker_t *worker);

//
//
//

/**
 * @brief Pause or resume a sliced worker.
 * @details While paused, the worker loop parks between slices 'til resumed or
 * stopped. No effect on single-shot workers.
 *
 * @param worker The worker to pause.
 * @param paused Whether to pause (true) or resume (false).
 *
 * @pre @c worker is a valid, running slot.
 */
void htils_worker_set_paused(htils_worker_t *worker, b32 paused);

//
//
//

/**
 * @brief Check if a worker is running a task.
 *
 * @param worker The worker to check.
 *
 * @return true if the worker is running a task, false if not.
 */
b32 htils_worker_running(const htils_worker_t *worker);

//
//
//

/**
 * @brief Check if a sliced worker is paused.
 *
 * @param worker The worker to check.
 *
 * @return true if the worker is paused, false if not.
 */
b32 htils_worker_paused(const htils_worker_t *worker);

//
//
//

/**
 * @brief Checks whether a stop was requested (Polled by single-shot workers)
 *
 * @param worker The worker to check.
 *
 * @return true if the worker was requested to stop, false if not.
 */
b32 htils_worker_should_stop(const htils_worker_t *worker);

//
//
//

/**
 * @brief Waits for the task to finish.
 * @details Joins the thread. Call exactly once for a non-detached worker; a
 * no-op for detached workers (poll running instead).
 *
 * @param worker The worker to join.
 */
void htils_worker_join(htils_worker_t *worker);

#endif // !HTILS_THREAD_SAFE
#endif // !HTILS_WORKER_H
