#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAIEditorAssistantChatPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantChatPanel) {}
    SLATE_END_ARGS()

    virtual ~SAIEditorAssistantChatPanel() override;

    void Construct(const FArguments& InArgs);

private:
    void RefreshFromController();

    TSharedPtr<class FAIEditorAssistantChatController> ChatController;
    TSharedPtr<class SEditableTextBox> ModelTextBox;
    TSharedPtr<class STextBlock> ContextTextBlock;
    TSharedPtr<class STextBlock> StatusTextBlock;
    TSharedPtr<class SAIEditorAssistantSessionTabBar> SessionTabBar;
    TSharedPtr<class SAIEditorAssistantConversationView> ConversationView;
    TSharedPtr<class SAIEditorAssistantToolConfirmationBar> ToolConfirmationBar;
    TSharedPtr<class SAIEditorAssistantComposer> Composer;
};
