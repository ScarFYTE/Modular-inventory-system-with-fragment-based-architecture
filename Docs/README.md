# Modular Inventory & Item System (Unreal Engine)

A data-driven, ECS-inspired inventory and item management system for Unreal Engine, built in C++ with a Blueprint-friendly authoring layer. Designed to scale to hundreds of unique items without ever touching core gameplay logic again.

## Why this exists

Traditional inventory systems tend to rely on giant `switch` statements or type-checking to decide what an item does. This system instead strips items down to blank containers (`UItemDefinition`, a `UPrimaryDataAsset`) and builds their behavior entirely from small, modular **Fragments** snapped onto them — a Mesh Fragment for how it looks when dropped, a UI Fragment for its icon, a Consumable Fragment for what happens when you use it, and so on. Adding a new item type never means editing the inventory's core code — only adding a new Fragment class.

## Architecture

The system is strictly separated into three layers:

- **The Data (Model)** — `UItemDefinition` + `UItemFragment` subclasses. Items are blank containers; behavior is entirely composed from fragments.
- **The Brains (Controller)** — `UInventoryComponent` (owns inventory state) and `UInventoryRulesSubsystem` (the single authoritative entry point for every gameplay action — drop, use, move). The UI never mutates inventory state directly; it only ever *requests* an action from the subsystem.
- **The Interface (View)** — a "dumb," reactive UMG layer. Widgets interrogate an item's fragments through a universal Blueprint Interface contract (`BPI_ContextAction`) to build context menus dynamically, without ever knowing what a "potion" or a "sword" is.

## Features implemented

- Config-driven, persistent-slot inventory grid — capacity, layout, and slot size are all data-asset-driven (`UInventoryUIConfig`), not hardcoded
- Modular Fragment system with runtime fragment lookup (`FindFragmentByClass`)
- Dynamic context menu generation, driven entirely by which fragments an item has
- Fragment-specific action execution (`RequestExecuteAction`) — consuming, and easily extensible to equip/throw/etc.
- Full drag-and-drop between inventory slots, backed by a slot-stable, fixed-size inventory array
- In-progress: three-tier visual styling system (Preset / Custom / Advanced) so end-users of the plugin can reskin the UI without touching C++

## Tech stack

- Unreal Engine 5.7
- C++ for all authoritative gameplay logic and data types
- Blueprint for fragment behavior, UI, and widget composition

## Project goal

This system is being built with the intent of being packaged and sold as a plugin on the Unreal Marketplace / Fab — so configurability, clean separation of concerns, and end-user-friendly setup are treated as first-class requirements throughout, not an afterthought.

## Status

Actively in development. See `/docs` for detailed running notes on architecture decisions, known issues, and the polish backlog.

## License

TBD — placeholder until a license is chosen for public/commercial release.
