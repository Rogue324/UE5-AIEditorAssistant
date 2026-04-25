// Copyright softdaddy-o 2024. All Rights Reserved.

#include "Tools/Write/AddBlueprintVariableTool.h"

#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/DefaultValueHelper.h"
#include "ScopedTransaction.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SoftUEBridgeEditorModule.h"
#include "UObject/NoExportTypes.h"

namespace
{
	FString GetJsonFieldAsDefaultString(const TSharedPtr<FJsonObject>& Arguments, const FString& FieldName)
	{
		const TSharedPtr<FJsonValue> Value = Arguments.IsValid() ? Arguments->TryGetField(FieldName) : nullptr;
		if (!Value.IsValid() || Value->IsNull())
		{
			return FString();
		}

		FString StringValue;
		if (Value->TryGetString(StringValue))
		{
			return StringValue;
		}

		double NumberValue = 0.0;
		if (Value->TryGetNumber(NumberValue))
		{
			return FString::SanitizeFloat(NumberValue);
		}

		bool BoolValue = false;
		if (Value->TryGetBool(BoolValue))
		{
			return BoolValue ? TEXT("true") : TEXT("false");
		}

		FString SerializedValue;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedValue);
		FJsonSerializer::Serialize(Value.ToSharedRef(), TEXT(""), Writer);
		return SerializedValue;
	}

	UClass* ResolveClass(const FString& ClassName)
	{
		if (ClassName.IsEmpty())
		{
			return nullptr;
		}

		UClass* Class = LoadClass<UObject>(nullptr, *ClassName);
		if (!Class && !ClassName.EndsWith(TEXT("_C")))
		{
			Class = LoadClass<UObject>(nullptr, *(ClassName + TEXT("_C")));
		}
		if (!Class)
		{
			Class = FindFirstObject<UClass>(*ClassName, EFindFirstObjectOptions::ExactClass);
		}
		if (!Class && !ClassName.StartsWith(TEXT("U")))
		{
			Class = FindFirstObject<UClass>(*(TEXT("U") + ClassName), EFindFirstObjectOptions::ExactClass);
		}
		if (!Class && !ClassName.StartsWith(TEXT("A")))
		{
			Class = FindFirstObject<UClass>(*(TEXT("A") + ClassName), EFindFirstObjectOptions::ExactClass);
		}

		return Class;
	}

	UEnum* ResolveEnum(const FString& EnumName)
	{
		if (EnumName.IsEmpty())
		{
			return nullptr;
		}

		UEnum* Enum = LoadObject<UEnum>(nullptr, *EnumName);
		if (!Enum)
		{
			Enum = FindFirstObject<UEnum>(*EnumName, EFindFirstObjectOptions::ExactClass);
		}

		return Enum;
	}

	bool BuildPinType(const FString& InType, const FString& InSubType, const FString& InContainer, FEdGraphPinType& OutPinType, FString& OutError)
	{
		const FString Type = InType.ToLower();
		const FString Container = InContainer.ToLower();
		OutPinType = FEdGraphPinType();

		if (Container.IsEmpty() || Container == TEXT("scalar") || Container == TEXT("none"))
		{
			OutPinType.ContainerType = EPinContainerType::None;
		}
		else if (Container == TEXT("array"))
		{
			OutPinType.ContainerType = EPinContainerType::Array;
		}
		else if (Container == TEXT("set"))
		{
			OutPinType.ContainerType = EPinContainerType::Set;
		}
		else
		{
			OutError = TEXT("Unsupported container. Use 'scalar', 'array', or 'set'.");
			return false;
		}

		if (Type == TEXT("bool") || Type == TEXT("boolean"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		}
		else if (Type == TEXT("int") || Type == TEXT("integer"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		}
		else if (Type == TEXT("int64") || Type == TEXT("integer64"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Int64;
		}
		else if (Type == TEXT("float"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Real;
			OutPinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
		}
		else if (Type == TEXT("double") || Type == TEXT("real"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Real;
			OutPinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
		}
		else if (Type == TEXT("string"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_String;
		}
		else if (Type == TEXT("name"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Name;
		}
		else if (Type == TEXT("text"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Text;
		}
		else if (Type == TEXT("vector"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutPinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
		}
		else if (Type == TEXT("rotator"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutPinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
		}
		else if (Type == TEXT("transform"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutPinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
		}
		else if (Type == TEXT("linearcolor") || Type == TEXT("color"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutPinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
		}
		else if (Type == TEXT("vector2d"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			OutPinType.PinSubCategoryObject = TBaseStructure<FVector2D>::Get();
		}
		else if (Type == TEXT("object") || Type == TEXT("actor") || Type == TEXT("component"))
		{
			UClass* ObjectClass = ResolveClass(InSubType.IsEmpty() ? (Type == TEXT("actor") ? TEXT("Actor") : TEXT("Object")) : InSubType);
			if (!ObjectClass)
			{
				OutError = FString::Printf(TEXT("Object class not found: %s"), *InSubType);
				return false;
			}

			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
			OutPinType.PinSubCategoryObject = ObjectClass;
		}
		else if (Type == TEXT("class"))
		{
			UClass* ObjectClass = ResolveClass(InSubType.IsEmpty() ? TEXT("Object") : InSubType);
			if (!ObjectClass)
			{
				OutError = FString::Printf(TEXT("Class type not found: %s"), *InSubType);
				return false;
			}

			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Class;
			OutPinType.PinSubCategoryObject = ObjectClass;
		}
		else if (Type == TEXT("enum"))
		{
			UEnum* Enum = ResolveEnum(InSubType);
			if (!Enum)
			{
				OutError = FString::Printf(TEXT("Enum not found: %s"), *InSubType);
				return false;
			}

			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
			OutPinType.PinSubCategoryObject = Enum;
		}
		else
		{
			OutError = FString::Printf(TEXT("Unsupported variable type '%s'."), *InType);
			return false;
		}

		return true;
	}
}

FString UAddBlueprintVariableTool::GetToolDescription() const
{
	return TEXT("Add a member variable to a Blueprint asset. Use this for creating Blueprint variables; do not use Python for variable creation.");
}

TMap<FString, FBridgeSchemaProperty> UAddBlueprintVariableTool::GetInputSchema() const
{
	TMap<FString, FBridgeSchemaProperty> Schema;

	FBridgeSchemaProperty AssetPath;
	AssetPath.Type = TEXT("string");
	AssetPath.Description = TEXT("Blueprint asset path, e.g. '/Game/Blueprints/BP_Player'");
	AssetPath.bRequired = true;
	Schema.Add(TEXT("asset_path"), AssetPath);

	FBridgeSchemaProperty VariableName;
	VariableName.Type = TEXT("string");
	VariableName.Description = TEXT("New variable name.");
	VariableName.bRequired = true;
	Schema.Add(TEXT("variable_name"), VariableName);

	FBridgeSchemaProperty VariableType;
	VariableType.Type = TEXT("string");
	VariableType.Description = TEXT("Variable type: bool, int, int64, float, double, string, name, text, vector, rotator, transform, linearcolor, vector2d, object, actor, component, class, or enum.");
	VariableType.bRequired = true;
	Schema.Add(TEXT("variable_type"), VariableType);

	FBridgeSchemaProperty SubType;
	SubType.Type = TEXT("string");
	SubType.Description = TEXT("Class, object, or enum subtype. Examples: Actor, StaticMeshComponent, /Game/BP_Item.BP_Item_C, /Script/Engine.StaticMesh.");
	SubType.bRequired = false;
	Schema.Add(TEXT("sub_type"), SubType);

	FBridgeSchemaProperty Container;
	Container.Type = TEXT("string");
	Container.Description = TEXT("Container type: scalar, array, or set. Default: scalar.");
	Container.bRequired = false;
	Schema.Add(TEXT("container"), Container);

	FBridgeSchemaProperty DefaultValue;
	DefaultValue.Type = TEXT("any");
	DefaultValue.Description = TEXT("Optional default value. Strings can use UE default syntax for structs, e.g. '(X=0,Y=0,Z=0)'.");
	DefaultValue.bRequired = false;
	Schema.Add(TEXT("default_value"), DefaultValue);

	FBridgeSchemaProperty Category;
	Category.Type = TEXT("string");
	Category.Description = TEXT("Optional Blueprint variable category.");
	Category.bRequired = false;
	Schema.Add(TEXT("category"), Category);

	FBridgeSchemaProperty Tooltip;
	Tooltip.Type = TEXT("string");
	Tooltip.Description = TEXT("Optional tooltip metadata.");
	Tooltip.bRequired = false;
	Schema.Add(TEXT("tooltip"), Tooltip);

	FBridgeSchemaProperty InstanceEditable;
	InstanceEditable.Type = TEXT("boolean");
	InstanceEditable.Description = TEXT("If true, makes the variable instance editable.");
	InstanceEditable.bRequired = false;
	Schema.Add(TEXT("instance_editable"), InstanceEditable);

	FBridgeSchemaProperty BlueprintReadOnly;
	BlueprintReadOnly.Type = TEXT("boolean");
	BlueprintReadOnly.Description = TEXT("If true, marks the variable Blueprint read-only.");
	BlueprintReadOnly.bRequired = false;
	Schema.Add(TEXT("blueprint_read_only"), BlueprintReadOnly);

	FBridgeSchemaProperty ExposeOnSpawn;
	ExposeOnSpawn.Type = TEXT("boolean");
	ExposeOnSpawn.Description = TEXT("If true, marks the variable Expose on Spawn and instance editable.");
	ExposeOnSpawn.bRequired = false;
	Schema.Add(TEXT("expose_on_spawn"), ExposeOnSpawn);

	FBridgeSchemaProperty SaveGame;
	SaveGame.Type = TEXT("boolean");
	SaveGame.Description = TEXT("If true, sets the SaveGame flag.");
	SaveGame.bRequired = false;
	Schema.Add(TEXT("save_game"), SaveGame);

	FBridgeSchemaProperty AdvancedDisplay;
	AdvancedDisplay.Type = TEXT("boolean");
	AdvancedDisplay.Description = TEXT("If true, marks the variable Advanced Display.");
	AdvancedDisplay.bRequired = false;
	Schema.Add(TEXT("advanced_display"), AdvancedDisplay);

	return Schema;
}

TArray<FString> UAddBlueprintVariableTool::GetRequiredParams() const
{
	return { TEXT("asset_path"), TEXT("variable_name"), TEXT("variable_type") };
}

FBridgeToolResult UAddBlueprintVariableTool::Execute(
	const TSharedPtr<FJsonObject>& Arguments,
	const FBridgeToolContext& Context)
{
	const FString AssetPath = GetStringArgOrDefault(Arguments, TEXT("asset_path"));
	const FString VariableNameString = GetStringArgOrDefault(Arguments, TEXT("variable_name"));
	const FString VariableTypeString = GetStringArgOrDefault(Arguments, TEXT("variable_type"));
	const FString SubTypeString = GetStringArgOrDefault(Arguments, TEXT("sub_type"));
	const FString ContainerString = GetStringArgOrDefault(Arguments, TEXT("container"), TEXT("scalar"));
	const FString CategoryString = GetStringArgOrDefault(Arguments, TEXT("category"));
	const FString TooltipString = GetStringArgOrDefault(Arguments, TEXT("tooltip"));
	const bool bInstanceEditable = GetBoolArgOrDefault(Arguments, TEXT("instance_editable"), false);
	const bool bBlueprintReadOnly = GetBoolArgOrDefault(Arguments, TEXT("blueprint_read_only"), false);
	const bool bExposeOnSpawn = GetBoolArgOrDefault(Arguments, TEXT("expose_on_spawn"), false);
	const bool bSaveGame = GetBoolArgOrDefault(Arguments, TEXT("save_game"), false);
	const bool bAdvancedDisplay = GetBoolArgOrDefault(Arguments, TEXT("advanced_display"), false);

	if (AssetPath.IsEmpty() || VariableNameString.IsEmpty() || VariableTypeString.IsEmpty())
	{
		return FBridgeToolResult::Error(TEXT("asset_path, variable_name, and variable_type are required"));
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		return FBridgeToolResult::Error(FString::Printf(TEXT("Failed to load Blueprint: %s"), *AssetPath));
	}

	const FName VariableName(*VariableNameString);
	if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, VariableName) != INDEX_NONE)
	{
		return FBridgeToolResult::Error(FString::Printf(TEXT("Blueprint already has a variable named '%s'."), *VariableNameString));
	}

	FEdGraphPinType PinType;
	FString TypeError;
	if (!BuildPinType(VariableTypeString, SubTypeString, ContainerString, PinType, TypeError))
	{
		return FBridgeToolResult::Error(TypeError);
	}

	const FString DefaultValue = GetJsonFieldAsDefaultString(Arguments, TEXT("default_value"));

	UE_LOG(LogSoftUEBridgeEditor, Log, TEXT("add-blueprint-variable: %s.%s type=%s subtype=%s container=%s"),
		*AssetPath, *VariableNameString, *VariableTypeString, *SubTypeString, *ContainerString);

	const FScopedTransaction Transaction(FText::Format(
		NSLOCTEXT("MCP", "AddBlueprintVariable", "Add Blueprint Variable {0}"),
		FText::FromString(VariableNameString)));

	Blueprint->Modify();

	if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, PinType, DefaultValue))
	{
		return FBridgeToolResult::Error(FString::Printf(TEXT("Failed to add Blueprint variable '%s'."), *VariableNameString));
	}

	if (!CategoryString.IsEmpty())
	{
		FBlueprintEditorUtils::SetBlueprintVariableCategory(Blueprint, VariableName, nullptr, FText::FromString(CategoryString));
	}

	if (!TooltipString.IsEmpty())
	{
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VariableName, nullptr, TEXT("tooltip"), TooltipString);
	}

	if (uint64* PropertyFlags = FBlueprintEditorUtils::GetBlueprintVariablePropertyFlags(Blueprint, VariableName))
	{
		*PropertyFlags |= CPF_BlueprintVisible;

		if (bInstanceEditable || bExposeOnSpawn)
		{
			*PropertyFlags |= CPF_Edit;
		}

		if (bBlueprintReadOnly)
		{
			*PropertyFlags |= CPF_BlueprintReadOnly;
		}

		if (bExposeOnSpawn)
		{
			*PropertyFlags |= CPF_ExposeOnSpawn;
		}
	}

	if (bSaveGame)
	{
		FBlueprintEditorUtils::SetVariableSaveGameFlag(Blueprint, VariableName, true);
	}

	if (bAdvancedDisplay)
	{
		FBlueprintEditorUtils::SetVariableAdvancedDisplayFlag(Blueprint, VariableName, true);
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	Blueprint->MarkPackageDirty();

	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
	Result->SetBoolField(TEXT("success"), true);
	Result->SetStringField(TEXT("asset_path"), AssetPath);
	Result->SetStringField(TEXT("variable_name"), VariableNameString);
	Result->SetStringField(TEXT("variable_type"), PinType.PinCategory.ToString());
	Result->SetStringField(TEXT("container"), ContainerString);
	if (PinType.PinSubCategoryObject.IsValid())
	{
		Result->SetStringField(TEXT("sub_type"), PinType.PinSubCategoryObject->GetName());
	}
	Result->SetBoolField(TEXT("needs_save"), true);

	return FBridgeToolResult::Json(Result);
}
