watch ((async_context_threadsafe_background_t*)0x20001590)->lock_mutex.owner
commands
  silent
  set $self = (async_context_threadsafe_background_t*)0x20001590
  set $caller_core = get_core_num()
  printf "\n=== Mutex Owner Change [core%d] ===\n", $caller_core
  printf "At: %s\n", $pc == 0 ? "startup" : $_function()
  printf "Old owner: %d\n", $old
  printf "New owner: %d\n", $self->lock_mutex.owner
  if $self->lock_mutex.enter_count > 0
    printf "Enter count: %d\n", $self->lock_mutex.enter_count
  end
  printf "=================\n"
  continue
end
