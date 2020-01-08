# QR Code Generator

> Hacking `qrCodeSVG` in BOT Frame 😎

## Input

<div style="text-align:center;">
<textarea id="qrCodeText" rows="5" style="width:80%" oninput="document.getElementById('qrCodeCanvas').innerHTML = qrCodeSVG(this.value, 320);"></textarea>
</div>

## Output

<div style="margin:0 auto;width:50%">
<p id="qrCodeCanvas" style="text-align:center;">
    Generate QR Code here 🙃
</p>
<p style="text-align:center;">
    <em>qrcode.js</em> by
    <a href="https://github.com/kazuhikoarase/qrcode-generator">
        kazuhikoarase
    </a>
</p>
</div>
