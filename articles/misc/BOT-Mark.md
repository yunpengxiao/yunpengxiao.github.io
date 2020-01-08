# BOT Mark

> BOT Mark is a BOT Man flavored Markdown specification.

[align-center]

## [Live Demo](BOT-Mark-Demo.md)

Click **[HERE](https://BOT-Man-JL.github.io/articles/?post=misc/BOT-Mark-Demo)** if you are from `GitHub.com`...

## Specs

Original markdown spec follows [chjj `marked`](https://github.com/chjj/marked).
Here we list the additional specs of _BOT Mark_:

> `<p>...</p>` means `...` must be surrounded by blank lines.

- auto heading numbering
  - enabled by `<p>[heading-numbering]</p>`
- heading setting
  - `[no-number]` => no number for this heading
  - `[no-toc]` => not add this heading to TOC
- TOC
  - `<p>[TOC]</p>` => TOC list
- LaTeX math (powered by [`KaTeX`](https://github.com/Khan/KaTeX))
  - `$$...$$`, `\[...\]` => block display math
  - `$...$`, `\(...\)` => inline display math
- citation
  - `[...]:` (note) => `^^^ [i]`
  - `[...]` (reference) => `[i]` (superscript)
  - use code/pre to escape
- cross-reference
  - `<p>[...||...]</p>` (target) => invisible anchor
  - `[...|...]` (item) => `i` (target index)
  - `[sec|...]` (item) => `§ NUMBER/TEXT` (section)
  - use code/pre to escape
- slide
  - enabled by `<p>[slide-mode]</p>`
  - `---` or `<hr>` or `<hr/>` => slide wrapper
  - slide wrapper would separate article into slides
- keyword
  - set next element's style (excluding keywords / style setters)
  - adjacent tags will be merged (still affect next valid element)
  - `<p>[page-break]</p>` => page break when print
  - `<p>[float-left]</p>`, `<p>[float-right]</p>` => float
  - `<p>[align-left]</p>`, `<p>[align-right]</p>`, `<p>[align-center]</p>` => alignment
  - `<p>[cite-sec]</p>` => citation style
  - `<p>[not-print]</p>` => not print (next element)
- style setter
  - `<p>[TAG=STYLE]</p>`
  - set next TAG's (if present) style to STYLE

## Renderer

> BOT Mark is rendered by [MarkdownRenderer](../../javascripts/bot-mark.js).

### `MarkdownRenderer.prototype.anchor`

- `function (text) => anchor`
- encode raw `text` to `anchor` format

### `MarkdownRenderer.prototype.heading`

- `function (level, headingNumber, anchor, text) => html`
- render heading to `html`

### `MarkdownRenderer.prototype.renderHeading`

- `function (mdHtml, tocArray) => mdHtml`
- render headings and patch tocArray

### `MarkdownRenderer.prototype.toc`

- `function (level, headingNumber, anchor, text) => html`
- render TOC item to `html`

### `MarkdownRenderer.prototype.tocTags`

- `['<div class="markdown-toc">', '</div>']`
- specify surrounding tag pair of TOC

### `MarkdownRenderer.prototype.renderToc`

- `function (mdHtml, tocArray) => mdHtml`
- render `<p>[TOC]</p>` with `tocArray`
- Note that it will omit first item by default

### `MarkdownRenderer.prototype.mathTags`

- `[ ['$', '$'], ['\\(', '\\)'], ['\\[', '\\]'], ['$$', '$$'] ]`
- specify LaTeX math tag pairs
- https://github.com/ViktorQvarfordt/marked

### `MarkdownRenderer.prototype.citeNote`

- `function (anchor, noteIndex) => html`
- render cite note into `html`

### `MarkdownRenderer.prototype.citeRef`

- `function (anchor, noteIndex, refIndex) => html`
- render cite reference into `html`, binding with cite note

### `MarkdownRenderer.prototype.citeDeref`

- `function (anchor, refIndex) => html`
- render cite de-reference into `html`, binding with corresponding cite note and cite reference

### `MarkdownRenderer.prototype.renderCitation`

- `function (mdHtml) => mdHtml`
- render `[...]:` as cite notes
- render `[...]` as cite references
- add de-references for each note

### `MarkdownRenderer.prototype.refTarget`

- `function (type, value) => html`
- render reference `value` with its `type` into target `html`

### `MarkdownRenderer.prototype.refItem`

- `function (type, value, index) => html`
- render reference item into `html`, binding with its target

### `MarkdownRenderer.prototype.refSection`

- `function (anchor, text) => html`
- render reference item into `html`, binding with its section heading

### `MarkdownRenderer.prototype.renderReference`

- `function (mdHtml, tocArray) => html`
- render `<p>[...||...]</p>` as reference targets
- render `[...|...]` as reference items
- render `[sec|...]` as spec for section heading
- detect invalid reference items

### `MarkdownRenderer.prototype.slideTags`

- `['<div class="markdown-slide">', '</div>']`
- specify surrounding tag pair of slides

### `MarkdownRenderer.prototype.renderSlides`

- `function (mdHtml) => mdHtml`
- render `<hr>` tags into slide wrappers

### `MarkdownRenderer.prototype.keywordTags`

``` json
[
    "page-break", "float-left", "float-right",
    "align-left", "align-right", "align-center",
    "cite-sec", "not-print"
]
```

- specify keyword tags
- always use for control the style of next element

### `MarkdownRenderer.prototype.renderKeywordTags`

- `function (mdHtml) => mdHtml`
- render `<p>[KEYWORD]</p>` into `<div class="KEYWORD"></div>`

### `MarkdownRenderer.prototype.renderStyleSetters`

- `function (mdHtml) => mdHtml`
- detect `<p>[...=...]</p>` as style setters (TAG = STYLE)
- add STYLE to the next TAG

### `MarkdownRenderer.prototype.render`

- `function (mdText) => mdHtml`
- rendering pipeline
  - `marked`
  - `renderToc`
  - `renderCitation`
  - `renderReference`
  - `renderSlides`
  - `renderKeywordTags`
  - `renderStyleSetters`
