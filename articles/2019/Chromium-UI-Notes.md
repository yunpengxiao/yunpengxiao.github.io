# Chromium/ui 开发笔记

> 2019/9/3
> 
> Windows Chromium Direct-UI 开发相关笔记

## 目录 [no-toc]

[TOC]

## 架构

- Direct-UI 核心思想：
  - 虚拟 视图/控件，接管 布局、绘制、消息
  - 对比：MFC/WTL 使用系统子窗口（效率低，占用大量句柄，通过窗口消息传递数据）
- View 作为视图的基类，通过树形结构管理
- Widget 封装了窗口的逻辑
- View 相关：
  - RootView 是 Widget 存储的树根，接收/分发窗口消息
  - NonClientView 是 RootView 的唯一子节点，负责整个窗口
  - NonClientFrameView 是 NonClientView 的非客户区部分（标题栏/系统栏）
  - ClientView 是 NonClientView 的客户区的根节点，只有一个子节点
- Widget 相关（Aura 引入是为了支持 `//cc` 和 GPU）：
  - NativeWidget 是 Widget 在不同平台下的实现对象
  - WindowTreeHost 是 Aura 的窗口管理器，继承于 NativeWidget
  - Window 是 Aura 的窗口，被 WindowTreeHost 管理
  - HWNDMessageHandler 是 Aura 在 Windows 平台的窗口消息处理器

## 布局

- LayoutManager 是布局管理器，实现子 View 的通用排版
- 递归流程：
  - 父容器在 `Layout` 时，调用 `GetPreferredSize` 计算子元素期望的尺寸
  - 父容器在 `Layout` 时，调用 `SetBounds` 设置子元素实际大小，导致子元素 `Layout`
- 内联/分块：Inline/Block
- 栈式/网格：StackPanel/Grid
- 边距/边框：Content -> Padding/Inset -> Border -> Margin
- XAML 语法布局器
  - 动态加载布局信息，支持原生渲染
  - 通过适配器模式，底层支持 MFC/WTL/Chromium UI 元素创建/绑定
  - 支持 WebView 和 嵌套布局

## 层次

- 创建 View/Widget 时，考虑 Z-Index
- 支持 平移/旋转/缩放 变换
- 在 Aura 中引入，支持 GPU 合成

## 绘制

- Canvas 是绘制区域的载体，与窗口系统隔离
- Background 封装了 View 的背景绘制逻辑
- Border 封装了 View 的边框绘制逻辑
- 脏区域：
  - 自己维护脏区域，与窗口系统隔离
  - 遍历整个 View 树，如果 View 不在无效区域 Rect，则不需要重新绘制
- 双缓冲：
  - FrontBuffer 用于渲染，BackBuffer（Canvas）用于当前绘制
  - 用户看到的每一帧都是正确的，避免无效重绘，避免闪屏
- 九宫格绘制：
  - 拉伸一张切成九份的图片（中间无效），用于绘制 NonClientFrameView 的外层阴影边框

## 焦点

- FocusManager 是焦点管理器，处理焦点的切换
- 使用 Widget 时，考虑是否 Inactive
- 使用 View 时，考虑是否 Focusable

## 快捷键

- FocusManager 同时管理 View 的快捷键逻辑
- AcceleratorTarget 是 View 提供的 响应快捷键 的接口
- Accessibility 无障碍功能
  - 支持朗读/选择
  - 可用于 Direct-UI 自动化测试

## 上下文菜单

- ContextMenuController 是 View 提供的 上下文菜单 的接口
- 使用者只需要设置 Controller 就可以在 View 上实现 右键/长按 菜单

## 拖拽

- DragController 是 View 提供的 处理拖拽逻辑 的接口
- DragDropClient 是 NativeWidget 创建/管理的 拖拽处理组件

## 提示气泡

- Tooltip 是鼠标 hover 到 View 时弹出气泡

## 滚动

- ListView 复用优化：
  - 只创建可见的 ListView（包括不完整显示的）
  - 针对不同滚动位置，填充不同 ListItem 数据

## 动画

- 周期：`时间间隔 == 1/帧率`
- 进度：不同效果对应不同的 `时间-进度` 曲线 `f(t % Interval)`
  - Linear 线性
  - Ease In/Out 渐入/渐出
  - Fast In/Out 快入/快出
  - Slide 先正放，后倒放
  - Throb 先正放，再暂停，后倒放（例如 hover）
- 重绘：
  - 定时器触发后，根据当前时间对应的进度，修改布局/元素样式（变成脏区域）
  - 需要注意动画的变化范围，脏区域过大可能有性能问题

## 消息

- 创建
  - WM_CREATE
- 绘制
  - WM_PAINT
  - WM_NCPAINT
- 销毁
  - WM_CLOSE
  - WM_DESTROY
  - WM_NCDESTROY
- 交互
  - Key/Mouse/Gesture
  - 通过 View 层次结构分发到所有子 View 上

## 进程

- [Chromium 多进程架构](https://developers.google.cn/web/updates/2018/09/inside-browser-part1)
  - 划分：1 Browser + n Renderer + n Extension (Renderer) + 1 GPU + (1 Plugin) + x Util
  - 模型：`process-per-site-instance`（内存最大）/`process-per-site`（内存相对小）/`process-per-tab`（默认）/`single-process`（内存最小，易于调试，Android WebView 使用该模型）
- 进程服务化
  - Browser 进程作为中心节点，能隔离/恢复其他进程的崩溃
  - Renderer/Plugin 进程在沙箱内运行，不能直接访问系统资源
  - Util 进程功能拆分为 网络服务/渲染服务/加密服务/打印服务...
- 多窗口架构
  - n 主窗口 (Browser) + n 渲染 Host 窗口 (Browser) + 1 立即 D3D 窗口 (GPU)
  - Browser 进程统一管理窗口，Renderer 进程通过 IPC 传输渲染数据
- 跨进程通信
  - 上层概念：RPC (Proxy + Stub)，底层实现：管道 / COM 组件对象模型
  - 路由问题：操作注册/映射
  - 类型问题：输入合法性检查
  - 安全问题：对端合法性校验
  - 顺序问题：相对于原始的管道，RPC 不保证消息顺序
- 渲染模型
  - 软件渲染：共享内存/存储（多数情况下比 GPU 加速快）
  - 硬件加速：指令缓存（Render 进程向 GPU 进程提交）+ 分层渲染（`//cc`）

## 线程

- [Chromium 多线程架构](https://github.com/chromium/chromium/blob/master/docs/threading_and_tasks.md#threads)
  - Browser 进程：1 UI + 1 IO (IPC/network) + x Spec + Worker-Pool
  - Renderer 进程：1 Main + 1 IO (IPC/network) + 1 Compositor + 1 Raster + n Worker + x Spec
- 无锁架构：可以在线程间抛（定时）任务（并设置结果回调）
  - 200ms 原则：不能在 UI 线程进行耗时操作（例如  I/O 操作、大量数据 CPU 密集计算、部分系统调用）
  - UI 响应：从 UI 线程将 I/O 任务抛到 Worker 线程池执行，任务完成后再将结果抛回 UI 线程
  - IPC 消息：在 IO 线程收到消息后，把任务抛到 UI 线程执行
- 消息泵：能同时处理 消息（窗口消息/libevent 回调/IO 完成消息）和 任务（普通/定时）
  - 基于超时机制：在规定的时间内 等待/处理 消息，超时后 处理 任务
  - 发送特殊消息：在等待消息时，发送 1 字节到管道，结束消息的等待，转到处理任务
- 数据竞争：不能在 非 UI 线程访问/操作 UI 数据（例如 缓存字体相关布局信息）

## 项目

- [浏览器](V8-Hippy-Share.md#Chromium-架构) + 插件（独立升级）+ 云控（灰度/策略/运营/捞取）+ 攻防（驱动/服务）
- 拆分 Browser/UI
  - 开发时，工程独立：快速编译/快速升核/支持多核
  - 运行时，进程独立：快速启动/崩溃隔离与恢复
- 修改大项目
  - 统一代码规范
  - 尽量只增不改
  - 新增的代码 存放在独立目录
  - 修改的代码 使用宏区分

## 优化

- 创建时，按需加载（不可见部分）-> 延迟加载（不影响首屏绘制）
- 布局时，缓存字体相关尺寸计算结果
- 绘制时，减小脏区域尺寸
- 避免阻塞 UI 调用（例如 `SendMessage` -> `PostMessage`）
- 缓存系统调用结果（例如 显示器/DPI）
