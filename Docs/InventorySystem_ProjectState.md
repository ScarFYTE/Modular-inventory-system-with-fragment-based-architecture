# Inventory System: current project state

Last reconciled: **2026-11-28 (Asia/Karachi)**. Conventions and goals: [README](README.md).

## 1. Current milestone and exact next step

**Active: finish manager-component migration.** Tab toggle, inventory-change refresh, and Blueprint-connected manager pickup work according to user reports. Pickup now routes through InventoryRulesSubsystem. Interaction remains empty.

**Resume with MIG-03, then MIG-06:** Single-item pickup and deterministic inventory selection are complete within their tested scopes. User reports all six selection cases passed, including shared selection for pickup/UI. InventoryName is an optional component tag: unset selects only a sole owner inventory; configured requires exactly one matching inventory; missing/ambiguous selection logs a warning and leaves Inventory null. Next: verify final screen initialization graphs and remove confirmed unused legacy UI paths; then run broader migration regression. Interaction/looting remains future work.

Earlier baseline inspected: `4093e16`. At the final repository check, HEAD had advanced to `00cfb5a` (2026-11-25), committing the local toggle implementation and character/screen asset changes. WBP_InventoryScreen.uasset still has additional uncommitted changes. The documentation updates are also uncommitted. Exact final Blueprint wiring is not established merely by changed files. The documenting agent performed no commit, build, or independent playtest.

**Current verification (2026-11-28):** Final diagnostic placeholder correction inspected. User reports all requested selection cases passed: no components; sole untagged configuration; multiple without selection; unique matching tag; missing tag; duplicate tag. Runtime evidence is USER-VERIFIED, not an independent assistant playtest. No gameplay implementation changed by the assistant.

| Area | State | Evidence |
| --- | --- | --- |
| Plugin source/content migration | Files reside in plugin; earlier gameplay tests reported | INSPECTED + HISTORICAL |
| Stable slots, capped stacking, same-inventory swap | Implemented; prior tests reported | INSPECTED + HISTORICAL |
| Manager Tab toggle | User reports working after rewiring | USER-VERIFIED; C++ INSPECTED |
| Screen initialization | Construct disconnected; interface event creates grid | INSPECTED pasted graph |
| Inventory-change UI refresh | Working after RefreshScreen hookup | USER-VERIFIED; final event graph not repasted |
| Manager pickup/interact | Single-item pickup routes through rules subsystem and works by user report. Interaction empty | C++ INSPECTED + pickup USER-VERIFIED overall |
| TransferItemTo | Implemented; historical debug conservation tests | INSPECTED + HISTORICAL |
| Cross-grid drag / RequestTransferItem wrapper | Not implemented in inspected native code | INSPECTED |
| Lootable/container assets | Present; no current container UI test | INSPECTED existence; HISTORICAL contracts |
| Vicinity, ground list, container grid, vicinity config | Future work | PLANNED |
| Styling | C++ configuration exists; integration unfinished | INSPECTED + HISTORICAL |
| Clean-project portability / release readiness | Not demonstrated | UNKNOWN / OPEN |

## 2. Locations

Paths are repository-relative.

| Content | Location |
| --- | --- |
| Native inventory code | Plugins/Modular_Inventory_System/Source/Modular_Inventory_System/ |
| Plugin assets | Plugins/Modular_Inventory_System/Content/InventorySystem/ |
| Screen, grid, slot, menus | Plugin assets: Widgets/ |
| Pickup, loot container | Plugin assets: Actors/ |
| Interfaces and action enum | Plugin assets: SubSystem/ |
| Fragment Blueprints | Plugin assets: Fragments/ |
| Item/config data assets | Plugin assets: ItemDefinitions/ |
| Demo character | Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset |

In Unreal, enable **Show Plugin Content** to locate InventorySystem/Widgets/WBP_InventoryScreen under the plugin.

## 3. Architecture and invariants

| Piece | Responsibility |
| --- | --- |
| UItemDefinition + UItemFragment | Shared item definitions and composed capabilities |
| UInventoryComponent | Slot state and low-level add/remove/move/transfer |
| UInventoryRulesSubsystem | Gameplay request/rule entry points used by UI |
| UInventoryManagerComponent | Host integration, screen creation/toggle, refresh forwarding; pickup/interact intended here |
| WBP_InventoryScreen | Panel layout and UI lifecycle; no data mutation |
| WBP_InventoryGrid | Display an injected inventory with persistent slots |
| WBP_InventorySlot | Render a slot and request actions with inventory identity |
| Host character/controller | Components, interfaces, input forwarding; pickup still awaiting migration |

- SlotCount is inventory data, not UIConfig.
- Clear depleted slots in place; do not shift indices with RemoveAt.
- MoveItem swaps slots.
- Transfers must preserve quantities and notify both inventories when changed.
- Plugin-to-host behavior uses interfaces, without casts to the demo character.
- Ground pickups are lightweight; containers hold inventories.
- UI mutation requests belong in the subsystem. Existing direct character/debug calls are migration work, not proof all actions already follow this rule.
- “Authority” here means architectural ownership; replication/server authority is not implemented or promised.

## 4. Native contracts (INSPECTED)

### Items and inventory

UItemDefinition is a primary data asset with instanced fragments, native/Blueprint lookup, and MaxStackSize (default 1). FInventoryItem stores ItemDef and StackCount (default 1); null ItemDef identifies an empty slot.

InventoryComponent::BeginPlay sizes InventoryItems to SlotCount.

| Method | Behavior and limitations |
| --- | --- |
| AddItem | Merge to stack cap, then fill empty slots. Returns int32 accepted count; invalid input returns 0; broadcasts only on change. Source inspected; user confirmed build fix works. Return-count runtime tests remain unverified. Unfitted remainder is not stored. |
| RemoveItem | Positive count decrements a valid slot; exhausted entry resets in place; broadcasts. |
| GetItemAtIndex | True means valid index, including empty slots. Check ItemDef for occupancy. |
| MoveItem | Reject invalid/same indices; otherwise swap and broadcast. |
| UseItem | Legacy stub; actions use RequestExecuteAction. |
| TransferItemTo | Same component uses MoveItem. Otherwise merge, first-empty placement, targeted single-item swap fallback. True means any amount moved, including partial success. |

ToIndex does not prioritize a specific empty destination slot. Swap checks remaining source count == 1 and occupied target count == 1; it can occur after a partial merge leaves one item. Both delegates fire when anything changes. Grand-total logging remains. Historical tests do not establish every edge case or transactional rollback.

### Rules subsystem

- RequestMoveItem delegates to MoveItem.
- RequestPickupItem validates inventory, definition, and positive count; returns the accepted count from one AddItem call. Manager uses it for single-item pickup.
- RequestDropItem removes one item after attempting spawn, without requiring spawn success. It also does not reject a valid empty slot by checking ItemDef. See SAFE-02.
- RequestExecuteAction dispatches the selected fragment through FindFunction("ExecuteAction") / ProcessEvent; raw result 2 consumes one item.
- Recorded enum order: Success=0, Failed=1, ConsumeItem=2.
- No RequestTransferItem wrapper yet.

### Manager

BeginPlay enumerates the owner's inventory components. If InventoryName is None, only a sole inventory is selected; otherwise exactly one component with that component tag is required. Missing/ambiguous cases log warnings and leave Inventory null. Successful selection binds OnInventoryUpdated, creates the configured screen using the world's first player controller, adds/collapses it, then calls InitializeScreen(Inventory). User verified all six selection cases on 2026-11-28.

ToggleInventory validates screen/controller. Closing uses Collapsed, GameOnly, cursor off. Opening uses Visible, GameAndUI, DoNotLock, and visible cursor during capture. It does not explicitly focus the widget; the old graph did. The user reports the new toggle works.

HandleInventoryUpdated calls interface RefreshScreen. InitializeScreen(UInventoryComponent*) and RefreshScreen() are native BlueprintNativeEvents. SetNearbyItem assigns a transient TWeakObjectPtr<AActor>. RequestPickup validates Inventory and the nearby actor, checks pickup-interface support, dispatches the definition getter, and validates its result. It obtains/validates GameInstance and its InventoryRulesSubsystem, calls RequestPickupItem once, and stores the accepted count; count 1 triggers Item->Destroy() and NearbyItem.Reset(), while count 0 leaves the pickup intact. User reports the subsystem-connected pickup flow works; final Blueprint graphs and individual edge-case test results remain uninspected. RequestInteract remains empty.

No explicit initial refresh is called by the manager; inspect grid construction before diagnosing startup display behavior. FindComponentByClass cannot distinguish gameplay Inventory from TestInventory by role. Controller selection is currently single-player-oriented.

## 5. Blueprint wiring and evidence

### Toggle

Original pasted graph: Tab -> FlipFlop -> visibility/cursor/input mode, targeting InventoryManager.InventoryScreen.
User was instructed to replace it with Tab.Pressed -> InventoryManager.ToggleInventory and reported “yep working fine.”
Removal of disconnected legacy nodes was requested afterward but not separately confirmed.

### Initialization

The supplied WBP_InventoryScreen graph showed:

```text
Event Construct: disconnected
Event InitializeScreen(TargetInventory)
  -> Create WBP_InventoryGrid (Target Inventory = event parameter)
  -> Add Child to PlayerGridArea
  -> Set Player Grid Reference = created grid
```

No Construct-triggered duplicate appears in that graph. Other callers and repeated initialization have not been exhaustively tested.

Subsequent instructions, without a final repasted graph:
- Wire screen Get Owning Player to grid creation Owning Player: requested, unconfirmed.
- Store the incoming parameter in screen.TargetInventory before creating the grid: requested, exact edit uninspected. Passing the parameter to the child does not automatically set the screen member.

### Refresh: names must not be conflated

```text
Inventory.OnInventoryUpdated
 -> Manager.HandleInventoryUpdated
 -> screen interface Event RefreshScreen
 -> screen function RefreshGrid (Self)
 -> grid widget function "Refresh Grid"
      widget receiver = Player Grid Reference
      inventory argument named "Target" = screen.TargetInventory
```

The pasted screen RefreshGrid function confirmed the last call and both inputs. It is separate from the interface RefreshScreen event. The user reported that event was absent. After instructions to implement it and call RefreshGrid, the user confirmed “yes now the updates work beutifully.”

**Refresh behavior is USER-VERIFIED.** Final event wiring and the new TargetInventory assignment have not been independently reinspected. The user reported overall update success rather than separate results for every action.

### Other Blueprint contracts (HISTORICAL unless stated)

- BPI_InventoryOwner: GetInventoryComponent(), SetNearbyItem(Actor Item).
- Actual asset spelling is BPI_Damagable; handoff reports DeltaHealth(Amount), not older Damageable/Heal naming. Signed-delta semantics unverified.
- BPI_ContextAction: GetActionData -> ActionText; ExecuteAction(Instigator) -> EActionResult.
- BPI_Lootable: GetLootInventory; pickup returns null, container returns its component.
- Grid uses injected TargetInventory; slots carry OwningInventory. Current menu and OnDrop inventory sources require inspection before multiple grids.
- Native drag payload has only SourceSlotIndex (INSPECTED); source inventory is missing.
- F-key pickup and NearByItem bookkeeping are now INSPECTED from pasted character graphs (2026-11-26). SetNearbyItem(Item: Actor) casts to BP_ItemPickup: success assigns NearByItem, failure clears it via an unconnected object-value input. Thus passing None clears the reference; it does not leave the previous pickup stored.
- F.Pressed -> IsValid(NearByItem) -> Inventory.AddItem(NearByItem.ItemDef, Count=1) -> DestroyActor(NearByItem). Invalid branch is unconnected. No add-success test exists before destruction. ItemDef is a Blueprint pickup member; native manager cannot access it as a native AActor property.

## 6. Verification boundaries

| Check | Latest result |
| --- | --- |
| Manager toggle | USER-VERIFIED in this chat before 2026-11-25 |
| Refresh after interface hookup | USER-VERIFIED 2026-11-25; aggregate report |
| Pickup/drop/consume/drag/menu full regression | HISTORICAL passes; fresh full run pending |
| Transfer quantity conservation | HISTORICAL debug harness near 7effd91; not cross-grid UI proof |
| Correct component selection, initial populated display, initialization count | Targeted checks pending |
| Clean-project setup and packaged plugin | Not verified |

## 7. Corrections and retained lessons

- Older notes mix obsolete plans with completed work. Historical copies are preserved; do not restore stale state.
- File migration is complete; clean-project portability is not proven.
- Character casts and duplicate DropItem were reported removed historically; retain a release audit instead of assuming original fixes are still pending.
- RequestMoveItem and UIConfig exist; capacity lives on the component.
- Styling has Preset and Custom modes. AdvancedDisplay fields are not a third mode.
- Success=0 already supports success without consumption. The old backlog's claim otherwise was incorrect; richer action results are optional.
- Verify physical source paths; VS filters are not folders. Preserve module API macros and append to Build.cs dependencies.
- Move Blueprint assets using Unreal asset tools; class/module changes may require class and struct redirects and Compile/Save.
- Use a Break struct node when both the whole struct and fields are needed. Check required pin connections and math operand order.
