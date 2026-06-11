#include "Tools/Write/RemoveDataTableRowTool.h"
#include "Utils/BridgeAssetModifier.h"
#include "SoftUEBridgeEditorModule.h"
#include "Engine/DataTable.h"
#include "ScopedTransaction.h"

FString URemoveDataTableRowTool::GetToolDescription() const
{
    return TEXT("Remove a row from a DataTable by row name.");
}

TMap<FString, FBridgeSchemaProperty> URemoveDataTableRowTool::GetInputSchema() const
{
    TMap<FString, FBridgeSchemaProperty> Schema;

    FBridgeSchemaProperty AssetPath;
    AssetPath.Type = TEXT("string");
    AssetPath.Description = TEXT("DataTable asset path");
    AssetPath.bRequired = true;
    Schema.Add(TEXT("asset_path"), AssetPath);

    FBridgeSchemaProperty RowName;
    RowName.Type = TEXT("string");
    RowName.Description = TEXT("Name of the row to remove");
    RowName.bRequired = true;
    Schema.Add(TEXT("row_name"), RowName);

    return Schema;
}

TArray<FString> URemoveDataTableRowTool::GetRequiredParams() const
{
    return { TEXT("asset_path"), TEXT("row_name") };
}

FBridgeToolResult URemoveDataTableRowTool::Execute(
    const TSharedPtr<FJsonObject>& Arguments,
    const FBridgeToolContext& Context)
{
    FString AssetPath = GetStringArgOrDefault(Arguments, TEXT("asset_path"));
    FString RowName = GetStringArgOrDefault(Arguments, TEXT("row_name"));

    if (AssetPath.IsEmpty() || RowName.IsEmpty())
    {
        return FBridgeToolResult::Error(TEXT("asset_path and row_name are required"));
    }

    UE_LOG(LogSoftUEBridgeEditor, Log, TEXT("remove-datatable-row: %s from %s"), *RowName, *AssetPath);

    FString LoadError;
    UDataTable* DataTable = FBridgeAssetModifier::LoadAssetByPath<UDataTable>(AssetPath, LoadError);
    if (!DataTable)
    {
        return FBridgeToolResult::Error(LoadError);
    }

    if (!DataTable->FindRowUnchecked(FName(*RowName)))
    {
        return FBridgeToolResult::Error(FString::Printf(TEXT("Row not found: %s"), *RowName));
    }

    TSharedPtr<FScopedTransaction> Transaction = FBridgeAssetModifier::BeginTransaction(
        FText::Format(NSLOCTEXT("MCP", "RemoveRow", "Remove row {0} from {1}"),
            FText::FromString(RowName), FText::FromString(AssetPath)));

    FBridgeAssetModifier::MarkModified(DataTable);
    DataTable->RemoveRow(FName(*RowName));
    FBridgeAssetModifier::MarkPackageDirty(DataTable);

    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("asset"), AssetPath);
    Result->SetStringField(TEXT("row_name"), RowName);
    Result->SetBoolField(TEXT("needs_save"), true);

    UE_LOG(LogSoftUEBridgeEditor, Log, TEXT("remove-datatable-row: Removed row %s"), *RowName);

    return FBridgeToolResult::Json(Result);
}
