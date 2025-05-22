set $cycle = 1
set $do_work = 0
set $next = 0

break async_context_base_remove_when_pending_worker
commands
  silent
  printf "\n[Cycle: %d] remove worker: %p\n", $cycle, worker
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  x/4wx worker
  set $worker = (async_when_pending_worker_t *)worker
  p *$worker
  if ($worker->do_work != $do_work)
    printf "[ERROR] do_work changed from %p to %p!\n", $do_work, $worker->do_work
    bt
  end
  if ($worker->next != $next)
    printf "[ERROR] next pointer changed from %p to %p!\n", $next, $worker->next
    bt
  end
  if ($worker->work_pending > 1)
    printf "[WARNING] work_pending invalid: %d\n", $worker->work_pending
  end
  set $cycle = $cycle + 1
  continue
end

break async_context_base_execute_once
commands
  silent
  set $worker = (async_when_pending_worker_t *)self->when_pending_list
  printf "\n[Cycle: %d] execute once: %p\n", $cycle, $worker
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  if $worker != 0
    x/4wx $worker
    p *$worker
    if $do_work == 0 && $next == 0
      set $do_work = $worker->do_work
      set $next = $worker->next
    end
  end
  continue
end
