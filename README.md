# ZJU Connect for Windows

基于 Qt 编写的 ZJU 网络客户端

![](docs/main.png)

## 功能

- [x] 有线网 L2TP 登录
  - [x] 创建 L2TP VPN 并解决注册表问题
  - [x] 一键设置/删除静态路由
  - [x] 检测网络并自动重连
- [x] 基于 [zju-web-login](https://github.com/Mythologyli/zju-web-login) 的 有线/无线网页认证登录
  - [x] 登录/登出
  - [x] 选择网络接口
- [x] 基于 [zju-connect](https://github.com/Mythologyli/zju-connect) 的 RVPN
  - [x] SOCKS5/HTTP 代理
  - [x] 端口转发
  - [x] 自动保活/自动重连
  - [x] 配合 [ZJU Rule](https://github.com/Mythologyli/ZJU-Rule) 使用
- [x] 开机启动并连接
- [x] 网络诊断

## 致谢

+ [EasierConnect](https://github.com/lyc8503/EasierConnect) 原作者 [lyc8503](https://github.com/lyc8503)
+ [zju-connect](https://github.com/Mythologyli/zju-connect) 的各位贡献者
+ [zju-web-login](https://github.com/Mythologyli/zju-web-login) 原作者 [Azuk 443](https://azuk.top/)