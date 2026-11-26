# Modular Inventory System: documentation

Last reconciled: **2026-11-25 (Asia/Karachi)**.

## Read in this order

1. [Project state](InventorySystem_ProjectState.md): current behavior, architecture, evidence, and resumption point.
2. [Checklist and backlog](InventorySystem_PolishBacklog.md): stable task IDs, priorities, completion checks, and unresolved decisions.
3. [Development log](InventorySystem_DevelopmentLog.md): changes, test reports, and corrections.
4. [Historical notes](Archive/2026-11-25/README.md): preserved input documents, not current instructions.

## Product goals

Build a **modular, fragment-based, plug-and-play, extremely user-friendly Unreal Engine inventory plugin**. Current engine: UE 5.7. Recorded distribution intent: Fab.

- Compose item capabilities with fragments without item-type branches in core inventory code.
- Integrate through components and interfaces without dependencies on the demo character.
- Keep buyer setup explicit and small: documented assignments, defaults, interfaces, and input routing.
- Support reusable inventory panels, lightweight ground pickups, and inventory-bearing containers.
- Configure layout and styling without rewriting widget graphs.
- Preserve quantities, stable slots, and correct UI updates.

These are goals, not a claim of release readiness. Manager pickup, looting UI, styling integration, and clean-project validation remain outstanding.

The recorded roadmap includes inventory, vicinity looting, containers, configurable visuals, and later a host for buyer-supplied tabs. Multiplayer, split-screen, save/load, equipment, crafting, and controller navigation have not been established as implementation commitments. Demo health is not a commitment to a full combat system.

## Evidence and status conventions

| Evidence | Meaning |
| --- | --- |
| USER-VERIFIED | User reported behavior working; not an independent agent playtest |
| INSPECTED | Current source or pasted graph supports the claim; runtime behavior not implied |
| HISTORICAL | Earlier notes, handoff, or commit report; not freshly verified |
| PLANNED | Intended work, not completed |
| UNKNOWN | Missing or contradictory evidence |

Task statuses: DONE, IN PROGRESS, OPEN, DECISION NEEDED, DEFERRED. A checked checkbox applies only to its stated scope. Old test success is not a new test pass.

Code establishes implementation, user decisions establish intent, and pasted graphs establish the supplied wiring. A document's age or confident wording cannot override direct evidence. Asset names alone do not prove graph execution.

## Ongoing maintenance

The user requested continuing documentation on 2026-11-25. After each meaningful change or test report:

1. Update the affected current-state contract, with exact names and evidence limits.
2. Update the existing checklist ID and completion checks.
3. Append a dated development-log entry: change, reason, verification, and outstanding work.
4. Update the resumption point. Instructions given but not confirmed remain open.
5. Distinguish local changes from committed changes; never invent build/playtest results.
6. Resolve stale claims explicitly and preserve useful lessons.

Use precise, small Blueprint steps followed by verification. This repository's Docs folder is the maintained copy. Supplied external originals remain unchanged.

## Current learning preference (2026-11-26)

The user wants to write RequestPickup themselves while learning Unreal C++. They explicitly clarified that they know standard C++; focus on Unreal APIs, reflection macros, UObject lifetime, components, and Blueprint/native integration. Avoid basic C++ syntax exercises. Review their attempts without writing the complete gameplay implementation or editing it on their behalf unless requested. Nearby-pickup ownership and the successful-add contract still require design and inspection.
