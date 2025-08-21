#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/abilities.h"

TEST("Print mons")
{
    u32 endVal = NUM_SPECIES;
    for (u32 i = 1; i < endVal; i++)
    {
        const struct SpeciesInfo *currSpecies = &gSpeciesInfo[i];

        //  Detect if species has data
        if (currSpecies->baseHP == 0)
            continue;

        //  Start printing species data
        DebugPrintf("{");
        //  Print species name
        DebugPrintf("    \"speciesName\": \"%S\",", currSpecies->speciesName);

        //  Print types
        if (currSpecies->types[0] != currSpecies->types[1])
            DebugPrintf("    \"types\": [\"%S\", \"%S\"],", gTypesInfo[currSpecies->types[0]].name, gTypesInfo[currSpecies->types[1]].name);
        else
            DebugPrintf("    \"types\": [\"%S\"],", gTypesInfo[currSpecies->types[0]].name);

        //  Print stats
        DebugPrintf("    \"stats\": {");
        DebugPrintf("        \"hp\": %u,", currSpecies->baseHP);
        DebugPrintf("        \"attack\": %u,", currSpecies->baseAttack);
        DebugPrintf("        \"defense\": %u,", currSpecies->baseDefense);
        DebugPrintf("        \"spAttack\": %u,", currSpecies->baseSpAttack);
        DebugPrintf("        \"spDefense\": %u,", currSpecies->baseSpDefense);
        DebugPrintf("        \"speed\": %u,", currSpecies->baseSpeed);
        DebugPrintf("    },");

        //  Print abilities
        if (currSpecies->abilities[0] != currSpecies->abilities[1] && currSpecies->abilities[1] != ABILITY_NONE)
            DebugPrintf("    \"Abilities\": [\"%S\", \"%S\"],", gAbilitiesInfo[currSpecies->abilities[0]].name, gAbilitiesInfo[currSpecies->abilities[1]].name);
        else
            DebugPrintf("    \"Abilities\": [\"%S\"],", gAbilitiesInfo[currSpecies->abilities[0]].name);
        if (currSpecies->abilities[2] != ABILITY_NONE)
            DebugPrintf("    \"Hidden Ability\": \"%S\",", gAbilitiesInfo[currSpecies->abilities[2]].name);

        //  Print moves
        //  Level up moves
        DebugPrintf("    \"levelUpMoves\": [");
        bool32 shouldContinue = TRUE;
        u32 moveIndex = 0;
        const struct LevelUpMove *levelUpLearnset = currSpecies->levelUpLearnset;
        while (shouldContinue)
        {
            DebugPrintf("        {");
            DebugPrintf("            \"level\": %u,", levelUpLearnset[moveIndex].level);
            DebugPrintf("            \"move\": \"%S\"", gMovesInfo[levelUpLearnset[moveIndex].move].name);
            moveIndex++;
            if (levelUpLearnset[moveIndex].move == LEVEL_UP_MOVE_END)
            {
                shouldContinue = FALSE;
                DebugPrintf("        }");
            }
            else
            {
                DebugPrintf("        },");
            }
        }
        DebugPrintf("    ],");

        //  Teachable moves
        shouldContinue = TRUE;
        moveIndex = 0;
        const u16 *teachableLearnset = currSpecies->teachableLearnset;
        if (teachableLearnset[moveIndex] != MOVE_UNAVAILABLE)
        {
            DebugPrintf("    \"teachableLearnset\": [");
            while (shouldContinue)
            {
                if (teachableLearnset[moveIndex + 1] != MOVE_UNAVAILABLE)
                {
                    DebugPrintf("        \"%S\",", gMovesInfo[teachableLearnset[moveIndex]].name);
                }
                else
                {
                    DebugPrintf("        \"%S\"", gMovesInfo[teachableLearnset[moveIndex]].name);
                    shouldContinue = FALSE;
                }
                moveIndex++;
            }
            DebugPrintf("    ],");
        }

        //  Egg moves
        shouldContinue = TRUE;
        moveIndex = 0;
        const u16 *eggMoves = GetSpeciesEggMoves(i);
        if (eggMoves[moveIndex] != MOVE_UNAVAILABLE)
        {
            DebugPrintf("    \"eggMoves\": [");
            while (shouldContinue)
            {
                if (eggMoves[moveIndex + 1] != MOVE_UNAVAILABLE)
                {
                    DebugPrintf("        \"%S\",", gMovesInfo[eggMoves[moveIndex]].name);
                }
                else
                {
                    DebugPrintf("        \"%S\"", gMovesInfo[eggMoves[moveIndex]].name);
                    shouldContinue = FALSE;
                }
                moveIndex++;
            }
            DebugPrintf("    ],");
        }

        //  Various data
        DebugPrintf("    \"catchRate\": %u,", currSpecies->catchRate);
        DebugPrintf("    \"expYield\": %u,", currSpecies->expYield);
        DebugPrintf("    \"eggCycles\": %u,", currSpecies->eggCycles);
        DebugPrintf("    \"monCategory\": \"%S\",", currSpecies->categoryName);
        DebugPrintf("    \"natDexNum\": %u,", currSpecies->natDexNum);
        DebugPrintf("    \"internalId\": %u,", i);

        //  Print forms
        if (currSpecies->isMegaEvolution)
            DebugPrintf("    \"form\": \"mega\",");
        else if (currSpecies->isGigantamax)
            DebugPrintf("    \"form\": \"gigantamax\",");

        DebugPrintf("}");
    }
}
