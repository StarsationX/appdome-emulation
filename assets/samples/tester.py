import hashlib
import json
from typing import Dict

blob_magic="XvzVOSiLDgDKmrXIMZJcprDFVbVUakNn"
def hash(s: str) -> str:
    return hashlib.sha256((s + blob_magic).encode()).hexdigest().upper()

def get_all_as_hashes(dec: str) -> Dict[str, str]:
    as_json = json.loads(dec)

    hash_map = {}
    for k, v in as_json.items():
        hashed = hash(v)
        hash_map[hashed] = v
    return hash_map

if __name__ == "__main__":
    with open("hashmap.json", "r") as f:
        hashmap = f.read()
    
    with open("string_dump.json", "r") as f:
        message = f.read()

    dec_map = get_all_as_hashes(message)
    for k, v in dec_map.items():
        print(f"{k} -> {v}")

    for k, v in dec_map.items():
        if k in hashmap:
            print(f"Found match for {k} : {v}")
