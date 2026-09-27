# Packed native exports

This module restores packed native code on demand. [object_data_parser.cpp](object_data_parser.cpp) interprets the asset records containing code and restoration metadata, while [exports.cpp](exports.cpp) handles initialization, including signal handling, restoration of the packed `JNI_OnLoad`, and preparation of the trampoline region. These setup steps make the later export-resolution path available.

When a packed export reaches the resolver, [resolver.cpp](resolver.cpp) calculates its continuation address and asks [object_manager.cpp](object_manager.cpp) for a cached code allocation or a new one. In the ARM64 path, the allocation contains the restored code and a branch back through `x17`; `x16` supplies the allocation's entry address to the trampoline. This lets expanded code execute outside its original location. The source also contains architecture-specific branches, but the reported testing covered ARM64.
