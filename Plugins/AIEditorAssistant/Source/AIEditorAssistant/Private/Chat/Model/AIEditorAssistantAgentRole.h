#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"

namespace AIEditorAssistantAgentRoles
{
	const FString RoleIdGeneral(TEXT("general"));
	const FString RoleIdLevelDesigner(TEXT("level-designer"));
	const FString RoleIdBlueprint(TEXT("blueprint"));
	const FString RoleIdAssetManager(TEXT("asset-manager"));
	const FString RoleIdPerformance(TEXT("performance"));
}

struct FAIEditorAssistantAgentRoleDefinition
{
	FString RoleId;
	FString DisplayName;
	FString SystemPrompt;
	TSet<FString> ToolNames;
};

TArray<FAIEditorAssistantAgentRoleDefinition> GetPredefinedAgentRoles();

const FAIEditorAssistantAgentRoleDefinition* FindAgentRole(const FString& RoleId);
