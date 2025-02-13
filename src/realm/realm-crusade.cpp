﻿#include "realm/realm-crusade.h"
#include "cmd-action/cmd-spell.h"
#include "effect/attribute-types.h"
#include "effect/effect-characteristics.h"
#include "effect/effect-processor.h"
#include "floor/cave.h"
#include "floor/floor-util.h"
#include "hpmp/hp-mp-processor.h"
#include "monster-floor/monster-summon.h"
#include "monster-floor/place-monster-types.h"
#include "player-base/player-class.h"
#include "player-info/class-info.h"
#include "spell-kind/spells-beam.h"
#include "spell-kind/spells-curse-removal.h"
#include "spell-kind/spells-detection.h"
#include "spell-kind/spells-floor.h"
#include "spell-kind/spells-launcher.h"
#include "spell-kind/spells-neighbor.h"
#include "spell-kind/spells-sight.h"
#include "spell-kind/spells-teleport.h"
#include "spell-realm/spells-crusade.h"
#include "spell/spells-diceroll.h"
#include "spell/spells-object.h"
#include "spell/spells-status.h"
#include "spell/summon-types.h"
#include "status/bad-status-setter.h"
#include "status/body-improvement.h"
#include "status/buff-setter.h"
#include "status/sight-setter.h"
#include "system/player-type-definition.h"
#include "target/target-getter.h"
#include "view/display-messages.h"
#include "world/world.h"

/*!
 * @brief 破邪領域魔法の各処理を行う
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param spell 魔法ID
 * @param mode 処理内容 (SpellProcessType::NAME / SPELL_DESC / SpellProcessType::INFO / SpellProcessType::CAST)
 * @return SpellProcessType::NAME / SPELL_DESC / SpellProcessType::INFO 時には文字列ポインタを返す。SpellProcessType::CAST時はnullptr文字列を返す。
 */
concptr do_crusade_spell(PlayerType *player_ptr, SPELL_IDX spell, SpellProcessType mode)
{
    bool name = mode == SpellProcessType::NAME;
    bool desc = mode == SpellProcessType::DESCRIPTION;
    bool info = mode == SpellProcessType::INFO;
    bool cast = mode == SpellProcessType::CAST;

    DIRECTION dir;
    PLAYER_LEVEL plev = player_ptr->lev;

    switch (spell) {
    case 0:
        if (name) {
            return _("懲罰", "Punishment");
        }
        if (desc) {
            return _("電撃のボルトもしくはビームを放つ。", "Fires a bolt or beam of lightning.");
        }
        {
            DICE_NUMBER dice = 3 + (plev - 1) / 5;
            DICE_SID sides = 4;
            if (info) {
                return info_damage(dice, sides, 0);
            }
            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                fire_bolt_or_beam(player_ptr, beam_chance(player_ptr) - 10, AttributeType::ELEC, dir, damroll(dice, sides));
            }
        }
        break;

    case 1:
        if (name) {
            return _("邪悪存在感知", "Detect Evil");
        }
        if (desc) {
            return _("近くの邪悪なモンスターを感知する。", "Detects all evil monsters in your vicinity.");
        }
        {
            POSITION rad = DETECT_RAD_DEFAULT;
            if (info) {
                return info_radius(rad);
            }
            if (cast) {
                detect_monsters_evil(player_ptr, rad);
            }
        }
        break;

    case 2:
        if (name) {
            return _("恐怖除去", "Remove Fear");
        }

        if (desc) {
            return _("恐怖を取り除く。", "Removes fear.");
        }

        if (cast) {
            (void)BadStatusSetter(player_ptr).set_fear(0);
        }

        break;
    case 3:
        if (name) {
            return _("威圧", "Scare Monster");
        }
        if (desc) {
            return _("モンスター1体を恐怖させる。抵抗されると無効。", "Attempts to scare a monster.");
        }

        {
            PLAYER_LEVEL power = plev;
            if (info) {
                return info_power(power);
            }
            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                fear_monster(player_ptr, dir, power);
            }
        }
        break;

    case 4:
        if (name) {
            return _("聖域", "Sanctuary");
        }
        if (desc) {
            return _("隣接した全てのモンスターを眠らせる。抵抗されると無効。", "Attempts to put to sleep monsters in the adjacent squares.");
        }
        {
            PLAYER_LEVEL power = plev;
            if (info) {
                return info_power(power);
            }
            if (cast) {
                sleep_monsters_touch(player_ptr);
            }
        }
        break;

    case 5:
        if (name) {
            return _("入口", "Portal");
        }
        if (desc) {
            return _("中距離のテレポートをする。", "Teleports you a medium distance.");
        }

        {
            POSITION range = 25 + plev / 2;
            if (info) {
                return info_range(range);
            }
            if (cast) {
                teleport_player(player_ptr, range, TELEPORT_SPONTANEOUS);
            }
        }
        break;

    case 6:
        if (name) {
            return _("スターダスト", "Star Dust");
        }
        if (desc) {
            return _("ターゲット付近に閃光のボルトを連射する。", "Fires many bolts of light near the target.");
        }

        {
            DICE_NUMBER dice = 3 + (plev - 1) / 9;
            DICE_SID sides = 2;
            if (info) {
                return info_multi_damage_dice(dice, sides);
            }
            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                fire_blast(player_ptr, AttributeType::LITE, dir, dice, sides, 10, 3);
            }
        }
        break;

    case 7:
        if (name) {
            return _("身体浄化", "Purify");
        }

        if (desc) {
            return _("傷、毒、朦朧から全快する。", "Heals all cuts, poisons and being stunned.");
        }

        if (cast) {
            BadStatusSetter bss(player_ptr);
            (void)bss.set_cut(0);
            (void)bss.set_poison(0);
            (void)bss.set_stun(0);
        }

        break;

    case 8:
        if (name) {
            return _("邪悪飛ばし", "Scatter Evil");
        }
        if (desc) {
            return _("邪悪なモンスター1体をテレポートさせる。抵抗されると無効。", "Attempts to teleport an evil monster away.");
        }

        {
            int power = MAX_PLAYER_SIGHT * 5;
            if (info) {
                return info_power(power);
            }
            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                fire_ball(player_ptr, AttributeType::AWAY_EVIL, dir, power, 0);
            }
        }
        break;

    case 9:
        if (name) {
            return _("聖なる光球", "Holy Orb");
        }
        if (desc) {
            return _("聖なる力をもつ宝珠を放つ。邪悪なモンスターに対して大きなダメージを与えるが、善良なモンスターには効果がない。",
                "Fires a ball with holy power. Hurts evil monsters greatly but doesn't affect good monsters.");
        }

        {
            DICE_NUMBER dice = 3;
            DICE_SID sides = 6;
            POSITION rad = (plev < 30) ? 2 : 3;
            int base;
            PlayerClass pc(player_ptr);
            if (pc.equals(PlayerClassType::PRIEST) || pc.equals(PlayerClassType::HIGH_MAGE) || pc.equals(PlayerClassType::SORCERER)) {
                base = plev + plev / 2;
            } else {
                base = plev + plev / 4;
            }

            if (info) {
                return info_damage(dice, sides, base);
            }

            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }

                fire_ball(player_ptr, AttributeType::HOLY_FIRE, dir, damroll(dice, sides) + base, rad);
            }
        }
        break;

    case 10:
        if (name) {
            return _("悪魔払い", "Exorcism");
        }
        if (desc) {
            return _("視界内の全てのアンデッド及び悪魔にダメージを与え、邪悪なモンスターを恐怖させる。",
                "Damages all undead and demons in sight, and scares all evil monsters in sight.");
        }
        {
            DICE_SID sides = plev;
            int power = plev;
            if (info) {
                return info_damage(1, sides, 0);
            }
            if (cast) {
                dispel_undead(player_ptr, randint1(sides));
                dispel_demons(player_ptr, randint1(sides));
                turn_evil(player_ptr, power);
            }
        }
        break;

    case 11:
        if (name) {
            return _("解呪", "Remove Curse");
        }
        if (desc) {
            return _("アイテムにかかった弱い呪いを解除する。", "Removes normal curses from equipped items.");
        }
        {
            if (cast) {
                (void)remove_curse(player_ptr);
            }
        }
        break;

    case 12:
        if (name) {
            return _("透明視認", "Sense Unseen");
        }
        if (desc) {
            return _("一定時間、透明なものが見えるようになる。", "Gives see invisible for a while.");
        }

        {
            int base = 24;

            if (info) {
                return info_duration(base, base);
            }

            if (cast) {
                set_tim_invis(player_ptr, randint1(base) + base, false);
            }
        }
        break;

    case 13:
        if (name) {
            return _("対邪悪結界", "Protection from Evil");
        }
        if (desc) {
            return _("邪悪なモンスターの攻撃を防ぐバリアを張る。", "Gives aura which protects you from evil monster's physical attack.");
        }

        {
            int base = 25;
            DICE_SID sides = 3 * plev;

            if (info) {
                return info_duration(base, sides);
            }

            if (cast) {
                set_protevil(player_ptr, randint1(sides) + base, false);
            }
        }
        break;

    case 14:
        if (name) {
            return _("裁きの雷", "Judgment Thunder");
        }
        if (desc) {
            return _("強力な電撃のボルトを放つ。", "Fires a powerful bolt of lightning.");
        }

        {
            int dam = plev * 5;

            if (info) {
                return info_damage(0, 0, dam);
            }

            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                fire_bolt(player_ptr, AttributeType::ELEC, dir, dam);
            }
        }
        break;

    case 15: {
        if (name) {
            return _("聖なる御言葉", "Holy Word");
        }

        if (desc) {
            return _("視界内の邪悪な存在に大きなダメージを与え、体力を回復し、毒、恐怖、朦朧状態、負傷から全快する。",
                "Damages all evil monsters in sight, heals HP somewhat and completely cures fear, poisons, cuts and being stunned.");
        }

        auto dam_sides = plev * 6;
        auto heal = 100;
        if (info) {
            return format(_("損:1d%d/回%d", "dam:d%d/h%d"), dam_sides, heal);
        }

        if (cast) {
            BadStatusSetter bss(player_ptr);
            dispel_evil(player_ptr, randint1(dam_sides));
            hp_player(player_ptr, heal);
            (void)bss.set_fear(0);
            (void)bss.set_poison(0);
            (void)bss.set_stun(0);
            (void)bss.set_cut(0);
        }

        break;
    }
    case 16:
        if (name) {
            return _("開かれた道", "Unbarring Ways");
        }
        if (desc) {
            return _("一直線上の全ての罠と扉を破壊する。", "Fires a beam which destroy traps and doors.");
        }

        {
            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }

                destroy_door(player_ptr, dir);
            }
        }
        break;

    case 17:
        if (name) {
            return _("封魔", "Arrest");
        }
        if (desc) {
            return _("邪悪なモンスターの動きを止める。", "Attempts to paralyze an evil monster.");
        }

        {
            int power = plev * 2;

            if (info) {
                return info_power(power);
            }

            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }
                stasis_evil(player_ptr, dir);
            }
        }
        break;

    case 18:
        if (name) {
            return _("聖なるオーラ", "Holy Aura");
        }
        if (desc) {
            return _("一定時間、邪悪なモンスターを傷つける聖なるオーラを得る。",
                "Gives a temporary aura of holy power that injures evil monsters which attack you.");
        }

        {
            int base = 20;

            if (info) {
                return info_duration(base, base);
            }

            if (cast) {
                set_tim_sh_holy(player_ptr, randint1(base) + base, false);
            }
        }
        break;

    case 19:
        if (name) {
            return _("アンデッド&悪魔退散", "Dispel Undead & Demons");
        }
        if (desc) {
            return _("視界内の全てのアンデッド及び悪魔にダメージを与える。", "Damages all undead and demons in sight.");
        }

        {
            DICE_SID sides = plev * 4;

            if (info) {
                return info_damage(1, sides, 0);
            }

            if (cast) {
                dispel_undead(player_ptr, randint1(sides));
                dispel_demons(player_ptr, randint1(sides));
            }
        }
        break;

    case 20:
        if (name) {
            return _("邪悪退散", "Dispel Evil");
        }
        if (desc) {
            return _("視界内の全ての邪悪なモンスターにダメージを与える。", "Damages all evil monsters in sight.");
        }

        {
            DICE_SID sides = plev * 4;

            if (info) {
                return info_damage(1, sides, 0);
            }

            if (cast) {
                dispel_evil(player_ptr, randint1(sides));
            }
        }
        break;

    case 21:
        if (name) {
            return _("聖なる刃", "Holy Blade");
        }
        if (desc) {
            return _("通常の武器に滅邪の属性をつける。", "Makes current weapon especially deadly against evil monsters.");
        }

        {
            if (cast) {
                brand_weapon(player_ptr, 13);
            }
        }
        break;

    case 22:
        if (name) {
            return _("スターバースト", "Star Burst");
        }
        if (desc) {
            return _("巨大な閃光の球を放つ。", "Fires a huge ball of powerful light.");
        }

        {
            int dam = 100 + plev * 2;
            POSITION rad = 4;

            if (info) {
                return info_damage(0, 0, dam);
            }

            if (cast) {
                if (!get_aim_dir(player_ptr, &dir)) {
                    return nullptr;
                }

                fire_ball(player_ptr, AttributeType::LITE, dir, dam, rad);
            }
        }
        break;

    case 23:
        if (name) {
            return _("天使召喚", "Summon Angel");
        }
        if (desc) {
            return _("天使を1体召喚する。", "Summons an angel.");
        }

        {
            if (cast) {
                bool pet = !one_in_(3);
                uint32_t flg = 0L;

                if (pet) {
                    flg |= PM_FORCE_PET;
                } else {
                    flg |= PM_NO_PET;
                }
                if (!(pet && (plev < 50))) {
                    flg |= PM_ALLOW_GROUP;
                }

                if (summon_specific(player_ptr, (pet ? -1 : 0), player_ptr->y, player_ptr->x, (plev * 3) / 2, SUMMON_ANGEL, flg)) {
                    if (pet) {
                        msg_print(_("「ご用でございますか、ご主人様」", "'What is thy bidding... Master?'"));
                    } else {
                        msg_print(_("「我は汝の下僕にあらず！ 悪行者よ、悔い改めよ！」", "Mortal! Repent of thy impiousness."));
                    }
                }
            }
        }
        break;

    case 24:
        if (name) {
            return _("士気高揚", "Heroism");
        }
        if (desc) {
            return _("一定時間、ヒーロー気分になる。", "Removes fear. Gives a bonus to hit for a while. Heals you for 10 HP.");
        }

        {
            int base = 25;

            if (info) {
                return info_duration(base, base);
            }

            if (cast) {
                (void)heroism(player_ptr, base);
            }
        }
        break;

    case 25:
        if (name) {
            return _("呪い退散", "Dispel Curse");
        }
        if (desc) {
            return _("アイテムにかかった強力な呪いを解除する。", "Removes normal and heavy curses from equipped items.");
        }

        {
            if (cast) {
                (void)remove_all_curse(player_ptr);
            }
        }
        break;

    case 26:
        if (name) {
            return _("邪悪追放", "Banish Evil");
        }
        if (desc) {
            return _("視界内の全ての邪悪なモンスターをテレポートさせる。抵抗されると無効。", "Teleports all evil monsters in sight away unless resisted.");
        }

        {
            int power = 100;

            if (info) {
                return info_power(power);
            }

            if (cast) {
                if (banish_evil(player_ptr, power)) {
                    msg_print(_("神聖な力が邪悪を打ち払った！", "The holy power banishes evil!"));
                }
            }
        }
        break;

    case 27:
        if (name) {
            return _("ハルマゲドン", "Armageddon");
        }
        if (desc) {
            return _("周辺のアイテム、モンスター、地形を破壊する。", "Destroys everything in nearby area.");
        }

        {
            int base = 12;
            DICE_SID sides = 4;

            if (cast) {
                destroy_area(player_ptr, player_ptr->y, player_ptr->x, base + randint1(sides), false);
            }
        }
        break;

    case 28:
        if (name) {
            return _("目には目を", "An Eye for an Eye");
        }
        if (desc) {
            return _("一定時間、自分がダメージを受けたときに攻撃を行ったモンスターに対して同等のダメージを与える。",
                "Gives special aura for a while. When you are attacked by a monster, the monster is injured with same amount of damage as you took.");
        }

        {
            int base = 10;

            if (info) {
                return info_duration(base, base);
            }

            if (cast) {
                set_tim_eyeeye(player_ptr, randint1(base) + base, false);
            }
        }
        break;

    case 29:
        if (name) {
            return _("神の怒り", "Wrath of the God");
        }
        if (desc) {
            return _("ターゲットの周囲に分解の球を多数落とす。", "Drops many balls of disintegration near the target.");
        }

        {
            int dam = plev * 3 + 25;
            POSITION rad = 2;

            if (info) {
                return info_multi_damage(dam);
            }

            if (cast) {
                if (!cast_wrath_of_the_god(player_ptr, dam, rad)) {
                    return nullptr;
                }
            }
        }
        break;

    case 30:
        if (name) {
            return _("神威", "Divine Intervention");
        }
        if (desc) {
            return _("隣接するモンスターに聖なるダメージを与え、視界内のモンスターにダメージ、減速、朦朧、混乱、恐怖、眠りを与える。さらに体力を回復する。",
                "Damages all adjacent monsters with holy power. Damages and attempt to slow, stun, confuse, scare and freeze all monsters in sight. And heals "
                "HP.");
        }

        {
            int b_dam = plev * 11;
            int d_dam = plev * 4;
            int heal = 100;
            int power = plev * 4;

            if (info) {
                return format(_("回%d/損%d+%d", "h%d/dm%d+%d"), heal, d_dam, b_dam / 2);
            }
            if (cast) {
                project(player_ptr, 0, 1, player_ptr->y, player_ptr->x, b_dam, AttributeType::HOLY_FIRE, PROJECT_KILL);
                dispel_monsters(player_ptr, d_dam);
                slow_monsters(player_ptr, plev);
                stun_monsters(player_ptr, power);
                confuse_monsters(player_ptr, power);
                turn_monsters(player_ptr, power);
                stasis_monsters(player_ptr, power);
                hp_player(player_ptr, heal);
            }
        }
        break;

    case 31:
        if (name) {
            return _("聖戦", "Crusade");
        }

        if (desc) {
            return _("視界内の善良なモンスターをペットにしようとし、ならなかった場合及び善良でないモンスターを恐怖させる。さらに多数の加速された騎士を召喚し、"
                     "ヒーロー、祝福、加速、対邪悪結界を得る。",
                "Attempts to charm all good monsters in sight and scares all non-charmed monsters. Summons a great number of knights. Gives heroism, bless, "
                "speed and protection from evil to the caster.");
        }

        if (cast) {
            auto base = 25;
            auto sp_sides = 20 + plev;
            auto sp_base = plev;
            crusade(player_ptr);
            for (auto i = 0; i < 12; i++) {
                auto attempt = 10;
                POSITION my = 0, mx = 0;
                while (attempt--) {
                    scatter(player_ptr, &my, &mx, player_ptr->y, player_ptr->x, 4, PROJECT_NONE);
                    if (is_cave_empty_bold2(player_ptr, my, mx)) {
                        break;
                    }
                }

                if (attempt < 0) {
                    continue;
                }

                summon_specific(player_ptr, -1, my, mx, plev, SUMMON_KNIGHTS, PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE);
            }

            set_hero(player_ptr, randint1(base) + base, false);
            set_blessed(player_ptr, randint1(base) + base, false);
            set_acceleration(player_ptr, randint1(sp_sides) + sp_base, false);
            set_protevil(player_ptr, randint1(base) + base, false);
            (void)BadStatusSetter(player_ptr).set_fear(0);
        }

        break;
    }

    return "";
}
