# Zomathon

**1. Project Overview**
A high-performance C-based signal denoiser designed to correct noisy Kitchen Preparation Time (KPT) predictions under chaotic order conditions. The system simulates 10,000+ live orders and evaluates algorithmic latency and prediction accuracy improvement.

**2. Problem Statement**
Baseline KPT predictions suffer from significant error under burst traffic and merchant load variability.
Baseline MAE: 4.08 minutes per order

**3. Solution Approach**
• Hybrid signal correction model
• Robust CSV ingestion with validation checks
• Defensive memory handling (malloc checks, safe parsing)
• Performance-tuned C implementation
• Optimized build configuration (-O3)

**4. Performance Benchmark**
Dataset Size: 10,000 orders
Optimized Build (-O3): ~0.2–0.4 ms latency
Valgrind Instrumented Run: ~35–56 ms (instrumentation overhead)
Baseline MAE: 4.08 minutes
Hybrid MAE: 0.56 minutes
Accuracy Improvement: 86.36%

**5. Memory Safety Validation**
Validated using Valgrind (Memcheck) with full leak detection and origin tracking.
Command Used:
valgrind --leak-check=full --track-origins=yes ./signaldenoiser
Valgrind Summary Results:
• In use at exit: 0 bytes in 0 blocks
• Total heap usage: 5 allocs, 5 frees, 1,285,592 bytes allocated
• All heap blocks were freed -- no leaks are possible
• ERROR SUMMARY: 0 errors from 0 contexts

**6. Build & Run Instructions**
Compile (Optimized Build):
gcc -O3 signaldenoiser.c -o signaldenoiser -lm
Run:
./signaldenoiser
Compile (Debug Build for Valgrind):
gcc -g -Wall -Wextra signaldenoiser.c -o signaldenoiser -lm
Run Memory Check:
valgrind --leak-check=full --track-origins=yes ./signaldenoiser
