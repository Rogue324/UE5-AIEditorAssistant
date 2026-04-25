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

    FAIEditorAssistantChatMessage CachedMessage;
    bool bRenderMarkdown = true;
    TSharedPtr<class SHorizontalBox> RootRow;
    TSharedPtr<class SBox> BubbleWidthBox;
    TSharedPtr<class SBorder> BubbleBorder;
    TSharedPtr<class SRichTextBlock> RoleTextBlock;
    TSharedPtr<class SAIEditorAssistantMarkdownMessageBody> MessageBody;
    TSharedPtr<class SMultiLineEditableText> PlainTextBody;
};
