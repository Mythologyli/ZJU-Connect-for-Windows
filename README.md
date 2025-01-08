# HITsz Connect for Windows

![Github Action](https://github.com/chenx-dust/HITsz-Connect-for-Windows/actions/workflows/build.yml/badge.svg)

针对哈工大深圳的修改版 ZJU-Connect-for-Windows

欢迎加入 HITSZ 开源技术协会 [@hitszosa](https://github.com/hitszosa)

## 使用方式

1. 在本项目的 [Releases](https://github.com/chenx-dust/HITsz-Connect-for-Windows/releases) 页面下载最新版本，将所有文件解压至同一目录下，运行 `HITszConnectForWindows.exe` ；

<div align="center">
<img src="docs/main.png" width="600px">
</div>

2. 在 “功能”-“设置”-“通用” 页面中输入账号（一般为学号）和密码，点击 OK 保存登录信息；

<div align="center">
<img src="docs/config.png" width="400px">
</div>

3. 在主界面中点击“连接服务器”。在默认配置下，若连接正常，软件中日志会显示 `KeepAlive: OK` ；

4. 如果只需进行校园网页浏览，则选择“设置系统代理”后即可使用。

如果需要配合 Clash / Mihomo 进行高级的分流操作，可以参见： [高级使用方式](/docs/ADVANCED_USAGE.md)

## 路线图

如有更多好的建议，可以在 Issue 中或是 OSA 群里提出！

- [X] 支持 macOS 系统
- [ ] 支持 Linux 系统
- [ ] 支持手动设置 Proxy Bypass

## 致谢

- [Mythologyli/ZJU-Connect-for-Windows](https://github.com/Mythologyli/ZJU-Connect-for-Windows)
- [Mythologyli/zju-connect](https://github.com/Mythologyli/zju-connect)
