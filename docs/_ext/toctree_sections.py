# Shows the captions of nested toctrees in the sidebar.
#
# Sphinx renders the caption of a toctree only for the toctrees of the root document; the captions of the toctrees of
# other documents are dropped from the sidebar. This extension inserts each such caption as a heading before the first
# entry of its toctree, so that a page may split its children into sections that stay at the same level of the menu.

import html
import re

from docutils import nodes
from sphinx import addnodes


def _sections(app) -> dict[str, str]:
	# Maps the first document of every captioned toctree outside the root document to the caption.
	sections = {}
	for docname, toc in app.env.tocs.items():
		if docname == app.config.root_doc:
			continue
		for toctree in toc.findall(addnodes.toctree):
			caption = toctree.get("caption")
			entries = [ref for _, ref in toctree["entries"] if ref in app.env.all_docs]
			if caption and entries:
				sections[entries[0]] = caption
	return sections


def _add_sections(app, pagename, templatename, context, doctree) -> None:
	toctree = context.get("toctree")
	if toctree is None:
		return

	def toctree_with_sections(**kwargs) -> str:
		result = toctree(**kwargs)
		for docname, caption in _sections(app).items():
			href = "#" if docname == pagename else app.builder.get_relative_uri(pagename, docname)
			pattern = re.compile(rf'<li class="toctree-l(\d+)[^"]*">\s*<a [^>]*href="{re.escape(href)}"')
			match = pattern.search(result)
			if match:
				heading = f'<li class="toctree-section toctree-section-l{match.group(1)}" role="heading">{html.escape(caption)}</li>'
				result = result[:match.start()] + heading + result[match.start():]
		return result

	context["toctree"] = toctree_with_sections


def setup(app):
	app.connect("html-page-context", _add_sections)
	app.add_css_file("toctree_sections.css")
	return {"parallel_read_safe": True, "parallel_write_safe": True}
