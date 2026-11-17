Scriptname BuffHUD extends MCM_ConfigBase
{ BuffHUD —— 状态效果面板的 MCM 注册入口。
  这个脚本故意不含任何界面逻辑：
    * 菜单结构、控件类型、取值范围与默认值全部写在
      Data/MCM/Config/BuffHUD/config.json 里；
    * 用户改动由 MCM Helper 落到 Data/MCM/Settings/BuffHUD.ini；
    * BuffHUD.dll 监听这份 ini 的写入时间，1 秒内热生效，
      所以不需要 Papyrus 把设置转发给插件，也就没有脚本侧的帧开销。
  它存在的唯一理由：SkyUI 只通过「挂在 Quest 上的、继承 SKI_ConfigBase 的脚本」
  来发现模组，而 MCM Helper 用脚本名（BuffHUD）去匹配同名配置目录。
  改动本文件后必须重新编译成 Scripts/BuffHUD.pex。
}

; 不重写 OnConfigInit / OnPageReset / OnSettingChange。
; MCM Helper 的 MCM_ConfigBase 已经在原生层实现它们（LoadConfig 由
; OnConfigManagerReady 触发）；重写反而会把 JSON 的加载逻辑顶掉。
