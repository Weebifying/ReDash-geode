#include "ui/RDStatsNode.hpp"
#include "ui/RDMainButton.hpp"

#include "hooks/onBackHooks.hpp"
#include "hooks/GameLevelManager.hpp"
#include "hooks/PlayLayer.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

#include <fmt/core.h>
#include <string>

#define MAX_SECRET_COINS 164

std::string getPathString(int n) {
	switch (n) {
		case 1: return "Fire";
		case 2: return "Ice";
		case 3: return "Poison";
		case 4: return "Shadow";
		case 5: return "Lava";
		case 6: return "Earth";
		case 7: return "Blood";
		case 8: return "Metal";
		case 9: return "Light";
		case 10: return "Souls";
		default: return "None";
	}
}

class $modify(CrazyLayer, MenuLayer) {
	static void onModify(auto& self) {
        (void)self.setHookPriority("MenuLayer::init", INT_MIN / 2 + 1); // making sure it's run before pages api
    }

	void onHideMenu(CCObject* sender) {
		if (!as<CCMenuItemToggler*>(sender)->isOn()) {
			this->getChildByID("redash-menu"_spr)->getChildByID("main-menu"_spr)->setVisible(false);
			this->getChildByID("redash-menu"_spr)->getChildByID("dailies-menu"_spr)->setVisible(false);
		} else {
			this->getChildByID("redash-menu"_spr)->getChildByID("main-menu"_spr)->setVisible(true);
			this->getChildByID("redash-menu"_spr)->getChildByID("dailies-menu"_spr)->setVisible(true);
		}
	}

	bool init() {
		if (!MenuLayer::init()) return false;

		auto loader = Loader::get();
		auto mod = Mod::get();
		auto gsm = GameStatsManager::sharedState();
		auto glm = GameLevelManager::sharedState();
		auto gm = GameManager::sharedState();
		
		if (Variables::WeeklyLeft < 1) {
			glm->getGJDailyLevelState(GJTimedLevelType::Weekly);
		}
		if (Variables::DailyLeft < 1) {
			glm->getGJDailyLevelState(GJTimedLevelType::Daily);
		}
		if (GJAccountManager::get()->m_accountID == 0) {
			Variables::GlobalRank = -1;
		} else if (Variables::OldStarsCount != gsm->getStat("6")) {
			Variables::GlobalRank = 0;
			glm->getLeaderboardScores("leaderboard_global");
		}

		if (auto closeMenu = this->getChildByID("close-menu")) {
			if (!closeMenu->getChildByID("close-button")) {
				auto cls_spr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
				cls_spr->setScale(0.7);
				auto cls_btn = CCMenuItemSpriteExtra::create(
					cls_spr, 
					this, 
					menu_selector(MenuLayer::onQuit));
				cls_btn->setID("close-button");
				closeMenu->addChild(cls_btn);
			}

			CCArrayExt<CCNode*> children = closeMenu->getChildren();
			for (auto& child : children) {
				if (child->getID() != "close-button") {
					child->setVisible(false);
				}
			}
		}

		this->getChildByID("main-title")->setVisible(false);
		this->getChildByID("more-games-menu")->setVisible(false);
		this->getChildByID("social-media-menu")->setVisible(false);
		this->getChildByID("main-menu")->setVisible(false);

		if (this->getChildByID("level-editor-hint")) this->getChildByID("level-editor-hint")->setVisible(false);
		if (this->getChildByID("character-select-hint")) this->getChildByID("character-select-hint")->setVisible(false);

		auto bottomMenu = this->getChildByID("bottom-menu");
		bottomMenu->setPosition(ccp(540.f, 190.f));
		bottomMenu->setScale(0.75f);
		bottomMenu->setLayout(ColumnLayout::create()->setAxisReverse(true)->setAutoScale(true)->setGap(0.f)->setCrossAxisOverflow(true));
		bottomMenu->setContentHeight(305.f);
		bottomMenu->updateLayout();
		bottomMenu->setZOrder(1);
		
		auto rightMenu = this->getChildByID("right-side-menu");
		rightMenu->setPosition(ccp(177.5f, 25.f));
		rightMenu->setScale(0.75f);
		rightMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setAxisReverse(true));
		rightMenu->setContentWidth(425.f);
		rightMenu->updateLayout();
		rightMenu->getChildByID("daily-chest-button")->setZOrder(4);

		auto questBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("RD_quests.png"_spr), this, menu_selector(CreatorLayer::onChallenge));
		rightMenu->addChild(questBtn, 3);

		auto mapPacksBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("RD_mappacks.png"_spr), this, menu_selector(CreatorLayer::onMapPacks));
		rightMenu->addChild(mapPacksBtn, 1);

		this->getChildByID("player-username")->setPositionY(this->getChildByID("player-username")->getPositionY() - 75.f);
		this->getChildByID("player-username")->setPositionX(540.f);
		this->getChildByID("player-username")->setScale(this->getChildByID("player-username")->getScale() - 0.1f);
		this->getChildByID("profile-menu")->setPosition(ccp(575.5f, 35.f));
		this->getChildByID("profile-menu")->setScale(0.75f);
		this->getChildByID("profile-menu")->setZOrder(1);		

		CCScale9Sprite* bottomMenuBG = CCScale9Sprite::create("square02b_001.png");
		bottomMenuBG->setID("bottom-menu-bg"_spr);
		bottomMenuBG->setColor({0,0,0});
		bottomMenuBG->setOpacity(100);
		bottomMenuBG->setPosition(bottomMenu->getPosition());
		bottomMenuBG->setPositionY(CCDirector::get()->getWinSize().height / 2.f);
		bottomMenuBG->setContentSize({54.f, 302.f});
		addChild(bottomMenuBG);

		auto menu = CCMenu::create();
		menu->setID("redash-menu"_spr);
		menu->setPosition({ 0.f , 0.f });
		this->addChild(menu, 99);

		CCMenu* mainMenu = CCMenu::create();
		mainMenu->setID("main-menu"_spr);
		mainMenu->setContentSize({675.f, 100.f});
		mainMenu->ignoreAnchorPointForPosition(false);
		mainMenu->setPositionX(256.5f);
		mainMenu->setScale(0.75f);
		mainMenu->setPositionY(105.f);
		mainMenu->setLayout(
			RowLayout::create()
				->setGap(10.f)
				->setGrowCrossAxis(true)
				->setAutoScale(false)
		);

		int activePath = gsm->m_activePath;
		int pathProgress = gsm->getStat(std::to_string(activePath).c_str());
		if (pathProgress > 1000) pathProgress = 1000;

		mainMenu->addChild(RDButton::create(this, "Create", fmt::format("You have\n{} Levels", LocalLevelManager::get()->m_localLevels->count()), "RD_create.png"_spr, menu_selector(CreatorLayer::onMyLevels), "create-button"));
		mainMenu->addChild(RDButton::create(this, "Saved", fmt::format("You have\n{} Saved\nLevels", glm->getSavedLevels(false, 0)->count()), "RD_saved.png"_spr, menu_selector(CreatorLayer::onSavedLevels), "saved-button"));
		mainMenu->addChild(RDButton::create(this, "Lists", "Nuh\nuh.", "RD_lists.png"_spr, menu_selector(CreatorLayer::onTopLists), "lists-button"));
		mainMenu->addChild(RDButton::create(this, "Scores", fmt::format("Global\n#{}", Variables::GlobalRank), "RD_leaderboards.png"_spr, menu_selector(CreatorLayer::onLeaderboards), "leaderboards-button"));
		mainMenu->addChild(RDButton::create(this, "Gauntlets", "Split\nGauntlet\nAdded", "RD_gauntlets.png"_spr, menu_selector(CreatorLayer::onGauntlets), "gauntlets-button"));
		mainMenu->addChild(RDButton::create(this, "Featured", "No.", "RD_featured.png"_spr, menu_selector(CreatorLayer::onFeaturedLevels), "featured-button"));
		mainMenu->addChild(RDButton::create(this, "Paths", fmt::format("{}\n{}/1000", getPathString(activePath - 29), pathProgress), "RD_paths_02.png"_spr,  menu_selector(CreatorLayer::onPaths), "paths-button"));
		mainMenu->addChild(RDButton::create(this, "Search", "Search\nFor levels\nOnline!", "RD_search.png"_spr, menu_selector(CreatorLayer::onOnlineLevels), "search-button"));
		mainMenu->updateLayout();
		menu->addChild(mainMenu);

		auto dailiesMenu = CCMenu::create();
		dailiesMenu->setID("dailies-menu"_spr);
		dailiesMenu->setContentSize({ 675.f , 100.f });
		dailiesMenu->ignoreAnchorPointForPosition(false);
		dailiesMenu->setPosition({ 255.f , 206.25f });
		dailiesMenu->setScale(0.75f);

		if (Mod::get()->getSettingValue<bool>("main-levels-leftmost")) {
			dailiesMenu->addChild(RDMainButton::create({ 25.f , 0.f }, { 150.f , 135.f }, menu_selector(MenuLayer::onPlay)));
			dailiesMenu->addChild(RDDailyNode::create(false, { 185.f , 0.f }, { 230.f , 135.f }, "daily-node"));
		} else {
			dailiesMenu->addChild(RDDailyNode::create(false, { 25.f , 0.f }, { 230.f , 135.f }, "daily-node"));
			dailiesMenu->addChild(RDMainButton::create({ 265.f , 0.f }, { 150.f , 135.f }, menu_selector(MenuLayer::onPlay)));
		}
		dailiesMenu->addChild(RDDailyNode::create(true, { 425.f , 0.f }, { 230.f , 135.f }, "weekly-node"));
		menu->addChild(dailiesMenu);

		auto statsMenu = CCMenu::create();
		statsMenu->setID("stats-menu"_spr);
		statsMenu->setContentSize({ 460.f , 25.f });
		statsMenu->ignoreAnchorPointForPosition(false);
		statsMenu->setPosition({ 190.f , 313.75f });
		statsMenu->setScale(0.6f);
		statsMenu->setLayout(
			RowLayout::create()
				->setAxisAlignment(AxisAlignment::Start)
				->setGap(25.f)
				->setAutoScale(false)
		);

		statsMenu->addChild(RDStatsNode::create("GJ_starsIcon_001.png", fmt::format("{}", gsm->getStat("6")), "stars-stats"));
		statsMenu->addChild(RDStatsNode::create("GJ_moonsIcon_001.png", fmt::format("{}", gsm->getStat("28")), "moons-stats"));
		statsMenu->addChild(RDStatsNode::create("GJ_coinsIcon_001.png", fmt::format("{}/{}", gsm->getStat("8"), MAX_SECRET_COINS), "sectet-coins-stats"));
		statsMenu->addChild(RDStatsNode::create("GJ_coinsIcon2_001.png", fmt::format("{}", gsm->getStat("12")), "user-coins-stats"));
		statsMenu->addChild(RDStatsNode::create("GJ_demonIcon_001.png", fmt::format("{}", gsm->getStat("5")), "demons-stats"));
		statsMenu->updateLayout();

		menu->addChild(statsMenu);

		auto menuButUnder = CCMenu::create();
		menuButUnder->setID("bottom-menu"_spr);
		menuButUnder->setAnchorPoint({ 1.f, 0.5f });
		menuButUnder->setContentSize({ 150.f, 48.25f });
		menuButUnder->setPosition({ 445.f, 25.f }); // ...
		menuButUnder->setScale(0.75f);
		menuButUnder->setLayout(
			RowLayout::create()
				->setAxisReverse(true)
				->setAxisAlignment(AxisAlignment::End)
				->setAutoScale(false)
				->setGap(0.f)
		);

		if (!Mod::get()->getSettingValue<bool>("disable-creator-button")) {
			auto creatorSpr = CCSprite::createWithSpriteFrameName("GJ_creatorBtn_001.png");
			creatorSpr->setScale(0.9f);
			menuButUnder->addChild(CCMenuItemSpriteExtra::create(creatorSpr, this, menu_selector(MenuLayer::onCreator)));
		}
		auto treasureSpr = CCSprite::createWithSpriteFrameName("secretDoorBtn_open_001.png");
		treasureSpr->setScale(1.25f);
		menuButUnder->addChild(CCMenuItemSpriteExtra::create(treasureSpr, this, menu_selector(CreatorLayer::onTreasureRoom)));

		menu->addChild(menuButUnder);
		menuButUnder->updateLayout();

		auto topMenu = CCMenu::create();
		topMenu->setID("top-menu"_spr);
		topMenu->setAnchorPoint({ 1.f, 0.5f});
		topMenu->setContentSize({ 150.f, 48.25f });
		topMenu->setPosition({ 490.f, 300.f });
		topMenu->setScale(0.75f);
		topMenu->setLayout(
			RowLayout::create()
				->setAxisReverse(true)
				->setAxisAlignment(AxisAlignment::End)
				->setAutoScale(false)
				->setGap(10.f)
		);

		auto garageSpr = CCSprite::createWithSpriteFrameName("garageRope_001.png");
		auto garageButton = CCMenuItemSpriteExtra::create(garageSpr, this, menu_selector(MenuLayer::onGarage));
		garageButton->setScale(4/3);
		topMenu->addChild(garageButton);
		topMenu->addChild(CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName(gsm->getStat("13") > 49 ? "GJ_lock_open_001.png" : "GJ_lockGray_001.png"), this, menu_selector(CreatorLayer::onSecretVault)));
		topMenu->updateLayout();

		garageButton->m_animationType = MenuAnimationType::Move;
		garageButton->m_startPosition = garageSpr->getPosition();
		garageButton->m_offset = ccp(0.f, -8.f);

		menu->addChild(topMenu);

		if (loader->isModLoaded("alphalaneous.pages_api")) {
			bottomMenu->setUserObject("orientation", CCInteger::create(0)); // VERTICAL
			bottomMenu->setUserObject("disable-pages", CCBool::create(true)); 
			bottomMenu->setLayout(as<ColumnLayout*>(bottomMenu->getLayout())->setAutoScale(false));

			rightMenu->setUserObject("orientation", CCInteger::create(1)); // HORIZONTAL
    		rightMenu->setUserObject("disable-pages", CCBool::create(true)); 
			rightMenu->setLayout(as<RowLayout*>(rightMenu->getLayout())->setAutoScale(false));
		}

		bottomMenu->updateLayout();
		rightMenu->updateLayout();

		auto hideBtnMenu = CCMenu::create();
		hideBtnMenu->setID("hide-button-menu"_spr);
		hideBtnMenu->setAnchorPoint({ 1.f, 0.5f });
		hideBtnMenu->setContentSize({ 48.25f, 48.25f });
		hideBtnMenu->setPosition({ 440.f, 0.f });
		hideBtnMenu->setScale(0.75f);
		this->addChild(hideBtnMenu, 100);

		auto spr1 = CCSprite::create("RD_hideButton_01.png"_spr);
		auto spr2 = CCSprite::create("RD_hideButton_02.png"_spr);
		auto hideToggler = CCMenuItemToggler::create(spr1, spr2, this, menu_selector(CrazyLayer::onHideMenu));
		hideToggler->setID("hide-button");
		hideToggler->setPosition({ hideBtnMenu->getContentWidth() / 2.f, hideBtnMenu->getContentHeight() / 2.f });
		hideBtnMenu->addChild(hideToggler);

		return true;
	}
};
