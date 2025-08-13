# Initialize list tracking
set $head = 0
set $count = 0

define push_state
  # Allocate memory for node (mutex + next pointer)
  # sizeof(recursive_mutex_t) + sizeof(void*)
  set $new = (void*)malloc(24)
  # Store mutex state
  set *(recursive_mutex_t*)$new = ((async_context_threadsafe_background_t*)self_base)->lock_mutex
  # Store next pointer after mutex
  set *(void**)($new + 16) = $head
  # Update head
  set $head = $new
  set $count = $count + 1
end

#break *0x1000a650
#break *0x20041fb0
break async_context_threadsafe_background_execute_sync
commands
  silent
  push_state
  continue
end
