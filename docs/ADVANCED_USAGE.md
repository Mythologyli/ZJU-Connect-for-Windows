# 高级使用方式

## Clash / Mihomo 分流配置

若需要使用 Clash / Mihomo 等其他代理软件，可以清空本软件的系统代理，在 “功能”-“设置”-“高级” 页面中设置 SOCK5 的代理端口（以下以 11080 为例）。以下为推荐配置：

在 Clash 的代理配置中添加一个代理服务器：

```yaml
# 代理服务器
proxies:
  - name: 🖥 EZ4Connect
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
      - 🖥 EZ4Connect
```

并在规则中加入：

```yaml
rules:
  - DOMAIN,vpn.hitsz.edu.cn,DIRECT
  - DOMAIN-SUFFIX,hitsz.edu.cn,🏫 校园网
  - IP-CIDR,10.0.0.0/8,🏫 校园网,no-resolve
  # 可在此添加其它你需要代理的 ip 段，如课程中心
```

这样即可通过简单的切换实现在校外使用本项目时选择 EZ4Connect 代理，在校内使用 DIRECT 直连。

![proxy_group.png](/docs/proxy_group.png)

## TUN 模式

（这里是个坑，没填的坑）
