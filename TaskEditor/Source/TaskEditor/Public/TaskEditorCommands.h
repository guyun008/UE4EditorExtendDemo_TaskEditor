// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "TaskEditorStyle.h"
#include "UICommandList.h"

class FTaskEditorCommands : public TCommands<FTaskEditorCommands>
{
public:

	FTaskEditorCommands()
		: TCommands<FTaskEditorCommands>(TEXT("TaskEditor"), NSLOCTEXT("Contexts", "TaskEditor", "TaskEditor Plugin"), NAME_None, FTaskEditorStyle::GetStyleSetName())
	{
		TaskActionList = MakeShareable(new FUICommandList);
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;

	//File menu Action
	TSharedPtr<FUICommandInfo> TaskAddAction;
	TSharedPtr<FUICommandInfo> RemoveTaskAction;

	TSharedPtr<FUICommandList> TaskActionList;
};
