// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "STextBlock.h"
#include "SWrapBox.h"
#include "SBoxPanel.h"
#include "TabManager.h"
#include "Widgets/Views/STreeView.h"

class TaskItemData;

/**
 * 
 */
class TASKEDITOR_API STaskEditWindow : public SWindow
{
public:
	SLATE_BEGIN_ARGS(STaskEditWindow)
	{}
	SLATE_END_ARGS()


	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& OwningTab);

	TSharedPtr<SVerticalBox> ContentBox;
	TSharedPtr<SWrapBox> ToolButtons;
	TSharedPtr<SHorizontalBox> DetailBox;
	TSharedPtr<STreeView<TSharedPtr<TaskItemData>>> TaskTreeWidget;

	//Task Data Array
	TArray< TSharedPtr<TaskItemData> > ItemsSource;

	//Task Data
	TSharedPtr<TaskItemData> SlectedItem;
	TSharedPtr<FTabManager> TabManager;

	// Init Functions
	void Init();
	void InitWindowContent();
	void InitMenuContent();
	void InitToolButton();
	void InitTaskTreeWindow();
	void InitDetailPanel();
	void InitTaskDataArray();

	TSharedRef<class ITableRow> OnGenerateRow(TSharedPtr<TaskItemData> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<TaskItemData> InItem, TArray< TSharedPtr<TaskItemData> >& OutChildren);
	void OnSelectionChanged(TSharedPtr<TaskItemData> InItem,ESelectInfo::Type type);
	void OnTaskClick(TSharedPtr<TaskItemData> InItem);


	static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, const TSharedPtr<FTabManager> TabManager);
	static void FillFileMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender);
	static void FillEditMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, const TSharedPtr<FTabManager> TabManager);
	static void FillHelpMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender);

	//Menu button actions
	static void OnAddTask();
	static void OnRomveTask();
	FReply AddTask();
	FReply RomveTask();
private:
	static TSharedPtr<STaskEditWindow> TaskEditorInstance;

	void FindAndRemoveItemsSource(TArray< TSharedPtr<TaskItemData>>& ItemsSourceIn,TSharedRef<TaskItemData> RemoveData);
public:
	static TSharedPtr<STaskEditWindow> GetTaskEditorInstance()
	{
		if (!TaskEditorInstance.IsValid())
		{
			TSharedPtr<SDockTab> TaskEditorTab = SNew(SDockTab)
				.TabRole(ETabRole::MajorTab)
				.ContentPadding(FMargin(0));
			TaskEditorInstance = SNew(STaskEditWindow, TaskEditorTab.ToSharedRef());
			FSlateApplication::Get().AddWindow(TaskEditorInstance.ToSharedRef());
		}
		return TaskEditorInstance;
	};
};



class TaskItemData
{
public:
	int64 id;
	TArray< TSharedPtr<TaskItemData> > next;
	FText name;
	TaskItemData() 
		:id(0)
		,name()
		,next()
	{};

	virtual ~TaskItemData();
};
