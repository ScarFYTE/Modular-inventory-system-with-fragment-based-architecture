# Inventory System — Project State Reference

**Purpose:** Ground-truth record of what actually exists in the project, confirmed from pasted blueprint node dumps / C++ files / screenshots. If Claude says something that contradicts this file, this file wins — paste the relevant section back in to correct course.

**How to keep this current:** After any structural change (new variable, new function, renamed node, new asset), update the relevant section below before moving to the next feature.

---

## C++ Classes

### `UItemFragment` (`ItemFragment.h`)
- Base class, `Abstract, EditInlineNew, DefaultToInstanced, Blueprintable, BlueprintType`
- No properties/functions of its own — pure marker base for Instanced fragment array

### `UItemDefinition` (`ItemDefinition.h`)
- `UPrimaryDataAsset`
- `TArray<UItemFragment*> Fragments` (EditDefaultsOnly, Instanced)
- `FindFragmentByClass<T>()` — C++ template lookup
- `FindFragmentByClass_BP(TSubclassOf<UItemFragment>)` — BlueprintCallable/Pure equivalent

### `FInventoryItem` (struct, in `InventoryComponent.h`)
- `UItemDefinition* ItemDef`
- `int32 StackCount = 1`

### `UInventoryComponent` (`InventoryComponent.h/.cpp`)
- `int32 SlotCount = 20` (EditAnywhere, BlueprintReadOnly) — **NEW.** Data-layer source of truth for inventory capacity. Set per-component in the editor. NOTE: `WBP_InventoryGrid` should read SlotCount from HERE, not from `UInventoryUIConfig` (UIConfig only holds visual concerns now: SlotSize, ColumnsPerRow, SlotPadding, ScreenPosition).
- `TArray<FInventoryItem> InventoryItems` (VisibleAnywhere, BlueprintReadOnly) — **BEHAVIOR CHANGED.** No longer a dynamically-growing dense array. Now FIXED-SIZE at `SlotCount`, initialized in `BeginPlay` via `InventoryItems.SetNum(SlotCount)`. Array index now directly and permanently corresponds to a UI slot position.
- `FOnInventoryUpdated OnInventoryUpdated` — unchanged, multicast delegate, broadcasts on any change
- `AddItem(UItemDefinition*, int32 Count=1)` — **BEHAVIOR CHANGED.** Still stacks onto existing matching entry first. If no match, now finds the FIRST EMPTY slot (ItemDef == nullptr) and fills it in place, instead of appending to array end. If inventory is full (no stack match, no empty slot), silently fails (no UFUNCTION event/feedback for this yet — noted in backlog).
- `RemoveItem(int32 Index, int32 Count=1)` — **BEHAVIOR CHANGED.** Still decrements StackCount. When StackCount <= 0, now RESETS the entry to a default `FInventoryItem()` IN PLACE instead of calling `RemoveAt` (which would have shifted all subsequent slots — this was the bug that had to be fixed for slot-stability).
- `UseItem(int32 Index)` — still a stub, unchanged, superseded by RequestExecuteAction path
- `MoveItem(int32 FromIndex, int32 ToIndex)` — **NEW.** Returns bool (false if either index invalid or FromIndex == ToIndex). Uses `InventoryItems.Swap()` — a true swap, no shifting. This is what makes drag-and-drop possible. Broadcasts OnInventoryUpdated on success.
- `GetItemAtIndex(int32 Index, FInventoryItem& OutItem) const` — **RETURN VALUE MEANING CHANGED — IMPORTANT.** Previously: false meant "no item at this index" (used for both out-of-range AND empty). NOW: since InventoryItems is fixed-size and always has SlotCount entries after BeginPlay, this will return TRUE for any valid slot index REGARDLESS of whether that slot has an item — including empty ones (OutItem.ItemDef will be nullptr for empty slots, but ReturnValue itself is true). False now ONLY means genuinely out-of-bounds (index >= SlotCount or negative), which should rarely/never happen with correctly-driven UI.
  - ⚠️ **DOWNSTREAM IMPACT — NOT YET FIXED:** `WBP_InventorySlot::SetSlotData`'s `bHasItem` parameter and `WBP_InventoryGrid::Refresh Grid`'s wiring currently pass the bool ReturnValue from GetItemAtIndex as `bHasItem`. This will now ALWAYS be true post-fix, breaking the empty-slot visual state. Must change to check `OutItem.ItemDef != nullptr` instead of relying on the bool return. This is the next task (Step 3, not yet done).

### `UInventoryRulesSubsystem` (`InventoryRulesSubsystem.h/.cpp`)
- `UGameInstanceSubsystem` — accessed via "Get Game Instance Subsystem" node, class = InventoryRulesSubsystem
- `RequestDropItem(UInventoryComponent*, int32 SlotIndex, FTransform SpawnTransform, TSubclassOf<AActor> PickupActorClass, AActor*& OutSpawnedPickup, UItemDefinition*& OutItemDef)` — authoritative drop logic, spawns pickup actor, calls RemoveItem
- `RequestExecuteAction(UInventoryComponent*, int32 SlotIndex, UItemFragment* TargetFragment, AActor* Instigator)` — CURRENT SIGNATURE (updated from original single-fragment-loop version to take a specific TargetFragment, to fix the "any button consumes" bug). Uses reflection (`FindFunction` + `ProcessEvent`) to call `ExecuteAction` on the fragment since `BPI_ContextAction` is Blueprint-only, no native header. Checks result == ConsumeItem (index 2) → calls RemoveItem.
- `RequestMoveItem(UInventoryComponent*, int32 FromIndex, int32 ToIndex) → bool` — thin wrapper around `InventoryComponent::MoveItem`, powers drag-and-drop. CONFIRMED WORKING.

### `UInventoryDragDropOperation` (`InventoryDragDropOperation.h/.cpp`)
- `UCLASS(Blueprintable, BlueprintType)` — **both specifiers required**; `BlueprintType` alone is NOT enough to appear in "Construct Object from Class" pickers.
- `UPROPERTY(BlueprintReadWrite) int32 SourceSlotIndex = -1` — carries which slot a drag originated from, read in `WBP_InventorySlot::OnDrop`.

⚠️ **Not yet added:** `RequestMoveItem` (planned for drag-and-drop, not yet implemented)
⚠️ **Not yet added:** `UInventoryUIConfig` data asset class (planned, not yet implemented)

---

## Blueprint Interfaces

### `BPI_ContextAction`
- **`GetActionData()` → `Text ActionText`** — no inputs, pure display label lookup
- **`ExecuteAction(Actor Instigator)` → `EActionResult Result`** — added later, performs the actual gameplay effect

## Blueprint Enums

### `EActionResult`
- Confirmed order: `Success (0), Failed (1), ConsumeItem (2)` — C++ reflection code checks `Params.Result == 2` for ConsumeItem. **If this enum's order ever changes, the C++ check must be updated to match.**

---

## Blueprint Classes (Fragments)

### `BP_ItemFragment_Mesh`
- Has `Item Mesh` (StaticMesh) variable
- Used by `BP_ItemPickup`'s `UpdateVisuals` to set the pickup actor's mesh

### `BP_ItemFragment_UI`
- Has `Icon` (Texture2D) variable
- Used by `WBP_InventorySlot` to set the slot's icon image

### `BP_ItemFragment_Consumable`
- Parent: `UItemFragment`
- Implements `BPI_ContextAction`
- Has `HealAmount` (float, EditAnywhere) — reused across different consumable items with different data
- `GetActionData` → returns "Consume" (or similar) text
- `ExecuteAction` → casts Instigator to character, calls `Heal(HealAmount)`, sets Result = ConsumeItem (on success) or Failed (on cast failure)

---

## Blueprint Classes (Actors)

### `BP_ItemPickup`
- Has `InteractZone` component (sphere/primitive), `ItemDef` variable
- `OnComponentBeginOverlap` / `OnComponentEndOverlap` on InteractZone → casts directly to `BP_ThirdPersonCharacter_C` ⚠️ (flagged in backlog — blocks generic interaction system), sets/clears `NearByItem` on the character
- `UpdateVisuals` (custom function) → gets Mesh fragment via `FindFragmentByClass_BP`, sets StaticMeshComponent's mesh
- `RefreshPickupVisuals` (custom event) → calls `UpdateVisuals`

### `BP_ThirdPersonCharacter`
- Has `Inventory` (InventoryComponent), `NearByItem` (BP_ItemPickup ref), `InventoryUIRef` (WBP_InventoryGrid ref)
- Has `Health` (float, default 50), `MaxHealth` (float, added later)
- Has `Heal(float Amount)` function — clamps Health to [0, MaxHealth]
- `DropItem` custom event ⚠️ (flagged in backlog — duplicates RequestDropItem logic instead of calling it, should be refactored)
- Tab key → toggles InventoryUIRef visibility + mouse cursor + input mode (GameOnly / GameAndUIEx)
- F key → IsValid(NearByItem) → Inventory->AddItem(NearByItem->ItemDef) → K2_DestroyActor(NearByItem) — pickup logic
- `BeginPlay` → creates WBP_InventoryGrid, adds to viewport, binds `OnInventoryUpdated` delegate to a custom event that calls `Refresh Grid` on the InventoryUIRef

---

## Blueprint Classes (Widgets)

### `WBP_InventorySlot`
- Has `SlotData` (FInventoryItem), `ItemIndex` (int32)
- **NEW:** `bHasItem` (bool) — set by `SetSlotData`, tracks whether this slot currently holds an item or is empty. Added specifically to support drag-and-drop (lets OnDragDetected cheaply check "should a drag even start from this slot" without re-querying the inventory component).
- `SetSlotData(FInventoryItem SlotData, bool bHasItem)` function — Branch on `bHasItem`: TRUE branch sets icon (via UI fragment lookup) + count text as before; FALSE branch (FIXED — was previously unwired/no-op, causing dropped/consumed last-item icons to persist) directly calls `SetBrushFromTexture` with empty Texture + `SetText` with empty Text, no fragment lookup needed. Also sets the new `bHasItem` variable to match.
- `Construct` → calls `SetSlotData(SlotData, false)` for safe initial empty state
- `OnMouseButtonDown` → RESTRUCTURED for drag-and-drop support:
  - **Right click** → `Right Click On Inventory Item` custom function → RequestDropItem (unchanged)
  - **Left click** → CHANGED from directly opening context menu to calling `Detect Drag If Pressed` (Drag Key = Left Mouse Button, Pointer Event = Mouse Event), returning its Event Reply. This arms Slate to watch for a drag.
- **NEW `OnMouseButtonUp`** → checks `Get Effecting Button(MouseEvent) == Left Mouse Button` (NOT `PointerEvent_IsMouseButtonDown`, which incorrectly returns false on an up-event) → if true, calls `Left Click On Inventory Item` (the context-menu-opening function, moved here from OnMouseButtonDown) → both branches return Handled via WidgetBlueprintLibrary::Handled. CONFIRMED WORKING (context menu still opens on a genuine click, right-click drop unaffected).
- - **`OnDragDetected`** override — CONFIRMED WORKING. `Has Item` → Branch → (False: no-op, no drag starts) / (True: `Construct Object (InventoryDragDropOperation)` → `Set SourceSlotIndex = ItemIndex` → feed into `Operation` output). Correctly refuses to start a drag from an empty slot.
- **`OnDrop`** override — CONFIRMED WORKING (after one bugfix, see below). Cast incoming `Operation` to `InventoryDragDropOperation` → (Cast Failed: return false) → (Succeeded: check `SourceSlotIndex == ItemIndex` self-drop guard → if same, return false / no-op → if different, Get Subsystem → Get Player Character → Cast to BP_ThirdPersonCharacter_C → Get `Inventory` → `RequestMoveItem(InventoryComp, FromIndex, ToIndex)` → return its bool result).

## Drag-and-Drop feature — COMPLETE, confirmed working end-to-end

**New C++ additions:**
- `UInventoryDragDropOperation : UDragDropOperation` (`InventoryDragDropOperation.h/.cpp`) — `UCLASS(Blueprintable, BlueprintType)`, single property `int32 SourceSlotIndex`. **`Blueprintable` is REQUIRED, not just `BlueprintType`** — without it, the class won't appear in "Construct Object from Class" node pickers even though it compiles and is otherwise valid.
- `UInventoryRulesSubsystem::RequestMoveItem(UInventoryComponent* InventoryComp, int32 FromIndex, int32 ToIndex) → bool` — thin wrapper calling `InventoryComponent::MoveItem`, same authority pattern as RequestDropItem/RequestExecuteAction.
- `UInventoryComponent::MoveItem(int32 FromIndex, int32 ToIndex) → bool` — uses `InventoryItems.Swap()`, works cleanly because InventoryItems is now a fixed-size, slot-stable array (see earlier restructure).

**Bugs found and fixed during D&D implementation:**

1. **[MAJOR TIME SINK — tooling, not code]** Visual Studio's "Add New Item" saved `InventoryDragDropOperation.h/.cpp` to the wrong physical folder (project root, not Source/LearningC), because VS's Solution Explorer folder view is a virtual filter, not real disk layout for UE projects. Files were fully valid and edited fine in VS, but completely invisible to UnrealBuildTool's file discovery — meaning the class compiled in isolation (when directly referenced) but never got a real UHT pass, never generated `.gen.cpp`, and never appeared in ANY Blueprint class picker, despite every build reporting "Succeeded." Diagnosed by checking for the missing `.generated.h`/`.gen.cpp` output and confirming the file was absent from Source/LearningC via File Explorer. **Fixed by recreating both files directly via File Explorer (New → Text Document → rename with correct extension) instead of VS's Add New Item, then running "Generate Visual Studio project files" from the .uproject.**
2. **`RequestMoveItem`'s `InventoryComp` parameter was never wired in `OnDrop`** — the graph never fetched a player/Inventory reference at all (no GetPlayerCharacter→Cast→Inventory chain existed anywhere in the graph), so the pin silently defaulted to `None`. `RequestMoveItem`'s null check caught this and returned `false` with zero visible symptoms — same silent-failure shape as the Config and Subtract bugs from the grid-restructure task. Fixed by adding the standard GetPlayerCharacter → Cast to BP_ThirdPersonCharacter_C → Get Inventory chain, wired into the InventoryComp pin.
3. (Minor, non-breaking, not fixed but harmless) `FromIndex`/`ToIndex` are wired swapped relative to their names (FromIndex=ItemIndex, ToIndex=SourceSlotIndex) — functionally fine since `MoveItem` uses a symmetric `Swap()`, but worth correcting for readability if touched again.

**Scope deliberately deferred for this pass:** no drag visual (icon following cursor during drag) — functional correctness was prioritized first, per established pattern this session.

### `WBP_ContextMenu`
- Has `ActionListContainer` (VerticalBox)
- `Populate Menu(ItemDef, SlotIndex)` custom event → loops `ItemDef->Fragments`, for each: checks `DoesImplementInterface(BPI_ContextAction)`, if true calls `GetActionData` → creates `WBP_MenuButton`, sets its `Action Name` + `ItemIndex`, adds to ActionListContainer
- ⚠️ **Not yet done:** setting `SourceFragment` on each button (needed so RequestExecuteAction knows which specific fragment to run — this was identified as necessary but may not be implemented in the BP yet, confirm before assuming it's done)

### `WBP_MenuButton`
- Has `Action Name` (Text), `ItemIndex` (int32), `ActionTextDisplay` (TextBlock)
- ⚠️ **Confirm added:** `SourceFragment` (UItemFragment ref) — was recommended to fix "any action button consumes" bug
- `Construct` → SetText(ActionTextDisplay, Action Name)
- Click handler (exact implementation not fully pasted, but confirmed working) → calls RequestExecuteAction with correct SourceFragment, then closes context menu via `GetAllWidgetsOfClass(WBP_ContextMenu)` → RemoveFromParent on all (accepted shortcut, see Polish Backlog)

### `WBP_InventoryGrid`
- Has `ItemGrid` (WrapBox), `Config` (InventoryUIConfig ref, Instance Editable — now only used for SlotSize/ColumnsPerRow/SlotPadding/ScreenPosition, NOT SlotCount), `SlotWidgets` (Array of WBP_InventorySlot ref, named "Slot Widget" in some older node dumps — confirm exact variable name if issues arise)
- `Construct` → Gets PlayerCharacter, casts to BP_ThirdPersonCharacter_C, gets its `Inventory` component, reads `Inventory->SlotCount` → ForLoop 0 to (SlotCount - 1) → CreateWidget(WBP_InventorySlot) → Set ItemIndex → AddChild to ItemGrid → Array Add to SlotWidgets. Builds persistent slot pool ONCE. CONFIRMED WORKING after fixing the Subtract node bug (see Current Task section for full bug list).
- `Refresh Grid(InventoryComponent Target)` custom event → ForEachLoop over `SlotWidgets` → per slot widget, `GetItemAtIndex(Target, Array Index)` → `Break` OutItem (NOT split pin) → `IsValid(ItemDef)` → `SetSlotData(slot widget, OutItem [whole struct], IsValid result)`. CONFIRMED WORKING.

---

## CURRENT TASK IN PROGRESS: Config-driven, persistent-slot inventory grid

**Goal:** Support plugin-style configurable SlotCount/SlotSize/ScreenPosition, and make slots persistent objects (not recreated every refresh) so drag-and-drop can safely reference them.

**Planned architecture:**
1. New C++ `UInventoryUIConfig : UDataAsset` — SlotCount, ColumnsPerRow, SlotSize, SlotPadding, ScreenPosition. ✅ DONE — compiled successfully.
2. `DA_InventoryUIConfig_Default` data asset instance created in Content Browser. ✅ DONE.
3. `WBP_InventoryGrid` gets new variable `Config` (InventoryUIConfig ref, Instance Editable, default = DA_InventoryUIConfig_Default). ✅ DONE — required an explicit default value assignment in the Details panel; a blank/unset default caused an "Accessed None" runtime error on Construct. Confirmed fixed.
4. `WBP_InventoryGrid` gets new variable `SlotWidgets` (Array of WBP_InventorySlot ref) — persistent slot pool. ✅ DONE.
5. `Construct` (new) → ForLoop 0 to (Config->SlotCount - 1) → CreateWidget(WBP_InventorySlot) ONCE each → Set ItemIndex = loop Index → AddChild to ItemGrid (WrapBox) → Add to SlotWidgets array. ✅ DONE — confirmed working, correct number of slot widgets now spawn into the WrapBox on Construct.
   - ⚠️ Recommended but NOT YET ADDED: an `IsValid(Config)` branch guard at the start of Construct, to fail gracefully with a Print String instead of "Accessed None" if a user forgets to assign a Config asset. Worth adding since Config is meant to be plugin-user-facing.
6. `Refresh Grid` → STILL NEEDS CHANGING from "recreate all slots from item list" (current/old behavior) to "loop SlotWidgets, call GetItemAtIndex per slot, call new SetSlotData() on existing widget to update in place." NOT YET DONE — this is the next step (Step 4).
7. `WBP_InventorySlot` needs new function `SetSlotData(FInventoryItem SlotData, bool bHasItem)` to replace/supplement what Construct currently does, since the same widget instance will be updated repeatedly now instead of built once. NOT YET DONE — this is Step 4/5.

**Status: COMPLETE. Config-driven, persistent-slot inventory grid fully implemented and confirmed working in-game — required fixing THREE separate bugs during implementation (see "Bugs found and fixed" below).**

**Final implementation summary:**
1. `UInventoryUIConfig : UDataAsset` (C++) — SlotCount, ColumnsPerRow, SlotSize, SlotPadding, ScreenPosition. ✅
2. `DA_InventoryUIConfig_Default` data asset instance. ✅
3. `WBP_InventoryGrid` variables: `Config` (InventoryUIConfig ref, Instance Editable, default assigned), `SlotWidgets` (Array of WBP_InventorySlot ref). ✅
4. `WBP_InventoryGrid → Construct`: ForLoop 0 to (Config->SlotCount - 1) → CreateWidget(WBP_InventorySlot) → Set ItemIndex = loop Index → AddChild to ItemGrid (WrapBox) → Array Add to SlotWidgets. Runs ONCE, builds persistent pool. ✅
   - ⚠️ Still recommended, not yet added: `IsValid(Config)` branch guard for graceful failure if Config is unassigned (currently throws "Accessed None" — confirmed reproducible, was hit once during dev and fixed by assigning the default).
5. `WBP_InventorySlot → SetSlotData(FInventoryItem SlotData, bool bHasItem)` — new function, replaces the data-setting logic that used to live only in Construct. True branch: sets icon (via UI fragment) + count text as before. False branch: hides/clears icon + count text (empty slot visual state). `Construct` now just calls `SetSlotData(SlotData, false)` for a safe initial empty state before first real refresh. ✅
6. `WBP_InventoryGrid → Refresh Grid`: REWRITTEN. Old logic (ClearChildren + ForEachLoop over InventoryItems + CreateWidget per item) fully removed. New logic: ForEachLoop over `SlotWidgets` → for each (Array Element = slot widget, Array Index = position) → `GetItemAtIndex(Target, Array Index)` → `SetSlotData(slot widget, OutItem, ReturnValue)`. No widgets destroyed/recreated on refresh — same persistent instances updated in place. ✅ CONFIRMED WORKING — items appear in correct slots with correct icon/count, empty slots render empty, no errors.

**This foundation is now ready for drag-and-drop implementation (original recommended next step), since slots are persistent objects that can be safely referenced across frames/drag operations.**

**Bugs found and fixed during this task (all confirmed resolved):**

1. **`Config` unassigned on `WBP_InventoryGrid`** — "Accessed None trying to read property Config" runtime error. Root cause: variable's default value was never actually set to `DA_InventoryUIConfig_Default` in the Details panel. Fixed by explicitly assigning it. (This bug predates the SlotCount move to InventoryComponent — was hit while Config still owned SlotCount.)

2. **`Refresh Grid`'s `OutItem` struct pin was split, breaking `Slot Data` input to `SetSlotData`.** After splitting `GetItemAtIndex`'s `OutItem` output into `OutItem_ItemDef`/`OutItem_StackCount` (to feed the `IsValid` check), the parent `OutItem` pin became unusable as a whole struct, so `SetSlotData`'s `Slot Data` parameter was left completely unconnected (silently defaulting to an empty struct every call). Fixed by NOT splitting `OutItem` — instead using a separate `Break InventoryItem` node fed from the same `OutItem` pin for the `ItemDef`/`IsValid` check, while `OutItem` itself feeds `SetSlotData` directly. **Lesson: splitting a struct output pin consumes/replaces the parent pin — if you need both the whole struct AND one of its fields elsewhere, use a separate Break node instead of splitting.**

3. **`WBP_InventoryGrid → Construct`'s ForLoop `LastIndex` calculation was wrong — THE MAIN BUG for "no slots at all."** The `Subtract_IntInt` node meant to compute `SlotCount - 1` had its pins effectively backwards/incomplete: pin A was unconnected (silently defaulted to 0), pin B was connected to the `SlotCount` getter. This computed `0 - SlotCount` (e.g. -20) instead of `SlotCount - 1` (e.g. 19). Result: `ForLoop(FirstIndex=0, LastIndex=-20)` ran ZERO iterations (First > Last), silently — no error thrown, no widgets created, no diagnostic fired even when CastFailed and other branches were checked. Fixed by reconnecting: A = SlotCount getter, B = literal 1. **Lesson: a promotable math operator node with an unconnected input pin does NOT error — it silently uses a default value (0 for int), which can produce logically-wrong-but-technically-valid results that fail completely silently.**

**Confirmed working end-to-end after all three fixes: SlotCount-driven persistent slot grid, correct item display, correct empty-slot rendering, AddItem placing items in first empty slot, RemoveItem clearing in place.**

---

## Known Open Backlog Items (see separate InventorySystem_PolishBacklog.md for full detail)

0. **[TOOLING GOTCHA, not a code bug]** Visual Studio's "Add New Item" on this UE project silently saved new files to the wrong physical folder (project root instead of Source/LearningC), because VS Solution Explorer's folder view is a virtual filter, not real disk structure. Files opened/edited fine in VS but were invisible to UnrealBuildTool's file discovery, causing hours of "compiles successfully but class never appears anywhere" confusion for `InventoryDragDropOperation`. **Fix/avoidance going forward: create new C++ files directly in File Explorer inside `Source/LearningC/` (New → Text Document → rename with correct extension, paste content), then run "Generate Visual Studio project files" from the .uproject — never use VS's Add New Item dialog for this project without manually verifying/setting the target path first.**
1. Context menu closes via GetAllWidgetsOfClass (accepted shortcut, fine for single-menu use)
2. Direct casts to BP_ThirdPersonCharacter_C in BP_ItemPickup + WBP_InventorySlot (blocks generic interaction system)
3. BP_ThirdPersonCharacter's DropItem event duplicates RequestDropItem logic
4. EActionResult conflates success/consume (fine until a non-consuming action type is needed)
5. RequestExecuteAction's reflection-based dispatch (fine, technical debt only if BPI_ContextAction goes native)
6. No UI feedback on failed actions
7. Health system is minimal (Health/MaxHealth/Heal only, no damage system integration confirmed)

---

*Last updated: Plugin restructure (Modular_Inventory_System) — C++ migration complete and verified.*

---

## Plugin Restructure — Modular_Inventory_System

**Goal:** Extract inventory-specific C++/Blueprint code out of the `LearningC` game project into a real Unreal Plugin (`Plugins/Modular_Inventory_System/`), so the system is genuinely sellable/portable, separate from game-specific integration code. **What stays in the game project (NOT migrated):** `BP_ThirdPersonCharacter`'s specific integration (F-key pickup, Tab-key UI toggle, owning an Inventory component instance, the `DropItem` event) — demo/example code showing how a game uses the plugin, not part of the plugin itself.

**C++ half: COMPLETE, confirmed working.**

1. Created plugin via editor wizard (Edit → Plugins → Add → Blank template, name `Modular_Inventory_System`). Confirmed compiled clean as an empty skeleton before moving any code.
2. Moved these files from `Source/LearningC/` into `Plugins/Modular_Inventory_System/Source/Modular_Inventory_System/`: `InventoryComponent.h/.cpp`, `InventoryRulesSubsystem.h/.cpp`, `ItemFragment.h/.cpp`, `ItemDefinition.h/.cpp`, `InventoryUIConfig.h/.cpp`, `InventoryUIStyle.h/.cpp`, `InventoryDragDropOperation.h/.cpp`.
3. **`LearningC.Build.cs`**: added `"Modular_Inventory_System"` to `PublicDependencyModuleNames`. ⚠️ Bugfix required: the edit initially overwrote rather than appended to the array, silently dropping `"EnhancedInput"`, `"AIModule"`, `"StateTreeModule"`, `"GameplayStateTreeModule"` — broke the unrelated template character classes (SideScrollingCharacter, CombatCharacter). Fixed by restoring the full original list alongside the new plugin dependency. **Lesson: always ADD to existing dependency arrays, never retype/replace them wholesale.**
4. **`Modular_Inventory_System.Build.cs`** needs `Core, CoreUObject, Engine, UMG, Slate, SlateCore` (SlateCore specifically required for `FSlateBrush`/`FSlateFontInfo`).
5. **Every moved header used `LEARNINGC_API`**, which doesn't exist outside its original module — caused a cascade of "uses undefined class" errors across all 6 moved headers (one root cause, dozens of symptom lines). Fixed by find-replacing `LEARNINGC_API` → `MODULAR_INVENTORY_SYSTEM_API` in all six headers. **Lesson: every `_API` macro is module-specific; moving a class to a new module always requires this rename.**
6. **Visual Studio Solution Explorer didn't initially show the moved files** — two separate causes: (a) needed "Generate Visual Studio project files" after the physical move, and (b) was initially looking at the wrong project node (`LearningCModuleRules`, an auto-generated C# helper that only ever contains `.Build.cs`/`.Target.cs` files for IntelliSense) instead of the real `LearningC` project (found under Games → LearningC → Plugins → Modular_Inventory_System → Source, simply collapsed). **Lesson: UBT compiles directly from disk regardless of what VS's Solution Explorer shows — a missing-from-VS-view file is a VS bookkeeping issue, not a build issue.**
7. **After the C++ move compiled clean, existing Blueprint assets failed to load/compile** ("Failed to load Outer for resource", "Break <unknown struct>") — Blueprints store native parent class/struct references by a full path including the module name (e.g. `/Script/LearningC.ItemFragment`), which goes stale when a class changes modules. **Fixed via `CoreRedirects` in `Config/DefaultEngine.ini`** (game project's config) — needed both `ClassRedirects` (ItemFragment, ItemDefinition, InventoryComponent, InventoryRulesSubsystem, InventoryUIConfig, InventoryUIStyle, InventoryDragDropOperation) and a separate `StructRedirects` entry (for the USTRUCT `FInventoryItem`, registered name `InventoryItem`). Redirects only take effect on a full editor restart, not hot-reload. After that, every affected Blueprint needed an individual Compile+Save pass to bake the corrected reference in permanently. **CONFIRMED: all Blueprints compile clean.**

**Status: C++ code fully migrated and verified working. NEXT: move the actual Blueprint/data assets (BPI_ContextAction, WBP_InventoryGrid, WBP_InventorySlot, WBP_ContextMenu, WBP_MenuButton, BP_ItemFragment_Mesh/UI/Consumable, DA_HealthPotion, DA_InventoryUIConfig_Default, BP_ItemPickup) from `/Game/InventorySystem/...` into the plugin's own Content folder — this needs the editor's own Migrate/Move tool, NEVER a raw filesystem move (`.uasset` files contain internal reference IDs a filesystem move would corrupt).**
