#pragma once

#include "QuickGetIllegalItem.h"
#include "Config.h"

#include <ll/api/Config.h>
#include <ll/api/Logger.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/BlockActorDataPacket.h>
#include <mc/network/packet/ContainerClosePacket.h>
#include <mc/network/packet/ContainerOpenPacket.h>
#include <mc/network/packet/InventorySlotPacket.h>
#include <mc/network/packet/PlaySoundPacket.h>
#include <mc/network/packet/SetLocalPlayerAsInitializedPacket.h>
#include <mc/network/packet/UpdateBlockPacket.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/inventory/network/ItemStackRequestAction.h>
#include <mc/world/inventory/network/ItemStackRequestActionHandler.h>
#include <mc/world/inventory/network/ItemStackRequestActionTransferBase.h>
#include <mc/world/item/components/ItemStackRequestActionType.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

QuickGetIllegalItem::Config                                                     config;
std::unordered_map<mce::UUID, std::pair<std::string, std::pair<BlockPos, int>>> actions;
ll::Logger                                                                      logger("QuickGetIllegalItem");

namespace QuickGetIllegalItem {

static std::unique_ptr<Entry> instance;

Entry& Entry::getInstance() { return *instance; }

bool Entry::load() {
    const auto path = getSelf().getConfigDir() / "config.json";
    try {
        if (!ll::config::loadConfig(config, path)) {
            ll::config::saveConfig(config, path);
        }
    } catch (...) {
        ll::config::saveConfig(config, path);
    }
    return true;
}

bool Entry::enable() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        config.command.command,
        "快捷获取非法方块",
        config.command.permLevel,
        config.command.flags
    );
    if (!config.command.alias.empty()) cmd.alias(config.command.alias);
    struct CmdParam {
        std::string typeName = "default";
    };
    cmd.overload<CmdParam>()
        .optional("typeName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, CmdParam const& result) -> void {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) return output.error("非玩家不可执行。");
            if (!config.illegalItems.contains(result.typeName)) return output.error("无此分类。");
            output.success("§6正在为您打开界面");
            auto* player = static_cast<Player*>(entity);
            // 容器方块坐标
            auto pos = player->getFeetBlockPos().add({0, 3, 0});

            // 更新方块类型(用于改变方块类型)
            UpdateBlockPacket(
                pos,
                static_cast<uint>(UpdateBlockPacket::BlockLayer::Standard),
                Block::tryGetFromRegistry("minecraft:chest")->getRuntimeId(), // chest可以改成别的，是容器就行
                static_cast<uchar>(BlockUpdateFlag::All)
            )
                .sendTo(*player);

            // 以下是修改方块实体NBT的发包，用于第一次更新物品，如果想要在打开容器的情况下更新物品，需要使用InventorySlotPacket或InventoryContentPacket数据包
            auto nbt             = CompoundTag();
            nbt.at("Findable")   = std::byte(1); // 未知，我也不知道干啥的
            nbt.at("id")         = "Chest";      // 方块实体的ID
            nbt.at("isMovable")  = std::byte(1); // 是否可以被活塞推动
            nbt.at("x")          = int(pos.x);   // 方块坐标的X轴
            nbt.at("y")          = int(pos.y);   // 方块坐标的Y轴
            nbt.at("z")          = int(pos.z);   // 方块坐标的Z轴
            nbt.at("CustomName") = fmt::format(
                "§r§d正在获取分类 §e{} §d的物品",
                result.typeName
            ); // 箱子的名称，也就是你打开容器，容器左上角显示的，可以通命名箱子物品再放置显示
            auto items = ListTag();                                      // 容器内的物品nbt列表
            for (auto& snbt : config.illegalItems.at(result.typeName)) { // 依次读取并写入物品NBT
                try {
                    auto itemNbt       = CompoundTag::fromSnbt(snbt).value();
                    itemNbt.at("Slot") = std::byte(items.size()); // 写入槽位(重要！)
                    items.add(itemNbt);
                } catch (...) {
                    getSelf().getLogger().error(
                        "在读取NBT {}-{} 的时候发生错误。",
                        result.typeName,
                        std::to_string(items.size())
                    );
                }
            }
            nbt.at("Items") = items;                        // 写入物品列表NBT
            BlockActorDataPacket(pos, nbt).sendTo(*player); // 构建并发送更新方块实体NBT数据包

            ContainerOpenPacket( // 构建并发送打开容器数据包，帮玩家打开容器(请注意，如果是打开大箱子，需要延迟一下)
                static_cast<ContainerID>(-64), // 这个地方比较特殊，这是容器的ID，不是类型ID，是会话ID(一般写死就行)
                ContainerType::Container,
                pos,
                ActorUniqueID(-1) // 如果要打开实体的容器就要写上实体的UniqueID，如果是方块，一定要写-1 一定要写-1！！！
            )
                .sendTo(*player);

            actions.try_emplace( // 记录缓存
                player->getUuid(),
                std::pair<std::string, std::pair<BlockPos, int>>(
                    result.typeName,
                    std::pair(pos, player->getDimensionId().id)
                )
            );

            logger.debug("玩家 {} 打开GUI 分类:{}", player->getRealName(), result.typeName);
        });
    return true;
}

bool Entry::disable() { return true; }

bool Entry::unload() { return true; }

} // namespace QuickGetIllegalItem

LL_REGISTER_MOD(QuickGetIllegalItem::Entry, QuickGetIllegalItem::instance);

// 处理物品请求Hook
LL_AUTO_TYPE_INSTANCE_HOOK(
    HandleRequestActionHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    "?handleRequestAction@ItemStackRequestActionHandler@@QEAA?AW4ItemStackNetResult@@AEBVItemStackRequestAction@@@Z",
    ItemStackNetResult,
    ItemStackRequestAction& requestAction
) {
    auto& action = static_cast<ItemStackRequestActionTransferBase&>(requestAction);
    if (!actions.contains(mPlayer.getUuid())
        || (action.mActionType != ItemStackRequestActionType::Take
            && action.mActionType != ItemStackRequestActionType::TakeFromItemContainer
            && action.mActionType != ItemStackRequestActionType::Place
            && action.mActionType != ItemStackRequestActionType::Drop)
        || action.mSrc.mOpenContainerNetId != ContainerEnumName::LevelEntityContainer
        || action.mDst.mOpenContainerNetId == ContainerEnumName::LevelEntityContainer)
        return origin(requestAction);
    logger.debug(
        "玩家 {} 获取非法物品 格子位置:{} 数量:{} SNBT:{}",
        mPlayer.getRealName(),
        std::to_string(action.mSrc.mSlot),
        std::to_string(action.mAmount),
        config.illegalItems.at(actions.at(mPlayer.getUuid()).first).at(action.mSrc.mSlot)
    );
    try {
        // 先将SNBT转换成物品
        auto item = ItemStack::fromTag(
            CompoundTag::fromSnbt(config.illegalItems.at(actions.at(mPlayer.getUuid()).first).at(action.mSrc.mSlot))
                .value()
        );
        item.mCount = action.mAmount; // 覆写物品数量
        // 丢出或给予物品(就这样吧，懒癌犯了)
        action.mActionType == ItemStackRequestActionType::Drop ? mPlayer.drop(item, false)
                                                               : mPlayer.addAndRefresh(item);
    } catch (...) {
        logger.error(
            "玩家 {} 获取非法物品发生错误！ SNBT:{}",
            mPlayer.getRealName(),
            config.illegalItems.at(actions.at(mPlayer.getUuid()).first).at(action.mSrc.mSlot)
        );
    }
    PlaySoundPacket("random.orb", mPlayer.getPosition(), 1.0f, 1.2f).sendTo(mPlayer); // 播放声音
    return ItemStackNetResult::
        Error; // 返回错误，拦截这次物品请求(前面已经处理好了，并且会刷新，这里就不用了，可能会出问题)
}

// 处理关闭容器数据包Hook
LL_AUTO_TYPE_INSTANCE_HOOK(
    ContainerClosePacketSendHook,
    HookPriority::Low,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVContainerClosePacket@@@Z",
    void,
    NetworkIdentifier&    identifier,
    ContainerClosePacket& packet
) {
    origin(identifier, packet);
    auto player = getServerPlayer(identifier, packet.mClientSubId);
    if (player.has_value() && actions.contains(player->getUuid())) {
        try {
            // 恢复方块
            // 这里先获取一下要恢复的方块
            auto& block = ll::service::getLevel()
                              ->getDimension(actions.at(player->getUuid()).second.second)
                              ->getBlockSourceFromMainChunkSource()
                              .getBlock(actions.at(player->getUuid()).second.first);
            // 先更新方块类型
            UpdateBlockPacket(
                actions.at(player->getUuid()).second.first,
                static_cast<uint>(UpdateBlockPacket::BlockLayer::Standard),
                block.getRuntimeId(),
                static_cast<uchar>(BlockUpdateFlag::All)
            )
                .sendTo(*player);
            // 再更新方块实体数据(如头颅什么的，虽然可以不写，但建议写一下)
            BlockActorDataPacket(actions.at(player->getUuid()).second.first, block.getSerializationId())
                .sendTo(*player);
        } catch (...) {}
        logger.debug("玩家 {} 关闭GUI", player->getRealName());
        actions.erase(player->getUuid());
    }
}

// 玩家进服Hook
LL_AUTO_TYPE_INSTANCE_HOOK(
    PlayerJoinHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::handle,
    void,
    NetworkIdentifier const&                 identifier,
    SetLocalPlayerAsInitializedPacket const& packet
) {
    origin(identifier, packet);
    // 没啥好说的，就清除缓存
    auto player = getServerPlayer(identifier, packet.mClientSubId);
    if (player.has_value() && actions.contains(player->getUuid())) actions.erase(player->getUuid());
}