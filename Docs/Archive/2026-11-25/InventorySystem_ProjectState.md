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

*Last updated: Blueprint cast-decoupling complete — see final section below for current status and next step (Content asset migration).*

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

## Blueprint cast-decoupling — COMPLETE, confirmed working

**Why this was necessary:** discovered while planning the Content migration that `BP_ItemPickup` and `WBP_InventorySlot` hardcoded `Cast To BP_ThirdPersonCharacter_C` in three call sites — this meant the plugin could not function with any other character class, defeating the point of packaging it for sale. Fixed by introducing an interface the plugin calls generically, implemented by whichever character the game actually uses.

**New: `BPI_InventoryOwner`** (Blueprint Interface, lives in the plugin's Content folder) — two functions:
- `GetInventoryComponent() → Inventory Component Object Reference` (no inputs)
- `SetNearbyItem(Actor Item)` (no output)

**`BP_ThirdPersonCharacter`** implements both: `GetInventoryComponent` returns its `Inventory` component; `SetNearbyItem` casts the incoming generic `Actor` to `BP_ItemPickup` and sets `NearByItem` (this cast is fine — it happens inside the GAME's own character, not inside the plugin).

**Three call sites fixed, each required checking what the OLD cast result was actually used for before replacing (uses varied per site):**

1. **`WBP_InventorySlot::Left Click On Inventory Item`** — cast was used for ONE thing (`Get Inventory` → `GetItemAtIndex`). Simple 1:1 swap: `Does Implement Interface(BPI_InventoryOwner)` → `Get Inventory Component` message call.
2. **`WBP_InventorySlot::Right Click On Inventory Item`** — cast was used for TWO different things: (a) `Get Inventory` → `RequestDropItem`'s `InventoryComp`, AND (b) `GetTransform` → `RequestDropItem`'s `SpawnTransform`. **Finding: `GetTransform` never actually needed the cast at all** — it's a plain `Actor` function, callable directly on the generic `Character` reference `GetPlayerCharacter` already returns. Only the Inventory part needed the interface swap; the Transform part had the cast removed entirely with no replacement needed.
3. **`BP_ItemPickup`'s `OnComponentBeginOverlap` / `OnComponentEndOverlap`** — DIFFERENT shape of problem: the cast result wasn't being READ, it was being WRITTEN to (`Set NearByItem` on the character). This is why `SetNearbyItem` was added to the interface as a second function, separate from the getter — a plugin interface needs to support both directions of data flow depending on what each call site actually does, not just "add a getter and assume that covers it." Replaced with `Does Implement Interface` → `Set Nearby Item` message (Item = Self on begin-overlap, Item = None on end-overlap). False branch left unconnected here specifically (any actor can overlap the pickup, most won't implement the interface — not an error case, unlike the other two sites where a Print String warning is more appropriate).

**Lesson for future interface-decoupling work: always check what each individual call site actually does with a cast result before assuming one interface function covers every case — cast usages found in this pass covered THREE different shapes (single read, double read of different things, and a write) that were only revealed by tracing each graph individually rather than assuming they'd match.**

**CONFIRMED: full pickup flow (nearby detection, F-key pickup, right-click drop, left-click context menu) all tested working after every cast was removed. Zero remaining hardcoded references to `BP_ThirdPersonCharacter_C` anywhere in the plugin's Blueprint assets.**

## Second decoupling pass — found via Reference Viewer during Migrate attempt

Running Migrate surfaced additional Mannequin/character-content dependencies the first pass missed — Reference Viewer traced them to FOUR more coupling points that predated `BPI_InventoryOwner`'s creation (built at different points earlier in the session, before the interface existed):

1. **`BP_ItemFragment_Mesh::ExecuteAction`** — cast to `BP_ThirdPersonCharacter` → called `Drop Item` (the character's OWN duplicate drop event, not `RequestDropItem`). **Root cause: this was leftover/dead scaffolding from before the RulesSubsystem existed — a Mesh fragment has no business triggering a drop at all, and `BPI_ContextAction`'s `ExecuteAction` signature has no SlotIndex/Transform to do a real drop even if it should.** Fixed by removing `BPI_ContextAction` from `BP_ItemFragment_Mesh` entirely — it shouldn't generate a context-menu action at all, dropping is exclusively a right-click→`RequestDropItem` action.
2. **`BP_ItemFragment_Consumable::ExecuteAction`** — cast to `BP_ThirdPersonCharacter` → directly set `Health`. Fixed with a NEW interface, **`BPI_Damageable`** (deliberately separate from `BPI_InventoryOwner` — health/damage is not an inventory-ownership concern), one function: `Heal(float Amount)`. `BP_ThirdPersonCharacter` implements it (clamps + sets Health, same logic as before). Fragment now does `Does Implement Interface(BPI_Damageable)` → `Heal` message call.
3. **`WBP_InventoryGrid::Construct`** — cast to get `Inventory → SlotCount` for the slot-spawning ForLoop bound (predates `BPI_InventoryOwner`, built during the config-driven-grid task). Fixed: `Does Implement Interface(BPI_InventoryOwner)` → `Get Inventory Component` message → read `SlotCount`.
4. **`WBP_MenuButton`'s click handler** — cast to get `Inventory` for the `RequestExecuteAction` call (predates `BPI_InventoryOwner`, built during the ExecuteAction-loop session). Fixed: same `Does Implement Interface` → `Get Inventory Component` swap.

**Bonus finding: `BP_ThirdPersonCharacter`'s own `DropItem` custom event was confirmed to be fully dead code** — its ONLY caller was the stray cast inside `BP_ItemFragment_Mesh` (item 1 above). Once that cast was removed, `DropItem` had zero remaining callers and was deleted outright. This incidentally resolves long-standing backlog item #3 ("DropItem duplicates RequestDropItem logic") — not by refactoring it to call the subsystem, but by confirming it was pure redundant scaffolding that was never load-bearing. Right-click drop (→ `RequestDropItem`) continued working correctly throughout, proving it was always the sole authoritative path.

**Lesson reinforced: this cast-coupling pattern got copy-pasted across MULTIPLE sessions/features, each time before `BPI_InventoryOwner` existed yet, so each new feature (grid construction, menu button clicks, fragment execution) independently reinvented the same "cast to get Inventory" shortcut. A full content-wide search for the cast pattern is worth doing before trusting any future Migrate attempt is truly clean — this took TWO Reference-Viewer-driven passes to fully surface, and there could plausibly be more (untested BP_ThirdPersonGameMode, other fragments not yet built).**

**STATUS: All 7 known coupling points (3 from first pass + 4 from second pass) fixed and confirmed working. Two interfaces now exist: `BPI_InventoryOwner` (GetInventoryComponent, SetNearbyItem) and `BPI_Damageable` (Heal). Ready to re-attempt Migrate — RECOMMENDED before doing so: search entire Modular_Inventory_System content folder for any remaining "Cast to BP_ThirdPersonCharacter" nodes as a final sweep, given the pattern of repeated independent occurrences found so far.**

---

**CURRENT STATUS: PLUGIN RESTRUCTURE FULLY COMPLETE AND VERIFIED.** C++ migration, both cast-decoupling passes (7 total coupling points found and fixed), and Content asset migration are all done. Old `/Game/InventorySystem/` folder deleted; `BP_ThirdPersonCharacter`'s references (Create Widget class, etc.) repointed to the plugin's `Content/` copies; resulting compile errors fixed. Full end-to-end playtest confirmed all 7 core features working from the migrated plugin location: pickup/nearby-detection, grid display (correct slot + icon + count, empty slots render correctly), left-click context menu, Consume (Heal + stack decrement), right-click drop (correct spawn location + inventory removal), drag-and-drop swap between slots, and context-menu auto-close after any action.

**The inventory system is now a genuinely self-contained, portable Unreal plugin (`Modular_Inventory_System`) with zero hardcoded dependencies on any specific game or character class** — the only integration surface a buyer needs is implementing two small interfaces (`BPI_InventoryOwner`: GetInventoryComponent/SetNearbyItem, `BPI_Damageable`: Heal) on their own character.

**Next possible directions (not yet started):** resume the UI/UX Overhaul (three-tier style system — Steps 3d onward, wiring `GetResolvedStyle()` into actual slot rendering, was paused mid-way for this restructure), or the previously-deferred Vicinity Looting System, or continued polish-backlog items.

---

## VICINITY LOOTING + UI ARCHITECTURE OVERHAUL — DESIGN LOCKED, NOT YET IMPLEMENTED

This is the next major feature. Design was deliberated at length before any code. Key decisions below are AGREED and should guide implementation.

### The three-layer UI contract (the guiding constraint for everything below)
- **Screen (`WBP_InventoryScreen`, NEW)** = the stage. Owns LAYOUT and LIFECYCLE only. It is the thing added to the viewport (grids are no longer top-level). Holds named areas for panels (player-grid area, loot area, vicinity-list area). Owns open/close of loot sessions, Escape handling, and which layout mode (Rust side-by-side vs PUBG list-overlay) is active. Does NOT know how slots render, how drag works, or how items stack. NEVER reaches into a grid's internals, and NEVER performs data operations itself (cross-grid transfer still goes through the subsystem).
- **Grid (`WBP_InventoryGrid`, REFACTORED)** = a reusable panel. Displays whatever `TargetInventory` it is handed. Does NOT know if it's the player's, does NOT know other grids exist, does NOT know about looting/layout. Currently self-fetches the player's inventory via BPI_InventoryOwner — this MUST change to an injected TargetInventory.
- **Subsystem (`UInventoryRulesSubsystem`)** = the director. All data mutations go here (existing authority model). Cross-grid transfer = a new `RequestTransferItem`, NOT logic living in the Screen.
- Mantra: **Screen arranges, Subsystem decides, Grid displays. Three clean layers, no leakage.**

### Ground item vs. container — DECIDED (do NOT make everything uniform)
- **Ground items** (`BP_ItemPickup`) stay lightweight/one-shot exactly as they are now — grab and destroy, no InventoryComponent. Rationale: a throwaway one-shot pickup shouldn't carry a full persistent InventoryComponent; forcing uniformity there is overhead with no payoff, and there could be hundreds/thousands on the ground.
- **Containers** (chests, corpses, NPC pockets, vending machines, etc.) DO get their own InventoryComponent — they're persistent, browsable, multi-slot.
- **Unifying interface: `BPI_Lootable`** with `GetLootInventory() → InventoryComponent`. Containers return their real component; ground items return NULL. Looting logic branches on null: null → simple direct transfer (existing AddItem + destroy); valid → open the container-grid UI. ONE detection path + ONE interface, TWO deliberately-different fulfillment strategies (not "everything is a container", not "two separate systems").

### FUTURE (design accommodates, not building yet): Tab system
- The Screen will eventually host TABS. One tab = the Inventory (OUR responsibility). Other tabs = user-generated logic/UI (e.g. a buyer's crafting system) — the Screen just hands control to the buyer's widget; the plugin's responsibility ends at the Inventory tab. This is exactly the kind of extension the shell layer makes trivial ("teach the Screen where to put a panel"), and it sharpens the product boundary: we own the Inventory tab, buyers own everything else they add. Design the Screen with this in mind (tab host = a natural extension of "named panel areas") but do NOT build tabs until the looting feature is done.

### Two hard risks identified upfront (these cause the expensive/silent bugs)
1. **`RequestTransferItem` is a decision tree, not one op — and getting it wrong causes ITEM DUPLICATION (the #1 commercial inventory bug).** Five outcomes to handle atomically: (a) dest empty slot → clean move, (b) dest partial stack of same item → merge, possibly with overflow leftover, (c) dest full → reject entirely, (d) dest slot holds a different item → decide swap vs reject vs find-first-empty, (e) source==destination component → degrade to existing MoveItem. Must remove-from-source and add-to-dest ATOMICALLY (partial failure either destroys or dupes an item). Must fire OnInventoryUpdated on BOTH components. **MUST be designed as a decision table on paper (Phase 0) and tested naked against all 5 cases via debug key (Phase 2) BEFORE any UI depends on it.**
2. **Two grids open breaks the "one inventory / one grid / one context menu" assumption.** `UInventoryDragDropOperation` currently carries only `SourceSlotIndex` — this CANNOT express a cross-grid drag (slot 3 of WHICH grid?). Payload MUST gain a `SourceInventory` (component reference). OnDrop then branches: same inventory → MoveItem (swap); different inventory → RequestTransferItem. Also: `GetAllWidgetsOfClass(WBP_ContextMenu)` close-hack and per-slot "fetch player's inventory" both assumed single-grid — the injected-TargetInventory refactor (each slot knows its owning inventory) fixes the ambiguity at the widget level.

### AGREED BUILD ORDER (de-risking: prove risky foundations naked before features depend on them)
- **Phase 0 (paper, no code):** Write the RequestTransferItem 5-outcome decision table. Design the revised drag payload (add SourceInventory; define OnDrop same-vs-different branch).
- **Phase 1:** Introduce `WBP_InventoryScreen` shell (becomes the viewport-level widget); refactor `WBP_InventoryGrid` into a reusable panel taking an injected `TargetInventory`; `WBP_InventorySlot` stores its owning inventory, not "the player's". VERIFICATION GATE: player inventory works exactly as before, zero regressions.
- **Phase 2:** Implement `RequestTransferItem` (C++). Test naked via debug key against all 5 decision-table cases — no dupe, no loss, both delegates fire. NON-NEGOTIABLE GATE before any UI uses it.
- **Phase 3:** `BPI_Lootable` interface; `BP_LootContainer` actor (real InventoryComponent); `BP_ItemPickup` implements it returning null. Data/actor layer only, no UI.
- **Phase 4:** Multi-source vicinity detection (sphere overlap tracking ALL nearby BPI_Lootable actors, replacing single NearByItem) + the loot-SESSION open/close lifecycle (state on player/controller: interacting opens a session vs a source, leaving range / Escape closes it).
- **Phase 5:** Ground-list UI (PUBG-style — thin, lists null-inventory lootables, click/key to grab, reuses existing AddItem+destroy). First consumer of vicinity detection.
- **Phase 6:** Container-grid UI (Rust-style — opens a second generalized grid pointed at container's inventory, cross-grid drag/right-click transfer). Mostly WIRING proven pieces (Phases 1+2+3) by this point.
- **Phase 7:** `UVicinityLootConfig` (data asset) — mode switch (GroundList/ContainerGrid/both), detection radius, layout mode (drives the Screen's arrangement), display settings. Last, once both modes work.

**STATUS: Design locked. Phase 1 COMPLETE. Phase 2 COMPLETE (see below). Next = Phase 3 (BPI_Lootable + container actor).**

### Phase 2 — DONE, all 5 decision-table cases proven bulletproof
- Implemented `UInventoryComponent::TransferItemTo(int32 FromIndex, UInventoryComponent* DestInv, int32 ToIndex = -1) → bool` (source-relative; the subsystem `RequestTransferItem` wrapper will be added in Phase 6 when UI needs it — same authority pattern as RequestMoveItem→MoveItem). Logic exactly follows the locked decision table: Step 0 same-component→MoveItem; Step 1 validate; Step 2 merge into same-item stacks up to MaxStackSize; Step 3 place remainder in first empty slot; Step 4 (only if dest full AND specific occupied ToIndex AND both single items) swap. Anti-dupe invariant: every sub-op computes one `amountMoved` and applies +X to dest / -X to source. Fires OnInventoryUpdated on BOTH components when anything moves. Accesses `DestInv->InventoryItems` (protected member, same-class access OK).
- **Tested NAKED via debug keys** (debug rig: a second `TestInventory` InventoryComponent added to BP_ThirdPersonCharacter with SlotCount 5, keys 0=AddItem-to-test, 1=transfer main→test, F2=transfer test→main, 3=transfer with hardcoded ToIndex=0 for swap; plus a GRAND TOTAL accounting UE_LOG summing all stacks across both inventories). **Confirmed across dozens of back-and-forth transfers: GRAND TOTAL only ever changed when a NEW item was added via key 0 — never on a transfer. No duplication, no loss, in any branch.** Merge respects MaxStackSize. Full→reject works. Swap (Step 4) confirmed working once dest was COMPLETELY full (learned: swap is the fallback for a full destination — if any empty slot exists, Step 3 correctly uses it instead; a partially-full dest with a targeted occupied slot correctly uses the empty slot, NOT swap, per the decision table).
- ⚠️ **CLEANUP TODO before shipping:** the debug `TestInventory` component, the 4 debug keys (0/1/F2/3), and the GRAND TOTAL UE_LOG in TransferItemTo are all TEST SCAFFOLDING — remove them once the real looting UI drives transfers. (The UE_LOG especially — it runs a full double-inventory sum on every transfer, wasteful in shipping.)

### Phase 0 — DONE (design locked, see decision table + payload design above/below)
- Transfer decision table finalized (5 outcomes, merge-before-place order, "find free slot else swap" for case 4, atomic single-`amountMoved` anti-dupe invariant).
- Drag payload design: `UInventoryDragDropOperation` gains `SourceInventory` (UInventoryComponent ref); OnDrop branches same-inventory→RequestMoveItem vs different→RequestTransferItem.
- PREREQUISITE DONE: added `int32 MaxStackSize = 1` to `UItemDefinition` (limit is a property of item TYPE); rewrote `AddItem` to respect the cap (merge into existing stacks up to max, then spill remainder into empty slots). Confirmed stacking now caps + spills correctly. `AddItem` and future `RequestTransferItem` now agree on stacking rules.

### Phase 1 — DONE, confirmed working (grid generalization + Screen shell)
- **1a:** `WBP_InventoryGrid` no longer self-fetches the player's inventory via BPI_InventoryOwner. Added `TargetInventory` (InventoryComponent ref) variable; `Construct` reads it (with IsValid guard) and sources SlotCount + slot loop + Refresh Grid from it. Grid is now a reusable, inventory-agnostic panel.
- **1b:** `WBP_InventorySlot` added `OwningInventory` (InventoryComponent ref), set by the grid's spawn loop = grid's TargetInventory. Slot's Left/Right-click handlers now read `OwningInventory` instead of fetching "the player's" via interface. (This is what makes cross-grid drag possible in Phase 6 — each slot knows which inventory it belongs to, resolving slot-index ambiguity.)
- **1d:** NEW `WBP_InventoryScreen` — the viewport-level shell. Root = Canvas Panel with a NamedSlot `PlayerGridArea`. Has `TargetInventory` (Expose-on-Spawn) + internal `PlayerGrid` ref (NOT exposed). `Construct` → calls `InitializeScreen(TargetInventory)` which Creates the grid (passing TargetInventory), adds it to PlayerGridArea, stores it in PlayerGrid. Also has a `RefreshInventory` function that calls Refresh Grid on its PlayerGrid (so the character routes refresh through the Screen, not directly to the grid — respects the layer contract).
- **BP_ThirdPersonCharacter changes:** `BeginPlay` now Creates `WBP_InventoryScreen` (not the grid directly) → Add to Viewport, passing its Inventory as TargetInventory. `InventoryUIRef` variable RETYPED from WBP_InventoryGrid → WBP_InventoryScreen (required repointing all references: the OnInventoryUpdated binding now calls `InventoryUIRef → RefreshInventory`, and the Tab-key toggle targets the Screen). Confirmed working: Tab opens/closes, all features (pickup/drop/consume/drag/context menu) work exactly as before, zero regressions.
- **Note on layout container choice:** went with Canvas Panel + NamedSlot for now. For the multi-panel layouts in Phase 6/7 (player grid beside loot grid), will likely want a Horizontal/Vertical Box or Grid Panel instead (they auto-arrange side-by-side content); Canvas is fine for the single-grid case today. Revisit at Phase 6.
