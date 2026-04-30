#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Widgets/SCompoundWidget.h"

class SAIEditorAssistantChatMessageCard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantChatMessageCard) {}
        SLATE_ARGUMENT(FAIEditorAssistantChatMessage, Message)
        SLATE_ARGUMENT(bool, RenderMarkdown)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIEditorAssistantChatMessage& InMessage, bool bInRenderMarkdown);

private:
    void RebuildForMessage(const FAIEditorAssistantChatMessage& InMessage, bool bInRenderMarkdown);
    FReply OnToggleCollapse();
    FReply OnToggleToolActivity();

    FAIEditorAssistantChatMessage CachedMessage;
    bool bRenderMarkdown = true;
    bool bIsToolMessage = false;
    bool bCollapsed = true;
    bool bToolActivityCollapsed = true;
    TSharedPtr<class SHorizontalBox> RootRow;
    TSharedPtr<class SBox> BubbleWidthBox;
    TSharedPtr<class SBorder> BubbleBorder;
    TSharedPtr<class SRichTextBlock> RoleTextBlock;
    TSharedPtr<class STextBlock> CollapseToggleText;
    TSharedPtr<class SAIEditorAssistantMarkdownMessageBody> MessageBody;
    TSharedPtr<class SMultiLineEditableText> PlainTextBody;
    TSharedPtr<class SVerticalBox> BubbleContentBox;
    TSharedPtr<class SBox> ContentArea;
    TSharedPtr<class SBox> ToolActivityArea;
    TSharedPtr<class STextBlock> ToolActivityToggleText;
};
