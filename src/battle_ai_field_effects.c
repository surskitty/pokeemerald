#include "global.h"
#include "battle_z_move.h"
#include "malloc.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_ai_field_effects.h"
#include "battle_ai_util.h"
#include "battle_ai_main.h"
#include "battle_ai_switch_items.h"
#include "battle_factory.h"
#include "battle_setup.h"
#include "event_data.h"
#include "data.h"
#include "item.h"
#include "move.h"
#include "pokemon.h"
#include "random.h"
#include "recorded_battle.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/battle_move_effects.h"
#include "constants/hold_effects.h"
#include "constants/moves.h"
#include "constants/items.h"

static bool32 DoesAbilityBenefitFromWeather(u32 ability, u32 weather)
{
    switch (ability)
    {
    case ABILITY_FORECAST:
        return (weather & (B_WEATHER_RAIN | B_WEATHER_SUN | B_WEATHER_ICY_ANY));
    case ABILITY_MAGIC_GUARD:
    case ABILITY_OVERCOAT:
        return (weather & B_WEATHER_DAMAGING_ANY);
    case ABILITY_SAND_FORCE:
    case ABILITY_SAND_RUSH:
    case ABILITY_SAND_VEIL:
        return (weather & B_WEATHER_SANDSTORM);
    case ABILITY_ICE_BODY:
    case ABILITY_ICE_FACE:
    case ABILITY_SNOW_CLOAK:
        return (weather & B_WEATHER_ICY_ANY);
    case ABILITY_SLUSH_RUSH:
        return (weather & B_WEATHER_SNOW);
    case ABILITY_DRY_SKIN:
    case ABILITY_HYDRATION:
    case ABILITY_RAIN_DISH:
    case ABILITY_SWIFT_SWIM:
        return (weather & B_WEATHER_RAIN);
    case ABILITY_CHLOROPHYLL:
    case ABILITY_FLOWER_GIFT:
    case ABILITY_HARVEST:
    case ABILITY_LEAF_GUARD:
    case ABILITY_ORICHALCUM_PULSE:
    case ABILITY_PROTOSYNTHESIS:
    case ABILITY_SOLAR_POWER:
        return (weather & B_WEATHER_SUN);
    }
    return FALSE;
}

static bool32 IsLightSensitiveMove(u32 move)
{
    switch (GetMoveEffect(move))
    {
    case EFFECT_SOLAR_BEAM:
    case EFFECT_MORNING_SUN:
    case EFFECT_SYNTHESIS:
    case EFFECT_MOONLIGHT:
    case EFFECT_GROWTH:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 HasLightSensitiveMove(u32 battler)
{
    s32 i;
    u16 *moves = GetMovesArray(battler);

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] != MOVE_NONE && moves[i] != MOVE_UNAVAILABLE && IsLightSensitiveMove(moves[i]))
            return TRUE;
    }

    return FALSE;
}

static enum BattlerBenefitsFromFieldEffect BenefitsFromSun(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, enum CheckPartner checkPartner)
{
    if (holdEffect == HOLD_EFFECT_UTILITY_UMBRELLA)
    {
        if (checkPartner == CHECK_PARTNER_NEXT)
            return BenefitsFromSun(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);
        else
            return FIELD_EFFECT_NEUTRAL;
    }
    else
    {
        if (DoesAbilityBenefitFromWeather(ability, B_WEATHER_SUN) || HasLightSensitiveMove(battler) || HasDamagingMoveOfType(battler, TYPE_FIRE))
            return FIELD_EFFECT_POSITIVE;
    }

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromSun(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);

    if (HasMoveWithFlag(battler, MoveHas50AccuracyInSun) || HasDamagingMoveOfType(battler, TYPE_WATER) || ability == ABILITY_DRY_SKIN)
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

// Sandstorm is positive if at least one Pokemon on a side does not take damage from it while its foe does. 
static enum BattlerBenefitsFromFieldEffect BenefitsFromSandstorm(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromWeather(ability, B_WEATHER_SANDSTORM)
     || IS_BATTLER_OF_TYPE(battler, TYPE_ROCK)
     || HasBattlerSideMoveWithEffect(battler, EFFECT_SHORE_UP))
    {
        return FIELD_EFFECT_POSITIVE;
    }

    if (holdEffect == HOLD_EFFECT_SAFETY_GOGGLES || IS_BATTLER_ANY_TYPE(battler, TYPE_ROCK, TYPE_GROUND, TYPE_STEEL))
    {
        if (!(IS_BATTLER_ANY_TYPE(FOE(battler), TYPE_ROCK, TYPE_GROUND, TYPE_STEEL)) 
         || gAiLogicData->holdEffects[FOE(battler)] == HOLD_EFFECT_SAFETY_GOGGLES 
         || DoesAbilityBenefitFromWeather(gAiLogicData->abilities[FOE(battler)], B_WEATHER_SANDSTORM))
        {
            return FIELD_EFFECT_POSITIVE;
        }
        else
        {
            if (checkPartner == CHECK_PARTNER_NEXT)
                return BenefitsFromSandstorm(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);
            else
                return FIELD_EFFECT_NEUTRAL;
        }
    }

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromSandstorm(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);
    else
        return FIELD_EFFECT_NEGATIVE;
}

static enum BattlerBenefitsFromFieldEffect BenefitsFromHailOrSnow(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, u32 weather, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromWeather(ability, weather)
     || IS_BATTLER_OF_TYPE(battler, TYPE_ICE)
     || HasMoveWithFlag(battler, MoveAlwaysHitsInHailSnow)
     || HasBattlerSideMoveWithEffect(battler, EFFECT_AURORA_VEIL))
    {
        return FIELD_EFFECT_POSITIVE;
    }

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromHailOrSnow(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], weather, CHECK_SELF_ONLY);

    if ((weather & B_WEATHER_DAMAGING_ANY) && holdEffect != HOLD_EFFECT_SAFETY_GOGGLES)
        return FIELD_EFFECT_NEGATIVE;

    if (HasLightSensitiveMove(battler))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

static enum BattlerBenefitsFromFieldEffect BenefitsFromRain(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, enum CheckPartner checkPartner)
{
    if (holdEffect == HOLD_EFFECT_UTILITY_UMBRELLA)
    {
        if (checkPartner == CHECK_PARTNER_NEXT)
            return BenefitsFromRain(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);
        else
            return FIELD_EFFECT_NEUTRAL;
    }
    
    if (DoesAbilityBenefitFromWeather(ability, B_WEATHER_RAIN)
      || HasMoveWithFlag(battler, MoveAlwaysHitsInRain)
      || HasDamagingMoveOfType(battler, TYPE_WATER))
        return FIELD_EFFECT_POSITIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromRain(BATTLE_PARTNER(battler), gAiLogicData->abilities[BATTLE_PARTNER(battler)], gAiLogicData->holdEffects[BATTLE_PARTNER(battler)], CHECK_SELF_ONLY);

    if (HasLightSensitiveMove(battler) || HasDamagingMoveOfType(battler, TYPE_FIRE))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

bool32 WeatherChecker(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, u32 weather, enum BattlerBenefitsFromFieldEffect desiredResult)
{
    if (IsWeatherActive(B_WEATHER_PRIMAL_ANY) != WEATHER_INACTIVE)
        return FIELD_EFFECT_BLOCKED;

    enum BattlerBenefitsFromFieldEffect result = FIELD_EFFECT_NEUTRAL;

    // checkPartner checks whether the ShouldSet subfunctions should recurse.
    // Recursion is used to cleanly loop through a battler's side during double battles.
    // Do not call the subfunctions directly so that it is not necessary to track whether or not it should recurse.
    enum CheckPartner checkPartner = CHECK_SELF_ONLY;

    // By default, it does not recurse.  Here, it checks if it ought to.
    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
        checkPartner = CHECK_PARTNER_NEXT;

    if (weather & B_WEATHER_RAIN)
        result = BenefitsFromRain(battler, ability, holdEffect, checkPartner);
    else if (weather & B_WEATHER_SUN)
        result = BenefitsFromSun(battler, ability, holdEffect, checkPartner);
    else if (weather & B_WEATHER_SANDSTORM)
        result = BenefitsFromSandstorm(battler, ability, holdEffect, checkPartner);
    else if (weather & B_WEATHER_ICY_ANY)
        result = BenefitsFromHailOrSnow(battler, ability, holdEffect, weather, checkPartner);

    return (result == desiredResult);
}

static bool32 DoesAbilityBenefitFromFieldStatus(u32 ability, u32 fieldStatus)
{
    switch (ability)
    {
    case ABILITY_MIMICRY:
        return (fieldStatus & STATUS_FIELD_TERRAIN_ANY);
    case ABILITY_HADRON_ENGINE:
    case ABILITY_QUARK_DRIVE:
    case ABILITY_SURGE_SURFER:
        return (fieldStatus & STATUS_FIELD_ELECTRIC_TERRAIN);
    case ABILITY_GRASS_PELT:
        return (fieldStatus & STATUS_FIELD_GRASSY_TERRAIN);
    // no abilities inherently benefit from Misty or Psychic Terrains
    // return (weather & STATUS_FIELD_MISTY_TERRAIN);
    // return (weather & STATUS_FIELD_PSYCHIC_TERRAIN);
    default:
        break;
    }
    return FALSE;
}

//TODO: when is electric terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromElectricTerrain(u32 battler, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromFieldStatus(gAiLogicData->abilities[battler], STATUS_FIELD_ELECTRIC_TERRAIN))
        return FIELD_EFFECT_POSITIVE;

    if (HasMoveWithEffect(battler, EFFECT_RISING_VOLTAGE))
        return FIELD_EFFECT_POSITIVE;

    if (HasMoveWithEffect(FOE(battler), EFFECT_REST) && IsBattlerGrounded(FOE(battler)))
        return FIELD_EFFECT_POSITIVE;

    bool32 grounded = IsBattlerGrounded(battler);
    if (grounded && HasBattlerSideUsedMoveWithAdditionalEffect(FOE(battler), MOVE_EFFECT_SLEEP))
        return FIELD_EFFECT_POSITIVE;

    if (grounded && ((gBattleMons[battler].status1 & STATUS1_SLEEP) 
    || (gStatuses3[battler] & STATUS3_YAWN) 
    || HasDamagingMoveOfType(battler, TYPE_ELECTRIC)))
        return FIELD_EFFECT_POSITIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromElectricTerrain(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is grassy terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromGrassyTerrain(u32 battler, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromFieldStatus(gAiLogicData->abilities[battler], STATUS_FIELD_GRASSY_TERRAIN))
        return FIELD_EFFECT_POSITIVE;

    if (HasBattlerSideMoveWithEffect(battler, EFFECT_GRASSY_GLIDE))
        return FIELD_EFFECT_POSITIVE;
    if (HasBattlerSideUsedMoveWithAdditionalEffect(battler, MOVE_EFFECT_FLORAL_HEALING))
        return FIELD_EFFECT_POSITIVE;

    bool32 grounded = IsBattlerGrounded(battler);

    // Weaken spamming Earthquake, Magnitude, and Bulldoze.
    if (grounded && (HasBattlerSideUsedMoveWithEffect(FOE(battler), EFFECT_EARTHQUAKE)
    || HasBattlerSideUsedMoveWithEffect(FOE(battler), EFFECT_MAGNITUDE)))
        return FIELD_EFFECT_POSITIVE;

    if (grounded && HasDamagingMoveOfType(battler, TYPE_GRASS))
        return FIELD_EFFECT_POSITIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromGrassyTerrain(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is misty terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromMistyTerrain(u32 battler, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromFieldStatus(gAiLogicData->abilities[battler], STATUS_FIELD_MISTY_TERRAIN))
        return FIELD_EFFECT_POSITIVE;

    if (HasBattlerSideMoveWithEffect(battler, EFFECT_MISTY_EXPLOSION))
        return FIELD_EFFECT_POSITIVE;

    bool32 grounded = IsBattlerGrounded(battler);
    bool32 allyGrounded = FALSE;
    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
        allyGrounded = IsBattlerGrounded(BATTLE_PARTNER(battler));

    if (HasMoveWithEffect(FOE(battler), EFFECT_REST) && IsBattlerGrounded(FOE(battler)))
        return FIELD_EFFECT_POSITIVE;

    // harass dragons
    if ((grounded || allyGrounded) 
        && (HasDamagingMoveOfType(FOE(battler), TYPE_DRAGON) || HasDamagingMoveOfType(BATTLE_PARTNER(FOE(battler)), TYPE_DRAGON)))
        return FIELD_EFFECT_POSITIVE;

    if ((grounded || allyGrounded) && HasBattlerSideUsedMoveWithAdditionalEffect(FOE(battler), MOVE_EFFECT_SLEEP))
        return FIELD_EFFECT_POSITIVE;

    if (grounded && ((gBattleMons[battler].status1 & STATUS1_SLEEP) 
    || (gStatuses3[battler] & STATUS3_YAWN)))
        return FIELD_EFFECT_POSITIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromMistyTerrain(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is Psychic Terrain negative?
static enum BattlerBenefitsFromFieldEffect BenefitsFromPsychicTerrain(u32 battler, enum CheckPartner checkPartner)
{
    if (DoesAbilityBenefitFromFieldStatus(gAiLogicData->abilities[battler], STATUS_FIELD_PSYCHIC_TERRAIN))
        return FIELD_EFFECT_POSITIVE;

    if (HasBattlerSideMoveWithEffect(battler, EFFECT_EXPANDING_FORCE))
        return FIELD_EFFECT_POSITIVE;

    bool32 grounded = IsBattlerGrounded(battler);
    bool32 allyGrounded = FALSE;
    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
        allyGrounded = IsBattlerGrounded(BATTLE_PARTNER(battler));

    // don't bother if we're not grounded
    if (grounded || allyGrounded)
    {
        // harass priority
        if (HasBattlerSideAbility(FOE(battler), ABILITY_GALE_WINGS, gAiLogicData) 
         || HasBattlerSideAbility(FOE(battler), ABILITY_TRIAGE, gAiLogicData)
         || HasBattlerSideAbility(FOE(battler), ABILITY_PRANKSTER, gAiLogicData))
            return FIELD_EFFECT_POSITIVE;
    }

    if (grounded && (HasDamagingMoveOfType(battler, TYPE_PSYCHIC)))
        return FIELD_EFFECT_POSITIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromPsychicTerrain(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);

    if (HasBattlerSideAbility(battler, ABILITY_GALE_WINGS, gAiLogicData) 
     || HasBattlerSideAbility(battler, ABILITY_TRIAGE, gAiLogicData)
     || HasBattlerSideAbility(battler, ABILITY_PRANKSTER, gAiLogicData))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

// It returns FALSE if the desired state is for Trick Room to be not set.
// In double battles, Trick Room desires for both pokemon on the side to benefit from Trick Room, while the others only need one yes.
// This means that it recurses if there is a partner that has not yet been checked when it would otherwise return TRUE
static enum BattlerBenefitsFromFieldEffect BenefitsFromTrickRoom(u32 battler, enum CheckPartner checkPartner)
{
    // If we're in singles, we literally only care about speed.
    if (!IsDoubleBattle())
    {
        if (GetBattlerSideSpeedAverage(battler) < GetBattlerSideSpeedAverage(FOE(battler)))
            return FIELD_EFFECT_POSITIVE;
        // If we tie, we shouldn't change trick room state.
        else if (GetBattlerSideSpeedAverage(battler) == GetBattlerSideSpeedAverage(FOE(battler)))
            return FIELD_EFFECT_NEUTRAL; 
        else
            return FIELD_EFFECT_NEGATIVE;
    }
    
    // First checking if Trick Room impacts us at all.
    if (!(gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN))
    {
        u16* aiMoves = GetMovesArray(battler);
        for (int i = 0; i < MAX_MON_MOVES; i++)
        {
            u16 move = aiMoves[i];
            if (GetBattleMovePriority(battler, gAiLogicData->abilities[battler], move) > 0 && !(GetMovePriority(move) > 0 && IsBattleMoveStatus(move)))
            {
                if (checkPartner == CHECK_PARTNER_NEXT)
                    return BenefitsFromTrickRoom(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);
                else
                    return FIELD_EFFECT_NEUTRAL;
            }
        }
    }

    if ((gAiLogicData->speedStats[battler] >= gAiLogicData->speedStats[FOE(battler)]) || (gAiLogicData->speedStats[battler] >= gAiLogicData->speedStats[BATTLE_PARTNER(FOE(battler))]))
        return FIELD_EFFECT_NEGATIVE;

    if (checkPartner == CHECK_PARTNER_NEXT)
        return BenefitsFromTrickRoom(BATTLE_PARTNER(battler), CHECK_SELF_ONLY);

    return FIELD_EFFECT_POSITIVE;
}

bool32 FieldStatusChecker(u32 battler, u32 fieldStatus, enum BattlerBenefitsFromFieldEffect desiredResult)
{
    enum BattlerBenefitsFromFieldEffect result = FIELD_EFFECT_NEUTRAL;

    // checkPartner checks whether the ShouldSet subfunctions should recurse.
    // Recursion is used to cleanly loop through a battler's side during double battles.
    // Do not call the subfunctions directly so that it is not necessary to track whether or not it should recurse.
    enum CheckPartner checkPartner = CHECK_SELF_ONLY;

    // By default, it does not recurse.  Here, it checks if it ought to.
    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
    {
        if (DoesAbilityBenefitFromFieldStatus(gAiLogicData->abilities[BATTLE_PARTNER(battler)], fieldStatus))
            return TRUE;
        checkPartner = CHECK_PARTNER_NEXT;
    }

    if (fieldStatus & STATUS_FIELD_ELECTRIC_TERRAIN)
        result = BenefitsFromElectricTerrain(battler, checkPartner);
    if (fieldStatus & STATUS_FIELD_GRASSY_TERRAIN)
        result = BenefitsFromGrassyTerrain(battler, checkPartner);
    if (fieldStatus & STATUS_FIELD_MISTY_TERRAIN)
        result = BenefitsFromMistyTerrain(battler, checkPartner);
    if (fieldStatus & STATUS_FIELD_PSYCHIC_TERRAIN)
        result = BenefitsFromPsychicTerrain(battler, checkPartner);

    if (fieldStatus & STATUS_FIELD_TRICK_ROOM)
        result = BenefitsFromTrickRoom(battler, checkPartner);

    return (result == desiredResult);
}

