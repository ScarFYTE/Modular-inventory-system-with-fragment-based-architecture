# Inventory System: checklist and backlog

Last reconciled: **2026-11-28 (Asia/Karachi)**.
Goals and evidence rules: [README](README.md). Implementation: [project state](InventorySystem_ProjectState.md).

This file owns task status. Priorities/order below are working recommendations unless identified as historical roadmap. An unchecked task may have partial implementation.

## A. Current milestone: manager migration

- [x] **MIG-01 — DONE:** Route Tab to manager ToggleInventory. USER-VERIFIED: “yep working fine.” Scope: runtime toggle behavior, not legacy-node cleanup.
- [x] **MIG-02 — DONE:** Restore inventory-change refresh through RefreshScreen -> screen RefreshGrid -> grid Refresh Grid. USER-VERIFIED 2026-11-25: updates work. Final wiring not repasted.
- [ ] **MIG-03 — OPEN:** Finish screen migration verification. Confirm Set TargetInventory before grid creation, owning-player injection, removal of disconnected legacy toggle nodes and unused InventoryUIRef if truly unused. Verify one grid on initialization and correct initial populated display. Completion: inspect final graphs and targeted runtime checks, recording individual outcomes.
- [x] **MIG-04 — DONE (single-item scope):** Native manager/interface/subsystem flow inspected; header cleanup corrected. User confirmed successful pickup, full-inventory rejection, repeated F without duplication, and leaving-range protection on 2026-11-27. Final Blueprint wiring is user-verified, not independently inspected. Multi-source vicinity and multi-count world pickups are outside this scope.
- [x] **MIG-05 — DONE:** Final implementation/diagnostics inspected. USER-VERIFIED 2026-11-28: all six requested cases passed (zero inventories; one without selection; multiple without selection; unique tag; missing tag; duplicate tag), including selected inventory shared by pickup/UI. Scope: BeginPlay selection; runtime inventory switching not implemented.
- [ ] **MIG-06 — OPEN:** Run full post-migration regression checklist below. Toggle and aggregate refresh reports do not close every case.

## B. Data correctness and integration

MIG-04 final review (2026-11-27): header cleanup inspected and targeted single-item pickup checks confirmed by user. Broader inventory regression and clean-project portability remain separate open tasks.

| ID / status | Finding and evidence | Completion check |
| --- | --- | --- |
| SAFE-01 / single-item pickup VERIFIED; batch regression OPEN | Accepted-count return and change-only broadcast inspected. User confirms successful/full-inventory/repeated-input/leaving-range checks. Manager removes actor only on acceptance. | General AddItem rejected/partial/full batch-count regression remains open. Current world pickup is one item; multi-count remainder handling is not implemented. |
| SAFE-02 / OPEN, before release | Drop removes an item after spawn attempt regardless of success; valid empty slots also pass the index check (INSPECTED). | Reject empty/invalid source; remove only after successful pickup creation/initialization; test failed spawn and success without loss/duplication. |
| SAFE-03 / OPEN, before container UI | Menu and OnDrop inventory ownership ambiguous in handoffs; exact current wiring uninspected. | Trace slot -> menu -> button -> subsystem with correct inventory, slot index, fragment, and instigator; test two inventories with different items at equal slot indices. |
| SAFE-04 / OPEN, before release | Grid Config validation recommended historically, not freshly inspected. | Missing Config fails clearly without Accessed None; assigned Config builds intended layout. |
| SAFE-05 / OPEN, before release | No current full dependency/clean-project audit. | Install in clean UE 5.7 project, configure documented integration, build/run core flows without demo-character or legacy project redirect dependency. |

## C. Historical looting roadmap and present progress

The former Phase 0-7 plan is preserved here with corrected implementation status.

- [x] **LOOT-00 — DONE (HISTORICAL):** Transfer decision table and source-inventory drag design recorded. Existing behavior is merge-first, first-empty next, targeted single-item swap fallback. Revisit targeted-slot UX explicitly before cross-grid drag.
- [x] **LOOT-01 — DONE (HISTORICAL; current screen repairs in MIG-02):** Screen shell, injected grid TargetInventory, slot OwningInventory. Earlier regression success reported.
- [x] **LOOT-02 — DONE (primitive only):** TransferItemTo implemented; conservation tests reported near 7effd91. This does not include the subsystem wrapper or cross-grid UI.
- [ ] **LOOT-03 — OPEN (assets present):** Verify BPI_Lootable, pickup null inventory, and container inventory at runtime. Completion: both contracts demonstrated; test seed data identified.
- [ ] **LOOT-04 — PLANNED/OPEN:** Multi-source vicinity detection and loot-session lifecycle through RequestInteract. Completion: all nearby lootables tracked, valid container opens session, leaving range/Escape closes it; ownership of session state documented. Historical notes split screen lifecycle and player/controller session state; settle the boundary before coding.
- [ ] **LOOT-05 — PLANNED/OPEN:** Ground-loot list. Depends on LOOT-04 and SAFE-01. Completion: nearby lightweight pickups listed, acquired safely, and removed from list when gone.
- [ ] **LOOT-06 — PLANNED/OPEN:** Container panel and cross-inventory actions. Add subsystem RequestTransferItem and SourceInventory drag payload; route same/different inventories appropriately. Depends on SAFE-03 and transfer decision DEC-02. Completion: two-grid transfer tests conserve quantities, refresh both panels, and cover partial/full/swap/self cases.
- [ ] **LOOT-07 — PLANNED/OPEN:** UVicinityLootConfig after both UI modes work. Completion: mode, radius, and layout settings produce documented behavior.

## D. Styling, polish, and cleanup

| ID / status | Work | Completion check |
| --- | --- | --- |
| UI-01 / OPEN | Finish style integration; C++ Preset/Custom exists, handoffs report no style assets or widget callers | Presets and custom assets visibly style filled/empty/hovered slots and text; missing-style behavior defined |
| UI-02 / OPEN | Drag visual, historically deferred | Correct item icon follows pointer and disappears on completion/cancel |
| UI-03 / OPEN when menu behavior expands | Replace or explicitly retain global GetAllWidgetsOfClass menu cleanup based on intended one-menu policy | Closing an action menu respects intended ownership with multiple panels; two grids alone do not prove two simultaneous menus are required |
| UI-04 / OPEN | Failed-action feedback | Failed pickup/action/transfer has clear feedback without false success |
| CLEAN-01 / OPEN after real transfer UI | Remove TestInventory, debug input harness, total-count logging; review container seed data | Production flow does not depend on scaffolding; deliberate demo content documented |
| DEBT-01 / DEFERRED | Reflection dispatch and raw result enum | Preserve ABI/order now; if replaced, verify consuming, non-consuming, and failed actions |
| DEBT-02 / DEFERRED | Richer action result | Only add when a concrete need exceeds existing Success/Failed/ConsumeItem semantics |
| EXT-01 / DEFERRED until looting complete | Screen host for buyer-supplied tabs | Product boundary defined; buyers own non-inventory content |

## E. Decisions not yet settled

| ID | Question | Needed by |
| --- | --- | --- |
| DEC-01 | Multiplayer, split-screen, persistence, controller input, equipment/crafting scope? | Before work depends on these capabilities; not all required to resume pickup |
| DEC-02 | Should a targeted drop honor the exact slot rather than current merge/first-empty behavior? | LOOT-06 |
| DEC-03 | Which manager behaviors need Blueprint extension hooks versus native implementation? | Pickup/interaction API design |
| DEC-04 | Is Advanced a distinct styling mode or advanced fields within Preset/Custom? | UI-01 |
| DEC-05 | Is DeltaHealth a signed delta or heal-only host convention? | Extending health integration |
| DEC-06 | What are exact buyer setup and release acceptance requirements? | SAFE-05 / release planning |

These are open questions, not assumed product requirements.

## F. Full regression checklist for MIG-06

Fresh full run pending. User toggle and aggregate refresh confirmations remain recorded in MIG-01/02.

- [ ] Tab opens/closes repeatedly with correct cursor/input behavior, including after UI clicks.
- [ ] Correct inventory selected; only one intended player grid initialized.
- [ ] Initially populated inventory displays before a new mutation.
- [ ] Nearby-item tracking sets/clears correctly; pickup succeeds once.
- [ ] Full inventory does not destroy an unaccepted world pickup; partial acceptance preserves remainder if supported.
- [ ] Stack caps and spill behavior correct.
- [ ] Correct slot icons/counts; last-item removal clears visuals.
- [ ] Context menu actions execute the selected fragment on the intended item.
- [ ] Consume applies intended host effect and decrements exactly once.
- [ ] Successful drop spawns the intended pickup and removes exactly one; failed drop preserves inventory.
- [ ] Same-inventory swap works; self-drop is a no-op; empty slots do not start drags.
- [ ] Menu closes according to intended policy; no relevant runtime Blueprint errors.

Record environment, build/revision, test cases, actual results, and failures in the log. New container features require their own transfer matrix, not this single-inventory checklist alone.

## G. Superseded old backlog items

- Direct demo-character casts: historically resolved in two passes; final portability audit remains SAFE-05.
- Duplicate character DropItem event: historically removed as dead code.
- “Success without consuming cannot be expressed”: incorrect; Success already represents that outcome.
- Full demo health/combat system: not an established inventory-plugin requirement.
