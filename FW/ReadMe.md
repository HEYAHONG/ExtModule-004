# 说明

CH32V208系列是基于32位RISC-V设计的微控制器。

## 容量配置

CH32V208可支持容量配置,可使用的容量配置如下:

- 128K ROM + 64K RAM
- 144K ROM + 48K RAM
- 160K ROM + 32K RAM

如未特殊说明,本工程默认采用128K ROM + 64K RAM。

CH32V208的最大代码空间为`Code Flash`的大小(480K),但需要注意的是超过容量配置的代码因为运行在Flash非0等待区应当降速运行。

对于需要高速运行的代码，应当放置在容量配置的Flash0等待区。

# 工具

## MRS2

MRS2是一个可用于WCH芯片开发的集成开发环境,其可集开发、调试于一体,类似MDK5。

官网:[http://www.mounriver.com/](http://www.mounriver.com/)

注意:本工程要求MRS的版本不低于2.2.0。

# 目录说明

- [数字]-[名称]：数字为固件编号，名称为固件名称。
- [3rdparty](3rdparty):第三方源代码。

# 固件列表

- [001-base_lowspeed](001-base_lowspeed):低速模式(采用48MHz主时钟,可能使用额外的`Code Flash`区域)的MCU模板。