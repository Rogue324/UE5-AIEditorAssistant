#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Delegates/Delegate.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnAIEditorAssistantDraftChanged, const FString&);
DECLARE_DELEGATE(FOnAIEditorAssistantSendRequested);
DECLARE_DELEGATE(FOnAIEditorAssistantCancelRequested);

class SAIEditorAssistantComposer : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantComposer) {}
        SLATE_EVENT(FOnAIEditorAssistantDraftChanged, OnDraftChanged)
        SLATE_EVENT(FOnAIEditorAssistantSendRequested, OnSendRequested)
        SLATE_EVENT(FOnAIEditorAssistantCancelRequested, OnCancelRequested)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIEditorAssistantChatPanelViewState& ViewState);

private:
    void HandlePromptTextChanged(const FText& InText);
    FReply HandlePromptKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

    FOnAIEditorAssistantDraftChanged OnDraftChanged;
    FOnAIEditorAssistantSendRequested OnSendRequested;
    FOnAIEditorAssistantCancelRequested OnCancelRequested;
    TSharedPtr<class SMultiLineEditableTextBox> PromptTextBox;
    TSharedPtr<class SButton> SendButton;
    FString SendButtonText;
    bool bCanSend = false;
    bool bCanCancel = false;
    bool bSyncingText = false;
};
