set $cycle = 1

break async_context_base_remove_when_pending_worker
commands
  silent
  printf "[Cycle: %d] remove worker: %p\n", $cycle, worker
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  x/4wx worker
  set $worker = (async_when_pending_worker_t *)worker
  p *$worker
  if ($worker->do_work != 0x1000a499)
    printf "[ERROR] do_work changed from 0x1000a499 to %p!\n", $worker->do_work
  end
  if ($worker->next != 0 && ($worker->next < 0x20000000 || $worker->next > 0x20042000))
    printf "[WARNING] next pointer invalid: %p\n", $worker->next
  end
  if ($worker->work_pending > 1)
    printf "[WARNING] work_pending invalid: %d\n", $worker->work_pending
  end
  bt
  set $cycle = $cycle + 1
  continue
end

break async_context_base_execute_once
commands
  silent
  set $worker = (async_when_pending_worker_t *)self->when_pending_list
  printf "[Cycle: %d] execute once: %p\n", $cycle, $worker
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  if $worker != 0
    x/4wx $worker
    p *$worker
  end
  continue
end
