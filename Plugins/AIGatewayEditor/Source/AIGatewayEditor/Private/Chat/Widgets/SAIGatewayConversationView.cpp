#include "Chat/Widgets/SAIGatewayConversationView.h"

#include "Chat/Widgets/SAIGatewayChatMessageCard.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SAIGatewayConversationView::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SAssignNew(ChatHistoryScrollBox, SScrollBox)
    ];
}

void SAIGatewayConversationView::Refresh(const FAIGatewayChatPanelViewState& ViewState)
{
    if (!ChatHistoryScrollBox.IsValid())
    {
        return;
    }

    if (ViewState.VisibleMessages.Num() == 0)
    {
        if (bShowingEmptyPlaceholder)
        {
            return;
        }

        ChatHistoryScrollBox->ClearChildren();
        MessageCards.Reset();
        CachedVisibleMessages.Reset();
        bShowingEmptyPlaceholder = true;

        ChatHistoryScrollBox->AddSlot()
        .Padding(0.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Messages from you and the model will appear here.")))
            .AutoWrapText(true)
            .ColorAndOpacity(FLinearColor(0.52f, 0.56f, 0.62f))
        ];
        return;
    }

    bShowingEmptyPlaceholder = false;

    if (!CanUpdateExistingCards(ViewState.VisibleMessages))
    {
        RebuildMessages(ViewState.VisibleMessages);
    }
    else
    {
        for (int32 Index = 0; Index < ViewState.VisibleMessages.Num(); ++Index)
        {
            if (CachedVisibleMessages[Index].Content != ViewState.VisibleMessages[Index].Content && MessageCards[Index].IsValid())
            {
                MessageCards[Index]->UpdateMessage(ViewState.VisibleMessages[Index]);
            }
        }

        CachedVisibleMessages = ViewState.VisibleMessages;
    }

    ChatHistoryScrollBox->ScrollToEnd();
}

bool SAIGatewayConversationView::CanUpdateExistingCards(const TArray<FAIGatewayChatMessage>& VisibleMessages) const
{
    if (VisibleMessages.Num() == 0 || VisibleMessages.Num() != CachedVisibleMessages.Num() || VisibleMessages.Num() != MessageCards.Num())
    {
        return false;
    }

    for (int32 Index = 0; Index < VisibleMessages.Num(); ++Index)
    {
        if (!MessageCards[Index].IsValid() || VisibleMessages[Index].Role != CachedVisibleMessages[Index].Role)
        {
            return false;
        }
    }

    return true;
}

void SAIGatewayConversationView::RebuildMessages(const TArray<FAIGatewayChatMessage>& VisibleMessages)
{
    ChatHistoryScrollBox->ClearChildren();
    MessageCards.Reset();

    for (const FAIGatewayChatMessage& Message : VisibleMessages)
    {
        TSharedPtr<SAIGatewayChatMessageCard> MessageCard;
        ChatHistoryScrollBox->AddSlot()
        .Padding(0.0f, 0.0f, 0.0f, 10.0f)
        [
            SAssignNew(MessageCard, SAIGatewayChatMessageCard)
            .Message(Message)
        ];
        MessageCards.Add(MessageCard);
    }

    CachedVisibleMessages = VisibleMessages;
}
