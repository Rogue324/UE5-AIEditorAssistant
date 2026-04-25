#pragma once

#include "CoreMinimal.h"

enum class EAIEditorAssistantMarkdownBlockType : uint8
{
    Paragraph,
    CodeBlock,
    Table
};

struct FAIEditorAssistantMarkdownBlock
{
    EAIEditorAssistantMarkdownBlockType Type = EAIEditorAssistantMarkdownBlockType::Paragraph;
    FString Text;
    TArray<TArray<FString>> TableRows;
};

class FAIEditorAssistantMarkdownParser
{
public:
    static FString NormalizeLineEndings(const FString& InText);
    static TArray<FAIEditorAssistantMarkdownBlock> ParseBlocks(const FString& MarkdownText);
};
