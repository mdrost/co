# Sphinx configuration for the co documentation.
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re
import subprocess
import sys
from pathlib import Path

docs_dir = Path(__file__).resolve().parent
root_dir = docs_dir.parent
doxygen_xml_dir = docs_dir / "_doxygen" / "xml"

sys.path.insert(0, str(docs_dir / "_ext"))


def _read_project_version() -> str:
	cmake_lists = (root_dir / "CMakeLists.txt").read_text(encoding="utf-8")
	match = re.search(r"project\s*\(\s*co\b[^)]*?\bVERSION\s+([0-9][0-9.]*)", cmake_lists, re.S)
	return match.group(1) if match else "0.0"


def _run_doxygen() -> None:
	doxygen = os.environ.get("DOXYGEN_EXECUTABLE", "doxygen")
	subprocess.run([doxygen, "Doxyfile"], cwd=docs_dir, check=True)


_typedef_memberdef_re = re.compile(r'<memberdef kind="typedef".*?</memberdef>', re.S)
_opaque_handle_type_re = re.compile(r"<type>struct\s+(?:<ref [^>]*>)?(\w+)_t(?:</ref>)?\s*\*\s*</type>")
_opaque_handle_definition_re = re.compile(r"<definition>typedef struct \w+_t\s*\*\s*(\w+)</definition>")


def _fix_function_pointer_typedef(memberdef: str) -> str:
	# Newer Doxygen emits function-pointer typedefs as <type>R(*)</type><argsstring>(args)</argsstring>,
	# which Breathe renders as the unparsable "R(*) name(args)". Restore the split Breathe expects:
	# <type>R(*</type><argsstring>)(args)</argsstring> -> "R(* name)(args)".
	if "(*)</type>" not in memberdef:
		return memberdef
	memberdef = memberdef.replace("(*)</type>", "(*</type>", 1)
	return memberdef.replace("<argsstring>(", "<argsstring>)(", 1)


def _fix_opaque_handle_typedef(memberdef: str) -> str:
	# Handles are declared as "typedef struct X_t* X;" where X_t is private.
	# Render them as "typedef unspecified X" so the private struct does not leak into the docs.
	match = _opaque_handle_type_re.search(memberdef)
	if not match or f"<name>{match.group(1)}</name>" not in memberdef:
		return memberdef
	memberdef = memberdef[:match.start()] + "<type>unspecified</type>" + memberdef[match.end():]
	return _opaque_handle_definition_re.sub(r"<definition>typedef unspecified \1</definition>", memberdef, 1)


def _fix_typedef(match: re.Match) -> str:
	memberdef = match.group(0)
	memberdef = _fix_function_pointer_typedef(memberdef)
	return _fix_opaque_handle_typedef(memberdef)


_memberdef_re = re.compile(r"<memberdef .*?</memberdef>", re.S)
_qualifiedname_re = re.compile(r"<qualifiedname>(\w+(?:::\w+)+)</qualifiedname>")


def _qualify_group_member(match: re.Match) -> str:
	# Breathe skips group compounds when building a member's scope, so namespace members documented
	# in a @defgroup lose their namespace (e.g. "co::timer_0" is rendered as "timer_0").
	# Use the qualified name Doxygen already provides.
	memberdef = match.group(0)
	qualified = _qualifiedname_re.search(memberdef)
	if not qualified:
		return memberdef
	short_name = qualified.group(1).rsplit("::", 1)[1]
	return memberdef.replace(f"<name>{short_name}</name>", f"<name>{qualified.group(1)}</name>", 1)


_empty_kind_sectiondef_re = re.compile(r'\s*<sectiondef kind="">.*?</sectiondef>', re.S)


def _fix_doxygen_xml() -> None:
	for xml_file in doxygen_xml_dir.glob("*.xml"):
		text = xml_file.read_text(encoding="utf-8")
		fixed = _typedef_memberdef_re.sub(_fix_typedef, text)
		if xml_file.name.startswith("group__"):
			# Doxygen copies the friends of a class in a @defgroup into the group as a section without a kind,
			# which Breathe rejects. They are documented with their class.
			fixed = _empty_kind_sectiondef_re.sub("", fixed)
			fixed = _memberdef_re.sub(_qualify_group_member, fixed)
		if fixed != text:
			xml_file.write_text(fixed, encoding="utf-8")


_run_doxygen()
_fix_doxygen_xml()

# -- Project information -----------------------------------------------------

project = "co"
author = "co contributors"
copyright = f"%Y, {author}"
version = release = _read_project_version()

# -- General configuration ---------------------------------------------------

extensions = [
	"breathe",
	"toctree_sections",
]

exclude_patterns = ["_build", "_doxygen", "Thumbs.db", ".DS_Store"]

# Signatures longer than this are rendered with each parameter on its own line; 1 splits every signature with parameters.
maximum_signature_line_length = 1

# -- Breathe -----------------------------------------------------------------

breathe_projects = {"co": str(doxygen_xml_dir)}
breathe_default_project = "co"
breathe_domain_by_extension = {"h": "c", "hpp": "cpp"}

# -- HTML output -------------------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
html_css_files = ["signatures.css"]
