#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Chat/Services/AIEditorAssistantChatService.h"
#include "Chat/Services/AIEditorAssistantChatSessionStore.h"
#include "Delegates/Delegate.h"

struct FAIEditorAssistantToolResult;

class FAIEditorAssistantChatController : public TSharedFromThis<FAIEditorAssistantChatController>
{
public:
    explicit FAIEditorAssistantChatController(
        TSharedRef<IAIEditorAssistantChatSessionStore> InSessionStore,
        TSharedRef<IAIEditorAssistantChatService> InChatService);

    void Initialize();
    void PersistActiveDraft();

    void SetModel(const FString& InModel);
    void UpdateDraft(const FString& DraftText);
    void AddPendingImagePaths(const TArray<FString>& ImagePaths);
    void RemovePendingImageAt(int32 ImageIndex);
    void ClearPendingImages();
    void SubmitPrompt();
    void CancelCurrentWork();
    void CreateSession();
    void ActivateSession(const FString& SessionId);
    void CloseSession(const FString& SessionId);
    void ApprovePendingTool();
    void RejectPendingTool();

    FAIEditorAssistantChatPanelViewState GetViewState() const;
    FSimpleMulticastDelegate& OnStateChanged();

private:
    void BroadcastStateChanged();
    bool CanSendRequest() const;
    bool CanCancelWork() const;
    bool CanEditSessions() const;
    bool IsAwaitingToolConfirmation() const;
    bool IsGeneratingTitle() const;
    bool ShouldDisplayConversationMessage(const FAIEditorAssistantChatMessage& Message) const;
    int32 GetConfiguredMaxToolRounds() const;
    bool ShouldShowToolActivityInChat() const;

    FAIEditorAssistantChatSession* GetActiveSession();
    const FAIEditorAssistantChatSession* GetActiveSession() const;
    FAIEditorAssistantChatSession* FindSessionById(const FString& SessionId);
    const FAIEditorAssistantChatSession* FindSessionById(const FString& SessionId) const;

    void LoadModelFromSettings();
    bool SaveModelToSettings(FString& OutError) const;
    bool ResolveServiceSettings(FAIEditorAssistantChatServiceSettings& OutSettings, FString& OutError) const;
    FString BuildContextSummary(const FAIEditorAssistantChatSession& Session) const;

    void AppendMessage(const FString& Role, const FString& Text);
    void UpsertLastMessage(const FString& Role, const FString& Text);
    void AddRequestMessage(const TSharedPtr<FJsonObject>& MessageObject);
    void TouchSession(FAIEditorAssistantChatSession& Session);
    void PersistActiveSession();
    void SaveSessionIndex() const;

    void LoadSessions();
    FAIEditorAssistantChatSession& CreateSessionInternal(bool bMakeActive);
    void EnsureActiveSession();
    FString MakeNewSessionTitle() const;
    FString GenerateFallbackTitle(const FAIEditorAssistantChatSession& Session) const;
    bool ShouldGenerateTitle(const FAIEditorAssistantChatSession& Session) const;
    void MaybeGenerateTitleForActiveSession();

    void StartAssistantResponse();
    void FinalizeAssistantResponse();
    void AbortAssistantResponse();
    void ResetStreamingState();
    void BeginUserTurn(const FString& UserDisplayText, const TSharedPtr<FJsonObject>& UserMessageObject);
    bool SendChatRequest();
    void ExecuteNextPendingToolCall();
    void ExecuteCurrentPendingToolCall(bool bApproved);
    void ResumeAfterToolConfirmation(bool bApproved);
    void FinishTurnWithError(const FString& ErrorMessage, bool bKeepAssistantPlaceholder = false);

    void HandleChatResponse(int32 RequestSerial, const FAIEditorAssistantChatServiceResponse& Response);
    void HandleStreamingPayloadChunk(int32 RequestSerial, const FString& ChunkText);
    bool HandleStreamingLine(const FString& LineText);
    bool TryAppendAssistantDelta(const TSharedPtr<FJsonObject>& ChoiceObject);
    bool TryAppendToolCallDelta(const TSharedPtr<FJsonObject>& ChoiceObject);
    bool ParseChatCompletionPayload(
        const FString& ResponseBody,
        FString& OutAssistantContent,
        FString& OutReasoningContent,
        TArray<FAIEditorAssistantPendingToolCall>& OutToolCalls,
        TSharedPtr<FJsonObject>& OutProviderPayload,
        bool& bOutHadChoices) const;
    bool TryParseToolCallsFromMessage(const TSharedPtr<FJsonObject>& MessageObject, TArray<FAIEditorAssistantPendingToolCall>& OutToolCalls) const;

    TSharedPtr<FJsonObject> BuildUserMessageObject(const FString& UserPrompt, const TArray<FString>& ImagePaths, FString& OutError) const;
    TSharedPtr<FJsonObject> BuildAssistantMessageObject(const FString& AssistantContent, const FString& ReasoningContent, const TArray<FAIEditorAssistantPendingToolCall>& ToolCalls, const TSharedPtr<FJsonObject>& ProviderPayload) const;
    TSharedPtr<FJsonObject> BuildToolResultMessageObject(const FString& ToolCallId, const FString& ToolName, const FString& Content) const;
    TArray<TSharedPtr<FJsonValue>> BuildRequestMessages() const;
    TArray<TSharedPtr<FJsonValue>> BuildToolDefinitions() const;

    FString BuildPendingAttachmentSummary(const FAIEditorAssistantChatSession& Session) const;
    FString ExtractErrorMessage(const FString& ResponseBody) const;
    FString FormatToolArgumentsSummary(const FAIEditorAssistantPendingToolCall& ToolCall) const;
    void AppendToolCallMessages(const TArray<FAIEditorAssistantPendingToolCall>& ToolCalls);
    void AppendToolResultMessage(const FAIEditorAssistantPendingToolCall& ToolCall, const FAIEditorAssistantToolResult& Result);
    FString GetPendingToolApprovalPrompt() const;
    FString GetSendButtonText() const;

    TSharedRef<IAIEditorAssistantChatSessionStore> SessionStore;
    TSharedRef<IAIEditorAssistantChatService> ChatService;
    TArray<FAIEditorAssistantChatSession> Sessions;
    FString ActiveSessionId;
    FString CurrentModel;
    FString StatusMessage;
    bool bIsSending = false;
    int32 NextRequestSerial = 0;
    int32 ActiveRequestSerial = 0;
    FSimpleMulticastDelegate StateChangedDelegate;
};
