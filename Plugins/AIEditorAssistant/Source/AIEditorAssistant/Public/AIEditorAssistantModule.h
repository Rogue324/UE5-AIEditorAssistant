#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSlateStyleSet;

class FAIEditorAssistantModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterStyle();
    void UnregisterStyle();
    void RegisterMenus();
    TSharedRef<class SDockTab> SpawnAIEditorAssistantTab(const class FSpawnTabArgs& SpawnTabArgs);

    static const FName AIEditorAssistantTabName;
    TSharedPtr<FSlateStyleSet> StyleSet;
};
