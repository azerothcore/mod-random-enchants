/*
* Converted from the original LUA script to a module for Azerothcore(Sunwell) :D
*/
#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "Chat.h"

class RandomEnchantsPlayer : public PlayerScript {
private:
    // Helper function to retrieve a configuration option
    template <typename T>
    T GetConfigOption(const std::string& optionName, const T& defaultValue) {
        if constexpr (std::is_same_v<T, double>) {
            // Retrieve the config value as a string
            std::string strValue = sConfigMgr->GetOption<std::string>(optionName, std::to_string(defaultValue));
            // Convert and return the string value to double
            return std::stod(strValue);
        }
        else {
            return sConfigMgr->GetOption<T>(optionName, defaultValue);
        }
    }

public:
    RandomEnchantsPlayer() : PlayerScript("RandomEnchantsPlayer") {}

    void OnLogin(Player* player) override {
        if (GetConfigOption<bool>("RandomEnchants.AnnounceOnLogin", true))
            ChatHandler(player->GetSession()).SendSysMessage(GetConfigOption<std::string>("RandomEnchants.OnLoginMessage", "This server is running a RandomEnchants Module.").c_str());
    }

    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override {
        if (GetConfigOption<bool>("RandomEnchants.OnLoot", true))
            RollPossibleEnchant(player, item);
    }

    void OnCreateItem(Player* player, Item* item, uint32 /*count*/) override {
        if (GetConfigOption<bool>("RandomEnchants.OnCreate", true))
            RollPossibleEnchant(player, item);
    }

    void OnQuestRewardItem(Player* player, Item* item, uint32 /*count*/) override {
        if (GetConfigOption<bool>("RandomEnchants.OnQuestReward", true))
            RollPossibleEnchant(player, item);
    }

    void RollPossibleEnchant(Player* player, Item* item) {
        uint32 Quality = item->GetTemplate()->Quality;
        uint32 Class = item->GetTemplate()->Class;

        if ((Quality > 5 || Quality < 1) || (Class != 2 && Class != 4)) {
            return;
        }

        int slotRand[3] = { -1, -1, -1 };
        uint32 slotEnch[3] = { 0, 1, 5 };

        // Retrieve the enchant chances from the config
        double roll1 = rand_chance();
        double enchantChance1 = GetConfigOption<double>("RandomEnchants.EnchantChance1", 70.0);
        if (roll1 >= enchantChance1)
            slotRand[0] = getRandEnchantment(item);

        if (slotRand[0] != -1) {
            double roll2 = rand_chance();
            double enchantChance2 = GetConfigOption<double>("RandomEnchants.EnchantChance2", 65.0);
            if (roll2 >= enchantChance2)
                slotRand[1] = getRandEnchantment(item);

            if (slotRand[1] != -1) {
                double roll3 = rand_chance();
                double enchantChance3 = GetConfigOption<double>("RandomEnchants.EnchantChance3", 60.0);
                if (roll3 >= enchantChance3)
                    slotRand[2] = getRandEnchantment(item);
            }
        }

        for (int i = 0; i < 2; i++) {
            if (slotRand[i] != -1) {
                if (sSpellItemEnchantmentStore.LookupEntry(slotRand[i])) {
                    player->ApplyEnchantment(item, EnchantmentSlot(slotEnch[i]), false);
                    item->SetEnchantment(EnchantmentSlot(slotEnch[i]), slotRand[i], 0, 0);
                    player->ApplyEnchantment(item, EnchantmentSlot(slotEnch[i]), true);
                }
            }
        }

        ChatHandler chathandle = ChatHandler(player->GetSession());
        if (slotRand[2] != -1)
            chathandle.PSendSysMessage("Newly Acquired |cffFF0000 %s |rhas received|cffFF0000 3 |rrandom enchantments!", item->GetTemplate()->Name1.c_str());
        else if (slotRand[1] != -1)
            chathandle.PSendSysMessage("Newly Acquired |cffFF0000 %s |rhas received|cffFF0000 2 |rrandom enchantments!", item->GetTemplate()->Name1.c_str());
        else if (slotRand[0] != -1)
            chathandle.PSendSysMessage("Newly Acquired |cffFF0000 %s |rhas received|cffFF0000 1 |rrandom enchantment!", item->GetTemplate()->Name1.c_str());
    }

    int getRandEnchantment(Item* item) {
        uint32 Class = item->GetTemplate()->Class;
        std::string ClassQueryString = "";
        switch (Class) {
        case 2:
            ClassQueryString = "WEAPON";
            break;
        case 4:
            ClassQueryString = "ARMOR";
            break;
        }

        if (ClassQueryString == "")
            return -1;

        uint32 Quality = item->GetTemplate()->Quality;
        int rarityRoll = -1;

        // Retrieve rarity rolls from the config
        switch (Quality) {
        case 0://grey
            rarityRoll = rand_norm() * GetConfigOption<int>("RandomEnchants.RarityRollGrey", 25);
            break;
        case 1://white
            rarityRoll = rand_norm() * GetConfigOption<int>("RandomEnchants.RarityRollWhite", 50);
            break;
        case 2://green
            rarityRoll = 45 + (rand_norm() * GetConfigOption<int>("RandomEnchants.RarityRollGreen", 20));
            break;
        case 3://blue
            rarityRoll = 65 + (rand_norm() * GetConfigOption<int>("RandomEnchants.RarityRollBlue", 15));
            break;
        case 4://purple
            rarityRoll = 80 + (rand_norm() * GetConfigOption<int>("RandomEnchants.RarityRollPurple", 14));
            break;
        case 5://orange
            rarityRoll = GetConfigOption<int>("RandomEnchants.RarityRollOrange", 93);
            break;
        }

        if (rarityRoll < 0)
            return -1;

        int tier = 0;
        if (rarityRoll <= 44)
            tier = 1;
        else if (rarityRoll <= 64)
            tier = 2;
        else if (rarityRoll <= 79)
            tier = 3;
        else if (rarityRoll <= 92)
            tier = 4;
        else
            tier = 5;

        QueryResult qr = WorldDatabase.Query("SELECT enchantID FROM item_enchantment_random_tiers WHERE tier='{}' AND exclusiveSubClass=NULL AND class='{}' OR exclusiveSubClass='{}' OR class='ANY' ORDER BY RAND() LIMIT 1", tier, ClassQueryString, item->GetTemplate()->SubClass);
        return qr->Fetch()[0].Get<uint32>();
    }
};

void AddRandomEnchantsScripts() {
    new RandomEnchantsPlayer();
}
