#include "Chat/Widgets/SAIEditorAssistantChatMessageCard.h"

#include "Chat/Markdown/AIEditorAssistantMarkdownRichTextRenderer.h"
#include "Chat/Widgets/SAIEditorAssistantMarkdownMessageBody.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    void ResolveMessageStyle(
        const FAIEditorAssistantChatMessage& Message,
        FString& OutRoleStyle,
        const FSlateBrush*& OutBubbleBrush,
        EHorizontalAlignment& OutBubbleAlignment,
        bool& bOutIsToolMessage)
    {
        OutRoleStyle = TEXT("RoleSystem");
        OutBubbleBrush = FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetBrush("BubbleSystem");
        OutBubbleAlignment = HAlign_Left;
        bOutIsToolMessage = false;

        if (Message.Role.Equals(TEXT("You"), ESearchCase::IgnoreCase))
        {
            OutRoleStyle = TEXT("RoleYou");
            OutBubbleBrush = FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetBrush("BubbleYou");
            OutBubbleAlignment = HAlign_Right;
        }
        else if (Message.Role.Equals(TEXT("AI"), ESearchCase::IgnoreCase))
        {
            OutRoleStyle = TEXT("RoleAI");
            OutBubbleBrush = FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetBrush("BubbleAI");
        }
        else if (Message.Role.Equals(TEXT("Tool"), ESearchCase::IgnoreCase))
        {
            OutRoleStyle = TEXT("RoleTool");
            OutBubbleBrush = FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetBrush("BubbleTool");
            bOutIsToolMessage = true;
        }
        else if (Message.Role.Equals(TEXT("Tool Result"), ESearchCase::IgnoreCase))
        {
            OutRoleStyle = TEXT("RoleToolResult");
            OutBubbleBrush = FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetBrush("BubbleToolResult");
            bOutIsToolMessage = true;
        }
    }
}

void SAIEditorAssistantChatMessageCard::Construct(const FArguments& InArgs)
{
    CachedMessage = InArgs._Message;
    bRenderMarkdown = InArgs._RenderMarkdown;

    ChildSlot
    [
        SAssignNew(RootRow, SHorizontalBox)
    ];

    RebuildForMessage(CachedMessage, bRenderMarkdown);
}

void SAIEditorAssistantChatMessageCard::Refresh(const FAIEditorAssistantChatMessage& InMessage, bool bInRenderMarkdown)
{
    const bool bRoleChanged = !CachedMessage.Role.Equals(InMessage.Role, ESearchCase::CaseSensitive);
    const bool bContentChanged = !CachedMessage.Content.Equals(InMessage.Content, ESearchCase::CaseSensitive);
    const bool bRenderModeChanged = (bRenderMarkdown != bInRenderMarkdown);
    if (!bRoleChanged && !bContentChanged && !bRenderModeChanged)
    {
        return;
    }

    CachedMessage = InMessage;
    bRenderMarkdown = bInRenderMarkdown;

    if (bRoleChanged || !RoleTextBlock.IsValid() || !BubbleBorder.IsValid() || !RootRow.IsValid())
    {
        RebuildForMessage(CachedMessage, bRenderMarkdown);
        return;
    }

    if (bRenderModeChanged)
    {
        RebuildForMessage(CachedMessage, bRenderMarkdown);
        return;
    }

    if (bRenderMarkdown && MessageBody.IsValid())
    {
        MessageBody->RefreshMarkdown(CachedMessage.Content);
    }
    else if (!bRenderMarkdown && PlainTextBody.IsValid())
    {
        PlainTextBody->SetText(FText::FromString(CachedMessage.Content));
    }
}

FReply SAIEditorAssistantChatMessageCard::OnToggleCollapse()
{
    bCollapsed = !bCollapsed;

    if (ContentArea.IsValid() && BubbleContentBox.IsValid() && CollapseToggleText.IsValid())
    {
        ContentArea->SetVisibility(bCollapsed ? EVisibility::Collapsed : EVisibility::Visible);
        CollapseToggleText->SetText(FText::FromString(bCollapsed ? FString(TEXT("\u25B6")) : FString(TEXT("\u25BC"))));
    }

    return FReply::Handled();
}

FReply SAIEditorAssistantChatMessageCard::OnToggleToolActivity()
{
    bToolActivityCollapsed = !bToolActivityCollapsed;

    if (ToolActivityArea.IsValid() && ToolActivityToggleText.IsValid())
    {
        ToolActivityArea->SetVisibility(bToolActivityCollapsed ? EVisibility::Collapsed : EVisibility::Visible);
        ToolActivityToggleText->SetText(FText::FromString(bToolActivityCollapsed
            ? FString(TEXT("\u25B6 Tool Activity"))
            : FString(TEXT("\u25BC Tool Activity"))));
    }

    return FReply::Handled();
}

void SAIEditorAssistantChatMessageCard::RebuildForMessage(const FAIEditorAssistantChatMessage& InMessage, bool bInRenderMarkdown)
{
    if (!RootRow.IsValid())
    {
        return;
    }

    FString RoleStyle;
    const FSlateBrush* BubbleBrush = nullptr;
    EHorizontalAlignment BubbleAlignment = HAlign_Left;
    bool bIsTool = false;
    ResolveMessageStyle(InMessage, RoleStyle, BubbleBrush, BubbleAlignment, bIsTool);
    bIsToolMessage = bIsTool;
    if (!bIsToolMessage)
    {
        bCollapsed = false;
    }

    const bool bUseStreamingPlainTextWidth =
        !bRenderMarkdown && InMessage.Role.Equals(TEXT("AI"), ESearchCase::IgnoreCase);
    bRenderMarkdown = bInRenderMarkdown;
    MessageBody.Reset();
    PlainTextBody.Reset();
    ContentArea.Reset();
    BubbleContentBox.Reset();
    ToolActivityArea.Reset();
    ToolActivityToggleText.Reset();
    CollapseToggleText.Reset();

    bToolActivityCollapsed = !InMessage.ToolActivityContent.IsEmpty();

    TSharedPtr<SWidget> ContentWidget;

    if (bRenderMarkdown)
    {
        SAssignNew(MessageBody, SAIEditorAssistantMarkdownMessageBody)
            .MarkdownText(InMessage.Content);
        ContentWidget = MessageBody;
    }
    else
    {
        SAssignNew(PlainTextBody, SMultiLineEditableText)
            .Text(FText::FromString(InMessage.Content))
            .TextStyle(&FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetWidgetStyle<FTextBlockStyle>("MarkdownBody"))
            .IsReadOnly(true)
            .AllowContextMenu(true)
            .AutoWrapText(true)
            .Margin(FMargin(0.0f))
            .ClearTextSelectionOnFocusLoss(false)
            .SelectWordOnMouseDoubleClick(true);
        ContentWidget = PlainTextBody;
    }

    RootRow->ClearChildren();
    RootRow->AddSlot()
    .FillWidth(1.0f)
    .HAlign(BubbleAlignment)
    [
        SAssignNew(BubbleWidthBox, SBox)
        .MinDesiredWidth(bUseStreamingPlainTextWidth ? 420.0f : 0.0f)
        .MaxDesiredWidth(860.0f)
        [
            SAssignNew(BubbleBorder, SBorder)
            .BorderImage(BubbleBrush)
            .Padding(FMargin(12.0f, 10.0f))
            .OnMouseButtonUp_Lambda([MessageText = InMessage.Content](const FGeometry&, const FPointerEvent& MouseEvent)
            {
                if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
                {
                    FPlatformApplicationMisc::ClipboardCopy(*MessageText);
                    return FReply::Handled();
                }

                return FReply::Unhandled();
            })
            [
                SAssignNew(BubbleContentBox, SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, bIsToolMessage ? 0.0f : 8.0f)
                [
                    SNew(SBorder)
                    .Padding(FMargin(0.0f))
                    .BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
                    .OnMouseButtonUp_Lambda([this](const FGeometry&, const FPointerEvent& MouseEvent)
                    {
                        if (bIsToolMessage && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
                        {
                            return OnToggleCollapse();
                        }
                        return FReply::Unhandled();
                    })
                    [
                        SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(0.0f, 0.0f, 6.0f, 0.0f)
                        [
                            SAssignNew(CollapseToggleText, STextBlock)
                            .Text(FText::FromString(bIsToolMessage ? FString(TEXT("\u25B6")) : FString()))
                            .Visibility_Lambda([this]() { return bIsToolMessage ? EVisibility::Visible : EVisibility::Collapsed; })
                        ]

                        + SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        [
                            SAssignNew(RoleTextBlock, SRichTextBlock)
                            .TextStyle(&FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetWidgetStyle<FTextBlockStyle>(*RoleStyle))
                            .DecoratorStyleSet(&FAIEditorAssistantMarkdownRichTextRenderer::GetStyle())
                            .Text(FText::FromString(FString::Printf(TEXT("<%s>%s</>"), *RoleStyle, *FAIEditorAssistantMarkdownRichTextRenderer::EscapeRichText(InMessage.Role))))
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SAssignNew(ContentArea, SBox)
                    .Visibility(bIsToolMessage ? EVisibility::Collapsed : EVisibility::Visible)
                    [
                        ContentWidget.ToSharedRef()
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, InMessage.ToolActivityContent.IsEmpty() ? 0.0f : 8.0f, 0.0f, 0.0f)
                [
                    SNew(SBorder)
                    .Padding(FMargin(0.0f))
                    .BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
                    .Visibility_Lambda([this]() { return !CachedMessage.ToolActivityContent.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
                    .OnMouseButtonUp_Lambda([this](const FGeometry&, const FPointerEvent& MouseEvent)
                    {
                        if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
                        {
                            return OnToggleToolActivity();
                        }
                        return FReply::Unhandled();
                    })
                    [
                        SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .VAlign(VAlign_Center)
                            .Padding(0.0f, 0.0f, 6.0f, 0.0f)
                            [
                                SAssignNew(ToolActivityToggleText, STextBlock)
                                .Text(FText::FromString(!InMessage.ToolActivityContent.IsEmpty() ? FString(TEXT("\u25B6 Tool Activity")) : FString()))
                                .ColorAndOpacity(FSlateColor(FLinearColor(0.56f, 0.56f, 0.60f)))
                                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
                            ]
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0.0f, 4.0f, 0.0f, 0.0f)
                        [
                            SAssignNew(ToolActivityArea, SBox)
                            .Visibility(EVisibility::Collapsed)
                            [
                                bRenderMarkdown
                                    ? StaticCastSharedRef<SWidget>(
                                        SNew(SAIEditorAssistantMarkdownMessageBody)
                                        .MarkdownText(InMessage.ToolActivityContent))
                                    : StaticCastSharedRef<SWidget>(
                                        SNew(SMultiLineEditableText)
                                        .Text(FText::FromString(InMessage.ToolActivityContent))
                                        .TextStyle(&FAIEditorAssistantMarkdownRichTextRenderer::GetStyle().GetWidgetStyle<FTextBlockStyle>("MarkdownBody"))
                                        .IsReadOnly(true)
                                        .AutoWrapText(true)
                                        .Margin(FMargin(0.0f))
                                        .ClearTextSelectionOnFocusLoss(false))
                            ]
                        ]
                    ]
                ]
            ]
        ]
    ];
}
