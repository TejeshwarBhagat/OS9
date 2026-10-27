This packet targets **classic 32-bit xv6-public for x86**. Kernel interfaces and syscall numbers can vary between xv6 forks, so review `kernel_changes.md` against the exact source tree you use.

## Files

- `kernel_changes.md` - kernel implementation guidance for page-table printing, lazy allocation, and syscall wiring.
- `pttest.c` - prints the process page table and checks the four-page growth result.
- `faulttest.c` - touches five pages, reads the page-fault counter, and checks that an invalid child access does not kill its parent.
- `pagerepl.c` - simulates FIFO and LRU and prints frame states, hits, faults, and ratios.
- `report.pdf` - one-page assignment report with name and roll number.
- `illustrative_terminal_transcripts.pdf` - terminal-style expected-output mockups for all three questions. These are **not screenshots from a running xv6 kernel**.
- `expected_output.txt` - expected summary values.
- `SCREENSHOTS_STATUS.md` - explains why genuine xv6 screenshots are not included yet.

## Build and run in xv6

1. Apply the kernel changes described in `kernel_changes.md`. Choose syscall numbers unused by your xv6 tree and update all syscall tables and user stubs consistently.
2. Add `_pttest`, `_faulttest`, and `_pagerepl` to `UPROGS` in the xv6 `Makefile`.
3. Rebuild and boot xv6 using the build instructions for your tree.
4. Run:

   ```text
   pttest
   faulttest
   pagerepl 3 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
   pagerepl 4 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
   pagerepl 5 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
   ```

5. Capture genuine terminal screenshots of the runs and save them in this folder to complete the assignment evidence requirement.

For the required 20-reference string, the expected FIFO fault totals for 3, 4, and 5 frames are **15, 10, and 9**. The expected LRU totals are **12, 8, and 7**. Their hit ratios are shown in the PDFs and `expected_output.txt`.

## Screenshots

The `screenshots/` subfolder contains three JPG screenshots downloaded from the repository linked by the user. See `screenshots/README.md` for attribution. They are reference screenshots from that repository, not captures from the xv6 environment used for this packet.
