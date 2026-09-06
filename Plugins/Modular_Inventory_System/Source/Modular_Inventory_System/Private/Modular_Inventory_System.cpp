// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modular_Inventory_System.h"

#define LOCTEXT_NAMESPACE "FModular_Inventory_SystemModule"

void FModular_Inventory_SystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FModular_Inventory_SystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModular_Inventory_SystemModule, Modular_Inventory_System)