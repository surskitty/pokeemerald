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

// Sun
static enum BattlerBenefitsFromFieldEffect BenefitsFromSun(u32 battler)
{
    if (gAiLogicData->holdEffects[battler] == HOLD_EFFECT_UTILITY_UMBRELLA)
        return FIELD_EFFECT_NEUTRAL;

    if (DoesAbilityBenefitFromWeather(gAiLogicData->abilities[battler], B_WEATHER_SUN) || HasLightSensitiveMove(battler) || HasDamagingMoveOfType(battler, TYPE_FIRE))
        return FIELD_EFFECT_POSITIVE;

    if (HasMoveWithFlag(battler, MoveHas50AccuracyInSun) || HasDamagingMoveOfType(battler, TYPE_WATER) || gAiLogicData->abilities[battler] == ABILITY_DRY_SKIN)
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

// Sandstorm is positive if at least one Pokemon on a side does not take damage from it while its foe does. 
static enum BattlerBenefitsFromFieldEffect BenefitsFromSandstorm(u32 battler)
{
    if (DoesAbilityBenefitFromWeather(gAiLogicData->abilities[battler], B_WEATHER_SANDSTORM)
     || IS_BATTLER_OF_TYPE(battler, TYPE_ROCK)
     || HasBattlerSideMoveWithEffect(battler, EFFECT_SHORE_UP))
        return FIELD_EFFECT_POSITIVE;

    if (gAiLogicData->holdEffects[battler] == HOLD_EFFECT_SAFETY_GOGGLES || IS_BATTLER_ANY_TYPE(battler, TYPE_ROCK, TYPE_GROUND, TYPE_STEEL))
    {
        if (!(IS_BATTLER_ANY_TYPE(FOE(battler), TYPE_ROCK, TYPE_GROUND, TYPE_STEEL)) 
         || gAiLogicData->holdEffects[FOE(battler)] == HOLD_EFFECT_SAFETY_GOGGLES 
         || DoesAbilityBenefitFromWeather(gAiLogicData->abilities[FOE(battler)], B_WEATHER_SANDSTORM))
            return FIELD_EFFECT_POSITIVE;
        else
            return FIELD_EFFECT_NEUTRAL;
    }

    return FIELD_EFFECT_NEGATIVE;
}

// Hail or Snow
// Hail is negative if a pokemon is damaged by it; Snow is only negative with light sensitive moves or against Blizzard et al
static enum BattlerBenefitsFromFieldEffect BenefitsFromHailOrSnow(u32 battler, u32 weather)
{
    if (DoesAbilityBenefitFromWeather(gAiLogicData->abilities[battler], weather)
     || IS_BATTLER_OF_TYPE(battler, TYPE_ICE)
     || HasMoveWithFlag(battler, MoveAlwaysHitsInHailSnow)
     || HasBattlerSideMoveWithEffect(battler, EFFECT_AURORA_VEIL))
        return FIELD_EFFECT_POSITIVE;

    if ((weather & B_WEATHER_DAMAGING_ANY) && gAiLogicData->holdEffects[battler] != HOLD_EFFECT_SAFETY_GOGGLES)
        return FIELD_EFFECT_NEGATIVE;

    if (HasLightSensitiveMove(battler))
        return FIELD_EFFECT_NEGATIVE;

    if (HasMoveWithFlag(FOE(battler), MoveAlwaysHitsInHailSnow))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

// Rain is negative if opponent has Thunder or Hurricane
static enum BattlerBenefitsFromFieldEffect BenefitsFromRain(u32 battler)
{
    if (gAiLogicData->holdEffects[battler] == HOLD_EFFECT_UTILITY_UMBRELLA)
        return FIELD_EFFECT_NEUTRAL;
    
    if (DoesAbilityBenefitFromWeather(gAiLogicData->abilities[battler], B_WEATHER_RAIN)
      || HasMoveWithFlag(battler, MoveAlwaysHitsInRain)
      || HasDamagingMoveOfType(battler, TYPE_WATER))
        return FIELD_EFFECT_POSITIVE;

    if (HasLightSensitiveMove(battler) || HasDamagingMoveOfType(battler, TYPE_FIRE))
        return FIELD_EFFECT_NEGATIVE;

    if (HasMoveWithFlag(FOE(battler), MoveAlwaysHitsInRain))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is electric terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromElectricTerrain(u32 battler)
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

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is grassy terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromGrassyTerrain(u32 battler)
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

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is misty terrain bad?
static enum BattlerBenefitsFromFieldEffect BenefitsFromMistyTerrain(u32 battler)
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

    return FIELD_EFFECT_NEUTRAL;
}

//TODO: when is Psychic Terrain negative?
static enum BattlerBenefitsFromFieldEffect BenefitsFromPsychicTerrain(u32 battler)
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

    if (HasBattlerSideAbility(battler, ABILITY_GALE_WINGS, gAiLogicData) 
     || HasBattlerSideAbility(battler, ABILITY_TRIAGE, gAiLogicData)
     || HasBattlerSideAbility(battler, ABILITY_PRANKSTER, gAiLogicData))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_NEUTRAL;
}

static enum BattlerBenefitsFromFieldEffect BenefitsFromTrickRoom(u32 battler)
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
    
    // First checking if we have enough priority for one pokemon to disregard Trick Room entirely.
    if (!(gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN))
    {
        u16* aiMoves = GetMovesArray(battler);
        for (int i = 0; i < MAX_MON_MOVES; i++)
        {
            u16 move = aiMoves[i];
            if (GetBattleMovePriority(battler, gAiLogicData->abilities[battler], move) > 0 && !(GetMovePriority(move) > 0 && IsBattleMoveStatus(move)))
            {
                return FIELD_EFFECT_POSITIVE;
            }
        }
    }

    // If we are faster or tie, we don't want trick room.
    if ((gAiLogicData->speedStats[battler] >= gAiLogicData->speedStats[FOE(battler)]) || (gAiLogicData->speedStats[battler] >= gAiLogicData->speedStats[BATTLE_PARTNER(FOE(battler))]))
        return FIELD_EFFECT_NEGATIVE;

    return FIELD_EFFECT_POSITIVE;
}

bool32 WeatherChecker(u32 battler, u32 weather, enum BattlerBenefitsFromFieldEffect desiredResult)
{
    if (IsWeatherActive(B_WEATHER_PRIMAL_ANY) != WEATHER_INACTIVE)
        return (FIELD_EFFECT_BLOCKED == desiredResult);

    enum BattlerBenefitsFromFieldEffect result = FIELD_EFFECT_NEUTRAL;
    enum BattlerBenefitsFromFieldEffect firstResult = FIELD_EFFECT_NEUTRAL;

    u32 i;
    u32 battlersOnSide = 1;

    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
        battlersOnSide = 2;

    for (i = 0; i < battlersOnSide; i++)
    {
        if (weather & B_WEATHER_RAIN)
            result = BenefitsFromRain(battler);
        else if (weather & B_WEATHER_SUN)
            result = BenefitsFromSun(battler);
        else if (weather & B_WEATHER_SANDSTORM)
            result = BenefitsFromSandstorm(battler);
        else if (weather & B_WEATHER_ICY_ANY)
            result = BenefitsFromHailOrSnow(battler, weather);

        battler = BATTLE_PARTNER(battler);

        if (result != FIELD_EFFECT_NEUTRAL)
        {
            if (weather & B_WEATHER_DAMAGING_ANY && i == 0 && battlersOnSide == 2)
                firstResult = result;
        }
    }
    if (firstResult != FIELD_EFFECT_NEUTRAL)
        return (firstResult == result) && (result == desiredResult);
    return (result == desiredResult);
}

bool32 FieldStatusChecker(u32 battler, u32 fieldStatus, enum BattlerBenefitsFromFieldEffect desiredResult)
{
    enum BattlerBenefitsFromFieldEffect result = FIELD_EFFECT_NEUTRAL;
    enum BattlerBenefitsFromFieldEffect firstResult = FIELD_EFFECT_NEUTRAL;
    u32 i;

    u32 battlersOnSide = 1;

    if (IsDoubleBattle() && IsBattlerAlive(BATTLE_PARTNER(battler)))
        battlersOnSide = 2;

    for (i = 0; i < battlersOnSide; i++)
    {
        // terrains
        if (fieldStatus & STATUS_FIELD_ELECTRIC_TERRAIN)
            result = BenefitsFromElectricTerrain(battler);
        if (fieldStatus & STATUS_FIELD_GRASSY_TERRAIN)
            result = BenefitsFromGrassyTerrain(battler);
        if (fieldStatus & STATUS_FIELD_MISTY_TERRAIN)
            result = BenefitsFromMistyTerrain(battler);
        if (fieldStatus & STATUS_FIELD_PSYCHIC_TERRAIN)
            result = BenefitsFromPsychicTerrain(battler);

        // other field statuses
        if (fieldStatus & STATUS_FIELD_TRICK_ROOM)
            result = BenefitsFromTrickRoom(battler);

        battler = BATTLE_PARTNER(battler);

        if (result != FIELD_EFFECT_NEUTRAL)
        {
            // Trick room wants both pokemon to agree, not just one
            if (fieldStatus & STATUS_FIELD_TRICK_ROOM && i == 0 && battlersOnSide == 2)
                firstResult = result;
        }
    }
    if (firstResult != FIELD_EFFECT_NEUTRAL)
        return (firstResult == result) && (result == desiredResult);
    return (result == desiredResult);
}

