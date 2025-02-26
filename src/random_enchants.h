/*
* Converted from the original LUA script to a module for Azerothcore (Sunwell)
*/

#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "Chat.h"

enum ItemQuality
{
    GREY = 0,
    WHITE,
    GREEN,
    BLUE,
    PURPLE,
    ORANGE
};

void rollPossibleEnchant(Player* player, Item* item);
[[nodiscard]] uint32 getRandEnchantment(Item* item);

class RandomEnchantsPlayer : public PlayerScript
{
public:
    RandomEnchantsPlayer() : PlayerScript("RandomEnchantsPlayer", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOOT_ITEM,
        PLAYERHOOK_ON_CREATE_ITEM,
        PLAYERHOOK_ON_QUEST_REWARD_ITEM,
        PLAYERHOOK_ON_GROUP_ROLL_REWARD_ITEM
    }) { }

    void OnPlayerLogin(Player* player) override;
    void OnPlayerLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override;
    void OnPlayerCreateItem(Player* player, Item* item, uint32 /*count*/) override;
    void OnPlayerQuestRewardItem(Player* player, Item* item, uint32 /*count*/) override;
    void OnPlayerGroupRollRewardItem(Player* player, Item* item, uint32 /*count*/, RollVote /*voteType*/, Roll* /*roll*/) override;
};

void AddRandomEnchantsScripts()
{
    new RandomEnchantsPlayer();
}
