#include "AIEditorAssistantModule.h"

#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"
#include "SAIEditorAssistantChatPanel.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Tools/AIEditorAssistantToolRuntime.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Brushes/SlateImageBrush.h"

#define LOCTEXT_NAMESPACE "FAIEditorAssistantModule"

const FName FAIEditorAssistantModule::AIEditorAssistantTabName(TEXT("AIEditorAssistantChatTab"));
namespace
{
    const FName StyleSetName(TEXT("AIEditorAssistantStyle"));
}

void FAIEditorAssistantModule::RegisterStyle()
{
    if (StyleSet.IsValid())
    {
        return;
    }

    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("AIEditorAssistant"));
    if (!Plugin.IsValid())
    {
        return;
    }

    TSharedRef<FSlateStyleSet> NewStyle = MakeShared<FSlateStyleSet>(StyleSetName);
    NewStyle->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
    NewStyle->Set(TEXT("AIEditorAssistant.Icon20"), new FSlateImageBrush(NewStyle->RootToContentDir(TEXT("Icon20"), TEXT(".png")), FVector2D(20.0f, 20.0f)));
    NewStyle->Set(TEXT("AIEditorAssistant.Icon40"), new FSlateImageBrush(NewStyle->RootToContentDir(TEXT("Icon40"), TEXT(".png")), FVector2D(40.0f, 40.0f)));
    FSlateStyleRegistry::RegisterSlateStyle(*NewStyle);
    StyleSet = NewStyle;
}

void FAIEditorAssistantModule::UnregisterStyle()
{
    if (!StyleSet.IsValid())
    {
        return;
    }

    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
    ensure(StyleSet.IsUnique());
    StyleSet.Reset();
}

void FAIEditorAssistantModule::StartupModule()
{
    FAIEditorAssistantToolRuntime::Get().Startup();
    RegisterStyle();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        AIEditorAssistantTabName,
        FOnSpawnTab::CreateRaw(this, &FAIEditorAssistantModule::SpawnAIEditorAssistantTab))
        .SetDisplayName(LOCTEXT("TabTitle", "AI Editor Assistant"))
        .SetIcon(FSlateIcon(StyleSetName, TEXT("AIEditorAssistant.Icon20")))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAIEditorAssistantModule::RegisterMenus));
}

void FAIEditorAssistantModule::ShutdownModule()
{
    FAIEditorAssistantToolRuntime::Get().Shutdown();

    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);

    if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AIEditorAssistantTabName);
    }

    UnregisterStyle();
}

void FAIEditorAssistantModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");

    Section.AddMenuEntry(
        "OpenAIEditorAssistantTab",
        LOCTEXT("OpenAIEditorAssistantTabLabel", "AI Editor Assistant"),
        LOCTEXT("OpenAIEditorAssistantTabTooltip", "Open the AI Editor Assistant chat panel."),
        FSlateIcon(StyleSetName, TEXT("AIEditorAssistant.Icon20")),
        FUIAction(FExecuteAction::CreateLambda([]
        {
            FGlobalTabmanager::Get()->TryInvokeTab(FAIEditorAssistantModule::AIEditorAssistantTabName);
        })));

    UToolMenu* PlayToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
    FToolMenuSection& PlayToolbarSection = PlayToolbarMenu->FindOrAddSection("PluginTools");

    PlayToolbarSection.AddEntry(FToolMenuEntry::InitToolBarButton(
        "OpenAIEditorAssistantToolbarButton",
        FUIAction(FExecuteAction::CreateLambda([]
        {
            FGlobalTabmanager::Get()->TryInvokeTab(FAIEditorAssistantModule::AIEditorAssistantTabName);
        })),
        LOCTEXT("OpenAIEditorAssistantToolbarLabel", "AI Editor Assistant"),
        LOCTEXT("OpenAIEditorAssistantToolbarTooltip", "Open the AI Editor Assistant chat panel."),
        FSlateIcon(StyleSetName, TEXT("AIEditorAssistant.Icon40"), TEXT("AIEditorAssistant.Icon20"))));
}

TSharedRef<SDockTab> FAIEditorAssistantModule::SpawnAIEditorAssistantTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SAIEditorAssistantChatPanel)
        ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAIEditorAssistantModule, AIEditorAssistant)
