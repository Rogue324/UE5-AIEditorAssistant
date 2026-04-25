// Copyright softdaddy-o 2024. All Rights Reserved.

#include "Tools/Write/AddBlueprintK2NodeTool.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "SoftUEBridgeEditorModule.h"
#include "Utils/BridgeAssetModifier.h"

namespace
{
	UClass* ResolveK2OwnerClass(const FString& ClassName)
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

	UClass* GetDefaultOwnerClass(UBlueprint* Blueprint)
	{
		if (Blueprint->GeneratedClass)
		{
			return Blueprint->GeneratedClass;
		}

		return Blueprint->ParentClass;
	}

	UFunction* FindCallableFunction(UBlueprint* Blueprint, const FString& FunctionName, const FString& OwnerClassName, FString& OutError)
	{
		UClass* OwnerClass = !OwnerClassName.IsEmpty() ? ResolveK2OwnerClass(OwnerClassName) : GetDefaultOwnerClass(Blueprint);
		if (!OwnerClass)
		{
			OutError = OwnerClassName.IsEmpty()
				? TEXT("Blueprint has no generated or parent class; compile the Blueprint first.")
				: FString::Printf(TEXT("Owner class not found: %s"), *OwnerClassName);
			return nullptr;
		}

		UFunction* Function = OwnerClass->FindFunctionByName(FName(*FunctionName), EIncludeSuperFlag::IncludeSuper);
		if (!Function)
		{
			OutError = FString::Printf(TEXT("Function '%s' was not found on class '%s'."), *FunctionName, *OwnerClass->GetName());
			return nullptr;
		}

		return Function;
	}

	FProperty* FindBlueprintVariableProperty(UBlueprint* Blueprint, const FString& VariableName)
	{
		UClass* OwnerClass = GetDefaultOwnerClass(Blueprint);
		return OwnerClass ? FindFProperty<FProperty>(OwnerClass, FName(*VariableName)) : nullptr;
	}

	void SetNodePosition(UEdGraphNode* Node, const TSharedPtr<FJsonObject>& Arguments)
	{
		FVector2D Position(0.0, 0.0);
		const TArray<TSharedPtr<FJsonValue>>* PositionArray = nullptr;
		if (Arguments.IsValid() && Arguments->TryGetArrayField(TEXT("position"), PositionArray) && PositionArray->Num() >= 2)
		{
			Position.X = (*PositionArray)[0]->AsNumber();
			Position.Y = (*PositionArray)[1]->AsNumber();
		}

		Node->NodePosX = FMath::RoundToInt(Position.X);
		Node->NodePosY = FMath::RoundToInt(Position.Y);
	}

	TSharedPtr<FJsonObject> PinToJson(const UEdGraphPin* Pin)
	{
		TSharedPtr<FJsonObject> PinObject = MakeShareable(new FJsonObject);
		PinObject->SetStringField(TEXT("name"), Pin->PinName.ToString());
		PinObject->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
		PinObject->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
		if (!Pin->PinType.PinSubCategory.IsNone())
		{
			PinObject->SetStringField(TEXT("sub_category"), Pin->PinType.PinSubCategory.ToString());
		}
		if (Pin->PinType.PinSubCategoryObject.IsValid())
		{
			PinObject->SetStringField(TEXT("sub_category_object"), Pin->PinType.PinSubCategoryObject->GetName());
		}
		return PinObject;
	}

	TSharedPtr<FJsonObject> BuildNodeResult(const FString& AssetPath, const FString& GraphName, const FString& NodeKind, UEdGraphNode* Node)
	{
		TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("asset"), AssetPath);
		Result->SetStringField(TEXT("graph"), GraphName);
		Result->SetStringField(TEXT("node_kind"), NodeKind);
		Result->SetStringField(TEXT("node_guid"), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
		Result->SetStringField(TEXT("node_class"), Node->GetClass()->GetName());
		Result->SetBoolField(TEXT("needs_compile"), true);
		Result->SetBoolField(TEXT("needs_save"), true);

		TSharedPtr<FJsonObject> PositionObject = MakeShareable(new FJsonObject);
		PositionObject->SetNumberField(TEXT("x"), Node->NodePosX);
		PositionObject->SetNumberField(TEXT("y"), Node->NodePosY);
		Result->SetObjectField(TEXT("position"), PositionObject);

		TArray<TSharedPtr<FJsonValue>> Pins;
		for (const UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin)
			{
				Pins.Add(MakeShareable(new FJsonValueObject(PinToJson(Pin))));
			}
		}
		Result->SetArrayField(TEXT("pins"), Pins);

		return Result;
	}
}

FString UAddBlueprintK2NodeTool::GetToolDescription() const
{
	return TEXT("Add a common Blueprint K2 node with correct binding. Prefer this over low-level add-graph-node for function calls and variable get/set nodes.");
}

TMap<FString, FBridgeSchemaProperty> UAddBlueprintK2NodeTool::GetInputSchema() const
{
	TMap<FString, FBridgeSchemaProperty> Schema;

	FBridgeSchemaProperty AssetPath;
	AssetPath.Type = TEXT("string");
	AssetPath.Description = TEXT("Blueprint asset path, e.g. '/Game/Blueprints/BP_MovingFloor'");
	AssetPath.bRequired = true;
	Schema.Add(TEXT("asset_path"), AssetPath);

	FBridgeSchemaProperty NodeKind;
	NodeKind.Type = TEXT("string");
	NodeKind.Description = TEXT("Node kind: 'call_function', 'variable_get', or 'variable_set'.");
	NodeKind.bRequired = true;
	Schema.Add(TEXT("node_kind"), NodeKind);

	FBridgeSchemaProperty GraphName;
	GraphName.Type = TEXT("string");
	GraphName.Description = TEXT("Blueprint graph name. Default: EventGraph.");
	GraphName.bRequired = false;
	Schema.Add(TEXT("graph_name"), GraphName);

	FBridgeSchemaProperty FunctionName;
	FunctionName.Type = TEXT("string");
	FunctionName.Description = TEXT("Function name for call_function nodes, e.g. 'SetActorLocation'.");
	FunctionName.bRequired = false;
	Schema.Add(TEXT("function_name"), FunctionName);

	FBridgeSchemaProperty OwnerClass;
	OwnerClass.Type = TEXT("string");
	OwnerClass.Description = TEXT("Optional owner class for function lookup, e.g. 'Actor', 'KismetMathLibrary', 'SceneComponent'. Defaults to the Blueprint generated/parent class.");
	OwnerClass.bRequired = false;
	Schema.Add(TEXT("owner_class"), OwnerClass);

	FBridgeSchemaProperty VariableName;
	VariableName.Type = TEXT("string");
	VariableName.Description = TEXT("Variable name for variable_get or variable_set nodes.");
	VariableName.bRequired = false;
	Schema.Add(TEXT("variable_name"), VariableName);

	FBridgeSchemaProperty Position;
	Position.Type = TEXT("array");
	Position.ItemsType = TEXT("number");
	Position.Description = TEXT("Node position as [x, y].");
	Position.bRequired = false;
	Schema.Add(TEXT("position"), Position);

	return Schema;
}

TArray<FString> UAddBlueprintK2NodeTool::GetRequiredParams() const
{
	return { TEXT("asset_path"), TEXT("node_kind") };
}

FBridgeToolResult UAddBlueprintK2NodeTool::Execute(
	const TSharedPtr<FJsonObject>& Arguments,
	const FBridgeToolContext& Context)
{
	const FString AssetPath = GetStringArgOrDefault(Arguments, TEXT("asset_path"));
	const FString NodeKind = GetStringArgOrDefault(Arguments, TEXT("node_kind")).ToLower();
	const FString GraphName = GetStringArgOrDefault(Arguments, TEXT("graph_name"), TEXT("EventGraph"));
	const FString FunctionName = GetStringArgOrDefault(Arguments, TEXT("function_name"));
	const FString OwnerClassName = GetStringArgOrDefault(Arguments, TEXT("owner_class"));
	const FString VariableName = GetStringArgOrDefault(Arguments, TEXT("variable_name"));

	if (AssetPath.IsEmpty() || NodeKind.IsEmpty())
	{
		return FBridgeToolResult::Error(TEXT("asset_path and node_kind are required"));
	}

	FString LoadError;
	UObject* Object = FBridgeAssetModifier::LoadAssetByPath(AssetPath, LoadError);
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	if (!Blueprint)
	{
		return FBridgeToolResult::Error(!LoadError.IsEmpty() ? LoadError : FString::Printf(TEXT("Failed to load Blueprint: %s"), *AssetPath));
	}

	UEdGraph* Graph = FBridgeAssetModifier::FindGraphByName(Blueprint, GraphName);
	if (!Graph)
	{
		return FBridgeToolResult::Error(FString::Printf(TEXT("Graph not found: %s"), *GraphName));
	}

	const FScopedTransaction Transaction(FText::Format(
		NSLOCTEXT("MCP", "AddBlueprintK2Node", "Add Blueprint K2 Node {0}"),
		FText::FromString(NodeKind)));

	FBridgeAssetModifier::MarkModified(Blueprint);

	UEdGraphNode* NewNode = nullptr;
	if (NodeKind == TEXT("call_function"))
	{
		if (FunctionName.IsEmpty())
		{
			return FBridgeToolResult::Error(TEXT("function_name is required for call_function nodes."));
		}

		FString FunctionError;
		UFunction* Function = FindCallableFunction(Blueprint, FunctionName, OwnerClassName, FunctionError);
		if (!Function)
		{
			return FBridgeToolResult::Error(FunctionError);
		}

		UK2Node_CallFunction* CallNode = NewObject<UK2Node_CallFunction>(Graph, NAME_None, RF_Transactional);
		CallNode->CreateNewGuid();
		CallNode->SetFromFunction(Function);
		SetNodePosition(CallNode, Arguments);
		CallNode->AllocateDefaultPins();
		Graph->AddNode(CallNode, false, false);
		NewNode = CallNode;
	}
	else if (NodeKind == TEXT("variable_get") || NodeKind == TEXT("variable_set"))
	{
		if (VariableName.IsEmpty())
		{
			return FBridgeToolResult::Error(TEXT("variable_name is required for variable_get and variable_set nodes."));
		}

		FProperty* VariableProperty = FindBlueprintVariableProperty(Blueprint, VariableName);
		if (!VariableProperty)
		{
			return FBridgeToolResult::Error(FString::Printf(TEXT("Variable '%s' was not found. Create it first with add-blueprint-variable."), *VariableName));
		}

		UK2Node_Variable* VariableNode = nullptr;
		if (NodeKind == TEXT("variable_get"))
		{
			VariableNode = NewObject<UK2Node_VariableGet>(Graph, NAME_None, RF_Transactional);
		}
		else
		{
			VariableNode = NewObject<UK2Node_VariableSet>(Graph, NAME_None, RF_Transactional);
		}

		VariableNode->CreateNewGuid();
		VariableNode->VariableReference.SetFromField<FProperty>(VariableProperty, GetDefaultOwnerClass(Blueprint));
		SetNodePosition(VariableNode, Arguments);
		VariableNode->AllocateDefaultPins();
		Graph->AddNode(VariableNode, false, false);
		NewNode = VariableNode;
	}
	else
	{
		return FBridgeToolResult::Error(TEXT("Unsupported node_kind. Use 'call_function', 'variable_get', or 'variable_set'."));
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FBridgeAssetModifier::MarkPackageDirty(Blueprint);

	return FBridgeToolResult::Json(BuildNodeResult(AssetPath, GraphName, NodeKind, NewNode));
}
