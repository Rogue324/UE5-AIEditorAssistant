#pragma once

#include "CoreMinimal.h"
#include "Tools/BridgeToolBase.h"
#include "RemoveDataTableRowTool.generated.h"

UCLASS()
class SOFTUEBRIDGEEDITOR_API URemoveDataTableRowTool : public UBridgeToolBase
{
	GENERATED_BODY()

public:
	virtual FString GetToolName() const override { return TEXT("remove-datatable-row"); }
	virtual FString GetToolDescription() const override;
	virtual TMap<FString, FBridgeSchemaProperty> GetInputSchema() const override;
	virtual TArray<FString> GetRequiredParams() const override;
	virtual FBridgeToolResult Execute(
		const TSharedPtr<FJsonObject>& Arguments,
		const FBridgeToolContext& Context) override;
};
