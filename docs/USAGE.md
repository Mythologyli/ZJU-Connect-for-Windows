# 使用方式

在本项目的 [releases](https://github.com/chenx-dust/HITsz-Connect-for-Windows/releases) 页面下载最新版本的安装包，无需安装，解压后直接运行 `HITszConnectForWindows.exe` 即可。

![main.png](/docs/main.png)

在 功能-设置-通用 页面中输入学号和密码，点击 OK，即可连接服务器。若连接正常，软件中日志会显示 `KeepAlive: OK`。

![config.png](/docs/config.png)

若只需要连接校园内网而不使用其他代理，可以在页面中选择“设置系统代理”;

若需要使用 Clash 等其他代理软件，可以清空本软件的系统代理，在 功能-设置-高级 页面中发现 SOCK5 的代理端口为 11080，则需要在 Clash 的代理配置中添加一个代理服务器：

```yaml
# 代理服务器
proxies:
  - name: "🖥 HITsz Connect"
    type: socks5
    server: 127.0.0.1
    port: 11080
    udp: true
```

并在代理组中添加一个单独的代理组：

```yaml
proxy-groups:
  - name: "🏫 校园网"
    type: select
    proxies:
      - DIRECT
      - 🖥 HITsz Connect
```

并在规则中加入：

```yaml
rules:
  - DOMAIN,vpn.hitsz.edu.cn,DIRECT
  - DOMAIN-SUFFIX,hitsz.edu.cn,🏫 校园网
  - IP-CIDR,10.0.0.0/8,🏫 校园网,no-resolve
  # 可在此添加其它你需要代理的 ip 段，如课程中心
```

这样即可通过简单的切换实现在校外使用本项目时选择 HITsz Connect 代理，在校内使用 DIRECT 直连。

![proxy_group.png](/docs/proxy_group.png)
