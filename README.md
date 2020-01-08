# BOT Frame

> BOT Frame is a **pure browser-rendering** Blog Framework for _GitHub Pages_, based on [BOT Mark](articles/misc/BOT-Mark.md).

## Features

- pure **JavaScript**, pure **browser-rendering** in your browser
- **responsive** page design (_desktop_, _mobile_ and _print_)
- Support _**[BOT Mark](articles/misc/BOT-Mark.md)**_ 👈
  - `[TOC]`
  - number headings
  - LaTeX math (powered by KaTeX)
  - citation
  - cross-reference
  - slides
  - style-setters

## Get Started

- Write [static/config.json](static/config.json) for your basic info
- Design [static/home.md](static/home.md) as your home page
- Replace articles in `articles/` with yours
- Write [articles/archive.md](articles/archive.md) as the content table of your articles
- Update [favicon.ico](favicon.ico) as your icon
- Upload to your _GitHub Pages repository_
- Enjoy it 😉

## BOT Frame

### [static/config.json](static/config.json)

Set `name`, `github`, `email`, (`phone`), `pages` (navigation), `footer` to yours.

``` json
{
  "name": "name_in_title",
  "contact": {
    "name": "name_in_contact_pane",
    "github": {
      "text": "xxx",
      "link": "https://github.com/xxx",
      "img": "/static/github.svg"
    },
    "email": {
      "text": "xxx@xxx.com",
      "link": "mailto:xxx@xxx.com",
      "img": "/static/email.svg"
    },
    "phone": {
      "text": "+86-xxxx",
      "link": "tel:+86-xxx",
      "img": "/static/phone.svg"
    }
  },
  "pages": {
    "🏠": "/",
    "📝": "/articles/"
  },
  "footer": [
    "<a href='https://xxx.github.io'>https://xxx.github.io</a>",
    "xxx, 2017"
  ]
}
```

### Articles Page

- BOT Frame will detect if the page is an **_article_ or _archive_ page**.
  - without `post` query string => load [articles/archive.md](articles/archive.md) as an _archive_
  - with a `post` query string => add extension `.md` to `post`, and load that file as an _article_
- BOT Frame will resolve correct paths for **relative _articles_ and _images_**.
  - _articles_ => relative markdown file path that ending with `.md`
  - _miscellaneous_ => relative file path that NOT ending with `.md`
  - _images_ => relative image path inside `<img>` tags
- BOT Frame will detect **article title** of a markdown file.
  - starting with `H1` _heading_ => set heading text as title, move first two elements of article to left pane, and remove contact section
  - NOT starting with `H1` => set filename as title, and keep contact section
- Article page supports **print style** setting by adding `style=STYLES` to query string.
  - `cover` => print left pane as a single cover page
  - `toc-page-break` => page break after TOC
  - `two-column` => two column style (except left pane)
  - `STYLE+STYLE` <= style strings can be concatenated by `+`

## Credits

- **Markdown parser** by
  - [chjj](https://github.com/chjj/marked) - orginal
  - [ViktorQvarfordt](https://github.com/ViktorQvarfordt/marked) - patch
  - [BOT Man](javascripts/bot-mark.js) - flavor
- **Original theme** by [orderedlist](https://github.com/orderedlist/minimal)
- **GitHub markdown css** by [sindresorhus](https://github.com/sindresorhus/github-markdown-css)
