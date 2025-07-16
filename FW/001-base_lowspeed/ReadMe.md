# 说明

此工程为CH32V208的基础模板。

默认情况下，以本工程为基础模板的工程主频不超过60M Hz,如需修改频率，应当同步修改链接脚本。

# 控制台

控制台将占用串口2(USART2),其引脚占用如下:

| 引脚编号 | 功能      | 备注                         |
| -------- | --------- | ---------------------------- |
| PA2      | USART2_TX |                              |
| PA3      | USART2_RX | 硬件上需要上拉(推荐100k电阻) |

默认情况下，将在控制台上运行一个`HShell`。

# 以太网

默认情况下,本工程将启用以太网(10M),其引脚占用如下：

| 引脚编号 | 功能 | 备注               |
| -------- | ---- | ------------------ |
| PC6      | RXP  | 固定分配，不可修改 |
| PC7      | RXN  | 固定分配，不可修改 |
| PC8      | TXP  | 固定分配，不可修改 |
| PC9      | TXN  | 固定分配，不可修改 |

## 帧格式

以太网发送与接收的数据帧均遵循以太网帧格式。

普通以太网数据帧格式如下：

| 前同步码 | SFD   | 目标地址 | 源地址 | 长度或类型 | 数据域           | CRC   |
| -------- | ----- | -------- | ------ | ---------- | ---------------- | ----- |
| 7字节    | 1字节 | 6字节    | 6字节  | 2字节      | 46字节～1500字节 | 4字节 |

- 
  前同步码：56 位（7 字节）交替的低电平和高电平跳跃，值固定，用十六进制表示即为 0xAA-0xAA-0xAA-0xAA-0xAA-0xAA-0xAA。此域用来进行时钟同步。该域由硬件自动添加/去除，用户不需要理会。
- SFD：帧首定界符，8 位（1 字节），值为 10101011b，SFD 用来提醒接收方这是最后一次进行时钟同步的机会，其后为目标地址。该字节由硬件自动添加/去除，用户不需要理会。
- 目标地址：发送这个帧的设备希望接收这个帧的设备地址，这里指的是硬件地址，又称 MAC 地址，由 IEEE 为生产商分配，全球唯一，6 字节 48 位。
  源地址：发送这个帧的设备的硬件地址。源地址必须为单播地址。
- 长度或类型：2 字节，以太网一般用作类型，IEEE802.3 标准中也用作长度，以 1536（即 0x0600）分界，1536 以上表示协议，表示数据部分是根据何种上层协议组织起来的，例如 0x0806 表示 ARP，0x0800 表示 IPv4，0x86dd 表示 IPv6；1536 以下表示数据长度。
- 数据域：最小 46 字节，最大 1500 字节。当数据域不足 46 字节需要加填充增至 46 字节，超过1500 字节请另组一帧。数据域中装载着需要实际发送的数据。
- CRC：循环冗余校验。

常见的以太网帧类型如下：

| 值 | 协议 |
| ------------ | ----------------------------------------------------------------------------------------------------------------------------- |
| 0x0800 | Internet Protocol Version 4 (IPv4) |
| 0x0801 | X.75 Internet |
| 0x0805 | X.25 Level 3 |
| 0x0806 | Address Resolution Protocol (ARP) |
| 0x0808 | Frame Relay ARP |
| 0x22F3 | TRILL |
| 0x22F4 | L2-IS-IS |
| 0x6558 |Trans Ether Bridging|
| 0x6559 |Raw Frame Relay|
| 0x8035 | Reverse Address Resolution Protocol (RARP) |
| 0x809b |Appletalk|
| 0x8100 |IEEE Std 802.1Q - Customer VLAN Tag Type (C-Tag, formerly called the Q-Tag) (initially Wellfleet)|
| 0x8137 |Novell NetWare IPX/SPX (old)|
| 0x8138 |Novell, Inc.|
| 0x814C |SNMP over Ethernet|
| 0x86DD |IP Protocol version 6 (IPv6)|
| 0x876B |TCP/IP Compression|
| 0x876C |IP Autonomous Systems|
| 0x876D |Secure Data|
| 0x8808 |IEEE Std 802.3 - Ethernet Passive Optical Network (EPON)|
| 0x880B |Point-to-Point Protocol (PPP)|
| 0x880C |General Switch Management Protocol (GSMP)|
| 0x8847 |MPLS (multiprotocol label switching)|
| 0x8848 |MPLS with upstream-assigned label|
| 0x8863 |PPP over Ethernet (PPPoE) Discovery Stage|
| 0x8864 |PPP over Ethernet (PPPoE) Session Stage|
| 0x888E |IEEE Std 802.1X - Port-based network access control|
| 0x88A8 |IEEE Std 802.1Q - Service VLAN tag identifier (S-Tag)|
| 0x88B7 |IEEE Std 802 - OUI Extended Ethertype|
| 0x88C7 |IEEE Std 802.11 - Pre-Authentication (802.11i)|
| 0x88CC |IEEE Std 802.1AB - Link Layer Discovery Protocol (LLDP)|
| 0x88E5 |IEEE Std 802.1AE - Media Access Control Security|
| 0x88F5 |IEEE Std 802.1Q - Multiple VLAN Registration Protocol (MVRP)|
| 0x88F6 |IEEE Std 802.1Q - Multiple Multicast Registration Protocol (MMRP)|
| 0x893B |TRILL Fine Grained Labeling (FGL)|
| 0x8946 |TRILL RBridge Channel|

**注意：**

- **无论是接收还是发送，软件中数据帧不包含前同步码与SFD,从目标地址开始。**
- **默认情况下，本工程中使用`TCP/IP`作为上层协议，但用户也可直接使用原始以太网帧，需要注意帧类型不能与常用的帧类型重合，否则可能引起混乱。**

## TCP/IP

默认情况下,采用[lwip](https://savannah.nongnu.org/projects/lwip/)作为TCP/IP栈，本工程中特点如下：

- 运行于`NO_SYS`模式（只能使用`raw api`）。
- 默认启用IPV4与IPV6双栈。
- IPV6默认启用`linklocal`地址，可使用TCP/IP在无配置( 无需路由)的情况下直接通信，这对设备的初始配置十分有用。
- 默认启用DHCP（IPV4与IPV6均启用），其中DHCP6采用无状态DHCP。
- 默认启用SNTP,可用于获取网络时间。

详细文档见[lwip_master.pdf](../../Doc/lwip_master.pdf)。

