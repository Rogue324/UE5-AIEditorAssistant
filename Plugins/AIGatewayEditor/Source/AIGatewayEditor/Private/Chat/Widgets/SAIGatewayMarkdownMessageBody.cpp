#include "Chat/Widgets/SAIGatewayMarkdownMessageBody.h"

#include "Chat/Markdown/AIGatewayMarkdownParser.h"
#include "Chat/Markdown/AIGatewayMarkdownRichTextRenderer.h"
#include "Framework/Text/RichTextLayoutMarshaller.h"
#include "Widgets/Text/SMultiLineEditableText.h"

namespace
{
    FString ConvertLegacyCodeBlockTagsToMarkdown(const FString& InText)
    {
        FString Working = InText;
        Working.ReplaceInline(TEXT("</MarkdownCodeBlock>"), TEXT("</>"));

        const FString OpenTag = TEXT("<MarkdownCodeBlock>");
        const FString CloseTag = TEXT("</>");
        FString Result;
        int32 SearchFrom = 0;

        while (true)
        {
            const int32 OpenIndex = Working.Find(OpenTag, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchFrom);
            if (OpenIndex == INDEX_NONE)
            {
                Result.Append(Working.Mid(SearchFrom));
                break;
            }

            Result.Append(Working.Mid(SearchFrom, OpenIndex - SearchFrom));
            const int32 ContentStart = OpenIndex + OpenTag.Len();
            const int32 CloseIndex = Working.Find(CloseTag, ESearchCase::CaseSensitive, ESearchDir::FromStart, ContentStart);
            if (CloseIndex == INDEX_NONE)
            {
                Result.Append(Working.Mid(OpenIndex));
                break;
            }

            const FString Inner = Working.Mid(ContentStart, CloseIndex - ContentStart).TrimStartAndEnd();
            Result.Append(TEXT("```\n"));
            Result.Append(Inner);
            Result.Append(TEXT("\n```"));

            SearchFrom = CloseIndex + CloseTag.Len();
        }

        return Result;
    }

    FString RenderCodeBlockText(const FString& CodeText)
    {
        const FString NormalizedCodeText = FAIGatewayMarkdownRichTextRenderer::NormalizeForDisplay(CodeText);
        TArray<FString> Lines;
        FAIGatewayMarkdownParser::NormalizeLineEndings(NormalizedCodeText).ParseIntoArray(Lines, TEXT("\n"), false);
        if (Lines.Num() == 0)
        {
            return TEXT("<MarkdownCodeBlock> </>");
        }

        FString Out;
        for (int32 Index = 0; Index < Lines.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out.Append(TEXT("\n"));
            }

            Out.Append(FString::Printf(TEXT("<MarkdownCodeBlock>%s</>"), *FAIGatewayMarkdownRichTextRenderer::EscapeRichText(Lines[Index].IsEmpty() ? TEXT(" ") : Lines[Index])));
        }

        return Out;
    }

    FString BuildTableLine(const TArray<FString>& Cells, const bool bBold)
    {
        FString Line;
        for (int32 Index = 0; Index < Cells.Num(); ++Index)
        {
            if (Index > 0)
            {
                Line.Append(TEXT(" | "));
            }

            const FString CellText = FAIGatewayMarkdownRichTextRenderer::RenderInlineMarkdown(Cells[Index], true);
            if (bBold)
            {
                Line.Append(FString::Printf(TEXT("<MarkdownBold>%s</>"), *CellText));
            }
            else
            {
                Line.Append(CellText);
            }
        }

        return Line;
    }

    FString BuildTableRichText(const FAIGatewayMarkdownBlock& Block)
    {
        FString Result;
        for (int32 RowIndex = 0; RowIndex < Block.TableRows.Num(); ++RowIndex)
        {
            if (RowIndex > 0)
            {
                Result.Append(TEXT("\n"));
            }

            const bool bIsHeader = RowIndex == 0;
            Result.Append(BuildTableLine(Block.TableRows[RowIndex], bIsHeader));
        }
        return Result;
    }
}

void SAIGatewayMarkdownMessageBody::Construct(const FArguments& InArgs)
{
    if (!RichTextMarshaller.IsValid())
    {
        const ISlateStyle& Style = FAIGatewayMarkdownRichTextRenderer::GetStyle();
        RichTextMarshaller = FRichTextLayoutMarshaller::Create(TArray<TSharedRef<ITextDecorator>>(), &Style);
    }

    ChildSlot
    [
        SAssignNew(RichTextWidget, SMultiLineEditableText)
        .TextStyle(&FAIGatewayMarkdownRichTextRenderer::GetStyle().GetWidgetStyle<FTextBlockStyle>("MarkdownBody"))
        .IsReadOnly(true)
        .AllowContextMenu(true)
        .AutoWrapText(true)
        .Margin(FMargin(0.0f))
        .ClearTextSelectionOnFocusLoss(false)
        .SelectWordOnMouseDoubleClick(true)
        .Marshaller(RichTextMarshaller)
    ];

    RebuildContent(InArgs._MarkdownText);
}

void SAIGatewayMarkdownMessageBody::SetMarkdownText(const FString& InMarkdownText)
{
    if (bHasBuiltContent && MarkdownText == InMarkdownText)
    {
        return;
    }

    RebuildContent(InMarkdownText);
}

FString SAIGatewayMarkdownMessageBody::BuildRenderableRichText(const FString& InMarkdownText) const
{
    const FString PreprocessedMarkdown = ConvertLegacyCodeBlockTagsToMarkdown(InMarkdownText);
    const TArray<FAIGatewayMarkdownBlock> Blocks = FAIGatewayMarkdownParser::ParseBlocks(PreprocessedMarkdown);
    if (Blocks.Num() == 0)
    {
        return TEXT(" ");
    }

    FString Output;
    for (int32 Index = 0; Index < Blocks.Num(); ++Index)
    {
        const FAIGatewayMarkdownBlock& Block = Blocks[Index];
        FString HeadingContent;
        FString BlockText;

        if (FAIGatewayMarkdownRichTextRenderer::TryExtractHeadingContent(Block.Text, HeadingContent))
        {
            BlockText = FString::Printf(TEXT("<MarkdownHeading>%s</>"), *HeadingContent);
        }
        else if (Block.Type == EAIGatewayMarkdownBlockType::CodeBlock)
        {
            BlockText = RenderCodeBlockText(Block.Text);
        }
        else if (Block.Type == EAIGatewayMarkdownBlockType::Table)
        {
            BlockText = BuildTableRichText(Block);
        }
        else
        {
            BlockText = FAIGatewayMarkdownRichTextRenderer::RenderMarkdownToRichText(Block.Text, true);
        }

        if (!Output.IsEmpty())
        {
            Output.Append(TEXT("\n\n"));
        }
        Output.Append(BlockText.IsEmpty() ? TEXT(" ") : BlockText);
    }

    return Output;
}

void SAIGatewayMarkdownMessageBody::RebuildContent(const FString& InMarkdownText)
{
    MarkdownText = InMarkdownText;
    bHasBuiltContent = true;

    if (RichTextWidget.IsValid())
    {
        RichTextWidget->SetText(FText::FromString(BuildRenderableRichText(MarkdownText)));
    }
}
