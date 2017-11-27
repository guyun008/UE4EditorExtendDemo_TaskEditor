// Fill out your copyright notice in the Description page of Project Settings.
#include "STaskEditWindow.h"
#include "SlateOptMacros.h"
#include "SHeaderRow.h"
#include "ITypedTableView.h"
#include "STableViewBase.h"
#include "EditorStyleSet.h"
#include "EditorDirectories.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/SNumericDropDown.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Input/SSpinBox.h"
#include "Interfaces/IMainFrameModule.h"
#include "Modules/ModuleManager.h"
#include "TaskEditorCommands.h"
#include "MultiBoxBuilder.h"
#include "SSlider.h"

TSharedPtr<STaskEditWindow> STaskEditWindow::TaskEditorInstance = nullptr;

#define LOCTEXT_NAMESPACE "STaskEditWindow" 

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STaskEditWindow::Construct(const FArguments& InArgs,const TSharedRef<SDockTab>& OwningTab)
{
	TabManager = FGlobalTabmanager::Get()->NewTabManager(OwningTab);
	Init();
	SWindow::Construct(SWindow::FArguments()
		.Title(FText::FromString(TEXT("TaskEditor")))
		.ClientSize(FVector2D(1200, 800))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			ContentBox.ToSharedRef()
		]);
}

void STaskEditWindow::Init()
{
	InitTaskDataArray();
	ContentBox = SNew(SVerticalBox);
	ToolButtons = SNew(SWrapBox).UseAllottedWidth(true);
	DetailBox = SNew(SHorizontalBox);
	InitMenuContent();
	InitToolButton();
	InitWindowContent();
}

void STaskEditWindow::InitWindowContent()
{
	ContentBox->AddSlot().AutoHeight()
	[
		ToolButtons.ToSharedRef()
	];
	ContentBox->AddSlot()
	[
		DetailBox.ToSharedRef()
	];
	InitTaskTreeWindow();
	InitDetailPanel();
}

void STaskEditWindow::InitMenuContent()
{
	const TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());
	FMenuBarBuilder MenuBuilder(FTaskEditorCommands::Get().TaskActionList, MenuExtender);

	// File
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("FileMenu", "File"),
		LOCTEXT("FileMenu_ToolTip", "Open the file menu"),
		FNewMenuDelegate::CreateStatic(&STaskEditWindow::FillFileMenu, MenuExtender),
		"File",
		FName(TEXT("FileMenu"))
	);

	// Edit
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("EditMenu", "Edit"),
		LOCTEXT("EditMenu_ToolTip", "Open the edit menu"),
		FNewMenuDelegate::CreateStatic(&STaskEditWindow::FillEditMenu, MenuExtender, TabManager),
		"Edit",
		FName(TEXT("EditMenu"))
	);

	// Window
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("WindowMenu", "Window"),
		LOCTEXT("WindowMenu_ToolTip", "Open new windows or tabs."),
		FNewMenuDelegate::CreateStatic(&STaskEditWindow::FillWindowMenu, MenuExtender, TabManager),
		"Window"
	);

	// Help
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("HelpMenu", "Help"),
		LOCTEXT("HelpMenu_ToolTip", "Open the help menu"),
		FNewMenuDelegate::CreateStatic(&STaskEditWindow::FillHelpMenu, MenuExtender),
		"Help"
	);

	TSharedRef<SWidget> MenuBarWidget = MenuBuilder.MakeWidget();

	ContentBox->AddSlot().AutoHeight()
	[
		MenuBarWidget
	];
}

void STaskEditWindow::InitToolButton()
{
	ToolButtons->AddSlot()
	[
		SNew(SBox)
		.MinDesiredWidth(50)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &STaskEditWindow::AddTask)
			/*.Visibility(this, &FPaperTileMapDetailsCustomization::GetNonEditModeVisibility)
			.IsEnabled(this, &FPaperTileMapDetailsCustomization::GetIsEditModeEnabled)*/
			.Text(FText::FromString(TEXT("Add Task")))
			.ToolTipText(FText::FromString(TEXT("Add Task")))
		]
	];


	ToolButtons->AddSlot()
	[
		SNew(SBox)
		.MinDesiredWidth(50)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &STaskEditWindow::RomveTask)
		/*.Visibility(this, &FPaperTileMapDetailsCustomization::GetNonEditModeVisibility)
		.IsEnabled(this, &FPaperTileMapDetailsCustomization::GetIsEditModeEnabled)*/
			.Text(FText::FromString(TEXT("Remove Task")))
			.ToolTipText(FText::FromString(TEXT("Remove Task")))
		]
	];

	ToolButtons->AddSlot()
	[
		SNew(SBox)
		.MinDesiredWidth(50)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			/*.OnClicked(this, &FPaperTileMapDetailsCustomization::EnterTileMapEditingMode)
			.Visibility(this, &FPaperTileMapDetailsCustomization::GetNonEditModeVisibility)
			.IsEnabled(this, &FPaperTileMapDetailsCustomization::GetIsEditModeEnabled)*/
			.Text(FText::FromString(TEXT("Import")))
			.ToolTipText(FText::FromString(TEXT("Import Task")))
		]
	];

	ToolButtons->AddSlot()
	[
		SNew(SBox)
		.MinDesiredWidth(50)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			/*.OnClicked(this, &FPaperTileMapDetailsCustomization::EnterTileMapEditingMode)
			.Visibility(this, &FPaperTileMapDetailsCustomization::GetNonEditModeVisibility)
			.IsEnabled(this, &FPaperTileMapDetailsCustomization::GetIsEditModeEnabled)*/
			.Text(FText::FromString(TEXT("Export")))
			.ToolTipText(FText::FromString(TEXT("Export Task")))
		]
	];
}

void STaskEditWindow::InitTaskTreeWindow()
{
	TaskTreeWidget = SNew(STreeView<TSharedPtr<TaskItemData>>)
		.SelectionMode(ESelectionMode::Single)
		.ClearSelectionOnClick(false)		// Don't allow user to select nothing.
		.TreeItemsSource(&ItemsSource)
		.OnGenerateRow(this, &STaskEditWindow::OnGenerateRow)
		.OnGetChildren(this, &STaskEditWindow::OnGetChildren)
		.OnSelectionChanged(this, &STaskEditWindow::OnSelectionChanged)
		.OnMouseButtonClick(this, &STaskEditWindow::OnTaskClick)
		.HeaderRow
		(
			SNew(SHeaderRow)
			+ SHeaderRow::Column("Task List")
			.DefaultLabel(FText::FromString(TEXT("Task List")))
			.FillWidth(1.0f)
		);
	DetailBox->AddSlot().FillWidth(0.2)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		//.DesiredSizeScale(FVector2D(1, 0))
		[
			TaskTreeWidget.ToSharedRef()
		]
	];
}

void STaskEditWindow::InitDetailPanel()
{
	TArray<SNumericDropDown<float>::FNamedValue> SnapValues;
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(0.001f, LOCTEXT("Snap_OneThousandth", "0.001"), LOCTEXT("SnapDescription_OneThousandth", "Set snap to 1/1000th")));
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(0.01f, LOCTEXT("Snap_OneHundredth", "0.01"), LOCTEXT("SnapDescription_OneHundredth", "Set snap to 1/100th")));
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(0.1f, LOCTEXT("Snap_OneTenth", "0.1"), LOCTEXT("SnapDescription_OneTenth", "Set snap to 1/10th")));
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(1.0f, LOCTEXT("Snap_One", "1"), LOCTEXT("SnapDescription_One", "Set snap to 1")));
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(10.0f, LOCTEXT("Snap_Ten", "10"), LOCTEXT("SnapDescription_Ten", "Set snap to 10")));
	SnapValues.Add(SNumericDropDown<float>::FNamedValue(100.0f, LOCTEXT("Snap_OneHundred", "100"), LOCTEXT("SnapDescription_OneHundred", "Set snap to 100")));
	
	DetailBox->AddSlot()
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		//.FillHeight(10)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(100)
			.DesiredSizeScale(FVector2D(800,800))
			[
				SNew(SGridPanel)
				.FillColumn(1, 0.5f)
				.FillColumn(2, 0.5f)

				// File Path
				+ SGridPanel::Slot(0, 0)
				.Padding(10, 0, 10, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FileLabel", "File name"))
				]
				+ SGridPanel::Slot(1, 0)
				.ColumnSpan(2)
				.Padding(100, 0, 0, 0)
				[
					SNew(SFilePathPicker)
					.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
					.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
					.BrowseButtonToolTip(LOCTEXT("FileButtonToolTipText", "Choose a post moves text file..."))
					.BrowseDirectory(FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_OPEN))
					.BrowseTitle(LOCTEXT("BrowseButtonTitle", "Choose a post moves text file"))
					.FileTypeFilter(TEXT("Text File (*.txt)|*.txt"))
					//.FilePath(this, &SComposurePostMoveSettingsImportDialog::GetFilePath)
					//.OnPathPicked(this, &SComposurePostMoveSettingsImportDialog::FilePathPicked)
				]

				// Frame Rate
				+ SGridPanel::Slot(0, 2)
				.Padding(10, 10, 10, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FrameRateLabel", "Frame Rate"))
				]
				+ SGridPanel::Slot(1, 2)
				.Padding(100, 10, 0, 0)
				[
					SNew(SNumericDropDown<float>)
					.DropDownValues(SnapValues)
					.bShowNamedValue(true)
					//.Value(this, &SComposurePostMoveSettingsImportDialog::GetFrameInterval)
					//.OnValueChanged(this, &SComposurePostMoveSettingsImportDialog::FrameIntervalChanged)
				]

				// Start Frame
				+ SGridPanel::Slot(0, 3)
				.Padding(10, 10, 10, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StartFrameLabel", "Start Frame"))
				]
				+ SGridPanel::Slot(1, 3)
				.Padding(100, 10, 0, 0)
				[
					SNew(SSpinBox<int32>)
					.MinValue(TOptional<int32>())
					.MaxValue(TOptional<int32>())
					.MaxSliderValue(TOptional<int32>())
					.MinSliderValue(TOptional<int32>())
					.Delta(1)
					//.Value(this, &SComposurePostMoveSettingsImportDialog::GetStartFrame)
					//.OnValueChanged(this, &SComposurePostMoveSettingsImportDialog::StartFrameChanged)
					]

					+ SGridPanel::Slot(0, 4)
					.Padding(10, 10, 10, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Task Text", "Task Text"))
					]
					+ SGridPanel::Slot(1, 4)
					.Padding(100, 10, 0, 0)
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(FString(TEXT("Input Task Text ......."))))
					]

					+ SGridPanel::Slot(0, 5)
					.Padding(10, 10, 10, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Task CheckBox", "Task CheckBox"))
					]
					+ SGridPanel::Slot(1, 5)
					.Padding(100, 10, 0, 0)
					[
						SNew(SCheckBox)
						//.IsChecked(this, &FMeshBuildSettingsLayout::ShouldRecomputeTangents)
						//.OnCheckStateChanged(this, &FMeshBuildSettingsLayout::OnRecomputeTangentsChanged)
					]

					+ SGridPanel::Slot(0, 6)
					.Padding(10, 10, 10, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Task SSlider", "Task SSlider"))
					]
					+ SGridPanel::Slot(1, 6)
					.Padding(100, 10, 0, 0)
					[
						SNew(SSlider)
						.ToolTipText(LOCTEXT("ThumbnailScaleToolTip", "Adjust the size of thumbnails."))
						/*.Value(this, &SAssetView::GetThumbnailScale)
						.OnValueChanged(this, &SAssetView::SetThumbnailScale)
						.Locked(this, &SAssetView::IsThumbnailScalingLocked)*/
					]
				]
		]
	];
}

void STaskEditWindow::InitTaskDataArray()
{
	TaskItemData* d1 = new TaskItemData();
	d1->id = 1;
	d1->name = NSLOCTEXT("STaskEditWindow", "Add Task", "Add Task");

	TaskItemData* d12 = new TaskItemData();
	d12->id = 2;
	d12->name = NSLOCTEXT("STaskEditWindow", "Add Task11", "Add Task11");
	d1->next.Add(TSharedPtr<TaskItemData>(d12));

	ItemsSource.Add(TSharedPtr<TaskItemData>(d1));

	TaskItemData* d2 = new TaskItemData();
	d2->id = 2;
	d2->name = NSLOCTEXT("STaskEditWindow", "Add Task22", "Add Task22");;
	ItemsSource.Add(TSharedPtr<TaskItemData>(d2));
}

TSharedRef<ITableRow> STaskEditWindow::OnGenerateRow(TSharedPtr<TaskItemData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow< TSharedPtr<TaskItemData> >, OwnerTable)
	[
		SNew(STextBlock).Text(Item->name)
	];
}

void STaskEditWindow::OnGetChildren(TSharedPtr<TaskItemData> InItem, TArray< TSharedPtr<TaskItemData> >& OutChildren)
{
	if (!InItem.IsValid() || InItem->next.Num() <= 0)
	{
		return;
	}

	OutChildren.Append(InItem->next);
}

void STaskEditWindow::OnSelectionChanged(TSharedPtr<TaskItemData> InItem, ESelectInfo::Type type)
{
	//SlectedItem = InItem;
}

void STaskEditWindow::OnTaskClick(TSharedPtr<TaskItemData> InItem)
{
	if (!InItem.IsValid()) return;
	SlectedItem = InItem;
}



void STaskEditWindow::OnAddTask()
{
	TaskItemData* d2 = new TaskItemData();
	d2->id = 2;
	d2->name = NSLOCTEXT("STaskEditWindow", "Add Task22", "Add Task22");
	STaskEditWindow::GetTaskEditorInstance().Get()->ItemsSource.Add(TSharedPtr<TaskItemData>(d2));
	STaskEditWindow::GetTaskEditorInstance().Get()->TaskTreeWidget.Get()->RequestTreeRefresh();
}

void STaskEditWindow::OnRomveTask()
{
	if (STaskEditWindow::GetTaskEditorInstance().Get()->SlectedItem.IsValid())
	{
		if (STaskEditWindow::GetTaskEditorInstance().Get()->SlectedItem.Get())
		{
			STaskEditWindow::GetTaskEditorInstance().Get()->FindAndRemoveItemsSource(
				STaskEditWindow::GetTaskEditorInstance().Get()->ItemsSource,
				STaskEditWindow::GetTaskEditorInstance().Get()->SlectedItem.ToSharedRef()
			);
			STaskEditWindow::GetTaskEditorInstance().Get()->TaskTreeWidget.Get()->RequestTreeRefresh();
		}
	}
}

FReply STaskEditWindow::AddTask()
{
	STaskEditWindow::OnAddTask();
	return FReply::Handled();
}

FReply STaskEditWindow::RomveTask()
{
	STaskEditWindow::OnRomveTask();
	return FReply::Handled();
}


void STaskEditWindow::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, const TSharedPtr<FTabManager> TabManager)
{

}

void STaskEditWindow::FillFileMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender)
{
	MenuBuilder.BeginSection("TaskAddAndRemove", LOCTEXT("LoadSandSaveHeading", "Task and Remove"));
	{
		// Add Task...
		MenuBuilder.AddMenuEntry(FTaskEditorCommands::Get().TaskAddAction,"AddTask");
		MenuBuilder.AddMenuEntry(FTaskEditorCommands::Get().RemoveTaskAction, "RemoveTask");
	}
	MenuBuilder.EndSection();
}

void STaskEditWindow::FillEditMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, const TSharedPtr<FTabManager> TabManager)
{

}

void STaskEditWindow::FillHelpMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender)
{

}


void STaskEditWindow::FindAndRemoveItemsSource(TArray< TSharedPtr<TaskItemData>>& ItemsSourceIn, TSharedRef<TaskItemData> RemoveData)
{
	for (auto It = ItemsSourceIn.CreateIterator(); It; It++)
	{
		TSharedPtr<TaskItemData> ItData = TSharedPtr<TaskItemData>(*It);
		if (ItData.IsValid() && ItData->id == RemoveData->id && ItData->name.EqualTo(RemoveData->name))
		{
			ItemsSourceIn.Remove(ItData);
			UE_LOG(LogSlate, Log, TEXT("TaskItemData refrenceCount 1: %d"), ItData.GetSharedReferenceCount());
			ItData.Reset();
			UE_LOG(LogSlate, Log, TEXT("TaskItemData refrenceCount 2: %d"), ItData.GetSharedReferenceCount());
			return;
		}
		/*else
		{
			if (ItData->next.Num() > 0)
			{
				FindAndRemoveItemsSource(ItData->next, RemoveData);
			}
		}*/
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TaskItemData::~TaskItemData()
{
	//delete &id;
	////delete &name;
	//if (next.Num()>0)
	//{
	//	next.Reset();
	//}
	//next.~TArray();
}
