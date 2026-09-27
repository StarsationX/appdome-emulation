from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Tuple


@dataclass
class ClassMap:
	obf_internal: str
	orig_internal: str
	member_map: Dict[str, str] = field(default_factory=dict)


def parse_mapping_text(text: str) -> List[ClassMap]:
	# #obf/pkg+orig/pkg^OrigClass,ObfClass,origMember,obfMember!...next class...
	entries = text.replace("\r", "").replace("\n", "").split("!")
	mappings: List[ClassMap] = []

	current_obf_pkg: str | None = None
	current_orig_pkg: str | None = None

	for raw_entry in entries:
		entry = raw_entry.strip()
		if not entry:
			continue

		if entry.startswith("#"):
			body = entry[1:]
			if "^" not in body or "+" not in body:
				continue
			pkg_part, class_part = body.split("^", 1)
			obf_pkg, orig_pkg = pkg_part.split("+", 1)
			current_obf_pkg = obf_pkg.strip("/")
			current_orig_pkg = orig_pkg.strip("/")
			entry = class_part

		if current_obf_pkg is None or current_orig_pkg is None:
			continue

		parts = [p for p in entry.split(",") if p]
		if len(parts) < 2:
			continue

		orig_class = parts[0]
		obf_class = parts[1]

		member_map: Dict[str, str] = {}
		member_tokens = parts[2:]
		for i in range(0, len(member_tokens) - 1, 2):
			orig_member = member_tokens[i]
			obf_member = member_tokens[i + 1]
			if obf_member and orig_member:
				member_map[obf_member] = orig_member

		mappings.append(
			ClassMap(
				obf_internal=f"{current_obf_pkg}/{obf_class}",
				orig_internal=f"{current_orig_pkg}/{orig_class}",
				member_map=member_map,
			)
		)

	return mappings


def read_mapping_file(path: Path) -> List[ClassMap]:
	raw = path.read_bytes()
	text = raw.decode("utf-8", errors="ignore")
	return parse_mapping_text(text)


def iter_smali_files(smali_root: Path) -> Iterable[Path]:
	return smali_root.rglob("*.smali")


def replace_descriptors(content: str, class_maps: List[ClassMap]) -> Tuple[str, int]:
	count = 0
	for m in class_maps:
		old_desc = f"L{m.obf_internal};"
		new_desc = f"L{m.orig_internal};"
		if old_desc in content:
			c = content.count(old_desc)
			content = content.replace(old_desc, new_desc)
			count += c
	return content, count


def replace_member_refs(content: str, class_maps: List[ClassMap]) -> Tuple[str, int]:
	total = 0
	for m in class_maps:
		obf_desc = f"L{m.obf_internal};"
		for obf_member, orig_member in m.member_map.items():
			pattern = re.compile(
				rf"({re.escape(obf_desc)}->){re.escape(obf_member)}(?=[(:])"
			)
			content, c = pattern.subn(rf"\1{orig_member}", content)
			total += c
	return content, total


def replace_member_decls(content: str, owning_map: ClassMap) -> Tuple[str, int]:
	total = 0
	for obf_member, orig_member in owning_map.member_map.items():
		# Method declarations: .method [flags] name(
		method_pat = re.compile(
			rf"(?m)^(\.method[^\n]*\s){re.escape(obf_member)}(\()"
		)
		content, c1 = method_pat.subn(rf"\1{orig_member}\2", content)
		total += c1

		# Field declarations: .field [flags] name:
		field_pat = re.compile(
			rf"(?m)^(\.field[^\n]*\s){re.escape(obf_member)}(:)"
		)
		content, c2 = field_pat.subn(rf"\1{orig_member}\2", content)
		total += c2
	return content, total


def detect_owner_map(content: str, by_obf_desc: Dict[str, ClassMap], by_orig_desc: Dict[str, ClassMap]) -> ClassMap | None:
	m = re.search(r"(?m)^\.class[^\n]*\s(L[^;]+;)", content)
	if not m:
		return None
	desc = m.group(1)
	return by_obf_desc.get(desc) or by_orig_desc.get(desc)


def process_smali_tree(smali_root: Path, class_maps: List[ClassMap]) -> None:
	by_obf_desc = {f"L{m.obf_internal};": m for m in class_maps}
	by_orig_desc = {f"L{m.orig_internal};": m for m in class_maps}

	files_changed = 0
	descriptor_changes = 0
	member_ref_changes = 0
	member_decl_changes = 0

	for smali_path in iter_smali_files(smali_root):
		original = smali_path.read_text(encoding="utf-8", errors="ignore")
		updated = original

		updated, c_ref = replace_member_refs(updated, class_maps)
		updated, c_desc = replace_descriptors(updated, class_maps)

		owner = detect_owner_map(updated, by_obf_desc, by_orig_desc)
		c_decl = 0
		if owner is not None:
			updated, c_decl = replace_member_decls(updated, owner)

		if updated != original:
			files_changed += 1
			descriptor_changes += c_desc
			member_ref_changes += c_ref
			member_decl_changes += c_decl
			smali_path.write_text(updated, encoding="utf-8")

	print(f"smali pass: files={files_changed}")
	print(f"class refs: {descriptor_changes}")
	print(f"member refs: {member_ref_changes}")
	print(f"member decls: {member_decl_changes}")


def rename_class_files(smali_root: Path, class_maps: List[ClassMap]) -> None:
	moves: List[Tuple[Path, Path]] = []

	for m in class_maps:
		src = smali_root / f"{m.obf_internal}.smali"
		dst = smali_root / f"{m.orig_internal}.smali"
		if src.exists() and src != dst:
			moves.append((src, dst))

	moves.sort(key=lambda t: len(str(t[0])), reverse=True)

	moved = 0
	skipped = 0
	for src, dst in moves:
		if dst.exists():
			skipped += 1
			continue
		moved += 1
		dst.parent.mkdir(parents=True, exist_ok=True)
		src.rename(dst)

	print(f"file moves: {moved}")
	print(f"move skips: {skipped}")


def process_natives_json(natives_json_path: Path, class_maps: List[ClassMap]) -> None:
	if not natives_json_path.is_file():
		print(f"no natives json at {natives_json_path}")
		return

	class_by_obf = {m.obf_internal: m for m in class_maps}
	class_by_orig = {m.orig_internal: m for m in class_maps}

	with natives_json_path.open("r", encoding="utf-8", errors="ignore") as f:
		data = json.load(f)

	if not isinstance(data, list):
		print(f"natives json looks off: {natives_json_path}")
		return

	class_changes = 0
	method_changes = 0
	rows_changed = 0

	for row in data:
		if not isinstance(row, dict):
			continue

		before_class = row.get("class_name")
		before_method = row.get("method_name")

		if not isinstance(before_class, str) or not isinstance(before_method, str):
			continue

		class_map = class_by_obf.get(before_class) or class_by_orig.get(before_class)
		if class_map is None:
			continue

		if before_class != class_map.orig_internal:
			row["class_name"] = class_map.orig_internal
			class_changes += 1

		orig_method = class_map.member_map.get(before_method)
		if orig_method and orig_method != before_method:
			row["method_name"] = orig_method
			method_changes += 1

		if row.get("class_name") != before_class or row.get("method_name") != before_method:
			rows_changed += 1

	print(f"natives rows: {rows_changed}")
	print(f"natives class names: {class_changes}")
	print(f"natives method names: {method_changes}")

	if rows_changed:
		with natives_json_path.open("w", encoding="utf-8") as f:
			json.dump(data, f, indent=2)
			f.write("\n")


def build_arg_parser() -> argparse.ArgumentParser:
	p = argparse.ArgumentParser(
		description="fix appdome class and member names from the mapping file"
	)
	p.add_argument(
		"--mapping",
		default="appdome_classes_obf_mapping.txt",
		help="mapping file",
	)
	p.add_argument(
		"--smali-root",
		default="classes",
		help="smali root",
	)
	p.add_argument(
		"--rename-files",
		action="store_true",
		help="move smali files too",
	)
	p.add_argument(
		"--natives-json",
		help="optional natives json",
	)
	return p


def main() -> int:
	args = build_arg_parser().parse_args()

	mapping_path = Path(args.mapping)
	smali_root = Path(args.smali_root)

	if not mapping_path.is_file():
		print(f"no mapping file at {mapping_path}")
		return 1
	if not smali_root.is_dir():
		print(f"no smali dir at {smali_root}")
		return 1

	class_maps = read_mapping_file(mapping_path)
	if not class_maps:
		print("couldnt parse anything from the mapping file")
		return 1

	print(f"found {len(class_maps)} mappings")
	process_smali_tree(smali_root, class_maps)

	if args.rename_files:
		rename_class_files(smali_root, class_maps)

	if args.natives_json:
		process_natives_json(Path(args.natives_json), class_maps)

	print("done")
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
