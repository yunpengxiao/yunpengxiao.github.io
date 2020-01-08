/*
	BOT-Mark - a BOT Man flavored Markdown renderer :-)

    - https://github.com/BOT-Man-JL/BOT-Man-JL.github.io
    - based on marked (Christopher Jeffrey, https://github.com/chjj/marked)
    - with math patch (ViktorQvarfordt, https://github.com/ViktorQvarfordt/marked)

	BOT Man, 2017 (MIT License)
*/

if (typeof require !== 'undefined')
    var marked = require('./marked');

if (typeof marked === 'undefined') {
    var errMsg = 'marked must be loaded before BOT-Mark';
    if (console) console.error(errMsg);
    if (alert) alert(errMsg);
}

function MarkdownRenderer() { }

// helpers

MarkdownRenderer.prototype._escapeRegExp = function (text) {
    return text.replace(/([.*+?^=!:${}()|\[\]\/\\])/g, '\\$1');
};

MarkdownRenderer.prototype._isInTag = function (mdHtml, tagName, pos) {
    var prevOpen = mdHtml.lastIndexOf('<' + tagName, pos);
    var prevClose = mdHtml.lastIndexOf('</' + tagName + '>', pos);
    var nextOpen = mdHtml.indexOf('<' + tagName, pos); if (nextOpen == -1) nextOpen = mdHtml.length;
    var nextClose = mdHtml.indexOf('</' + tagName + '>', pos); if (nextClose == -1) nextClose = mdHtml.length;
    return prevClose < prevOpen && nextClose < nextOpen;
};

MarkdownRenderer.prototype.anchor = function (text) {
    return text.toLowerCase()
        .replace(/[^\w\u4E00-\u9FFF]+/g, '-')
        .replace(/^-+/g, '')
        .replace(/-+$/g, '');
};

// TOC and headings

MarkdownRenderer.prototype.heading = function (level, headingNumber, anchor, text) {
    if (headingNumber != '') headingNumber += ' ';
    return '<h' + level + ' id="' + anchor + '">' +
        headingNumber + text + '</h' + level + '>\n';
};

MarkdownRenderer.prototype.renderHeading = function (mdHtml, headingArr) {
    var that = this;

    // Check heading-numbering
    var hasHeadingNumbering = false;
    mdHtml = mdHtml.replace(/<p>\[heading-numbering\]<\/p>/g, function () {
        hasHeadingNumbering = true;
        return '';
    });

    // Set headings
    var anchorMap = {};
    var headingIndice = [0, 0, 0, 0, 0, 0];
    var headingIndex = 0;
    mdHtml = mdHtml.replace(/<h1>HEADING_PLACEHOLDER<\/h1>/g, function () {
        var thisHeading = headingArr[headingIndex++];

        // Check additional flags
        thisHeading.noNumber = !hasHeadingNumbering;
        thisHeading.noToc = false;
        thisHeading.text = thisHeading.text.replace(/[\s]*\[([^\].]+)\][\s]*/g, function (text, flag) {
            if (flag == 'no-number') {
                thisHeading.noNumber = true;
                thisHeading.raw = thisHeading.raw.replace(/[\s]*\[no-number\][\s]*/g, '');
                return '';
            }
            if (flag == 'no-toc') {
                thisHeading.noToc = true;
                thisHeading.raw = thisHeading.raw.replace(/[\s]*\[no-toc\][\s]*/g, '');
                return '';
            }
            return text;
        });

        // Set anchor
        var anchor = that.anchor(thisHeading.raw);
        var count = (anchorMap[anchor] || 0) + 1;
        anchorMap[anchor] = count;
        if (count > 1) anchor += '_' + count;
        thisHeading.anchor = anchor;

        // Add heading numbers
        var headingNumber = '';
        if (!thisHeading.noNumber) {
            var index = thisHeading.level - 1;
            if (index < headingIndice.length && index != 0) {
                // Increase index
                ++headingIndice[index];

                // Set higher indice
                headingNumber += headingIndice[1];
                for (var i = 2; i <= index; i++)
                    headingNumber += '.' + headingIndice[i];

                // Clear lower indice
                for (var i = index + 1; i < headingIndice.length; i++)
                    headingIndice[i] = 0;
            }
        }
        thisHeading.headingNumber = headingNumber;

        return that.heading(thisHeading.level,
            thisHeading.headingNumber, thisHeading.anchor, thisHeading.text);
    });
    return mdHtml;
};

MarkdownRenderer.prototype.toc = function (level, headingNumber, anchor, text) {
    if (headingNumber != '') headingNumber += ' ';
    return '<p style="padding-left:' + level * 1.5 + 'em"><a href="#' +
        anchor + '">' + headingNumber + text + '</a></p>';
};

MarkdownRenderer.prototype.tocTags = ['<div class="markdown-toc">', '</div>'];

MarkdownRenderer.prototype.renderToc = function (mdHtml, headingArr) {
    // omit first toc item (H1 heading)
    var minLevel = 1024;
    for (var i = 1; i < headingArr.length; i++)
        minLevel = Math.min(minLevel, headingArr[i].level);

    var tocHtml = this.tocTags[0];
    for (var i = 1; i < headingArr.length; i++) {
        var thisHeading = headingArr[i];
        if (thisHeading.noToc) continue;

        tocHtml += this.toc(thisHeading.level - minLevel,
            thisHeading.headingNumber, thisHeading.anchor, thisHeading.text);
    }
    tocHtml += this.tocTags[1];

    return mdHtml.replace(/<p>\[TOC\]<\/p>/g, tocHtml);
};

// math tags

// Patch for math support
// Ref: https://github.com/ViktorQvarfordt/marked
MarkdownRenderer.prototype.mathTags = [
    ['$', '$'], ['\\(', '\\)'], ['\\[', '\\]'], ['$$', '$$']
];

// Math support could be implemented by KaTeX
// https://github.com/Khan/KaTeX

// citation

MarkdownRenderer.prototype.citeNote = function (anchor, noteIndex) {
    // note: align (index < 10), assuming (max index < 100)
    return '<span class="cite-note" id="cite-note-' + anchor + '">' +
        (noteIndex < 10 ? '&nbsp; ' : '') + '[' + noteIndex + ']</a></span>';
};

MarkdownRenderer.prototype.citeRef = function (anchor, noteIndex, refIndex) {
    return '<span class="cite-ref" id="cite-ref-' + anchor + '-' + refIndex +
        '"><a href="#cite-note-' + anchor + '">[' + noteIndex + ']</a></span>';
};

MarkdownRenderer.prototype.citeDeref = function (anchor, refIndex) {
    return '<span class="cite-deref"><a href="#cite-ref-' +
        anchor + '-' + refIndex + '">^</a></span>';
};

MarkdownRenderer.prototype.renderCitation = function (mdHtml) {
    var that = this;
    var citeTags = [];

    // Render Note
    var noteIndex = 0;
    mdHtml = mdHtml.replace(/\[.*\]:/g, function (text, pos) {
        // Escape in <code> and <pre>
        if (that._isInTag(mdHtml, 'code', pos) || that._isInTag(mdHtml, 'pre', pos))
            return text;

        citeTags.push(text.substr(0, text.length - 1));
        var anchor = that.anchor(text.substr(1, text.length - 3));
        return that.citeNote(anchor, ++noteIndex);
    });

    for (var i = 0; i < citeTags.length; i++) {
        var anchor = this.anchor(citeTags[i].substr(1, citeTags[i].length - 2));
        var noteIndex = i + 1;

        // Render Ref
        var refIndex = 0;
        var derefHTML = ' ';  // Use RegExp to construct global replacement
        mdHtml = mdHtml.replace(new RegExp(that._escapeRegExp(citeTags[i]), 'g'), function (text, pos) {
            // Escape in <code> and <pre>
            if (that._isInTag(mdHtml, 'code', pos) || that._isInTag(mdHtml, 'pre', pos))
                return text;

            derefHTML += that.citeDeref(anchor, ++refIndex) + ' ';
            return that.citeRef(anchor, noteIndex, refIndex);
        });

        // Render Deref
        var noteStr = this.citeNote(anchor, noteIndex);
        var indexToInsert = mdHtml.indexOf(noteStr) + noteStr.length;
        mdHtml = mdHtml.substr(0, indexToInsert) + derefHTML + mdHtml.substr(indexToInsert);
    }

    return mdHtml;
};

// reference

MarkdownRenderer.prototype.refTarget = function (type, value) {
    return '<div class="ref-target" id="ref-' +
        type + '-' + value + '"></div>';
};

MarkdownRenderer.prototype.refItem = function (type, value, index) {
    return '<span class="ref-item"><a href="#ref-' +
        type + '-' + value + '">' + index + '</a></span>';
};

MarkdownRenderer.prototype.refSection = function (anchor, text) {
    return '<span class="ref-item"><a href="#' +
        anchor + '">&sect; ' + text + '</a></span>';
};

MarkdownRenderer.prototype.renderReference = function (mdHtml, headingArr) {
    var that = this;
    var indexMap = {};

    // Ref target
    var len1 = '<p>['.length;
    var len2 = ']</p>'.length + len1;
    var countRefs = {};
    mdHtml = mdHtml.replace(/<p>\[[^\].]+\|\|[^\].]+\]<\/p>/g, function (text) {
        text = text.substr(len1, text.length - len2);
        var fragments = text.split('||');
        fragments[0] = that.anchor(fragments[0]);

        var count = (countRefs[fragments[0]] || 0) + 1;
        countRefs[fragments[0]] = count;

        fragments[1] = that.anchor(fragments[1]);
        indexMap[fragments.join('|')] = count;

        return that.refTarget(fragments[0], fragments[1]);
    });

    // Ref entry (or section)
    mdHtml = mdHtml.replace(/\[[^\].]+\|[^\].]+\]/g, function (text, pos) {
        // Escape in <code> and <pre>
        if (that._isInTag(mdHtml, 'code', pos) || that._isInTag(mdHtml, 'pre', pos))
            return text;

        var fragments = text.substr(1, text.length - 2).split('|');
        fragments[0] = that.anchor(fragments[0]);

        if (fragments[0] == 'sec') {
            for (var i = 0; i < headingArr.length; i++)
                if (headingArr[i].text == fragments[1]) {
                    return that.refSection(headingArr[i].anchor,
                        headingArr[i].noNumber ?
                            headingArr[i].text :
                            headingArr[i].headingNumber);
                }
        }
        else {
            fragments[1] = that.anchor(fragments[1]);
            var index = indexMap[fragments.join('|')];
            if (index) {
                return that.refItem(fragments[0], fragments[1], index);
            }
        }

        var errMsg = 'Invalid ref text: ' + text;
        if (console) console.error(errMsg);
        if (alert) alert(errMsg);
        return text;
    });

    return mdHtml;
};

// slides

MarkdownRenderer.prototype.slideTags = ['<div class="markdown-slide">', '</div>'];

MarkdownRenderer.prototype.renderSlides = function (mdHtml) {
    var that = this;

    // Check slide-mode
    var isSlideMode = false;
    mdHtml = mdHtml.replace(/<p>\[slide-mode\]<\/p>/g, function () {
        isSlideMode = true;
        return '';
    });

    if (!isSlideMode) return mdHtml;

    var matchHr = mdHtml.match(/<hr\/?>/g);
    if (!matchHr) return mdHtml;

    var count = 0;
    mdHtml = mdHtml.replace(/<hr\/?>/g, function () {
        ++count;
        var ret = '';

        if (count != 1) ret += that.slideTags[1];
        if (count != matchHr.length) ret += that.slideTags[0];

        return ret;
    });

    return mdHtml;
};

// keyword and style setter

MarkdownRenderer.prototype.keywordTags = [
    'page-break', 'float-left', 'float-right',
    'align-left', 'align-right', 'align-center',
    'cite-sec', 'not-print'
];

MarkdownRenderer.prototype.renderKeywordTags = function (mdHtml) {
    if (this.keywordTags.lenght == 0) return mdHtml;

    var condStr = this.keywordTags[0];
    for (var i = 1; i < this.keywordTags.length; i++)
        condStr += '|' + this.keywordTags[i];

    var regExp = new RegExp('<p>\\[(' + condStr + ')\\]<\\/p>', 'g');
    mdHtml = mdHtml.replace(regExp, '<div class="$1"></div>');

    // Merge adjacent tags
    return mdHtml.replace(/"><\/div>[\r\n]*<div class="/g, ' ');
};

MarkdownRenderer.prototype.renderStyleSetters = function (mdHtml) {
    var len1 = '<p>['.length;
    var len2 = ']</p>'.length + len1;
    var styleSetters = mdHtml.match(/<p>\[[^\].]+=[^\].]+\]<\/p>/g) || [];  // avoid ']' inside pairs

    for (var j = 0; j < styleSetters.length; j++) {
        var styleSetter = styleSetters[j];
        var tagPrefix = '<' + styleSetter.substr(len1,
            styleSetter.indexOf('=') - len1);
        var tagStyle = styleSetter.substr(1 + styleSetter.indexOf('='),
            styleSetter.length - len2 - tagPrefix.length);

        var firstTagIndex = mdHtml.indexOf(styleSetter) + styleSetter.length;

        // Avoid prefix matches
        while (true) {
            firstTagIndex = mdHtml.indexOf(tagPrefix, firstTagIndex);
            if (firstTagIndex == -1) break;
            if (mdHtml[firstTagIndex + tagPrefix.length] == ' ' ||
                mdHtml[firstTagIndex + tagPrefix.length] == '>') {
                break;
            }
            else {
                firstTagIndex += tagPrefix.length;  // find next
            }
        }

        if (firstTagIndex == -1) continue;  // not found
        mdHtml = mdHtml.substr(0, firstTagIndex + tagPrefix.length).replace(styleSetter, '') +
            ' style="' + tagStyle + '"' + mdHtml.substr(firstTagIndex + tagPrefix.length);
    }
    return mdHtml;
};

// render function

MarkdownRenderer.prototype.render = function (mdText) {

    // Patch for TOC support
    // Ref: https://github.com/chjj/marked/issues/545

    var headingArr = [];
    var renderer = new marked.Renderer();
    renderer.heading = function (text, level, raw) {
        headingArr.push({ text: text, level: level, raw: raw });
        return '<h1>HEADING_PLACEHOLDER</h1>';
    };

    var mdHtml = marked(mdText, {
        renderer: renderer,
        mathDelimiters: this.mathTags
    });

    // in/out: headingArr
    mdHtml = this.renderHeading(mdHtml, headingArr);
    mdHtml = this.renderToc(mdHtml, headingArr);
    mdHtml = this.renderCitation(mdHtml);
    mdHtml = this.renderReference(mdHtml, headingArr);
    mdHtml = this.renderSlides(mdHtml);
    mdHtml = this.renderKeywordTags(mdHtml);
    mdHtml = this.renderStyleSetters(mdHtml);
    return mdHtml;
};

if (typeof module !== 'undefined' && typeof exports === 'object') {
    module.exports = MarkdownRenderer;
}
