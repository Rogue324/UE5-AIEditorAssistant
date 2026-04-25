#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Delegates/Delegate.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnAIEditorAssistantToolApproved);
DECLARE_DELEGATE(FOnAIEditorAssistantToolRejected);

class SAIEditorAssistantToolConfirmationBar : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantToolConfirmationBar) {}
        SLATE_EVENT(FOnAIEditorAssistantToolApproved, OnApproved)
        SLATE_EVENT(FOnAIEditorAssistantToolRejected, OnRejected)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIEditorAssistantChatPanelViewState& ViewState);

private:
    FOnAIEditorAssistantToolApproved OnApproved;
    FOnAIEditorAssistantToolRejected OnRejected;
    TSharedPtr<class STextBlock> PromptTextBlock;
    bool bIsVisible = false;
};
