# Appdome emulator source

This source reproduces the initialization, data interfaces, and packed-export restoration explored in the investigation. Start with [entry.cpp](entry.cpp): `JNI_OnLoad` registers the fixed `lkjhgf()V` initialization method, whose handler loads the configuration map and registers the remaining native methods. [appdome.hpp](appdome.hpp) holds shared declarations and constants used across the modules.

| Module | Role |
| --- | --- |
| [assets](assets/README.md) | Read `base.apk`, resolve logical asset names, and load blob contents or inline configuration values |
| [crypt](crypt/README.md) | Decrypt native asset payloads using the extracted AES material |
| [natives](natives/README.md) | Map JNI identities to handlers and supply data requested by Java |
| [exports](exports/README.md) | Parse packed code, prepare restoration, and resolve exports on demand |
| [replaceable](replaceable/README.md) | Isolate the filename-hashing salt and AES material associated with a protected build |

See the [proof-of-concept overview](../README.md) for the historical testing scope and build context. Each module README links to the files that implement its part of the process.
