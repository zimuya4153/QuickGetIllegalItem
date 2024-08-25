#pragma once

#include <mc/server/commands/CommandFlag.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <string>
#include <unordered_map>
#include <vector>


namespace QuickGetIllegalItem {
struct Config {
    int version = 1; // 配置文件版本
    struct {
        std::string            command   = "illegalitem";                         // 指令名称
        std::string            alias     = "it";                                  // 指令别名
        CommandPermissionLevel permLevel = CommandPermissionLevel::GameDirectors; // 指令权限
        CommandFlagValue       flags     = CommandFlagValue::None;                // 指令类型
    } command;
    std::unordered_map<std::string, std::vector<std::string>> illegalItems =
        {
            // 非法物品列表 {"分类名称":["非法物品snbt"]} 每个分类最多27(别问，问就是懒:()
            {
             "default", {"{\"Count\":64b,\"Name\":\"minecraft:end_gateway\"}",      // 末地折跃门
                 "{\"Count\":64b,\"Name\":\"minecraft:end_portal\"}",       // 末地传送门
                 "{\"Count\":64b,\"Name\":\"minecraft:portal\"}",           // 地狱传送门
                 "{\"Count\":64b,\"Name\":\"minecraft:glowingobsidian\"}",  // 发光的黑曜石
                 "{\"Count\":64b,\"Name\":\"minecraft:netherreactor\"}",    // 下界反应堆
                 "{\"Count\":64b,\"Name\":\"minecraft:invisiblebedrock\"}", // 隐形基岩
                 "{\"Count\":64b,\"Name\":\"minecraft:fire\"}",             // 火
                 "{\"Count\":64b,\"Name\":\"minecraft:soul_fire\"}",        // 灵魂火
                 "{\"Count\":64b,\"Name\":\"minecraft:water\"}",            // 水
                 "{\"Count\":64b,\"Name\":\"minecraft:flowing_water\",\"tag\":{\"display\":{\"Name\":\"§r§f流动水§r\"}}}", // 流动水
                 "{\"Count\":64b,\"Name\":\"minecraft:lava\"}",         // 岩浆
                 "{\"Count\":64b,\"Name\":\"minecraft:flowing_lava\"}", // 流动岩浆
                 "{\"Count\":1b,\"Name\":\"minecraft:wooden_sword\",\"WasPickedUp\":0b,\"tag\":{"
                 "\"Unbreakable\":1b,\"display\":{\"Lore\":[\"§r§9不可破坏§r\",\"§r§bb§6y§a:§g小小的子沐呀 "
                 "§cQQ§a:§p1756150362§r\"],\"Name\":\"§r§l§e子§a沐§b的§d桃木剑§r\"},\"ench\":[{\"id\":9s,\"lvl\":"
                 "32767s},{"
                 "\"id\":13s,\"lvl\":32767s},{\"id\":14s,\"lvl\":100s},{\"id\":17s,\"lvl\":32767s},{\"id\":12s,\"lvl\":"
                 "32767s}"
                 ",{\"id\":26s,\"lvl\":32767s},{\"id\":10s,\"lvl\":32767s},{\"id\":11s,\"lvl\":32767s}],\"keep_on_"
                 "death\":1b}}"}
            }
    };
};
} // namespace QuickGetIllegalItem