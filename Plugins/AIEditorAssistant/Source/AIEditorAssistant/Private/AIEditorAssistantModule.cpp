#include "AIEditorAssistantModule.h"

#include "LevelEditor.h"
#include "SAIEditorAssistantChatPanel.h"
#include "Tools/AIEditorAssistantToolRuntime.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FAIEditorAssistantModule"

const FName FAIEditorAssistantModule::AIEditorAssistantTabName(TEXT("AIEditorAssistantChatTab"));

void FAIEditorAssistantModule::StartupModule()
{
    FAIEditorAssistantToolRuntime::Get().Startup();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        AIEditorAssistantTabName,
        FOnSpawnTab::CreateRaw(this, &FAIEditorAssistantModule::SpawnAIEditorAssistantTab))
        .SetDisplayName(LOCTEXT("TabTitle", "AI Editor Assistant"))
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
        FSlateIcon(),
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
        FSlateIcon()));
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
