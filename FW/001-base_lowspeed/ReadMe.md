# 说明

此工程为CH32V208的基础模板。

默认情况下，以本工程为基础模板的工程主频为48MHz,如需修改频率，应当同步修改链接脚本。

# 控制台

控制台将占用串口2(USART2),其引脚占用如下:

| 引脚编号 | 功能      | 备注                         |
| -------- | --------- | ---------------------------- |
| PA2      | USART2_TX |                              |
| PA3      | USART2_RX | 硬件上需要上拉(推荐100k电阻) |

默认情况下，将在控制台上运行一个`HShell`。