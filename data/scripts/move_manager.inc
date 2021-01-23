Common_EventScript_MoveManager::
	lockall
	faceplayer
	goto_if_unset FLAG_TEMP_1, Common_EventScript_MoveManager_2
Common_EventScript_MoveManager_1:
	switch VAR_MOVE_MANAGER
	case MOVE_DELETER, Common_EventScript_MoveManager_6
	msgbox Common_EventScript_MoveManager_Text_1, MSGBOX_YESNO
Common_EventScript_MoveManager_4:
	compare VAR_RESULT, NO
	goto_if_eq Common_EventScript_MoveManager_9
Common_EventScript_MoveManager_8:
	goto MoveManager_CheckItem
	return

Common_EventScript_MoveManager_2:
	goto MoveManager_Introduction
	goto Common_EventScript_MoveManager_1

Common_EventScript_MoveManager_6:
	msgbox Common_EventScript_MoveManager_Text_0, MSGBOX_YESNO
	goto Common_EventScript_MoveManager_4

Common_EventScript_MoveManager_9:
	msgbox MoveManager_ComeBack
	goto MoveManager_Exit
	goto Common_EventScript_MoveManager_8


MoveManager_Introduction::
	switch VAR_MOVE_MANAGER
	case MOVE_REMINDER, MoveManager_Introduction_3
	case MOVE_REMINDER_LEARN_ALL_MOVES, MoveManager_Introduction_4
	case MOVE_TUTOR_EGG_MOVES, MoveManager_Introduction_5
	case MOVE_DELETER, MoveManager_Introduction_6
MoveManager_Introduction_3:
	msgbox MoveManager_Introduction_Text_0
MoveManager_Introduction_1:
	setflag FLAG_TEMP_1
	msgbox MoveManager_Introduction_Text_4, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveManager_Introduction_8
MoveManager_Introduction_7:
	goto MoveManager_CheckItem
	return

MoveManager_Introduction_4:
	msgbox MoveManager_Introduction_Text_1
	goto MoveManager_Introduction_1

MoveManager_Introduction_5:
	msgbox MoveManager_Introduction_Text_2
	goto MoveManager_Introduction_1

MoveManager_Introduction_6:
	msgbox MoveManager_Introduction_Text_3
	goto MoveManager_Introduction_1

MoveManager_Introduction_8:
	msgbox MoveManager_ComeBack
	goto MoveManager_Exit
	goto MoveManager_Introduction_7


MoveManager_CheckItem::
	switch VAR_MOVE_MANAGER
	case MOVE_REMINDER_LEARN_ALL_MOVES, MoveManager_CheckItem_2
	case MOVE_TUTOR_EGG_MOVES, MoveManager_CheckItem_3
	goto MoveManager_ChooseMon
	return

MoveManager_CheckItem_2:
	msgbox MoveManager_CheckItem_Text_0
	waitmessage
	checkitem ITEM_HEART_SCALE, 1
	compare VAR_RESULT, 0
	goto_if_eq MoveManager_CheckItem_5
	return

MoveManager_CheckItem_3:
	msgbox MoveManager_CheckItem_Text_1
	waitmessage
	checkitem ITEM_TINY_MUSHROOM, 1
	compare VAR_RESULT, 0
	goto_if_eq MoveManager_CheckItem_7
	return

MoveManager_CheckItem_5:
	msgbox MoveManager_ComeBackWithItem
	goto MoveManager_Exit
	return

MoveManager_CheckItem_7:
	msgbox MoveManager_ComeBackWithItem
	goto MoveManager_Exit
	return


MoveManager_ChooseMon::
	switch VAR_MOVE_MANAGER
	case MOVE_DELETER, MoveManager_ChooseMon_3
	special ChooseMonForMoveRelearner
MoveManager_ChooseMon_1:
	waitstate
	compare VAR_0x8004, 255
	goto_if_eq MoveManager_ChooseMon_6
MoveManager_ChooseMon_5:
	special IsSelectedMonEgg
	compare VAR_RESULT, 0
	goto_if_ne MoveManager_ChooseMon_9
MoveManager_ChooseMon_8:
	switch VAR_MOVE_MANAGER
	case MOVE_DELETER, MoveManager_ChooseMon_13
	compare VAR_0x8005, 0
	goto_if_eq MoveManager_ChooseMon_20
MoveManager_ChooseMon_11:
	compare VAR_MOVE_MANAGER, MOVE_DELETER
	goto_if_eq MoveManager_ChooseMon_15
	msgbox MoveManager_ChooseMon_Text_7
	special TeachMoveRelearnerMove
	waitstate
	compare VAR_0x8004, 0
	goto_if_eq MoveManager_ChooseMon_26
MoveManager_ChooseMon_25:
	goto MoveManager_SpendItem
	return

MoveManager_ChooseMon_3:
	special ChoosePartyMon
	goto MoveManager_ChooseMon_1

MoveManager_ChooseMon_6:
	msgbox MoveManager_ComeBack
	goto MoveManager_Exit
	goto MoveManager_ChooseMon_5

MoveManager_ChooseMon_9:
	msgbox MoveManager_ChooseMon_Text_0
	goto MoveManager_ChooseMon
	goto MoveManager_ChooseMon_8

MoveManager_ChooseMon_13:
	special GetNumMovesSelectedMonHas
	compare VAR_RESULT, 1
	goto_if_eq MoveManager_ChooseMon_18
	goto MoveManager_ChooseMon_11

MoveManager_ChooseMon_15:
	msgbox MoveManager_ChooseMon_Text_3
	fadescreen FADE_TO_BLACK
	special MoveDeleterChooseMoveToForget
	fadescreen FADE_FROM_BLACK
	compare VAR_0x8005, MAX_MON_MOVES
	goto_if_eq MoveManager_ChooseMon_23
MoveManager_ChooseMon_22:
	special BufferMoveDeleterNicknameAndMove
	msgbox MoveManager_ChooseMon_Text_4, MSGBOX_YESNO
	compare VAR_RESULT, YES
	goto_if_eq MoveManager_ChooseMon_28
	msgbox MoveManager_ComeBack
	goto MoveManager_Exit
	return

MoveManager_ChooseMon_18:
	special BufferMoveDeleterNicknameAndMove
	msgbox MoveManager_ChooseMon_Text_1
	goto MoveManager_Exit
	goto MoveManager_ChooseMon_11

MoveManager_ChooseMon_20:
	msgbox MoveManager_ChooseMon_Text_2
	goto MoveManager_Exit
	goto MoveManager_ChooseMon_11

MoveManager_ChooseMon_23:
	goto MoveManager_ChooseMon
	goto MoveManager_ChooseMon_22

MoveManager_ChooseMon_26:
	goto MoveManager_ChooseMon
	goto MoveManager_ChooseMon_25

MoveManager_ChooseMon_28:
	special IsLastMonThatKnowsSurf
	compare VAR_RESULT, 0
	goto_if_ne MoveManager_ChooseMon_32
MoveManager_ChooseMon_31:
	special MoveDeleterForgetMove
	playfanfare MUS_MOVE_DELETED
	waitfanfare
	msgbox MoveManager_ChooseMon_Text_6
	goto MoveManager_Exit
	return

MoveManager_ChooseMon_32:
	special BufferMoveDeleterNicknameAndMove
	msgbox MoveManager_ChooseMon_Text_5
	goto MoveManager_Exit
	goto MoveManager_ChooseMon_31


MoveManager_SpendItem::
	switch VAR_MOVE_MANAGER
	case MOVE_REMINDER_LEARN_ALL_MOVES, MoveManager_SpendItem_3
	case MOVE_TUTOR_EGG_MOVES, MoveManager_SpendItem_4
MoveManager_SpendItem_1:
	msgbox MoveManager_ComeBack
	goto MoveManager_Exit
	return

MoveManager_SpendItem_3:
	msgbox MoveManager_SpendItem_Text_0
	removeitem ITEM_HEART_SCALE
	goto MoveManager_SpendItem_1

MoveManager_SpendItem_4:
	msgbox MoveManager_SpendItem_Text_1
	removeitem ITEM_TINY_MUSHROOM
	goto MoveManager_SpendItem_1


MoveManager_Exit::
	setvar VAR_MOVE_MANAGER, 0
	releaseall
	return


Common_EventScript_MoveManager_Text_0:
	.string "Would you like to forget a move?$"

Common_EventScript_MoveManager_Text_1:
	.string "Would you like to learn a move?$"

MoveManager_Introduction_Text_0:
	.string "I'm the Move Reminder.\pI know how to remind Pokémon\nof moves they have forgotten.$"

MoveManager_Introduction_Text_1:
	.string "I'm the Move Tutor.\pI know all the moves that Pokémon\nlearn, and I can teach yours, too.$"

MoveManager_Introduction_Text_2:
	.string "I'm the Egg Move Tutor.\pI specialise in teaching moves\nPokémon learn from their parents.$"

MoveManager_Introduction_Text_3:
	.string "Uh…\nOh, yes, I'm the Move Deleter.\pI can make Pokémon forget their moves.$"

MoveManager_Introduction_Text_4:
	.string "Would you like me to do that?$"

MoveManager_CheckItem_Text_0:
	.string "In that case, I would like\na Heart Scale.$"

MoveManager_CheckItem_Text_1:
	.string "In that case, I would like\na Tiny Mushroom.$"

MoveManager_ChooseMon_Text_0:
	.string "Eggs don't know moves yet.$"

MoveManager_ChooseMon_Text_1:
	.string "{STR_VAR_1} knows only one move\nso it can't be forgotten…$"

MoveManager_ChooseMon_Text_2:
	.string "Sorry…\pIt doesn't appear as if I have any move\nI can teach that Pokémon.$"

MoveManager_ChooseMon_Text_3:
	.string "Which move should be forgotten?$"

MoveManager_ChooseMon_Text_4:
	.string "Hm! {STR_VAR_1}'s {STR_VAR_2}?\nThat move should be forgotten?$"

MoveManager_ChooseMon_Text_5:
	.string "Hm!\p Your {STR_VAR_1} doesn't seem willing\n to forget {STR_VAR_2}.$"

MoveManager_ChooseMon_Text_6:
	.string "It worked to perfection!\p{STR_VAR_1} has forgotten\n{STR_VAR_2} completely.$"

MoveManager_ChooseMon_Text_7:
	.string "Which move should I teach?$"

MoveManager_SpendItem_Text_0:
	.string "{PLAYER} handed over one Heart Scale\nin exchange.$"

MoveManager_SpendItem_Text_1:
	.string "{PLAYER} handed over a single\nTiny Mushroom$"

MoveManager_ComeBack::
	.string "Come back soon.$"

MoveManager_ComeBackWithItem::
	.string "You don't have any?\nCome back another time.$"
