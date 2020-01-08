/*
	QR Code SVG Helper

    - based on qrcode.js (kazuhikoarase, https://github.com/kazuhikoarase/qrcode-generator)

	BOT Man, 2017 (MIT License)
*/

function utf16To8(str) {
    var ret = '';
    for (var i = 0; i < str.length; i++) {
        var c = str.charCodeAt(i);
        if ((c >= 0x0001) && (c <= 0x007F)) {
            ret += str.charAt(i);
        }
        else if (c > 0x07FF) {
            ret += String.fromCharCode(0xE0 | ((c >> 12) & 0x0F));
            ret += String.fromCharCode(0x80 | ((c >> 6) & 0x3F));
            ret += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
        }
        else {
            ret += String.fromCharCode(0xC0 | ((c >> 6) & 0x1F));
            ret += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
        }
    }
    return ret;
}

function qrCodeSVG(text, size) {

    // typeNumber: 1 ~ 40, 0 => auto
    // correctLevel: L(7%), M(15%), Q(25%), H(30%)
    var qrCode = new QRCode(0, QRErrorCorrectLevel.L);

    qrCode.addData(utf16To8(text));
    qrCode.make();

    var cellSize = size / qrCode.getModuleCount();
    var rect = 'l' + cellSize + ',0 0,' + cellSize +
        ' -' + cellSize + ',0 0,-' + cellSize + 'z ';
    var color = '#000000';

    // <svg ...>
    var svg = '';
    svg += '<svg';
    svg += ' viewBox="0 0 ' + size + ' ' + size + '"';
    svg += ' xmlns="http://www.w3.org/2000/svg"';
    svg += '>';

    // content
    svg += '<path d="';
    for (var r = 0; r < qrCode.getModuleCount(); ++r)
        for (var c = 0; c < qrCode.getModuleCount(); ++c)
            if (qrCode.isDark(r, c)) {
                svg += 'M' + (c * cellSize) +
                    ',' + (r * cellSize) + rect;
            }
    svg += '" stroke="transparent" fill="' + color + '"/>';

    // </svg>
    svg += '</svg>';
    return svg;
};
