#pragma once

#include "Chat/Model/AIEditorAssistantChatTypes.h"
#include "Delegates/Delegate.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnAIEditorAssistantDraftChanged, const FString&);
DECLARE_DELEGATE(FOnAIEditorAssistantSendRequested);
DECLARE_DELEGATE(FOnAIEditorAssistantCancelRequested);
DECLARE_DELEGATE(FOnAIEditorAssistantImageAttachRequested);
DECLARE_DELEGATE(FOnAIEditorAssistantImageClearRequested);
DECLARE_DELEGATE_OneParam(FOnAIEditorAssistantImageRemoveRequested, int32);

class SAIEditorAssistantComposer : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIEditorAssistantComposer) {}
        SLATE_EVENT(FOnAIEditorAssistantDraftChanged, OnDraftChanged)
        SLATE_EVENT(FOnAIEditorAssistantSendRequested, OnSendRequested)
        SLATE_EVENT(FOnAIEditorAssistantCancelRequested, OnCancelRequested)
        SLATE_EVENT(FOnAIEditorAssistantImageAttachRequested, OnImageAttachRequested)
        SLATE_EVENT(FOnAIEditorAssistantImageClearRequested, OnImageClearRequested)
        SLATE_EVENT(FOnAIEditorAssistantImageRemoveRequested, OnImageRemoveRequested)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void Refresh(const FAIEditorAssistantChatPanelViewState& ViewState);
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    void HandlePromptTextChanged(const FText& InText);
    FReply HandlePromptKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
    void RebuildAttachmentThumbnails();
    void RefreshAttachmentPreviewWidgets();

    FOnAIEditorAssistantDraftChanged OnDraftChanged;
    FOnAIEditorAssistantSendRequested OnSendRequested;
    FOnAIEditorAssistantCancelRequested OnCancelRequested;
    FOnAIEditorAssistantImageAttachRequested OnImageAttachRequested;
    FOnAIEditorAssistantImageClearRequested OnImageClearRequested;
    FOnAIEditorAssistantImageRemoveRequested OnImageRemoveRequested;
    TSharedPtr<class SMultiLineEditableTextBox> PromptTextBox;
    TSharedPtr<class SButton> SendButton;
    TSharedPtr<class SBox> AttachmentPreviewAreaBox;
    TSharedPtr<class SVerticalBox> AttachmentPreviewContainer;
    TArray<TSharedPtr<struct FSlateDynamicImageBrush>> AttachmentThumbnailBrushes;
    TArray<FVector2D> AttachmentThumbnailDisplaySizes;
    TArray<int32> AttachmentThumbnailSourceIndices;
    FString PendingAttachmentSummary;
    TArray<FString> PendingAttachmentPaths;
    FString SendButtonText;
    float CachedAttachmentLayoutWidth = 0.0f;
    bool bCanSend = false;
    bool bCanCancel = false;
    bool bSyncingText = false;
};
