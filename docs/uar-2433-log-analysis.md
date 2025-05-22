# UAR Bug Analysis in async_context_threadsafe_background

## Memory Layout Context

### RP2040 Stack Configuration
- Each Cortex M0+ core has its own stack
- Stacks grow downward from high to low addresses
- Default configuration:
  - Single core: 8KB stack for core 0
  - Dual core: 4KB per core (shared from same 8KB)
  - Stacks placed in separate scratch banks (X/Y) for performance
- In our case: `bool core1_separate_stack = true;` gives each core 8KB

### Stack Usage Analysis
- Worker struct allocated at 0x20041f74
- Printf buffer pushing against worker memory
- Critical 48-byte alignment causing corruption
- Separate stack configuration prevents core0/core1 stack collision

## Debugging Setup

### 1. Timestamp Collection
```cpp
timestamp_enter = to_us_since_boot(get_absolute_time());
const auto rc = async_context_execute_sync(&async_ctx.core, do_some_work, &ref_counter);
timestamp_exit = to_us_since_boot(get_absolute_time());
```

### 2. GDB Instrumentation
```gdb
break async_context_base_execute_once
commands
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  x/4wx $worker
  p *$worker
end

break async_context_base_remove_when_pending_worker
commands
  printf "[INFO] Enter: %llu; Exit: %llu\n", timestamp_enter, timestamp_exit
  x/4wx worker
  p *$worker
end
```

## Findings

### 1. Stack Memory Pattern
```
0x20041f74 (Worker struct location):
|-------------------|
| next (4 bytes)    | <-- Corrupted to 0xa in cycle 11
| do_work (4 bytes) | <-- Remains valid at 0x1000a529
| work_pending (4)  | <-- Corrupted to 232 from cycle 1
| user_data (4)     |
|-------------------|
     ↑
Printf buffer (grows down)
- 47 chars (safe)
- 48 chars (aligns with worker)
```

### 2. Serial Output and Memory Impact
```
// String lengths affecting stack layout
Cycle 9:  "Counter 9; ..."    // 47 chars, stack safe
Cycle 10: "[Counter 10]..."   // 47 chars, stack safe
Cycle 11: "[Counter 11]..."   // 48 chars, fatal alignment
```

### 3. Memory Corruption Pattern
```
Initial state (cycle 1):
- work_pending corrupted (1->232)
- Stack overlap present but non-fatal
- Core fields (next/do_work) protected by alignment

Fatal state (cycle 11):
- String reaches 48 chars
- Perfect stack alignment
- next pointer corrupted
- Stack corruption becomes fatal
```

### 4. Stack Layout Analysis
- Worker struct (16 bytes) at 0x20041f74
- Printf buffer allocated above worker
- 48-byte boundary critical because:
  1. Stack is 8-byte aligned
  2. Printf buffer rounded up for alignment
  3. Counter digits affect buffer size
  4. At 48 bytes, buffer end perfectly aligns with worker start

### 5. Crash Analysis
```asm
// Fatal access in async_context_base_execute_once during list traversal:
0x1000a7be: ldr r3, [r5, #4]  // Try to read next field from corrupted worker (0xa)
0x1000a7c0: blx r3            // Never reached

Code context:
for(async_when_pending_worker_t *when_pending_worker = self->when_pending_list;
    when_pending_worker;  // r5 = 0xa here (corrupted next pointer)
    when_pending_worker = when_pending_worker->next) // Crash when trying to read next

Registers at crash:
r5 = 0xa        // This is the corrupted 'next' pointer we're trying to traverse
r0 = 0x20001560 // async_context pointer
```

## Key Insights
1. Stack Layout Significance:
   - 8KB per core (due to core1_separate_stack = true)
   - Stack alignment requirements affect corruption pattern
   - Worker struct position relative to printf buffer critical

2. UAR Manifestation:
   - work_pending corrupted to 232 immediately (early warning)
   - next field corrupted to 0xa when string hits 48 bytes
   - Crash happens when trying to traverse corrupted linked list
   - Fatal because of invalid memory access (0xa is not a valid address)

3. Corruption Chain:
   - Printf buffer (48 bytes) overwrites worker struct
   - Worker still in linked list but memory reused
   - Next traversal attempts to dereference 0xa
   - System crashes trying to read from invalid address

This analysis shows how a stack-based UAR vulnerability manifests through linked list corruption, where the crash occurs during list traversal rather than during worker execution.
