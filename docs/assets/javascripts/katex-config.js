document$.subscribe(function () {
  var mathElements = document.querySelectorAll(".arithmatex");
  for (var i = 0; i < mathElements.length; i++) {
    var el = mathElements[i];
    var tex = el.textContent.trim();
    var isDisplay = el.tagName === "DIV" || tex.startsWith("\\[");
    if (tex.startsWith("\\[") && tex.endsWith("\\]")) {
      tex = tex.slice(2, -2);
    } else if (tex.startsWith("\\(") && tex.endsWith("\\)")) {
      tex = tex.slice(2, -2);
    }
    if (tex) {
      try {
        katex.render(tex, el, { displayMode: isDisplay, throwOnError: false });
      } catch (e) {
        console.error("KaTeX render error:", e, tex);
      }
    }
  }
});
