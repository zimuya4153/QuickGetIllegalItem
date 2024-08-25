# QuickGetIllegalItem - 快捷获取非法物品

# 简介
可以打开GUI界面快速的获取想要的非法物品   
~~感觉就是一个Java的那个快捷保存与加载，只不过少了个保存~~
> 想要了解箱子界面怎么弄的开发者可以自行查看代码，已写注释   

> 使用BSD开源协议，您可以自由的修改与分发插件，但请务必保留原作者信息

![图片展示](https://github.com/zimuya4153/QuickGetIllegalItem/blob/main/image/1.png?raw=true)

## 安装方法

- 手动安装
  - 前往[Releases](https://github.com/zimuya4153/QuickGetIllegalItem/releases)下载最新版本的`QuickGetIllegalItem-windows-x64.zip`
  - 解压`压缩包内的`文件夹到`./plugins/`目录
- Lip 安装
  - 输入命令`lip install -y github.com/zimuya4153/QuickGetIllegalItem`
- ~~一条龙安装~~
  - ~~去 Q 群，喊人，帮你安装~~

## 配置文件(一般情况下不用动)
```jsonc
{
    "version": 1, // 配置文件版本(勿动)
    "command": {
        "command": "illegalitem", // 命令名称(小写英文)
        "alias": "it", // 命令别名(小写英文)
        "permLevel": "GameDirectors", // 命令执行权限(0:Any 1:GameDirectors 2:Admin 3:Host 4:Owner 5:Internal)
        "flags": "None" // 命令flags
    },
    "illegalItems": { // 非法物品
        "default": [ // default分类，default可以改成任意字符串(内容是SNBT实例，想要更多可以自己加)
            "{\"Count\":64b,\"Name\":\"minecraft:end_gateway\"}", // 末地折跃门
            "{\"Count\":64b,\"Name\":\"minecraft:end_portal\"}", // 末地传送门
            "{\"Count\":64b,\"Name\":\"minecraft:portal\"}", // 地狱传送门
            "{\"Count\":64b,\"Name\":\"minecraft:glowingobsidian\"}", // 发光的黑曜石
            "{\"Count\":64b,\"Name\":\"minecraft:netherreactor\"}", // 下界反应堆
            "{\"Count\":64b,\"Name\":\"minecraft:invisiblebedrock\"}", // 隐形基岩
            "{\"Count\":64b,\"Name\":\"minecraft:fire\"}", // 火
            "{\"Count\":64b,\"Name\":\"minecraft:soul_fire\"}", // 灵魂火
            "{\"Count\":64b,\"Name\":\"minecraft:water\"}", // 水
            "{\"Count\":64b,\"Name\":\"minecraft:flowing_water\",\"tag\":{\"display\":{\"Name\":\"§r§f流动水§r\"}}}", // 流动水(我这里为了便于区分，加了个命名)
            "{\"Count\":64b,\"Name\":\"minecraft:lava\"}", // 岩浆
            "{\"Count\":64b,\"Name\":\"minecraft:flowing_lava\"}" // 熔岩(流动的岩浆)
        ]
    }
}
```