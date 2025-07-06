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
These are almost all named in the format HasMoveWith, HasMoveThat, or Has[x]Move.

Under most circumstances, you want to use these two:
> `bool32 HasBattlerSideUsedMoveWithEffect(u32 battler, u32 effect)`
> `bool32 HasBattlerSideUsedMoveWithAdditionalEffect(u32 battler, u32 moveEffect);`

If you are checking more than two move effects or additional effects, please consider making more of these little functions. `bool32 HasTrappingMoveEffect(u32 battler)` is a good model to follow.

