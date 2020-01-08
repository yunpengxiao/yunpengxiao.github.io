[align-center]

> BOT Man is... not a BOT. 😂

[align-center]

# 📝 [~/articles](/articles/)

[align-center]

> 最新文章会第一时间在 **公众号** 推送（微信搜一搜 **BOTManJL**）

<div style="margin:0 auto;width:30%">
  <p id="qrCodeCanvas" style="text-align:center;"></p>
</div>

[align-center]

# 🙈 [CV](/articles/?post=misc/CV-en) | [简历](/articles/?post=misc/CV-zh)

[align-center]

> 关于 [BOT Frame](/articles/?post=../README) | [BOT Mark](/articles/?post=misc/BOT-Mark)

<script>
  function setBackground() {
    var vertical = window.innerWidth > 940 ? 'bottom' : 'top';
    document.body.style.background='url(/static/botman-light.jpg) repeat-y center center fixed';
    document.body.style.backgroundSize = '100% auto';
  }
  setBackground();
  window.onresize = setBackground;

  try {
    var mpUrl = 'http://weixin.qq.com/r/WCifh4jEydW2rczk931K';
    document.getElementById('qrCodeCanvas').innerHTML = qrCodeSVG(mpUrl, 320);
  } catch (e) {}
</script>
