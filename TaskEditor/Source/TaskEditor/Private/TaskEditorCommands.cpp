// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TaskEditorCommands.h"
#include "STaskEditWindow.h"

#define LOCTEXT_NAMESPACE "FTaskEditorModule"

void FTaskEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "TaskEditor", "Open TaskEditor", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(TaskAddAction, "Add Task", "Add Task", EUserInterfaceActionType::Button, FInputGesture());
	TaskActionList->MapAction(TaskAddAction, FExecuteAction::CreateStatic(&STaskEditWindow::OnAddTask));

	UI_COMMAND(RemoveTaskAction, "Remove Task", "Romove Task", EUserInterfaceActionType::Button, FInputGesture());
	TaskActionList->MapAction(RemoveTaskAction, FExecuteAction::CreateStatic(&STaskEditWindow::OnRomveTask));
}

#undef LOCTEXT_NAMESPACE
