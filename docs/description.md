# BuffHUD 1.2.0 — 宣传文案（可直接粘贴）

> 事实底稿：全部数字与功能均已对照 `src/`、`SKSE/Plugins/BuffHUD/` 下的资源表核实。  
> Credits 里的中文署名声明是固定中文，所有语言版本（含英文正文）都照抄、不翻译。  
> 各平台变体的版本号、前置、热键、默认阈值、设置项数必须逐项一致。



---

## 一、Nexus Mods 简介（Summary 栏，一句话）

> Every active effect on your character, on one page: icon, name, time left, source. Hold F for the full list.



> 角色身上所有生效中的效果收成一页：图标、名称、剩余时间、来源。长按 F 展开。

---


## 二、Nexus Mods 正文（English，BBCode）

\[b\]\[size=6]BuffHUD — Status Effect Panel\[/size\]\[/b]  
[i]Every buff and debuff on your character, on one page. Hold a key to see the whole picture.[/i]

\[size=4\]\[b]What it does\[/b\]\[/size]

BuffHUD reads the player's active effect list — spells, potions, poisons, diseases, blessings, racial abilities, shouts, food, worn enchantments — and lays it out the way SkyUI's Active Effects page does: [b]icon, name, time left, source[/b], in three aligned columns.

Hold the hotkey ([b]F[/b] by default, [b]350 ms[/b]) and the panel appears with a detail section underneath the list, showing the full vanilla description of the first, highest-priority effect. Release to hide it. A short tap pins the panel open so you can read it hands-free.

\[size=4\]\[b]Features\[/b\]\[/size]

[list]  
[*]Icon + name + remaining time + source in three aligned columns, laid out like SkyUI's Active Effects page  
[*]Countdown comes from the effect's own in-game elapsed time, so it stops when the game pauses instead of running on a wall clock  
[*]Effects with no duration show [b]constant[/b] (持续), not a made-up number  
[*]Stacked castings of the same effect collapse into one row with an [b]x3[/b] counter, keeping the longest remaining time  
[*]Overflow collapses into a "[b]N more[/b]" row instead of covering the screen — 12 rows by default, 1–40 configurable  
[*]Buffs tinted amber, debuffs crimson; a thin accent bar keeps the verdict readable even with tinting off  
[*]Three colour schemes, 70–200% scale, 0–100% background opacity, five screen anchors  
[*]Drawn by SKSE Menu Framework's HUD layer — no private D3D11 hook, no second ImGui context; Skyrim never pauses and never loses focus  
[*]Optional "combat only" mode: the panel disappears the moment the fight ends, even when pinned  
[/list]

\[size=4\]\[b]Compatibility\[/b\]\[/size]

There is no FormID whitelist. Anything in the actor's active effect list is picked up automatically, which in practice means [b]every mod that adds a spell effect, enchantment, ingredient, disease or ability[/b] — including mods released after this one.

Icons are chosen by [b]86 rules[/b] matching keywords, archetypes and actor values. Effects that match none of them are never dropped: they fall back to a generic icon. An unknown effect is still an effect you should see.

[b]Load order:[/b] no ESP, no ESL, no quest record. BuffHUD is one SKSE DLL plus config and data files; nothing is injected into the load order, so it cannot conflict.

\[size=4\]\[b]Interface\[/b\]\[/size]

[b]24 icons[/b] in a single 1536×1024 atlas (256 px per cell): alpha-only monochrome glyphs, tinted at runtime, so changing the palette never re-encodes a texture. That is 23 named icons plus one generic fallback. The layout follows the SkyUI Active Effects page on purpose — this is meant to feel like the screen Skyrim forgot to ship.

[b]Chinese and English, switchable at any time.[/b] The panel and the settings page share one 104-key language pack. The default follows your system UI language; switch to 简体中文 or English from the settings page without restarting. The Mod Control Panel page label is named at startup from the same pack — 设置 in Chinese, Settings in English.

\[size=4\]\[b]Settings\[/b\]\[/size]

BuffHUD registers a [b]BuffHUD → Settings[/b] page (设置 in Chinese) in SKSE Menu Framework's [b]Mod Control Panel[/b] — the same place as every other SMF-based mod. No MCM Helper, no SkyUI, no second overlay, no extra settings hotkey.

[b]18 settings on one page, in five groups:[/b] display (enable, max rows, anchor, sort, merge duplicates, time and source columns, hide inactive), appearance (scale, opacity, colour scheme, tinting), controls (hotkey, hold threshold, combat only), compatibility (show vanilla-hidden effects, debug view), and language. The hotkey row captures the next key you press through the SKSE input sink.

Settings live in [code]Data/SKSE/Plugins/BuffHUD/BuffHUD.ini[/code]. You can edit that file by hand — the plugin checks its timestamp once a second and reloads within a second. Nothing is stored in your save.

\[size=4\]\[b]Performance\[/b\]\[/size]

While the panel is hidden, the HUD callback returns immediately: no window is begun, no draw call is issued, nothing is collected. While it is visible, a lightweight timer requests a snapshot refresh [b]four times a second[/b] and checks the ini [b]once a second[/b]. Both only run while the panel is on screen.

A refresh is queued as an SKSE task and builds a new snapshot on the game thread; the render thread only reads a double-buffered snapshot through [b]one atomic index load[/b]. No hooks are installed in combat, magic casting or any other hot path — the only game-side hook is the input event sink used for the hotkey. While a menu is open (inventory, Mod Control Panel, dialogue) the hotkey is ignored and the HUD does not paint over the game UI.

\[size=4\]\[b]Requirements\[/b\]\[/size]

[list]  
[*]Skyrim SE 1.5.97 or AE 1.6.x — one DLL, both layouts  
[*]SKSE64 (matching your game version)  
[*]Address Library for SKSE Plugins (matching your game version)  
[*\]\[b]SKSE Menu Framework 3.x\[/b] — supplies the Mod Control Panel and the HUD layer. Without it BuffHUD still loads and logs a warning, but draws nothing.  
[/list]

[b]Not required:[/b] SkyUI, MCM Helper, any ESP, or any other UI mod.

\[size=4\]\[b]Installation\[/b\]\[/size]

Install with MO2 or Vortex alongside SKSE Menu Framework 3.x. There is no ESP to enable and nothing to add to your load order. Launch the game and open [b]Mod Control Panel → BuffHUD → Settings[/b] to configure it; hold the panel hotkey ([b]F[/b] by default) in game to bring up the effect list.

Log: [code]Documents\My Games\Skyrim Special Edition\SKSE\BuffHUD.log[/code] — it prints the settings it read, the icon count, the rule count, and whether SKSE Menu Framework was detected at startup.

\[size=4\]\[b]Known limitations\[/b\]\[/size]

[list]  
[*]Effects that only change an actor value without creating an active effect instance (a few script-driven or perk-static buffs) are not listed.  
[*]Gamepad and VR input are not mapped; the hotkey is keyboard-only, and is ignored while a menu is open.  
[*]The detail section always describes the [b]first row[/b] — the one your sort order ranks highest. There is no mouse selection: with a hold-to-show hotkey there is no pointer to select with.  
[*]The detail text is the vanilla description string with its magnitude, duration and area placeholders filled in. If a mod ships an empty description, the pane shows a "no description" placeholder — that is the mod's data, not a bug here.  
[*]The Mod Control Panel page label is registered once when the game loads. Switching the UI language mid-session does not rename it until the next restart.  
[/list]

\[size=4\]\[b]Credits\[/b\]\[/size]

BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。

The 24 atlas icons were made for this mod as alpha-only monochrome glyphs, and are free to use with it. Built on CommonLibSSE-NG; the UI runs on SKSE Menu Framework's shared Dear ImGui context — ImGui is provided by the framework, not bundled in this download.

---

## 三、中文通用版（下面三份变体的事实底稿）

[b]BuffHUD — 状态效果面板[/b]

把角色当前生效的所有效果收成一页：图标、名称、剩余时间、来源，三栏对齐，排版照 SkyUI「激活效果」页做的。法术、药水、毒、疾病、祝福、种族能力、龙吼、食物、穿戴附魔，凡是挂在角色身上的都读。

长按热键（默认 F，350 毫秒）面板展开，列表下方带一块详情区，显示排在最前那条效果的完整原版说明；松手收起。短按则把面板钉住，可以腾出手慢慢看。

[b]特点[/b]

· 图标、名称、剩余时间、来源三栏对齐，照 SkyUI「激活效果」页排版  
· 剩余时间取效果自身的游戏内计时，游戏暂停时它跟着停，不按现实时钟跑  
· 没有持续时间的效果显示「持续」，不会瞎算一个数字  
· 同一效果被多次施加时合并成一条、标 x3，保留最长的剩余时间  
· 超出上限折叠成「另有 N 项」，默认 12 条，可在 1–40 之间调  
· 增益暖金、减益绯红；关掉着色也有一条细色条标明增益还是减益  
· 三套配色、70–200% 缩放、0–100% 背景不透明度、四角加居中共 5 个位置  
· 由 SKSE Menu Framework 的 HUD 层绘制，不自己挂 D3D11、不另起 ImGui，游戏不暂停也不抢焦点  
· 可选「仅战斗中显示」，脱战立刻收起，钉住也一样

[b]兼容性[/b]

没有 FormID 白名单。凡是出现在角色激活效果列表里的都会被自动捕获——法术、药水、毒、疾病、祝福、种族能力、龙吼、食物、穿戴附魔，以及日后新出的模组。

图标由 86 条规则按关键字、原型、属性值匹配。一条规则都对不上的效果也不会被丢掉，会落到通用图标上。不认识，不等于不该看见。

插件本体就是一个 SKSE DLL 加配置文件和资源表，不含 ESP/ESL，没有任务记录，不改任何原有记录，不进加载顺序，因此不会冲突。

[b]界面与语言[/b]

24 个图标合成一张 1536×1024 的图集，每格 256 像素，是纯 alpha 的单色描边字形，运行时着色，换配色不需要重新编码贴图；其中 23 个具名图标，另加 1 个通用兜底。

[b]中英双语，随时可切。[/b] 面板与设置页共用一套 104 键语言包，默认跟随系统界面语言，可在设置页直接切简体中文或 English，不用重进游戏。设置页标题在游戏加载时按语言包取名：中文「设置」，英文 Settings。

[b]配置[/b]

设置页注册在 SKSE Menu Framework 的 [b]Mod Control Panel → BuffHUD → 设置[/b]，和其它 SMF 模组放在同一处；不需要 MCM Helper、SkyUI，也没有第二个覆盖层或额外的设置热键。

[b]一页 5 组共 18 项[/b]：显示（启用、最大条数、屏幕位置、排序、合并同类、时间/来源栏、隐藏条件未满足项）、外观（缩放、透明度、配色、着色）、控制（热键、长按阈值、仅战斗中）、兼容（显示原版隐藏项、调试视图）、语言。热键那一行点一下，再按任意键就绑定，走的是 SKSE 输入事件。

设置存在 `Data/SKSE/Plugins/BuffHUD/BuffHUD.ini`，手改也生效——插件每秒查一次文件时间戳，一秒内重载。存档里不写任何设置，重装不丢配置。

[b]性能[/b]

面板关着时 HUD 回调直接返回：不开窗口、不发绘制调用、不采集数据。面板可见时，一个轻量计时器每 250 毫秒请求刷新一次快照、每秒查一次 ini，且只在这段时间运行。

刷新走 SKSE 任务接口，在游戏线程重建快照；渲染线程只读双缓冲，一次原子索引读。战斗、施法这些高频路径上没挂任何钩子，唯一的游戏侧钩子是热键用的输入事件。背包、模组控制面板、对话等菜单打开时，热键不响应，HUD 也不会盖在游戏界面上。

[b]前置[/b]

上古卷轴 5 特别版 1.5.97 或 周年纪念版 1.6.x（同一个 DLL 兼容两种布局）、SKSE64（版本与游戏匹配）、Address Library for SKSE Plugins（版本与游戏匹配），以及 [b]SKSE Menu Framework 3.x[/b]——它提供模组控制面板与 HUD 绘制层；没装它 BuffHUD 仍能加载并写一条警告日志，但不会画任何东西。

不需要 SkyUI、MCM Helper、任何 ESP 或其它 UI 模组。

[b]安装[/b]

用 MO2 或 Vortex 装上（和 SKSE Menu Framework 一起）。没有 ESP 要勾选，加载顺序里也不用加东西。进游戏打开 [b]Mod Control Panel → BuffHUD → 设置[/b] 调整；游戏里长按面板热键（默认 F）呼出效果列表。

日志在 `文档\My Games\Skyrim Special Edition\SKSE\BuffHUD.log`，启动时会打印读到的设置值、图标数、规则条数，以及是否检测到 SKSE Menu Framework。

[b]已知限制[/b]

· 只改属性值、不产生效果实例的加成（少数脚本驱动或天赋静态加成）不会列出  
· 手柄与 VR 输入尚未映射，热键只支持键盘；菜单打开时热键不响应  
· 详情区固定显示排序后的第一条，不能用鼠标选行——长按式热键没有指针可选  
· 详情正文取的是原版描述串并代入强度/时长/范围；模组本身没写描述时，这里显示「该效果没有提供详细说明」占位，不是空白 bug  
· 设置页标题在加载时注册一次，游戏中途换语言不会立刻改名，下次重进游戏才变

[b]署名[/b]

BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。

24 个图标为本项目自制的纯 alpha 单色描边字形，可随本模组自由使用。程序基于 CommonLibSSE-NG；界面跑在 SKSE Menu Framework 共享的 Dear ImGui 上下文里，ImGui 由框架提供，不包含在本下载中。

---

## 四、贴吧首楼

标题（≤30 字）：`【原创】BuffHUD v1.2——一身 buff 一页看完`

```
【原创】BuffHUD v1.2——一身 buff 一页看完

玩到后期，身上挂着一堆药水、祝福、附魔、诅咒，想看还剩多久，得按 Tab 翻两页激活效果，翻完还忘了刚看的是哪个。

这个 MOD 把它们收成一页：图标、名字、剩多久、谁给的，三栏对齐。长按 F，列表下方带一块详情区看完整说明；松手收起，短按可以钉住慢慢看。

功能
· 图标 + 名称 + 剩余时间 + 来源，三栏对齐
· 剩余时间跟着游戏走，暂停时它也停，不按现实时钟跑
· 长按默认 350 毫秒展开，短按钉住
· 没持续时间的效果显示「持续」，不瞎算数字
· 同类效果合成一条标 x3，超出 12 条折叠成「另有 N 项」
· 增益暖金、减益绯红，也能切回素白；三套配色、缩放透明度可调
· 由 SMF 的 HUD 层画，不挂 D3D11，开面板不暂停游戏
· 可选「仅战斗中显示」，脱战立刻收起

兼容
没有 FormID 白名单，凡是出现在角色激活效果列表里的都会被读出来——法术、药水、毒、疾病、祝福、种族能力、龙吼、食物、身上的附魔，以后新出的 MOD 也一样。图标靠 86 条规则匹配，一条都对不上的效果也不会消失，会落到通用图标。插件是纯 SKSE DLL 加配置和资源表，不含 ESP/ESL，不改任何原记录，不进加载顺序，不会冲突。

前置 / 安装
游戏 1.5.97 或 1.6.x 都行，同一个 DLL。需要 SKSE64、Address Library for SKSE Plugins（版本对上即可），以及 SKSE Menu Framework 3.x。不需要 SkyUI、MCM Helper，也不用勾 ESP。
MO2 装上即可，进游戏打开 Mod Control Panel → BuffHUD → 设置 调设置（一页 18 项），长按 F 看效果面板。设置在 Data/SKSE/Plugins/BuffHUD/BuffHUD.ini，手改也行，一秒内重载，不写存档。

已知问题
· 只改属性值、不产生效果实例的加成（少数脚本或天赋）读不到
· 手柄和 VR 还没做，热键只认键盘；开菜单时热键不响应
· 详情固定显示排序第一条，不能鼠标选行
· 原 MOD 没写描述的话，这里显示「该效果没有提供详细说明」

署名：BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。

下载见楼中楼。进游戏有问题回帖说，日志在 文档\My Games\Skyrim Special Edition\SKSE\BuffHUD.log。
```

配图 3 张就够：面板全貌、详情区、Mod Control Panel 里的设置页。

---

## 五、3DM 论坛

标题：`[原创] BuffHUD v1.2 —— 把身上所有增益减益收成一页（SKSE 插件，接入 SMF 模组控制面板，中英双语）`

```
[b][size=4]BuffHUD v1.2 —— 把身上所有增益减益收成一页[/size][/b]

[b]【说明】[/b]
把角色当前生效的所有状态效果收进一个面板，图标、名称、剩余时间、来源三栏对齐，排版参照 SkyUI 的激活效果页。
长按热键（默认 F，350 毫秒）展开面板，列表下方带详情区显示第一条效果的完整原版说明；松手收起，短按钉住。中英双语，设置页可随时切换。

[b]【功能】[/b]
· 图标 + 名称 + 剩余时间 + 来源，三栏对齐
· 剩余时间取自效果自身的游戏内计时，游戏暂停时同步停止
· 无持续时间的效果显示「持续」；同类合并标 x3；超出上限折叠成「另有 N 项」（默认 12 条）
· 增益暖金 / 减益绯红，可关着色；三套配色、70–200% 缩放、0–100% 不透明度、四角加居中共 5 个位置
· 由 SKSE Menu Framework 的 HUD 层绘制，不另起 D3D11 覆盖层，不暂停、不抢焦点
· 可选「仅战斗中显示」，脱战自动收起
· 面板关着时零开销；可见时每 250 毫秒刷新快照、每秒查一次 ini，渲染线程只读双缓冲

[b]【兼容性】[/b]
无 FormID 白名单，凡出现在角色激活效果列表里的都会被捕获：法术、药水、毒、疾病、祝福、种族能力、龙吼、食物、穿戴附魔，以及后续新增的 MOD。
图标由 86 条规则按关键字 / 原型 / 属性值匹配，无法匹配的效果不会被丢弃，回落到通用图标。
插件为纯 SKSE 插件（单个 DLL + 配置文件 + 资源表），不含 ESP/ESL，不改任何原有记录，不进加载顺序，不存在冲突。

[b]【前置】[/b]
· 上古卷轴 5 特别版 1.5.97 / 周年纪念版 1.6.x（同一 DLL 兼容两种布局）
· SKSE64（版本与游戏匹配）
· Address Library for SKSE Plugins（版本与游戏匹配）
· SKSE Menu Framework 3.x（提供模组控制面板与 HUD 绘制层；缺了它插件能加载但不显示）
· 不需要 SkyUI、MCM Helper、任何 ESP

[b]【安装】[/b]
1. 用 MO2 或 Vortex 安装压缩包，与 SKSE Menu Framework 3.x 一起
2. 装上即可，无需在插件列表勾选任何 ESP
3. 进游戏：Mod Control Panel → BuffHUD → 设置，一页 5 组共 18 项；长按 F 呼出效果面板

[b]【设置】[/b]
实际生效的设置写在 [code]Data/SKSE/Plugins/BuffHUD/BuffHUD.ini[/code]，可以直接手改，插件每秒检查一次时间戳，一秒内热生效。存档里不写任何设置。

[b]【已知问题】[/b]
· 只修改属性值、不产生效果实例的加成（少数脚本驱动或天赋静态加成）不会列出
· 手柄与 VR 输入尚未映射，热键仅支持键盘；菜单打开时热键不响应
· 详情区固定显示排序后的第一条，不支持鼠标选行
· 详情正文取自原版描述串，若来源 MOD 未填写描述则显示「该效果没有提供详细说明」占位

[b]【署名】[/b]
BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。

[b]【下载】[/b]
{{附件 / 网盘 + 提取码}}

日志：文档\My Games\Skyrim Special Edition\SKSE\BuffHUD.log
```

---

## 六、B站视频简介

```
BuffHUD｜一身 buff 一页看完｜上古卷轴5 MOD

v1.2 原创 · 接入 SKSE Menu Framework 模组控制面板 · 中英双语 · 支持 SE 1.5.97 与 AE 1.6.x

—— 时间轴 ——
00:00 一堆 buff 看不清的问题
00:25 面板全貌：图标 / 名称 / 剩余时间 / 来源
01:05 长按展开、短按钉住、详情区看什么
01:40 在 Mod Control Panel 里调 18 项设置
02:15 兼容性：为什么不需要白名单
02:45 安装（不用勾 ESP）

—— 前置 ——
上古卷轴 5 特别版 1.5.97 或 周年纪念版 1.6.x
SKSE64、Address Library for SKSE Plugins（版本与游戏匹配）、SKSE Menu Framework 3.x
不需要 SkyUI / MCM Helper

—— 下载 ——
见置顶

—— 已知问题 ——
· 只改属性值的加成读不到
· 手柄 / VR 未支持，开菜单时热键不响应
· 详情固定显示排序第一条，不能选行

—— 署名 ——
BuffHUD设计创意出自 Icarus ，本人享有创意署名权，项目工程由 WorkBuddy 中转DS-V4.1 Flash & V4-Pro 完成。受众人喜爱的项目，将在三个工作日内开源。
```

标签：`上古卷轴5` `天际` `SKSE` `MOD` `UI` `游戏模组`

---

## 七、更新日志

[b]1.2.0[/b]  
[list]  
[*]Replaced the standalone D3D11/ImGui overlay with [b]SKSE Menu Framework 3.x[/b]. Settings now live in the Mod Control Panel under [b]BuffHUD → Settings[/b], and the effect panel is drawn as an SMF HUD element — one shared ImGui context, no second Present hook, no imgui.dll in the download.  
[*]Removed the separate settings hotkey (F4) and its config/translation entries; the panel hotkey (F) is unchanged. Key capture now goes through the SKSE input sink.  
[*]Per-frame maintenance moved out of the old Present hook into the HUD callback: ini polled once a second, snapshot refreshed four times a second, only while the panel is visible.  
[*\]\[b]SKSE Menu Framework 3.x is now a requirement.\[/b] Without it the plugin still loads and logs a warning, but draws nothing.  
[*]Localised the Mod Control Panel page label (设置 / Settings), dropped the redundant hint line, and added the fixed Chinese credit notice to the About section — it is deliberately left untranslated in the English language pack.  
[/list]

[b]1.1.0[/b]  
[list]  
[*]Removed the MCM Helper dependency. The plugin now draws its own settings menu as a D3D11 overlay — no SkyUI, no MCM Helper, no ESP. Press the settings hotkey (F4 by default) to open it.  
[*]All menu text is localised through the bundled translation files (lang/zh.tsv, lang/en.tsv); the UI language switches from inside the menu.  
[*]Settings now persist to [code]Data/SKSE/Plugins/BuffHUD/BuffHUD.ini[/code] instead of the MCM folder.  
[/list]

[b]1.0.1[/b]  
[list]  
[*]Fixed: 1.5.97 players got "incompatible with the address library for this version of the game" on load. The DLL was built for AE only, so every address lookup used an AE id against the SE database.  
[*]The plugin is now a single build for SE 1.5.97 and AE 1.6.x. Same file, no separate download.  
[/list]

[b]1.0.0[/b]  
[list]  
[*]First release: effect list, long-press detail pane, 24 icons, settings menu, Chinese/English  
[/list]

---

## 八、回复支持者（历史存档，1.0.1 时期的话术，可直贴）

英文：

> That's on us, not on your install — and it's fixed.
>
> The 1.0.0 DLL was compiled for AE only. On 1.5.97 the plugin therefore asked the SE address library for AE ids; the first id that doesn't exist there aborts the load with exactly the message you saw.
>
> 1.0.1 is one DLL for both runtimes. Replace the file, and check that your log now says `built for SE + AE; this process is SE (1.5.97)`. You still need Address Library for SKSE Plugins (the SE 1.5.97 version) — that one is a hard requirement, not a workaround.
>
> If it still fails after that, send me the first ten lines of `Documents\My Games\Skyrim Special Edition\SKSE\BuffHUD.log`.

中文：

> 这是我们的问题，不是你的安装问题，已经修了。
>
> 1.0.0 的 DLL 只按 AE 编译。在 1.5.97 上它会拿 AE 的地址 id 去查 SE 的地址库，查到第一个不存在的 id 就直接中断加载，报的正是你看到的那句。
>
> 1.0.1 是同一份 DLL 同时支持 SE 与 AE，覆盖文件即可。装好后看一眼日志，应该出现 `built for SE + AE; this process is SE (1.5.97)`。另外 Address Library for SKSE Plugins（1.5.97 那一版）仍然必须装，这是硬性前置，不是绕路方案。
>
> 如果还是不行，把 `文档\My Games\Skyrim Special Edition\SKSE\BuffHUD.log` 前十行发我。
