# BuffHUD

Skyrim SE/AE 状态效果面板插件。把角色身上所有来源的 buff/debuff 汇总到一页：图标、名称、剩余时间、来源对齐显示；长按热键显示完整描述。

**本版接入 SKSE Menu Framework 3.x**：设置页在 Mod Control Panel → BuffHUD → 设置；HUD 通过 SMF 的 HUD 元素绘制，共享其 ImGui 上下文。不再有独立 D3D11 覆盖层，也不需要 MCM Helper / SkyUI / ESP / Papyrus。

## 功能

- 汇总法术、药水、毒药、疾病、祝福、种族能力、呐喊、食物、附魔等一切经标准魔法系统生效的效果
- 三列对齐（图标+名称 / 剩余时间 / 来源），布局参考 SkyUI Active Effects
- 长按 F（默认 350 ms）显示排序第一条的完整描述；短按钉住面板
- 倒计时读效果自身经过时间，游戏暂停时一起冻结
- 同类叠加以 `x3` 计数；无时限效果显示「持续」
- 增益暖金 / 减益绯红，可切回原版骨白；三套配色、70–200% 缩放、0–100% 不透明度、五个锚点（四角 + 居中）
- 86 条图标匹配规则；匹配不上时用通用图标，不丢效果
- 24 格单色描边图标集，运行时着色（不重新编码贴图）
- 18 项设置分 5 组，存于 `Data/SKSE/Plugins/BuffHUD/BuffHUD.ini`，插件监视文件时间戳，1 秒内热重载；不写入存档
- 中英双语，104 键语言包，默认跟随系统 UI 语言，设置页可切；**含固定中文署名段，英文版刻意不翻译**
- 兼容 SE 1.5.97 与 AE 1.6.x（单 DLL 双布局），无 FormID 白名单，后装模组的效果自动纳入

## 依赖

- Skyrim SE 1.5.97 或 AE 1.6.x
- SKSE64
- Address Library for SKSE Plugins（与游戏版本匹配）
- [SKSE Menu Framework 3.x](https://www.nexusmods.com/skyrimspecialedition/mods/96892)（必需，提供设置页与 ImGui 运行时）

## 构建

Windows + VS2022 + CMake ≥ 3.21 + C++23。

`CMakeLists.txt` 里有两处本机绝对路径，换机器需改：

| 位置 | 用途 |
|---|---|
| `D:/dev/CommonLibSSE-NG` | CommonLibSSE-NG 源码目录（`add_subdirectory`） |
| `D:/dev/deps` | spdlog / rapidcsv 的 CMake prefix |

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

产物：`build/Release/BuffHUD.dll`。`build.py` 是本机封装（vcvars 环境捕获 + 去代理变量），`deploy.py` 负责暂存打包与部署，两者都含本机路径，按需改。

**不要链接自带 ImGui**：SMF 头把 ImGui API 暴露为 `SKSEMenuFramework.dll` 内的函数指针，插件自身编译 ImGui 会得到第二个上下文。

## 目录

| 路径 | 内容 |
|---|---|
| `src/` | 插件源码 |
| `external/smf/SKSEMenuFramework.h` | SMF 3.x SDK 头（vendored） |
| `mod/SKSE/Plugins/BuffHUD/` | 运行时资产：图标图集、规则表、语言包（从 MO2 模组目录同步，不含 DLL） |
| `docs/description.md` | 全平台发布文案：Nexus / 中文通用 / 贴吧 / 3DM / B站 / 更新日志 |
| `scripts/BuffHUD.psc` | 早期 Papyrus 脚本（历史留档，1.2.0 不参与构建） |

## 署名

BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。
