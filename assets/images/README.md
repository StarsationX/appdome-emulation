# Investigation screenshots

These are the 20 screenshots used in the Appdome article, covering the asset system, parser, solver, and export resolver. The accompanying [samples](../samples/README.md) let you inspect the data and recovered classes discussed alongside them.

I covered identifying text with black boxes and left the rest of each image unchanged. In `MAP.png`, that includes the full symbol column, since the method names could identify the application.

| Image | Use in the investigation |
| --- | --- |
| [EXPOSED_SHARED_OBJECTS_STRING.png](EXPOSED_SHARED_OBJECTS_STRING.png) | Plaintext export-asset name that guided the investigation |
| [RESOLVER_CFG.png](RESOLVER_CFG.png) | Control-flow complexity encountered during analysis |
| [EXPORT_ENTRY.png](EXPORT_ENTRY.png) | Export indirection, with application-specific symbols covered |
| [TRAMPOLINE.png](TRAMPOLINE.png) | Register preservation and resolver transfer |
| [EXPORT_RESOLVER_SET.png](EXPORT_RESOLVER_SET.png) | Resolver-pointer initialization |
| [SIGNAL_HANDLER.png](SIGNAL_HANDLER.png) | Saved-context handling related to export resolution |
| [COMMON_XREF.png](COMMON_XREF.png) | Shared helper references in the analyzed execution paths |
| [ELF_RESOLV.png](ELF_RESOLV.png) | ELF resolver call excerpt |
| [LOOKUP.png](LOOKUP.png) | Object lookup logic |
| [OBJECTS_PARSER.png](OBJECTS_PARSER.png) | Parsing code-record structures |
| [MMAP.png](MMAP.png) | Allocation-related pseudocode |
| [IKNEWIT.png](IKNEWIT.png) | Identifying recovered code through a known call |
| [NICERLOOKING.png](NICERLOOKING.png) | Alternative disassembly view of the same region |
| [FIRSTVER.png](FIRSTVER.png) | Initial parser output |
| [SECOND_VER.png](SECOND_VER.png) | Parser output with saved object files |
| [MAP.png](MAP.png) | Object addresses and the captured match count; identifying symbol column covered |
| [FIRST_VER_SS.png](FIRST_VER_SS.png) | Reported reductions in an intermediate solver run |
| [FIRST_RESOLVER_ATTEMPT.png](FIRST_RESOLVER_ATTEMPT.png) | Early instrumented resolver attempt, with both author-label occurrences covered |
| [FIRST_VER_RESOLVER.png](FIRST_VER_RESOLVER.png) | Captured resolver activity |
| [HONORABLE.png](HONORABLE.png) | Asset-format identifier checks, including the investigated `0x10ff1ce1` format |
