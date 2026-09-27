# Build-dependent material

These headers isolate extracted inputs associated with the protected application build. [blobs_magic.h](blobs_magic.h) defines `BLOBS_MAGIC`, the salt appended to logical asset names before hashing. [ctr_tables.h](ctr_tables.h) contains the AES material consumed by the native blob decryptor. The salt affects asset-name lookup and is separate from the material used for content decryption. Keeping these inputs together makes their role visible without mixing their values into the asset-loading and callback logic.
