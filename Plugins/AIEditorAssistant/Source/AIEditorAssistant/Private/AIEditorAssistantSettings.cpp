#include "AIEditorAssistantSettings.h"

UAIEditorAssistantSettings::UAIEditorAssistantSettings()
{
    BaseUrl = TEXT("https://api.openai.com/v1");
    ApiKey = TEXT("");
    Provider = EAIEditorAssistantAPIProvider::OpenAICompatible;
    Model = TEXT("gpt-4o-mini");
    ReasoningIntensity = EAIEditorAssistantReasoningIntensity::Medium;
    MaxToolRounds = 8;
    bShowToolActivityInChat = false;
}

FName UAIEditorAssistantSettings::GetContainerName() const
{
    return FName(TEXT("Project"));
}

FName UAIEditorAssistantSettings::GetCategoryName() const
{
    return FName(TEXT("Plugins"));
}
