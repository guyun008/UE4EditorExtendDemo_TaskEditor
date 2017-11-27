// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TaskEditor.h"
#include "TaskEditorStyle.h"
#include "Misc/MessageDialog.h"
#include "LevelEditor.h"
#include "UnrealEdGlobals.h"
#include "Engine.h"

//DECLARE_LOG_CATEGORY_EXTERN(LogTaskEditor, Log, All);

class STaskEditWindow;

#define LOCTEXT_NAMESPACE "FTaskEditorModule"

static const FName TaskEditorTabName("TaskEditor");

void FTaskEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FTaskEditorStyle::Initialize();
	FTaskEditorStyle::ReloadTextures();

	FTaskEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FTaskEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FTaskEditorModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FTaskEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FTaskEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	IsOPened = false;
}

void FTaskEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FTaskEditorStyle::Shutdown();

	FTaskEditorCommands::Unregister();
	IsOPened = false;
}

void FTaskEditorModule::PluginButtonClicked()
{
	STaskEditWindow::GetTaskEditorInstance().Get()->ShowWindow();
	STaskEditWindow::GetTaskEditorInstance().Get()->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateRaw(this, &FTaskEditorModule::OnRequestDestroyWindow));
	//STaskEditWindow::GetTaskEditorInstance().Get()->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FTaskEditorModule::OnWindowClosed));
}


void FTaskEditorModule::OnRequestDestroyWindow(const TSharedRef<SWindow>& ClosedWindow)
{
	//STaskEditWindow::GetTaskEditorInstance().Get()->DestroyWindowImmediately();

	STaskEditWindow::GetTaskEditorInstance().Get()->HideWindow();
}

void FTaskEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FTaskEditorCommands::Get().PluginAction);
}

void FTaskEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FTaskEditorCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTaskEditorModule, TaskEditor)