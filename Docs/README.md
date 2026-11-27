# Modular Inventory System: documentation

Last reconciled: **2026-11-28 (Asia/Karachi)**.

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

These are goals, not a claim of release readiness. Single-item manager pickup through the rules subsystem, its rejection/cleanup checks, and deterministic multiple-inventory selection work by user report. Final screen migration verification, broader regression, looting UI, styling integration, and clean-project validation remain outstanding.

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

The user wants to write the implementation themselves while learning Unreal C++. They know standard C++. On 2026-11-27 they explicitly reinforced: provide code snippets only for Unreal-specific features (APIs, reflection macros, UObject lifetime, components, Blueprint/native integration). Do not supply complete function bodies, ordinary C++ conditions/loops, or algorithmic solutions unless explicitly requested. Describe desired behavior and acceptance criteria, explain the Unreal boundary, then review their attempt. This applies even to small functions; do not turn learning steps into copy/paste implementations.

On 2026-11-27 the user additionally requested exact change locations: always name the file and the class/function or insertion point for each step. Use clickable absolute file links in chat.

Latest learning preference (2026-11-27): explain the next task's purpose, responsibilities, behavior, and acceptance checks first, then let the user implement independently. Offer Unreal-specific syntax/API help when requested rather than supplying it proactively. Review their implementation afterward.
