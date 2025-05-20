set pagination off
set print pretty on

# Initialize tracking variables
set $cycle = 1

break async_context_base_remove_when_pending_worker
commands
  silent
  printf "\n[Cycle: %d] remove worker: %p\n", $cycle, worker
  printf "[TIMESTAMP] Exit: %llu\n", timestamp_exit
  info args
  info address worker
  x/16wx worker
  set $w = (async_when_pending_worker_t *)worker
  p *$w
  if ($w->do_work != 0x1000a499)
    printf "\n[CORRUPTED] do_work changed from 0x1000a499 to %p!\n", $w->do_work
  end
  # Log but don't break for these fields
  if ($w->next != 0 && ($w->next < 0x20000000 || $w->next > 0x20042000))
    printf "\n[WARNING] next pointer invalid: %p\n", $w->next
  end
  if ($w->work_pending > 1)
    printf "\n[WARNING] work_pending invalid: %d\n", $w->work_pending
  end
  bt
  set $cycle = $cycle + 1
  continue
end

break async_context_base_execute_once
commands
  silent
  set $worker = (async_when_pending_worker_t *)self->when_pending_list
  printf "\n[Cycle: %d] execute once: %p\n", $cycle, $worker
  printf "[TIMESTAMP] Enter: %llu\n", timestamp_enter
  if $worker != 0
    x/4wx $worker
    p *$worker
    # Print timestamp info if worker exists
  end
  continue
end
