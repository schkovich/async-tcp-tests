break isr_hardfault
commands
  silent
  echo \n=========== HardFault detected ===========\n
  info reg
  echo \nStacked registers (r0‑r3,r12,lr,pc,xPSR):\n
  x/8wx $sp
  set $fault_pc = *((unsigned int *)$sp + 6)
  printf "\nCode around faulting PC (0x%08x):\n", $fault_pc
  disassemble $fault_pc-8, $fault_pc+8
  thread apply all backtrace
  echo \n==========================================\n
end