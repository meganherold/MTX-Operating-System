Megan's solution for problem 5.6 in KC's book:

Assume: In MTX, P1 is a Casanova process, which hops, not from bed to bed,
but from segment to segment. Initially, P1 runs in the segment 0×2000. By a
hop(u16 segment) syscall, it enter kernel to change segment. When it returns to
Umode, it returns to an IDENTICAL Umode image but in a different segment,
e.g. 0×4000. Devise an algorithm for the hop() syscall, and implement it in
MTX to satisfy the lusts of Casanova processes.
