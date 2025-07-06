# AI Helpers: An Overview
This is not a cohesive list! Here are a number of things to help you work with the AI.

Most functions referred to here are in `src/battle_ai_util.c`.  Some others are in `src/battle_ai_main.c`, `src/battle_ai_switch_items.c`, `src/battle_main.c` or `src/battle_util.c`.

## Important constants, macros, and simple functions
- `FOE(battler)`, `PARTNER_OF(battler)`, `PARTNER_OF(FOE(battler))` -- the battler is the pokemon considering acting.
- `bool32 IsTargetingPartner(u32 battlerAtk, u32 battlerDef)` -- This currently only looks at positioning. If you want an AI that gladly kills its ally, this is where you edit.

### Speed Awareness
> `bool32 AI_IsFaster(u32 battlerAi, u32 battlerDef, u32 move)` and `bool32 AI_IsSlower(u32 battlerAi, u32 battlerDef, u32 move)`
These determine what order the AI expects the turn to go in, assuming it uses that move. It already takes into account Trick Room and priority brackets for you.

The functions to edit to both teach the AI about other effects and directly implement them are:
- `src/battle_ai_util.c`  `s32 AI_WhoStrikesFirst(u32 battlerAI, u32 battler, u32 moveConsidered)`
- `src/battle_main.c` `s32 GetBattleMovePriority(u32 battler, u32 ability, u32 move)`

## What moves does a Pok&eacute;mon have?
These are almost all named in the format HasMoveWith, HasMoveThat, or Has[x]Move. They universally return a boolean.

Under most circumstances, you want to use these two:
> `bool32 HasBattlerSideUsedMoveWithEffect(u32 battler, u32 effect)`
> `bool32 HasBattlerSideUsedMoveWithAdditionalEffect(u32 battler, u32 moveEffect)`

If you are checking more than two move effects or additional effects, please consider making more of these little functions. `bool32 HasTrappingMoveEffect(u32 battler)` is a good model to follow.

### `src/battle_ai_util.c`
BattlerSide functions check both the battler and its ally for the effect. If you are not certain you do not want a functionality to apply to Double battles, preferentially use those.

`HasBattlerSideMoveWithEffect(u32 battler, u32 effect)` is the Omniscient version of `HasBattlerSideUsedMoveWithEffect(u32 battler, u32 effect)`

`HasBattlerSideMoveWithAdditionalEffect` is the Omniscient version of `HasBattlerSideUsedMoveWithAdditionalEffect(u32 battler, u32 moveEffect)`.

Checking the side or party.
- `PartyHasMoveCategory(u32 battlerId, enum DamageCategory category)`
- `SideHasMoveCategory(u32 battlerId, enum DamageCategory category)`
- PartnerHasSameMoveEffectWithoutTarget(u32 battlerAtkPartner, u32 move, u32 partnerMove)

Checking a specific Pok&eacute;mon.
- `HasMove(u32 battlerId, u32 move)`
- `HasDamagingMove(u32 battlerId)`
- `HasDamagingMoveOfType(u32 battlerId, u32 type)`
- `HasOnlyMovesWithCategory(u32 battlerId, enum DamageCategory category, bool32 onlyOffensive)` -- The boolean is to decide if it counts status moves or not.
- `HasMoveWithCategory(u32 battler, enum DamageCategory category)`
- `HasMoveWithType(u32 battler, u32 type)`
- `HasMoveWithEffect(u32 battlerId, enum BattleMoveEffects moveEffect)`

- `HasNonVolatileMoveEffect(u32 battlerId, u32 effect)`
- `HasMoveWithAdditionalEffect(u32 battlerId, u32 moveEffect)`
- `HasMoveWithCriticalHitChance(u32 battlerId)`
- `HasMoveWithMoveEffectExcept(u32 battlerId, u32 moveEffect, enum BattleMoveEffects exception)`
- `HasMoveThatLowersOwnStats(u32 battlerId)` -- Intended for Contrary and White Herb.
- `HasAnyKnownMove(u32 battlerId)`
- `HasSleepMoveWithLowAccuracy(u32 battlerAtk, u32 battlerDef)`

- `HasTrappingMoveEffect(u32 battler)`
- `HasLightSensitiveMove(u32 battler)` -- Moves like Solar Beam and Synthesis that are weakened by `B_WEATHER_LOW_LIGHT` like Rain, Snow, or Fog, and strengthened by Sun.
- `HasThawingMove(u32 battler)`
- `HasMoveWithFlag(u32 battler, MoveFlag getFlag)`
- `HasLowAccuracyMove(u32 battlerAtk, u32 battlerDef)` -- This uses `include/config/ai.h` `LOW_ACCURACY_THRESHOLD` which is by default 70%. It is used for Blunder Policy and No Guard.
- `HasMoveWithLowAccuracy(u32 battlerAtk, u32 battlerDef, u32 accCheck, bool32 ignoreStatus, u32 atkAbility, u32 defAbility, u32 atkHoldEffect, u32 defHoldEffect)` -- This lets you pick the low accuracy threshold per usage, but in practice, it's for Compound Eyes.
- `HasHealingEffect(u32 battler)`


### `src/battle_util.c`
- `MoveHasAdditionalEffect(u32 move, u32 moveEffect)`
- `MoveHasAdditionalEffectWithChance(u32 move, u32 moveEffect, u32 chance)`
- `MoveHasAdditionalEffectSelf(u32 move, u32 moveEffect)`
- `MoveHasChargeTurnAdditionalEffect(u32 move)`

## Using Types of Moves
> `bool32 ShouldTrap(u32 battlerAtk, u32 battlerDef, u32 move)`
This is for trapping-based move effects. AI_CanBattlerEscape() and IsBattlerTrapped() determine if the Pok&eacute;mon is able to switch.

>`ShouldSetWeather(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, u32 weather)` and `ShouldSetFieldStatus(u32 battler, u32 ability, enum ItemHoldEffect holdEffect, u32 fieldStatus)`
Overarching handlers for whether a weather effect or a terrain effect are beneficial to a pokemon and its partner. Called primarily in Check Viability.

## Teaching the AI about Abilities
`s32 BattlerBenefitsFromAbilityScore()` is the big function for informing the AI if the current Ability it has is good in context.  This is the function you need to edit to improve logic for using Skill Swap, Role Play, Entrainment, Gastro Acid, and so on. If you are not adding new move effects that change abiities, you can ignore `CanEffectChangeAbility()` and `AbilityChangeScore()` entirely.

Ability ratings as in `src/data/abilities.c` are for the AI to determine the likelihood it can predict the Ability with the AI flag `AI_FLAG_WEIGH_ABILITY_PREDICTION`.  Negative ratings are treated as always bad to have for the purposes of Skill Swap.

- `DoesAbilityBenefitFromFieldStatus(u32 ability, u32 fieldStatus)` -- This includes Terrains.
- `DoesAbilityBenefitFromWeather(u32 ability, u32 weather)`
- `IsMoxieTypeAbility(u32 ability)` -- Abilities that raise stats upon KO.
- `DoesAbilityRaiseStatsWhenLowered(u32 ability)` -- Abilities like Defiant or Contrary.
- `ShouldTriggerAbility(u32 battlerAtk, u32 battlerDef, u32 ability)` -- Under what circumstances do you attack into an Ability that buffs the Pokemon hit?


<-- 
bool32 CanTargetFaintAi(u32 battlerDef, u32 battlerAtk)
u32 NoOfHitsForTargetToFaintAI(u32 battlerDef, u32 battlerAtk)
u32 GetBestDmgMoveFromBattler(u32 battlerAtk, u32 battlerDef, enum DamageCalcContext calcContext)
u32 GetBestDmgFromBattler(u32 battler, u32 battlerTarget, enum DamageCalcContext calcContext)
bool32 CanTargetMoveFaintAi(u32 move, u32 battlerDef, u32 battlerAtk, u32 nHits)
bool32 CanTargetFaintAiWithMod(u32 battlerDef, u32 battlerAtk, s32 hpMod, s32 dmgMod)
s32 AI_DecideKnownAbilityForTurn(u32 battlerId)
enum ItemHoldEffect AI_DecideHoldEffectForTurn(u32 battlerId)
bool32 DoesBattlerIgnoreAbilityChecks(u32 battlerAtk, u32 atkAbility, u32 move)
u32 AI_GetWeather(void)
bool32 CanAIFaintTarget(u32 battlerAtk, u32 battlerDef, u32 numHits)
bool32 CanIndexMoveFaintTarget(u32 battlerAtk, u32 battlerDef, u32 index, enum DamageCalcContext calcContext)
bool32 CanIndexMoveGuaranteeFaintTarget(u32 battlerAtk, u32 battlerDef, u32 index)
u32 GetBattlerSecondaryDamage(u32 battlerId)
bool32 BattlerWillFaintFromWeather(u32 battler, u32 ability)
bool32 BattlerWillFaintFromSecondaryDamage(u32 battler, u32 ability)
bool32 ShouldTryOHKO(u32 battlerAtk, u32 battlerDef, u32 atkAbility, u32 defAbility, u32 move)
bool32 ShouldUseRecoilMove(u32 battlerAtk, u32 battlerDef, u32 recoilDmg, u32 moveIndex)
u32 GetBattlerSideSpeedAverage(u32 battler)
bool32 ShouldAbsorb(u32 battlerAtk, u32 battlerDef, u32 move, s32 damage)
bool32 ShouldRecover(u32 battlerAtk, u32 battlerDef, u32 move, u32 healPercent, enum DamageCalcContext calcContext)
bool32 ShouldSetScreen(u32 battlerAtk, u32 battlerDef, enum BattleMoveEffects moveEffect)
enum AIPivot ShouldPivot(u32 battlerAtk, u32 battlerDef, u32 defAbility, u32 move, u32 moveIndex)
bool32 IsRecycleEncouragedItem(u32 item)
bool32 ShouldRestoreHpBerry(u32 battlerAtk, u32 item)
bool32 IsStatBoostingBerry(u32 item)
bool32 CanKnockOffItem(u32 battler, u32 item)
bool32 AI_IsAbilityOnSide(u32 battlerId, u32 ability)
bool32 AI_MoveMakesContact(u32 ability, enum ItemHoldEffect holdEffect, u32 move)
bool32 ShouldUseZMove(u32 battlerAtk, u32 battlerDef, u32 chosenMove)
void SetAIUsingGimmick(u32 battler, enum AIConsiderGimmick use)
bool32 IsAIUsingGimmick(u32 battler)
void DecideTerastal(u32 battler)

// stat stage checks
bool32 AnyStatIsRaised(u32 battlerId)
bool32 CanLowerStat(u32 battlerAtk, u32 battlerDef, u32 abilityDef, u32 stat)
bool32 BattlerStatCanRise(u32 battler, u32 battlerAbility, u32 stat)
bool32 AreBattlersStatsMaxed(u32 battler)
u32 CountPositiveStatStages(u32 battlerId)
u32 CountNegativeStatStages(u32 battlerId)

// move checks
bool32 IsAffectedByPowder(u32 battler, u32 ability, enum ItemHoldEffect holdEffect)
bool32 MovesWithCategoryUnusable(u32 attacker, u32 target, enum DamageCategory category)
s32 AI_WhichMoveBetter(u32 move1, u32 move2, u32 battlerAtk, u32 battlerDef, s32 noOfHitsToKo)
struct SimulatedDamage AI_CalcDamageSaveBattlers(u32 move, u32 battlerAtk, u32 battlerDef, uq4_12_t *typeEffectiveness, enum AIConsiderGimmick considerGimmickAtk, enum AIConsiderGimmick considerGimmickDef)
struct SimulatedDamage AI_CalcDamage(u32 move, u32 battlerAtk, u32 battlerDef, uq4_12_t *typeEffectiveness, enum AIConsiderGimmick considerGimmickAtk, enum AIConsiderGimmick considerGimmickDef, u32 weather)
bool32 AI_IsDamagedByRecoil(u32 battler)
u32 GetNoOfHitsToKO(u32 dmg, s32 hp)
u32 GetNoOfHitsToKOBattlerDmg(u32 dmg, u32 battlerDef)
u32 GetNoOfHitsToKOBattler(u32 battlerAtk, u32 battlerDef, u32 moveIndex, enum DamageCalcContext calcContext)
u32 GetCurrDamageHpPercent(u32 battlerAtk, u32 battlerDef, enum DamageCalcContext calcContext)
uq4_12_t AI_GetMoveEffectiveness(u32 move, u32 battlerAtk, u32 battlerDef)
u16 *GetMovesArray(u32 battler)
bool32 IsConfusionMoveEffect(enum BattleMoveEffects moveEffect)
bool32 IsPowerBasedOnStatus(u32 battlerId, enum BattleMoveEffects effect, u32 argument)
bool32 IsAromaVeilProtectedEffect(enum BattleMoveEffects moveEffect)
bool32 IsNonVolatileStatusMove(u32 moveEffect)
bool32 IsMoveRedirectionPrevented(u32 battlerAtk, u32 move, u32 atkAbility)
bool32 IsMoveEncouragedToHit(u32 battlerAtk, u32 battlerDef, u32 move)
bool32 IsHazardMove(u32 move)
bool32 IsTwoTurnNotSemiInvulnerableMove(u32 battlerAtk, u32 move)
bool32 IsBattlerDamagedByStatus(u32 battler)
s32 ProtectChecks(u32 battlerAtk, u32 battlerDef, u32 move, u32 predictedMove)
bool32 HasSleepMoveWithLowAccuracy(u32 battlerAtk, u32 battlerDef)
bool32 IsHealingMove(u32 move)
bool32 ShouldFakeOut(u32 battlerAtk, u32 battlerDef, u32 move)
bool32 IsStatRaisingEffect(enum BattleMoveEffects effect)
bool32 IsStatLoweringEffect(enum BattleMoveEffects effect)
bool32 IsSelfStatLoweringEffect(enum BattleMoveEffects effect)
bool32 IsSelfStatRaisingEffect(enum BattleMoveEffects effect)
bool32 IsSwitchOutEffect(enum BattleMoveEffects effect)
bool32 IsChaseEffect(enum BattleMoveEffects effect)
bool32 IsAttackBoostMoveEffect(enum BattleMoveEffects effect)
bool32 IsUngroundingEffect(enum BattleMoveEffects effect)
bool32 IsSemiInvulnerable(u32 battlerDef, u32 move)
bool32 HasMoveWithFlag(u32 battler, MoveFlag getFlag)
bool32 IsHazardClearingMove(u32 move)
bool32 IsSubstituteEffect(enum BattleMoveEffects effect)

// status checks
bool32 IsBattlerIncapacitated(u32 battler, u32 ability)
bool32 ShouldPoison(u32 battlerAtk, u32 battlerDef)
bool32 ShouldBurn(u32 battlerAtk, u32 battlerDef, u32 abilityDef)
bool32 ShouldFreezeOrFrostbite(u32 battlerAtk, u32 battlerDef, u32 abilityDef)
bool32 ShouldParalyze(u32 battlerAtk, u32 battlerDef, u32 abilityDef)
bool32 AnyPartyMemberStatused(u32 battlerId, bool32 checkSoundproof)
u32 ShouldTryToFlinch(u32 battlerAtk, u32 battlerDef, u32 atkAbility, u32 defAbility, u32 move)
bool32 ShouldTrap(u32 battlerAtk, u32 battlerDef, u32 move)
bool32 IsWakeupTurn(u32 battler)
bool32 AI_IsBattlerAsleepOrComatose(u32 battlerId)

// partner logic
bool32 IsTargetingPartner(u32 battlerAtk, u32 battlerDef)
u32 GetAllyChosenMove(u32 battlerId)
bool32 IsValidDoubleBattle(u32 battlerAtk)
bool32 DoesPartnerHaveSameMoveEffect(u32 battlerAtkPartner, u32 battlerDef, u32 move, u32 partnerMove)
bool32 PartnerHasSameMoveEffectWithoutTarget(u32 battlerAtkPartner, u32 move, u32 partnerMove)
bool32 PartnerMoveEffectIsStatusSameTarget(u32 battlerAtkPartner, u32 battlerDef, u32 partnerMove)
bool32 IsMoveEffectWeather(u32 move)
bool32 PartnerMoveEffectIsTerrain(u32 battlerAtkPartner, u32 partnerMove)
bool32 PartnerMoveEffectIs(u32 battlerAtkPartner, u32 partnerMove, enum BattleMoveEffects effectCheck)
bool32 PartnerMoveIs(u32 battlerAtkPartner, u32 partnerMove, u32 moveCheck)
bool32 PartnerMoveIsSameAsAttacker(u32 battlerAtkPartner, u32 battlerDef, u32 move, u32 partnerMove)
bool32 PartnerMoveIsSameNoTarget(u32 battlerAtkPartner, u32 move, u32 partnerMove)
bool32 PartnerMoveActivatesSleepClause(u32 move)
bool32 ShouldUseWishAromatherapy(u32 battlerAtk, u32 battlerDef, u32 move)
u32 GetFriendlyFireKOThreshold(u32 battler)
bool32 IsAllyProtectingFromMove(u32 battlerAtk, u32 attackerMove, u32 allyMove)

// party logic
struct BattlePokemon *AllocSaveBattleMons(void)
void FreeRestoreBattleMons(struct BattlePokemon *savedBattleMons)
s32 CountUsablePartyMons(u32 battlerId)
bool32 IsPartyFullyHealedExceptBattler(u32 battler)
bool32 PartyHasMoveCategory(u32 battlerId, enum DamageCategory category)
bool32 SideHasMoveCategory(u32 battlerId, enum DamageCategory category)

// score increases
u32 IncreaseStatUpScore(u32 battlerAtk, u32 battlerDef, enum StatChange statId)
u32 IncreaseStatUpScoreContrary(u32 battlerAtk, u32 battlerDef, enum StatChange statId)
u32 IncreaseStatDownScore(u32 battlerAtk, u32 battlerDef, u32 stat)
void IncreasePoisonScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
void IncreaseBurnScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
void IncreaseParalyzeScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
void IncreaseSleepScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
void IncreaseConfusionScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
void IncreaseFrostbiteScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)

s32 AI_CalcPartyMonDamage(u32 move, u32 battlerAtk, u32 battlerDef, struct BattlePokemon switchinCandidate, enum DamageCalcContext calcContext)
u32 AI_WhoStrikesFirstPartyMon(u32 battlerAtk, u32 battlerDef, struct BattlePokemon switchinCandidate, u32 moveConsidered)
s32 AI_TryToClearStats(u32 battlerAtk, u32 battlerDef, bool32 isDoubleBattle)
bool32 AI_ShouldCopyStatChanges(u32 battlerAtk, u32 battlerDef)
bool32 AI_ShouldSetUpHazards(u32 battlerAtk, u32 battlerDef, struct AiLogicData *aiData)
void IncreaseTidyUpScore(u32 battlerAtk, u32 battlerDef, u32 move, s32 *score)
bool32 AI_ShouldSpicyExtract(u32 battlerAtk, u32 battlerAtkPartner, u32 move, struct AiLogicData *aiData)
u32 IncreaseSubstituteMoveScore(u32 battlerAtk, u32 battlerDef, u32 move)
bool32 IsBattlerItemEnabled(u32 battler)
bool32 IsBattlerPredictedToSwitch(u32 battler)
u32 GetIncomingMove(u32 battler, u32 opposingBattler, struct AiLogicData *aiData)
bool32 HasBattlerSideAbility(u32 battlerDef, u32 ability, struct AiLogicData *aiData)
u32 GetThinkingBattler(u32 battler)
-->
