#pragma once

#include "Chat/Model/AIGatewayChatTypes.h"
#include "Widgets/SCompoundWidget.h"

class SAIGatewayConversationView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIGatewayConversationView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIGatewayChatPanelViewState& ViewState);

private:
    bool CanUpdateExistingCards(const TArray<FAIGatewayChatMessage>& VisibleMessages) const;
    void RebuildMessages(const TArray<FAIGatewayChatMessage>& VisibleMessages);

    TSharedPtr<class SScrollBox> ChatHistoryScrollBox;
    TArray<TSharedPtr<class SAIGatewayChatMessageCard>> MessageCards;
    TArray<FAIGatewayChatMessage> CachedVisibleMessages;
    bool bShowingEmptyPlaceholder = false;
};
