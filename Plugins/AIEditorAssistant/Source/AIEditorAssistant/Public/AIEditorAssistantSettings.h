#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AIEditorAssistantSettings.generated.h"

UENUM()
enum class EAIEditorAssistantAPIProvider : uint8
{
    OpenAICompatible UMETA(DisplayName = "OpenAI Compatible"),
    DeepSeek UMETA(DisplayName = "DeepSeek"),
    Anthropic UMETA(DisplayName = "Anthropic Claude"),
    Gemini UMETA(DisplayName = "Google Gemini"),
};

UENUM()
enum class EAIEditorAssistantReasoningIntensity : uint8
{
    ProviderDefault UMETA(DisplayName = "Provider Default"),
    Disabled UMETA(DisplayName = "Disabled"),
    Minimal UMETA(DisplayName = "Minimal"),
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Maximum UMETA(DisplayName = "Maximum"),
};

UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig, meta = (DisplayName = "AI Editor Assistant"))
class UAIEditorAssistantSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UAIEditorAssistantSettings();

    virtual FName GetContainerName() const override;
    virtual FName GetCategoryName() const override;

    UPROPERTY(Config, EditAnywhere, Category = "Connection", meta = (DisplayName = "Base URL"))
    FString BaseUrl;

    UPROPERTY(Config, EditAnywhere, Category = "Connection", meta = (DisplayName = "API Key"))
    FString ApiKey;

    UPROPERTY(Config, EditAnywhere, Category = "Connection", meta = (DisplayName = "API Provider", ToolTip = "Select the request format to use. OpenAI Compatible works for OpenAI-compatible providers such as OpenAI, Qwen-compatible servers, Moonshot, and many self-hosted services."))
    EAIEditorAssistantAPIProvider Provider;

    UPROPERTY(Config, EditAnywhere, Category = "Request", meta = (DisplayName = "Model"))
    FString Model;

    UPROPERTY(Config, EditAnywhere, Category = "Request", meta = (DisplayName = "Reasoning Intensity", ToolTip = "Recommended starting points: OpenAI/DeepSeek use Medium or High for coding agents, Anthropic Sonnet 4.6 is usually best at Medium, Claude Opus 4.7 often starts at High or Maximum, Gemini Flash starts at Medium and Gemini Pro starts at High."))
    EAIEditorAssistantReasoningIntensity ReasoningIntensity;

    UPROPERTY(Config, EditAnywhere, Category = "Chat", meta = (DisplayName = "Max Tool Rounds", ClampMin = "1", UIMin = "1"))
    int32 MaxToolRounds;

    UPROPERTY(Config, EditAnywhere, Category = "Chat", meta = (DisplayName = "Show Tool Activity In Chat"))
    bool bShowToolActivityInChat;
};
