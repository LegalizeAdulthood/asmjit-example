#include <asmjit/x86.h>
#include <stdio.h>
 
using namespace asmjit;
 
// Signature of the generated function.
using SumFunc = int (*)(const int* arr, size_t count);
 
int main() {
  JitRuntime rt;                    // Create a runtime specialized for JIT.
  CodeHolder code;                  // Create a CodeHolder.
 
  code.init(rt.environment(),       // Initialize code to match the JIT environment.
            rt.cpuFeatures());
  x86::Assembler a(&code);          // Create and attach x86::Assembler to code.
 
  // Decide between 32-bit CDECL, WIN64, and SysV64 calling conventions:
  //   32-BIT - passed all arguments by stack.
  //   WIN64  - passes first 4 arguments by RCX, RDX, R8, and R9.
  //   UNIX64 - passes first 6 arguments by RDI, RSI, RDX, RCX, R8, and R9.
  x86::Gp arr, cnt;
  x86::Gp sum = x86::eax;           // Use EAX as 'sum' as it's a return register.
 
  if (ASMJIT_ARCH_BITS == 64) {
  #if defined(_WIN32)
    arr = x86::rcx;                 // First argument (array ptr).
    cnt = x86::rdx;                 // Second argument (number of elements)
  #else
    arr = x86::rdi;                 // First argument (array ptr).
    cnt = x86::rsi;                 // Second argument (number of elements)
  #endif
  }
  else {
    arr = x86::edx;                 // Use EDX to hold the array pointer.
    cnt = x86::ecx;                 // Use ECX to hold the counter.
    // Fetch first and second arguments from [ESP + 4] and [ESP + 8].
    a.mov(arr, x86::ptr(x86::esp, 4));
    a.mov(cnt, x86::ptr(x86::esp, 8));
  }
 
  Label Loop = a.newLabel();        // To construct the loop, we need some labels.
  Label Exit = a.newLabel();
 
  a.xor_(sum, sum);                 // Clear 'sum' register (shorter than 'mov').
  a.test(cnt, cnt);                 // Border case:
  a.jz(Exit);                       //   If 'cnt' is zero jump to 'Exit' now.
 
  a.bind(Loop);                     // Start of a loop iteration.
  a.add(sum, x86::dword_ptr(arr));  // Add int at [arr] to 'sum'.
  a.add(arr, 4);                    // Increment 'arr' pointer.
  a.dec(cnt);                       // Decrease 'cnt'.
  a.jnz(Loop);                      // If not zero jump to 'Loop'.
 
  a.bind(Exit);                     // Exit to handle the border case.
  a.ret();                          // Return from function ('sum' == 'eax').
  // ----> x86::Assembler is no longer needed from here and can be destroyed <----
 
  SumFunc fn;
  Error err = rt.add(&fn, &code);   // Add the generated code to the runtime.
 
  if (err) return 1;                // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----
 
  static const int array[6] = { 4, 8, 15, 16, 23, 42 };
 
  int result = fn(array, 6);        // Execute the generated code.
  printf("%d\n", result);           // Print sum of array (108).
 
  rt.release(fn);                   // Explicitly remove the function from the runtime
  return 0;                         // Everything successful...
}
