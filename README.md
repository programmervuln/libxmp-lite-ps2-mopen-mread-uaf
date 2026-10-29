# libxmp-lite-ps2-mopen-mread-uaf
Heap Use-After-Free in a Memory-Read Reproducer (CWE-416)

Overview

This repository documents a heap use-after-free (UAF) demonstrated by a minimal C reproducer involving a memory-backed file abstraction.

AddressSanitizer (ASan) reports a heap-use-after-free read in memcpy() after the backing buffer has been freed.

Vulnerability class: CWE-416 — Use After Free
Affected operation: Memory-backed read (mread)
Detection tool: AddressSanitizer
Status: Reproducer confirmed; upstream vulnerability status not yet verified

Technical Details

The reproducer defines an MFILE structure containing a pointer to a backing buffer, a current position, and a size.

The mopen() function allocates the MFILE structure and stores the supplied buffer pointer. It does not copy the buffer.

The following sequence triggers the reported error:

Allocate a 256-byte heap buffer.

Initialize an MFILE object using the buffer.

Free the backing buffer.

Call mread() using the MFILE object.

mread() passes a pointer into the freed buffer to memcpy(), resulting in a heap-use-after-free read.

Proof of Concept

The relevant code sequence is:

uint8_t *buf = malloc(256);
if (!buf) return 1;

MFILE *mf = mopen(buf, 256);

free(buf);

uint8_t tmp[16];
mread(tmp, 1, 16, mf);

mclose(mf);

The mread() implementation performs the read as follows:

memcpy(buf, m->start + m->pos, should_read);

Because m->start still points to the freed backing buffer, the read accesses deallocated memory.

AddressSanitizer Evidence

The supplied ASan log reports:

ERROR: AddressSanitizer: heap-use-after-free
READ of size 16

#0 in __asan_memcpy
#1 in mread .../uaf_poc.c:29
#2 in main .../uaf_poc.c:58

freed by thread T0 here:
#0 in free
#1 in main .../uaf_poc.c:54

previously allocated by thread T0 here:
#0 in __interceptor_malloc
#1 in main .../uaf_poc.c:48

The report identifies the allocation, deallocation, and subsequent read of the same heap region.

Reproduction

Compile with GCC or Clang and AddressSanitizer:

gcc -g -O1 -fsanitize=address \
    -fno-omit-frame-pointer \
    uaf_poc.c -o uaf_poc

Run the reproducer:

./uaf_poc

To save the sanitizer output:

./uaf_poc 2> asanlog.txt

The expected result is an AddressSanitizer report identifying a heap-use-after-free read in memcpy().

Exact output and source line numbers may vary by compiler and build configuration.

Security Considerations

A use-after-free is a memory-safety defect that may cause a process crash or other undefined behavior. 
Its actual security impact depends on the affected application, the execution path, 
and whether an attacker can trigger the invalid access.

The current reproducer establishes the memory-safety error in the supplied test program. 
It does not, by itself, establish remote exploitability, arbitrary code execution, or a 
vulnerability in an upstream release.

Scope and Limitations

The supplied reproducer defines its own MFILE, mopen(), and mread() functions. Further 
investigation is required to determine whether the same lifetime-management defect exists 
in the actual upstream project implementation.

Before attributing this issue to libxmp-lite-ps2, verify:

The corresponding upstream source code and affected version.

Whether the real API or application call path can trigger the same invalid read.

Whether the behavior violates the documented API contract.

The security impact and the availability of a fix.

Suggested Repository Contents

.
├── README.md
├── uaf_poc.c
├── asanlog.txt
└── screenshots/
    ├── asan-run-1.png
    └── asan-run-2.png

Classification

CWE-416: Use After Free

This repository is a proof of concept for a heap use-after-free read. The relationship between the reproducer and any upstream vulnerability remains subject to source-code verification.
