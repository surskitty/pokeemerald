## Day/Night system FAQ

### Q: How do I disable DNS?
A: Set `OW_ENABLE_DNS` to `FALSE` in `include/config/overworld.h`.

### Q: What map changes need to be made for DNS?
A: If your project uses vanilla Emerald maps, and you have _not_ edited them already, revert commit [a5b079d833f18f66ebd53ac77f00227ae4a1f389](https://github.com/rh-hideout/pokeemerald-expansion/pull/6562/commits/a5b079d833f18f66ebd53ac77f00227ae4a1f389).

If you _have_ edited them already, you will need to edit the Lavaridge Town map to change the metatiles the two old ladies are on in the hot springs to be the normal hot spring water tile. You will also want to add some of the lighting object events from that commit, and copy the tileset changes and pla files to have as much of Hoenn light-blended as possible.

If you are not using Hoenn maps, the primary concern is that you do not use the exact same colors in a palette for both a window light and a standard color. By default, there are no lights, which prevents any strange effects like yellow water.

When writing map scripts, `fadescreenswapbuffers` should be preferred over `fadescreen` unless you are using `OW_OBJECT_VANILLA_SHADOWS`.

### Q: How do I make lightbulbs glow?

![Rustboro before adding lamp object events](/docs/tutorials/img/dns/without_lamp.png)
![Rustboro after adding lamp object events](/docs/tutorials/img/dns/with_lamp.png)

A: Making lamps glow is not part of the tileset itself.  Instead, place certain object events on top of where you desire a glowing effect.

These object events should use `OBJ_EVENT_GFX_LIGHT_SPRITE` and then as their `trainer_sight_or_berry_tree_id`, use `LIGHT_TYPE_BALL` for round lights (such as candles or gas lamps), `LIGHT_TYPE_PKMN_CENTER_SIGN` over a Pok&eacute;mon Center sign, or `LIGHT_TYPE_POKE_MART_SIGN` over a Pok&eacute;mart sign.

### Q: How do I mark certain colors in a palette as light-blended?
A: Create a `.pla` file in the same folder as the `.pal` with the same name. This can be done on any kind of palette; the commit to revert listed up above only applies it to tilesets, but you could easily do it for object events as well. Of note, there is a [commit reverted for being out of scope](https://github.com/rh-hideout/pokeemerald-expansion/pull/6562/commits/348f5967ac8d383c827b415e1040234a3f28626f) to make a follower Ampharos's tail glow.

In this file you can enter color indices [0,15]
on separate lines to mark those colors as being light-blended, i.e:

`06.pla:`
```
# A comment
0 # if color 0 is listed, uses it to blend with instead of the default!
1
9
10
```

During the day time, these color indices appear as normal, but will be blended with either yellow or the 0 index at night. These indices should only be used for things you expect to light up. If you are using porytiles, palette overrides and using slight alterations to a color will aid you in avoiding color conflicts where the wrong index is assigned.

![Rustboro gym after light-blending the windows](/docs/tutorials/img/dns/window_lights.png)

The windows appear as normal during the day time (blue) and light up in the night. These use the default color.

### Q: How do I return to using regular shadows?
A: Set `OW_OBJECT_VANILLA_SHADOWS` to `TRUE` in `include/config/overworld.h`.

### Q: How do I disable shadows for certain locations?
A: Shadows can be disabled for certain locations by modifying the `CurrentMapHasShadows` function in `src/overworld.c`.
