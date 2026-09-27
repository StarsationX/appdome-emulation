# Native asset decryption

[aes_ctr.cpp](aes_ctr.cpp) implements the AES-CTR path used to decrypt native asset blobs. It reads the payload offset and initial counter from the blob header, generates the counter-mode keystream using the material in [ctr_tables.h](../replaceable/ctr_tables.h), and returns the decrypted payload to the asset loader. [aes_ctr.hpp](aes_ctr.hpp) exposes the decryption functions and header-offset configuration. For string databases, this is the native decryption stage; the application's existing Java code still handles database indexing, string construction, and caching.

Appdome uses an identifier at the start of each encrypted asset to select its decryption and handling path. My samples used `0x10ff1ce1`. This decryptor assumes that format; it does not dispatch on the identifier. I found [checks for other formats](../../../assets/images/HONORABLE.png), but didn't investigate them.
