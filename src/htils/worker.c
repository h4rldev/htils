#ifdef HTILS_THREAD_SAFE

/***********************************/

#include <stdio.h>
#include <threads.h>

#include <htils/arena.h>
#include <htils/worker.h>

/***********************************/

/**
 * @brief The worker entry point / thread lifecycle.
 * @details Initializes and runs the worker or task, and clears the scratch
 * arena when done.
 *
 * @param arg The worker to run.
 *
 * @return 0 on success, or -1 on failure.
 */
static int htils_worker_entry(void *arg) {
  htils_worker_t *worker = (htils_worker_t *)arg;

  if (worker->task) {
    while (atomic_load(&worker->running)) {
      mtx_lock(&worker->pause_mtx);
      while (atomic_load(&worker->pause) && atomic_load(&worker->running))
        cnd_wait(&worker->pause_cnd, &worker->pause_mtx);
      mtx_unlock(&worker->pause_mtx);

      if (!atomic_load(&worker->running))
        break;

      htils_worker_step_t step =
          worker->task(worker->userdata, worker->scratch);
      if (step == HTILS_WORKER_DONE)
        break;
    }
  } else if (worker->fn)
    worker->fn(worker->userdata, worker->scratch);

  if (worker->scratch)
    arena_clear(worker->scratch);

  atomic_store(&worker->running, false);
  return 0;
}

//
//
//

/**
 * @brief Starts and initializes a worker.
 * @details Creates a new scratch arena, and starts the worker thread, making
 * sure each required field is set.
 *
 * @param worker The worker to start.
 * @param config Worker options, or null for defaults.
 *
 * @return true on success, false if the thread couldn't be created.
 */
static b32 htils_worker_start(htils_worker_t *worker,
                              const htils_worker_config_t *config) {
  if (!worker) {
    fprintf(stderr, "[htils] worker is null\n");
    return false;
  }

  if (htils_worker_running(worker)) {
    fprintf(stderr, "[htils] worker is already running\n");
    return false;
  }

  htils_worker_config_t defaults = htils_worker_config_default();
  if (!config)
    config = &defaults;

  worker->detached = config->detach;
  atomic_store(&worker->running, true);
  atomic_store(&worker->pause, false);

  if (!worker->scratch)
    worker->scratch = arena_new(GiB(1), MiB(1));

  if (mtx_init(&worker->pause_mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "[htils] failed to create worker mutex for worker %p\n",
            (void *)worker);
    return false;
  }

  if (cnd_init(&worker->pause_cnd) != thrd_success) {
    fprintf(stderr, "[htils] failed to create worker condition for worker %p\n",
            (void *)worker);
    return false;
  }

  if (thrd_create(&worker->thread, htils_worker_entry, worker) !=
      thrd_success) {
    fprintf(stderr, "[htils] failed to create worker thread for worker %p\n",
            (void *)worker);
    atomic_store(&worker->running, false);
    return false;
  }

  if (config->detach)
    thrd_detach(worker->thread);

  return true;
}

//
//
//

htils_worker_config_t htils_worker_config_default(void) {
  return (htils_worker_config_t){0}; // 0 == false
}

b32 htils_worker_spawn(htils_worker_t *worker,
                       const htils_worker_config_t *config,
                       htils_worker_fn_t fn, void *userdata) {
  if (!worker || !fn) {
    fprintf(stderr, "[htils] worker or fn is null\n");
    return false;
  }

  worker->fn = fn;
  worker->task = null;
  worker->userdata = userdata;
  return htils_worker_start(worker, config);
}

b32 htils_worker_spawn_task(htils_worker_t *worker,
                            const htils_worker_config_t *config,
                            htils_worker_task_t task, void *userdata) {
  if (!worker || !task) {
    fprintf(stderr, "[htils] worker or task is null\n");
    return false;
  }

  worker->fn = null;
  worker->task = task;
  worker->userdata = userdata;
  return htils_worker_start(worker, config);
}

void htils_worker_request_stop(htils_worker_t *worker) {
  if (!worker)
    return;
  atomic_store(&worker->running, false);
  mtx_lock(&worker->pause_mtx); // wake a paused worker so it can exit
  cnd_signal(&worker->pause_cnd);
  mtx_unlock(&worker->pause_mtx);
}

void htils_worker_set_paused(htils_worker_t *worker, b32 paused) {
  if (!worker)
    return;
  atomic_store(&worker->pause, paused ? true : false);
  if (!paused) {
    mtx_lock(&worker->pause_mtx);
    cnd_signal(&worker->pause_cnd);
    mtx_unlock(&worker->pause_mtx);
  }
}

b32 htils_worker_running(const htils_worker_t *worker) {
  if (!worker)
    return false;

  return (b32)atomic_load(&worker->running);
}

b32 htils_worker_paused(const htils_worker_t *worker) {
  if (!worker)
    return false;

  return (b32)atomic_load(&worker->pause);
}

b32 htils_worker_should_stop(const htils_worker_t *worker) {
  if (!worker)
    return true;

  return !(b32)atomic_load(&worker->running);
}

void htils_worker_join(htils_worker_t *worker) {
  if (!worker || worker->detached)
    return;
  thrd_join(worker->thread, null);
}

#endif // !HTILS_THREAD_SAFE
