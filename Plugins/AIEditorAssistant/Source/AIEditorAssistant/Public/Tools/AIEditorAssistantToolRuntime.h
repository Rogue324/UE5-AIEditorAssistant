#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

enum class EAIEditorAssistantToolConfirmationPolicy : uint8
{
    None,
    ExplicitApproval
};

struct FAIEditorAssistantToolDefinition
{
    FString Name;
    FString Description;
    TSharedPtr<FJsonObject> Parameters;
    EAIEditorAssistantToolConfirmationPolicy ConfirmationPolicy = EAIEditorAssistantToolConfirmationPolicy::None;
};

struct FAIEditorAssistantToolResult
{
    bool bSuccess = false;
    bool bWasRejected = false;
    FString Summary;
    TSharedPtr<FJsonObject> Payload;

    static FAIEditorAssistantToolResult Success(const FString& InSummary, const TSharedPtr<FJsonObject>& InPayload = nullptr);
    static FAIEditorAssistantToolResult Error(const FString& InSummary, const TSharedPtr<FJsonObject>& InPayload = nullptr);
    static FAIEditorAssistantToolResult Rejected(const FString& InSummary);

    FString ToMessageContent() const;
};

class FAIEditorAssistantToolRuntime
{
public:
    static FAIEditorAssistantToolRuntime& Get();

    void Startup();
    void Shutdown();

    const TArray<FAIEditorAssistantToolDefinition>& GetToolDefinitions() const;
    const FAIEditorAssistantToolDefinition* FindDefinition(const FString& ToolName) const;
    FAIEditorAssistantToolResult ExecuteTool(const FString& ToolName, const TSharedPtr<FJsonObject>& Arguments) const;

private:
    void BuildDefinitions();

    TArray<FAIEditorAssistantToolDefinition> Definitions;
    TMap<FString, int32> DefinitionIndexByName;
    bool bStarted = false;
};
