# Inventory System — Polish & Backlog

Running list of known gaps, shortcuts, and cleanup items to revisit once core functionality is stable. Not urgent — captured so nothing gets forgotten.

---

## 1. Context Menu closes via `GetAllWidgetsOfClass` (accepted shortcut)

**Current state:** `WBP_MenuButton`'s click handler closes the context menu by calling `GetAllWidgetsOfClass(WBP_ContextMenu)` and removing all of them, rather than holding a direct reference to its owning menu.

**Why it's fine for now:** Matches an existing pattern already used in `Left Click On Inventory Item` (which does the same thing before spawning a new menu). Works perfectly for single-player, single-menu-at-a-time use.

**Revisit when:** You ever need multiple context menus open simultaneously (split-screen, multi-window UI, etc.) — at that point, switch to:
- **Option A:** Add an `OwningMenu` (WBP_ContextMenu ref) variable to `WBP_MenuButton`, set it in `Populate Menu`, call `RemoveFromParent` on that specific reference.
- **Option B:** Event dispatcher on `WBP_MenuButton` (`OnActionSelected`) that `WBP_ContextMenu` binds to per-button in `Populate Menu`.

---

## 2. Direct casts to `BP_ThirdPersonCharacter` scattered across blueprints

**Where:**
- `BP_ItemPickup` — both overlap events (`OnComponentBeginOverlap`, `OnComponentEndOverlap`) cast directly to `BP_ThirdPersonCharacter_C`
- `WBP_InventorySlot` — `Left Click On Inventory Item` and `Right Click On Inventory Item` both cast directly to `BP_ThirdPersonCharacter_C`

**Why it matters:** Blocks building a clean, reusable Interaction System — any future actor type (AI, other player characters, etc.) that should be able to pick up/interact with items can't, because the logic is hardcoded to one concrete class.

**Fix direction:** Introduce an interaction interface (e.g. `BPI_Interactor` or similar) that exposes what's actually needed (get inventory component, get instigator actor) instead of casting to the concrete character class.

---

## 3. `DropItem` custom event on `BP_ThirdPersonCharacter` duplicates `RequestDropItem` logic

**Current state:** The character's `DropItem` custom event does `GetArrayItem → SpawnActorFromClass → RemoveItem` directly, instead of calling `UInventoryRulesSubsystem::RequestDropItem` (which already does this authoritatively and is used elsewhere, e.g. right-click drop from the inventory slot).

**Why it matters:** Two different code paths for "drop an item" means bug fixes / rule changes (e.g. spawn offset, collision handling) have to be made twice, and they can silently drift out of sync.

**Fix direction:** Replace the body of `DropItem` with a call to `RequestExecuteAction`-style subsystem call — i.e. route it through `RequestDropItem` like everything else does.

---

## 4. `EActionResult` conflates "did it succeed" with "should it consume a stack"

**Current state:** Single enum (`Success`, `Failed`, `ConsumeItem`) used as `ExecuteAction`'s return value. Works for Consumable today.

**Why it might matter later:** An action could succeed *and not* consume a stack (e.g. Equip — equipping shouldn't remove the item from inventory), or fail but still need some cleanup. The current enum can't cleanly express "succeeded, don't consume."

**Fix direction (only if/when a non-consuming action type is added):** Split into two outputs — a bool `bSuccess` and a separate bool/enum for `bShouldConsumeStack` — or a small struct `FActionResult` with both fields.

---

## 5. `RequestExecuteAction`'s reflection-based dispatch (technical debt, works fine for now)

**Current state:** Since `BPI_ContextAction` is a pure Blueprint interface (no native C++ class), `RequestExecuteAction` calls it via `FindFunction` + manual `ProcessEvent` + a hand-built param struct, rather than a clean native `Execute_X()` call.

**Why it's fine for now:** Works correctly, matches how Blueprint's own `K2Node_Message` does it internally.

**Revisit when:** If `BPI_ContextAction` ever gets promoted to a native C++ interface (e.g. for performance reasons, or because more C++ systems need to call it), this can be simplified to a direct `IBPI_ContextAction::Execute_ExecuteAction(...)` call.

---

## 6. No UI feedback on failed actions

**Current state:** If `ExecuteAction` returns `Failed` (e.g. wrong instigator type), nothing currently informs the player — no message, no sound, no menu shake, etc.

**Fix direction:** Once core actions are all in, add a simple feedback path (Print String → replace with actual UI/audio feedback) for failed action attempts.

---

## 7. Health system is minimal

**Current state:** `Health` float + `MaxHealth` + `Heal(float Amount)` on the character. No damage system integration confirmed yet, no UI health bar mentioned.

**Fix direction:** Revisit once combat/damage systems are further along — confirm `Heal` plays nicely with whatever damage/death flow exists, add UI hookup if not already present.

---

## 8. No visual feedback while dragging (drag-and-drop)

**Current state:** Drag-and-drop between inventory slots is fully functional (`OnDragDetected` → `UInventoryDragDropOperation` → `OnDrop` → `RequestMoveItem`), but there's no icon/ghost image following the cursor during the drag — the item visually stays put in its original slot until the drop completes and the grid refreshes.

**Why it's fine for now:** Functional correctness was the priority for this pass — the actual move logic works correctly end-to-end. Purely cosmetic gap.

**Fix direction:** In `OnDragDetected`, before returning the `Operation`, set its `DefaultDragVisual` (a widget, e.g. a simple Image showing the dragged item's icon) and `Pivot` (usually `MouseDown` so it follows the cursor naturally). Can reuse the slot's own icon texture (`SlotData`'s UI fragment icon) to build this visual on the fly.

---

*Last updated: Drag-and-drop feature completed — full move loop confirmed working (OnDragDetected → OnDrop → RequestMoveItem), one bugfix along the way (unwired InventoryComp pin on RequestMoveItem call).*
