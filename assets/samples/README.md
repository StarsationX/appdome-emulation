# Anonymized research samples

These are the anonymized samples used in the Appdome article. I've linked them where I discuss each mechanism so you can inspect the mappings, classes, and blobs as you read.

## Sample files

| File | What it supports |
| --- | --- |
| [hashmap.json](hashmap.json) | A 65-entry representation of configuration values and asset identifiers indexed by hashed names; this file comes from the second application in the collection |
| [tester.py](tester.py) | The logical-name plus salt hashing comparison used during analysis |
| [appdome_classes_obf_mapping.txt](appdome_classes_obf_mapping.txt) | Original and obfuscated class/member relationships; 59 class records and 232 member pairs under the recovery script's syntax |
| [recover_names.py](recover_names.py) | Name recovery and rewriting of smali references, declarations, and registration data |
| [native_methods_to_register_.json](native_methods_to_register_.json) | 26 native-registration records, including class, method, signature, and pointer fields |
| [arm64-v8a.json](arm64-v8a.json) | A second 26-record registration table |
| [string_dump.json](string_dump.json) | 1,053 recovered entries from native-string analysis |
| [classes.zip](classes.zip) | 52 recovered smali files |
| [decrypted_blobs_sample.zip](decrypted_blobs_sample.zip) | 37 payloads and two metadata files, including Java mappings, eight caller-prefixed string-database assets, and packed ARM64 code |

## Directly readable classes

I extracted these four classes from `classes.zip` without changing their contents so you can open them directly from the article.

| Class | Relevant methods and observations |
| --- | --- |
| [StringIndexer](recovered-classes/runtime/Strings/StringIndexer.smali) | `_getString`, `_internalGetString`, `getStringDbForPackage`, `_getStringFromDB`, and `base62ToInt`; separate caches for strings and database bytes |
| [NativeBridge](recovered-classes/runtime/loading/NativeBridge.smali) | Fixed initialization call and native declarations, including the no-argument `getStringIndexerDB` |
| [Initialization class](recovered-classes/qwerty/asdfgh/zxcvbn.smali) | Declaration of the static native `lkjhgf()V` method |
| [ADConfig](recovered-classes/runtime/ADConfig/ADConfig.smali) | Delegation to native configuration getters |

The native caller-selection implementation lives in [callbacks.cpp](../../poc/src/natives/callbacks.cpp). It retrieves a class name from the stack trace, derives its package prefix, and uses that prefix to select the asset returned to Java.

## About these copies

The files come from two applications, mostly the first one where I encountered packed exports. `hashmap.json` comes from the other application. Treat them as separate examples, since they don't form one consistent build.

I removed application and author identifiers, cleaned up archive metadata, and renamed some entries and identifiers inside the payloads. Renamed assets may no longer match their original hashes, so these copies are for following the analysis rather than running the emulator.

The blobs are decrypted payloads with their encrypted-asset headers removed. The original assets I collected used the `0x10ff1ce1` format identifier. Appdome also recognized other identifiers, visible in [HONORABLE.png](../images/HONORABLE.png), but I didn't investigate those formats or collect samples of them.

I left out the full semantic solver and AES helper scripts because of their size. The extraction logs included in the blob archive are from the original research run.

[manifest.json](manifest.json) lists SHA-256 hashes for the nine sample files and four extracted classes, along with each class's path inside the archive. Original files and private notes stay outside this directory.
