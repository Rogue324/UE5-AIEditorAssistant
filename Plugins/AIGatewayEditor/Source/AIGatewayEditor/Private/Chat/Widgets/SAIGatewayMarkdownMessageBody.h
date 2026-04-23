#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAIGatewayMarkdownMessageBody : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAIGatewayMarkdownMessageBody) {}
        SLATE_ARGUMENT(FString, MarkdownText)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetMarkdownText(const FString& InMarkdownText);

private:
    void RebuildContent(const FString& InMarkdownText);
    FString BuildRenderableRichText(const FString& InMarkdownText) const;

    FString MarkdownText;
    bool bHasBuiltContent = false;
    TSharedPtr<class SMultiLineEditableText> RichTextWidget;
    TSharedPtr<class ITextLayoutMarshaller> RichTextMarshaller;
};
