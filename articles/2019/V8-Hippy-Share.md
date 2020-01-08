# 从 V8 到 Hippy

> 2019/6/13
> 
> 最近在做 [Hippy 项目](https://github.com/Tencent/Hippy)，分享一些对 V8/Hippy 的理解

[slide-mode]

---

### 你想知道的：

- [sec|Chromium 架构]
- [sec|V8 引擎]
- [sec|Hippy 框架]
- [sec|Hippy Core 项目]
- [sec|Hippy Core 工作内容]
- [sec|展望：大前端时代的 Windows 开发]

---

### Chromium 架构

- 浏览器 ([Chromium](https://developers.google.cn/web/updates/2018/09/inside-browser-part1)) = 窗口/标签 + 历史/密码/下载/书签/打印 + 升级/同步/上报 + 插件/扩展/安全 + 网络/缓存 + **浏览模块**
- 浏览模块 ([content](https://www.chromium.org/developers/content-module)) = 多进程架构 (PC)/WebView (Android) + 网络/存储 + **渲染引擎**
- 渲染引擎 ([blink](https://www.chromium.org/blink)) = DOM 解析/样式/布局 + 绘制/动画/合成 (GPU) + 输入 + devtools + **JavaScript 引擎** 绑定
- JavaScript 引擎 ([V8](https://v8.dev/)) = 编译/运行 JavaScript 代码 + 分配/回收内存 + 绑定扩展功能 (console/DOM/...)

> - [Chromium Embedded Framework (CEF) 项目](https://github.com/chromiumembedded/cef) 基于浏览模块，封装可嵌入的浏览功能；headless 封装了无图形界面的浏览功能
> - 演进：WebKit -> blink，JavaScriptCore (JSC) -> V8 (但 iOS 策略限制只能用 WebKit/JSC 🤮)

---

### V8 引擎

- 高性能，跨平台，易于集成，线程/网络无关
- [Chromium](https://www.chromium.org/developers)：绑定 HTML 标准的 DOM 元素，实现浏览功能
- [Node.js](https://nodejs.org/)：绑定 网络模块、文件操作，用作服务器/开发工具的运行时
- [React Native](https://facebook.github.io/react-native/)：绑定 Android/iOS 原生组件（移动应用）
- [Electron](https://electronjs.org)：Chromium + Node.js + Native 操作（桌面应用）

> Node.js 优势：
> 
> - 起于 JavaScript：天然的 **事件驱动异步 I/O**，适合开发高并发服务
> - 反哺前端：打包工具 [webpack](https://webpack.js.org/)、JavaScript 转码工具 [babel](https://babeljs.io/)、页面自动化测试 [Nightwatch.js](https://nightwatchjs.org/)
> - 基于 [npm](https://www.npmjs.com/) 模块化管理，促使 JavaScript 标准不断演进

---

### Hippy 框架

- 业务精细化运营（类似 React Native/虚拟主页）
- 小包，高性能，支持 Java/Objective-C 绑定
- 支持动画接口（非前端驱动，避免 Native 调用开销）
- 支持资源分包（可复用代码，小 JavaScript 包）

划分为三层：

- JavaScript 代码包（包括 SDK/React/业务）
- Core 层（C++ 封装 V8/JSC 虚拟机，JSBridge：JavaScript <-> Native，DOM 布局）
- Native 层（DOM <-> Native 组件，网络/存储 支持）

---

### Hippy Core 项目

- Android/iOS 共享的 C++ 层代码
- 真机调试：通过 Inspector（复用 Node.js 调试通路）调试手机 V8/JSC 引擎
  - 之前在 Devtools（Chrome）运行 JavaScript 计算 DOM，再传给手机显示，可能有差异
- 在 C++ 层实现平台无关的 JavaScript 绑定： `CallNative` -> `InternalBinding`
  - 目前支持 `console.log`/`setTimeout`/`setInterval`
  - 之前通过 C++ 转发到 Java/Objective-C 代码实现，Native 调用开销严重（JNI 30ms/次）

---

### Hippy Core 工作内容

- 制定代码规范（命名/文件/目录/lint）
- 完善基础设施（USE/DISALLOW_COPY/LOG/CHECK）
- 重构代码
  - vector/string/map/_ptr
  - 生命周期管理/线程模型（何时何地创建/销毁）
  - 输入合法性检查/排查死锁/排查内存泄漏
- 支持 Visual Studio 完整调试（高亮/智能提示/断点）

---

### 展望：大前端时代的 Windows 开发

- Electron
- [React Native for Windows](https://github.com/Microsoft/react-native-windows)
- [Universal Windows Platform (UWP).js](https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide)
- [Progressive Web App (PWA)](https://docs.microsoft.com/en-us/microsoft-edge/progressive-web-apps) on UWP

<br/>

### Q & A

[align-right]

# 谢谢 🙂

---
