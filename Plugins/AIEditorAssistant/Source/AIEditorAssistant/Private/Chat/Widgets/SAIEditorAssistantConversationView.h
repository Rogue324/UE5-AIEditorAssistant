#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Widgets/SCompoundWidget.h"

class SAIEditorAssistantConversationView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantConversationView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIEditorAssistantChatPanelViewState& ViewState);

private:
    bool AreMessagesUnchanged(const TArray<FAIEditorAssistantChatMessage>& InMessages) const;
    bool ShouldRenderMarkdownForMessage(const FAIEditorAssistantChatPanelViewState& ViewState, int32 MessageIndex, const FAIEditorAssistantChatMessage& Message) const;
    void RebuildMessageList(const TArray<FAIEditorAssistantChatMessage>& InMessages);

    TSharedPtr<class SScrollBox> ChatHistoryScrollBox;
    TArray<FAIEditorAssistantChatMessage> CachedMessages;
    TArray<TSharedPtr<class SAIEditorAssistantChatMessageCard>> MessageCards;
    bool bCachedEmptyState = true;
};
