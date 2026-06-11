#include "Tools/Write/EditDataTableRowTool.h"
#include "Utils/BridgeAssetModifier.h"
#include "SoftUEBridgeEditorModule.h"
#include "Engine/DataTable.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ScopedTransaction.h"

FString UEditDataTableRowTool::GetToolDescription() const
{
    return TEXT("Edit an existing row in a DataTable. Provide field_name:value pairs to update specific columns.");
}

TMap<FString, FBridgeSchemaProperty> UEditDataTableRowTool::GetInputSchema() const
{
    TMap<FString, FBridgeSchemaProperty> Schema;

    FBridgeSchemaProperty AssetPath;
    AssetPath.Type = TEXT("string");
    AssetPath.Description = TEXT("DataTable asset path");
    AssetPath.bRequired = true;
    Schema.Add(TEXT("asset_path"), AssetPath);

    FBridgeSchemaProperty RowName;
    RowName.Type = TEXT("string");
    RowName.Description = TEXT("Name of the row to edit");
    RowName.bRequired = true;
    Schema.Add(TEXT("row_name"), RowName);

    FBridgeSchemaProperty RowDataJson;
    RowDataJson.Type = TEXT("string");
    RowDataJson.Description = TEXT("JSON string with field values to update. Only provided fields are changed, others remain untouched. Example: {\"Damage\":50}");
    RowDataJson.bRequired = true;
    Schema.Add(TEXT("row_data"), RowDataJson);

    return Schema;
}

TArray<FString> UEditDataTableRowTool::GetRequiredParams() const
{
    return { TEXT("asset_path"), TEXT("row_name"), TEXT("row_data") };
}

FBridgeToolResult UEditDataTableRowTool::Execute(
    const TSharedPtr<FJsonObject>& Arguments,
    const FBridgeToolContext& Context)
{
    FString AssetPath = GetStringArgOrDefault(Arguments, TEXT("asset_path"));
    FString RowName = GetStringArgOrDefault(Arguments, TEXT("row_name"));
    FString RowDataString = GetStringArgOrDefault(Arguments, TEXT("row_data"));

    if (AssetPath.IsEmpty() || RowName.IsEmpty() || RowDataString.IsEmpty())
    {
        return FBridgeToolResult::Error(TEXT("asset_path, row_name, and row_data are required"));
    }

    TSharedPtr<FJsonObject> RowDataDelta;
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RowDataString);
        if (!FJsonSerializer::Deserialize(Reader, RowDataDelta) || !RowDataDelta.IsValid())
        {
            return FBridgeToolResult::Error(TEXT("row_data must be valid JSON string"));
        }
    }

    UE_LOG(LogSoftUEBridgeEditor, Log, TEXT("edit-datatable-row: %s in %s"), *RowName, *AssetPath);

    FString LoadError;
    UDataTable* DataTable = FBridgeAssetModifier::LoadAssetByPath<UDataTable>(AssetPath, LoadError);
    if (!DataTable)
    {
        return FBridgeToolResult::Error(LoadError);
    }

    const uint8* ExistingRow = DataTable->FindRowUnchecked(FName(*RowName));
    if (!ExistingRow)
    {
        return FBridgeToolResult::Error(FString::Printf(TEXT("Row not found: %s"), *RowName));
    }

    const UScriptStruct* RowStruct = DataTable->GetRowStruct();
    if (!RowStruct)
    {
        return FBridgeToolResult::Error(TEXT("DataTable has no row struct"));
    }

    TSharedPtr<FScopedTransaction> Transaction = FBridgeAssetModifier::BeginTransaction(
        FText::Format(NSLOCTEXT("MCP", "EditRow", "Edit row {0} in {1}"),
            FText::FromString(RowName), FText::FromString(AssetPath)));

    FBridgeAssetModifier::MarkModified(DataTable);

    uint8* RowMemory = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
    RowStruct->InitializeStruct(RowMemory);
    RowStruct->CopyScriptStruct(RowMemory, ExistingRow);

    FJsonObjectConverter::JsonObjectToUStruct(RowDataDelta.ToSharedRef(), RowStruct, RowMemory);

    DataTable->RemoveRow(FName(*RowName));
    DataTable->AddRow(FName(*RowName), *reinterpret_cast<FTableRowBase*>(RowMemory));

    RowStruct->DestroyStruct(RowMemory);
    FMemory::Free(RowMemory);

    FBridgeAssetModifier::MarkPackageDirty(DataTable);

    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("asset"), AssetPath);
    Result->SetStringField(TEXT("row_name"), RowName);
    Result->SetBoolField(TEXT("needs_save"), true);

    UE_LOG(LogSoftUEBridgeEditor, Log, TEXT("edit-datatable-row: Edited row %s"), *RowName);

    return FBridgeToolResult::Json(Result);
}
