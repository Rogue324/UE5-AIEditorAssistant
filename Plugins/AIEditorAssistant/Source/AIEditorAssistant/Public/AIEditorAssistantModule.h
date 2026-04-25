#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAIEditorAssistantModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterMenus();
    TSharedRef<class SDockTab> SpawnAIEditorAssistantTab(const class FSpawnTabArgs& SpawnTabArgs);

    static const FName AIEditorAssistantTabName;
};
