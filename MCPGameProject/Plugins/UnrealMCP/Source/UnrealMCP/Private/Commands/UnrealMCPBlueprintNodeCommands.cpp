#include "Commands/UnrealMCPBlueprintNodeCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/GameplayStatics.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_Select.h"
#include "K2Node_SwitchEnum.h"
#include "K2Node_VariableSet.h"
#include "Kismet/BlueprintInstancedStructLibrary.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/BlueprintPathsLibrary.h"
#include "Kismet/BlueprintPlatformLibrary.h"
#include "Kismet/BlueprintSetLibrary.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/ImportanceSamplingLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetGuidLibrary.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetNodeHelperLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetStringTableLibrary.h"
#include "Kismet/KismetTextLibrary.h"

// Declare the log category
DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCP, Log, All);

FUnrealMCPBlueprintNodeCommands::FUnrealMCPBlueprintNodeCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("connect_blueprint_nodes"))
    {
        return HandleConnectBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_blueprint_get_self_component_reference"))
    {
        return HandleAddBlueprintGetSelfComponentReference(Params);
    }
    else if (CommandType == TEXT("add_event_node"))
    {
        return HandleAddEventNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_member_variable"))
    {
        return HandleAddBlueprintMemberVariable(Params);
    }
    else if (CommandType == TEXT("add_blueprint_local_variable"))
    {
        return HandleAddBlueprintLocalVariable(Params);
    }
    else if (CommandType == TEXT("add_blueprint_input_action_node"))
    {
        return HandleAddBlueprintInputActionNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_self_reference"))
    {
        return HandleAddBlueprintSelfReference(Params);
    }
    else if (CommandType == TEXT("find_blueprint_nodes"))
    {
        return HandleFindBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_function_call_node"))
    {
        return HandleAddFunctionCallNode(Params);
    }
    else if (CommandType == TEXT("add_math_node"))
    {
        return HandleAddMathNode(Params);
    }
    else if (CommandType == TEXT("add_control_node"))
    {
        return HandleAddControlNode(Params);
    }
    else if (CommandType == TEXT("add_sequence_node"))
    {
        return HandleAddSequenceNode(Params);
    }
    else if (CommandType == TEXT("add_select_node"))
    {
        return HandleAddSelectNode(Params);
    }
    else if (CommandType == TEXT("add_enum_switch_node"))
    {
        return HandleAddEnumSwitchNode(Params);
    }
    else if (CommandType == TEXT("add_make_struct_node"))
    {
        return HandleAddMakeStructNode(Params);
    }
    else if (CommandType == TEXT("add_break_struct_node"))
    {
        return HandleAddBreakStructNode(Params);
    }
    else if (CommandType == TEXT("find_event_node_by_name"))
    {
        return HandleFindEventNodeByName(Params);
    }
    else if (CommandType == TEXT("get_node_pins"))
    {
        return HandleGetNodePins(Params);
    }
    else if (CommandType == TEXT("set_node_pin_default_value"))
    {
        return HandleSetNodePinDefaultValue(Params);
    }
    else if (CommandType == TEXT("get_all_nodes"))
    {
        return HandleGetAllNodes(Params);
    }
    else if (CommandType == TEXT("add_variable_get_node"))
    {
        return HandleAddVariableGetNode(Params);
    }
    else if (CommandType == TEXT("add_variable_set_node"))
    {
        return HandleAddVariableSetNode(Params);
    }
    else if (CommandType == TEXT("add_component_node"))
    {
        return HandleAddComponentNode(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown blueprint node command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintGetSelfComponentReference(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }
    // We'll skip component verification since the GetAllNodes API may have changed in UE5.5
    
    // Create the variable get node directly
    UK2Node_VariableGet* GetComponentNode = NewObject<UK2Node_VariableGet>(EventGraph);
    if (!GetComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create get component node"));
    }
    
    // Set up the variable reference properly for UE5.5
    FMemberReference& VarRef = GetComponentNode->VariableReference;
    VarRef.SetSelfMember(FName(*ComponentName));
    
    // Set node position
    GetComponentNode->NodePosX = NodePosition.X;
    GetComponentNode->NodePosY = NodePosition.Y;
    
    // Add to graph
    EventGraph->AddNode(GetComponentNode);
    GetComponentNode->CreateNewGuid();
    GetComponentNode->PostPlacedNewNode();
    GetComponentNode->AllocateDefaultPins();
    
    // Explicitly reconstruct node for UE5.5
    GetComponentNode->ReconstructNode();
    
    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), GetComponentNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddEventNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString EventName;
    if (!Params->TryGetStringField(TEXT("event_name"), EventName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Create the event node
    UK2Node_Event* EventNode = FUnrealMCPCommonUtils::CreateEventNode(EventGraph, EventName, NodePosition);
    if (!EventNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create event node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintMemberVariable(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    FString VariableType;
    if (!Params->TryGetStringField(TEXT("variable_type"), VariableType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_type' parameter"));
    }
    FString VariablePinType;
    if (!Params->TryGetStringField(TEXT("variable_pin_type"), VariablePinType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_pin_type' parameter"));
    }
    // Get optional parameters
    bool IsExposed = false;
    if (Params->HasField(TEXT("is_exposed")))
    {
        IsExposed = Params->GetBoolField(TEXT("is_exposed"));
    }    

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    FKB_FunctionPinInformations VariableInfo;
    VariableInfo.Name = FName(VariableName);
    VariableInfo.Type = FUnrealMCPCommonUtils::GetVariableTypeFromString(VariableType);
    VariableInfo.VarType = FUnrealMCPCommonUtils::GetVariablePinTypeFromString(VariablePinType);

    if (FUnrealMCPCommonUtils::CreateMemberVariable(Blueprint, VariableInfo))
    {
        // Set variable properties
        FBPVariableDescription* NewVar = nullptr;
        for (FBPVariableDescription& Variable : Blueprint->NewVariables)
        {
            if (Variable.VarName == FName(*VariableName))
            {
                NewVar = &Variable;
                break;
            }
        }
        if (NewVar)
        {
            // Set exposure in editor
            if (IsExposed)
            {
                NewVar->PropertyFlags |= CPF_Edit;
            }
        }
        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("variable_name"), VariableName);
        ResultObj->SetStringField(TEXT("variable_type"), VariableType);
        return ResultObj;
    }
    return FUnrealMCPCommonUtils::CreateErrorResponse(
        FString::Printf(TEXT("Failed to create member variable: %s of type %s"), *VariableName, *VariableType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintLocalVariable(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }
    FString VariableType;
    if (!Params->TryGetStringField(TEXT("variable_type"), VariableType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_type' parameter"));
    }
    FString VariablePinType;
    if (!Params->TryGetStringField(TEXT("variable_pin_type"), VariablePinType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_pin_type' parameter"));
    }
    FKB_FunctionPinInformations VariableInfo;
    VariableInfo.Name = FName(VariableName);
    VariableInfo.Type = FUnrealMCPCommonUtils::GetVariableTypeFromString(VariableType);
    VariableInfo.VarType = FUnrealMCPCommonUtils::GetVariablePinTypeFromString(VariablePinType);

    if (FUnrealMCPCommonUtils::CreateLocalVariable(Blueprint, EventGraph, VariableInfo))
    {
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("variable_name"), VariableName);
        ResultObj->SetStringField(TEXT("variable_type"), VariableType);
        return ResultObj;
    }
    return FUnrealMCPCommonUtils::CreateErrorResponse(
        FString::Printf(TEXT("Failed to create local variable: %s of type %s"), *VariableName, *VariableType));
    
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintInputActionNode(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the input action node
    UK2Node_InputAction* InputActionNode = FUnrealMCPCommonUtils::CreateInputActionNode(EventGraph, ActionName, NodePosition);
    if (!InputActionNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create input action node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), InputActionNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintSelfReference(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the self node
    UK2Node_Self* SelfNode = FUnrealMCPCommonUtils::CreateSelfReferenceNode(EventGraph, NodePosition);
    if (!SelfNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create self node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), SelfNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleFindBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString NodeType;
    if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_type' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create a JSON array for the node GUIDs
    TArray<TSharedPtr<FJsonValue>> NodeGuidArray;
    
    // Filter nodes by the exact requested type
    if (NodeType == TEXT("Event"))
    {
        FString EventName;
        if (!Params->TryGetStringField(TEXT("event_name"), EventName))
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter for Event node search"));
        }
        
        // Look for nodes with exact event name (e.g., ReceiveBeginPlay)
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
            if (EventNode && EventNode->EventReference.GetMemberName() == FName(*EventName))
            {
                UE_LOG(LogTemp, Display, TEXT("Found event node with name %s: %s"), *EventName, *EventNode->NodeGuid.ToString());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(EventNode->NodeGuid.ToString()));
            }
        }
    }
    // Add other node types as needed (InputAction, etc.)
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("node_guids"), NodeGuidArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddFunctionCallNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }
    // Check for target parameter (optional)
    FString TargetClassName;
    Params->TryGetStringField(TEXT("target_class"), TargetClassName);
    FString TargetFunctionName;
    Params->TryGetStringField(TEXT("target_function"), TargetFunctionName);

    // Try to find the target class
    UClass* TargetClass = nullptr;
        
    // First try without a prefix
    TargetClass = FindFirstObjectSafe<UClass>(*TargetClassName);
    UE_LOG(LogTemp, Display, TEXT("Tried to find class '%s': %s"), *TargetClassName, TargetClass ? TEXT("Found") : TEXT("Not found"));
        
    // If not found, try with U prefix (common convention for UE classes)
    if (!TargetClass && !TargetClassName.StartsWith(TEXT("U")))
    {
        FString TargetWithPrefix = FString(TEXT("U")) + TargetClassName;
        TargetClass = FindFirstObjectSafe<UClass>(*TargetWithPrefix);
        UE_LOG(LogTemp, Display, TEXT("Tried to find class '%s': %s"), *TargetWithPrefix, TargetClass ? TEXT("Found") : TEXT("Not found"));
    }
    // Special case handling for common classes like UGameplayStatics
    if (!TargetClass)
    {
        if (TargetClassName == TEXT("UGameplayStatics"))
        {
            TargetClass = UGameplayStatics::StaticClass();
        }
        else if (TargetClassName == TEXT("UImportanceSamplingLibrary"))
        {
            TargetClass = UImportanceSamplingLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UBlueprintInstancedStructLibrary"))
        {
            TargetClass = UBlueprintInstancedStructLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UBlueprintMapLibrary"))
        {
            TargetClass = UBlueprintMapLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UBlueprintPathsLibrary"))
        {
            TargetClass = UBlueprintPathsLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UBlueprintPlatformLibrary"))
        {
            TargetClass = UBlueprintPlatformLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UBlueprintSetLibrary"))
        {
            TargetClass = UBlueprintSetLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UDataTableFunctionLibrary"))
        {
            TargetClass = UDataTableFunctionLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetArrayLibrary"))
        {
            TargetClass = UKismetArrayLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetGuidLibrary"))
        {
            TargetClass = UKismetGuidLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetInputLibrary"))
        {
            TargetClass = UKismetInputLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetInternationalizationLibrary"))
        {
            TargetClass = UKismetInternationalizationLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetMaterialLibrary"))
        {
            TargetClass = UKismetMaterialLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetMathLibrary"))
        {
            TargetClass = UKismetMathLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetNodeHelperLibrary"))
        {
            TargetClass = UKismetNodeHelperLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetRenderingLibrary"))
        {
            TargetClass = UKismetRenderingLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetStringLibrary"))
        {
            TargetClass = UKismetStringLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetStringTableLibrary"))
        {
            TargetClass = UKismetStringTableLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetSystemLibrary"))
        {
            TargetClass = UKismetSystemLibrary::StaticClass();
        }
        else if (TargetClassName == TEXT("UKismetTextLibrary"))
        {
            TargetClass = UKismetTextLibrary::StaticClass();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find target class %s"), *TargetClassName);
        }
    }
    
    if (!TargetClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to find target class %s"), *TargetClassName));
    }

    UEdGraphNode* FunctionNode = nullptr;
    FUnrealMCPCommonUtils::SpawnFunctionCallNode(EventGraph, FName(TargetFunctionName), TargetClass, FunctionNode);
    if (FunctionNode != nullptr)
    {        
        UE_LOG(LogTemp, Display, TEXT("Created function call node for %s in graph %s at position (%f, %f)"), 
               *TargetFunctionName, *EventGraph->GetName(), NodePosition.X, NodePosition.Y);
        
        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), FunctionNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create function call node for %s"), *TargetFunctionName));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddMathNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString OperationStr;
    if (!Params->TryGetStringField(TEXT("operation"), OperationStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'operation' parameter"));
    }
    EArithmeticOperation operation = FUnrealMCPCommonUtils::GetArithmeticOperationFromString(OperationStr);

    FString DataTypeStr;
    if (!Params->TryGetStringField(TEXT("data_type"), DataTypeStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'data_type' parameter"));
    }
    EArithmeticDataType dataType = FUnrealMCPCommonUtils::GetArithmeticDataTypeFromString(DataTypeStr);

    UEdGraphNode* FunctionNode = nullptr;
    FUnrealMCPCommonUtils::SpawnMathNode(EventGraph, operation, dataType, FunctionNode);
    if (FunctionNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created math node for %s in graph %s"), 
               *OperationStr, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), FunctionNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create math node for %s"), *OperationStr));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddControlNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString ControlTypeStr;
    if (!Params->TryGetStringField(TEXT("control_type"), ControlTypeStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'control_type' parameter"));
    }
    EK2NodeType nodeType = FUnrealMCPCommonUtils::GetK2NodeTypeFromString(ControlTypeStr);
    
    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnNodeByType(EventGraph, nodeType, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created control node for %s in graph %s"), *ControlTypeStr, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create control node for %s"), *ControlTypeStr));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddSequenceNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnSequenceNode(EventGraph, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created sequence node in graph %s"), *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create sequence node"));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddSelectNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    // FString PinTypeStr;
    // if (!Params->TryGetStringField(TEXT("pin_type"), PinTypeStr))
    // {
    //     return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pin_type' parameter"));
    // }
    // ESelectPinType PinType = FUnrealMCPCommonUtils::GetSelectPinTypeFromString(PinTypeStr);

    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnSelectNode(EventGraph, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created select node in graph %s"), *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create select node"));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddEnumSwitchNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString EnumPath;
    if (!Params->TryGetStringField(TEXT("enum_path"), EnumPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'enum_path' parameter"));
    }
    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnEnumSwitch(EventGraph, EnumPath, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created enum switch node for %s in graph %s"), *EnumPath, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create enum switch node for %s"), *EnumPath));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddMakeStructNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }
    UScriptStruct* StructType = LoadObject<UScriptStruct>(nullptr, *StructPath);
    if (!StructType)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load struct at path %s"), *StructPath));
    }
    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnStructNode(EventGraph, StructType, true, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created make struct node for %s in graph %s"), *StructPath, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create make struct node for %s"), *StructPath));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBreakStructNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }
    UScriptStruct* StructType = LoadObject<UScriptStruct>(nullptr, *StructPath);
    if (!StructType)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load struct at path %s"), *StructPath));
    }
    UEdGraphNode* NewNode = nullptr;
    FUnrealMCPCommonUtils::SpawnStructNode(EventGraph, StructType, false, NewNode);

    if (NewNode != nullptr)
    {
        UE_LOG(LogTemp, Display, TEXT("Created make struct node for %s in graph %s"), *StructPath, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create make struct node for %s"), *StructPath));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddVariableGetNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }
    UEdGraphNode* VariableNode = nullptr;
    if (FUnrealMCPCommonUtils::SpawnVariableNode(EventGraph, FName(VariableName), true, EVariableScopeType::Global, EVariableOperateType::GetValue, VariableNode))
    {
        UE_LOG(LogTemp, Display, TEXT("Created variable get node for %s in graph %s"), *VariableName, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), VariableNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create variable get node for %s"), *VariableName));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddVariableSetNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }
    UEdGraphNode* VariableNode = nullptr;
    if (FUnrealMCPCommonUtils::SpawnVariableNode(EventGraph, FName(VariableName), true, EVariableScopeType::Global, EVariableOperateType::SetValue, VariableNode))
    {
        UE_LOG(LogTemp, Display, TEXT("Created variable set node for %s in graph %s"), *VariableName, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), VariableNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create variable set node for %s"), *VariableName));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddComponentNode(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString ComponentClassName;
    if (!Params->TryGetStringField(TEXT("component_class"), ComponentClassName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_class' parameter"));
    }
    UClass* ComponentClass = FindObject<UClass>(ANY_PACKAGE, *ComponentClassName);
    if (!ComponentClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component class not found: %s"), *ComponentClassName));
    }
    UEdGraphNode* ComponentNode = nullptr;
    if (FUnrealMCPCommonUtils::SpawnAddComponentNode(EventGraph, ComponentClass, ComponentNode))
    {
        UE_LOG(LogTemp, Display, TEXT("Created component node for %s in graph %s"), *ComponentClassName, *EventGraph->GetName());

        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), ComponentNode->NodeGuid.ToString());
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create component node for %s"), *ComponentClassName));
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleFindEventNodeByName(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString EventName;
    if (!Params->TryGetStringField(TEXT("event_name"), EventName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter"));
    }
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
        {
            FString name = EventNode->EventReference.GetMemberName().ToString();
            if (name ==EventName)
            {
                TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
                ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
                UE_LOG(LogTemp, Display, TEXT("Found event node: %s with ID: %s"), *EventName, *EventNode->NodeGuid.ToString());
                return ResultObj;
            } 
        }
    }
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Event Node of name '%s' not found"), *EventName));    
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetNodePins(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString NodeIdStr;
    if (!Params->TryGetStringField(TEXT("node_id"), NodeIdStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_id' parameter"));
    }
    FGuid NodeId;
    if (!FGuid::Parse(NodeIdStr, NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Invalid node ID format: %s"), *NodeIdStr));
    }
    TObjectPtr<UEdGraphNode>* NodePtr = EventGraph->Nodes.FindByPredicate([&NodeId](const UEdGraphNode* N) { return N->NodeGuid == NodeId; });
    if (!NodePtr || !(*NodePtr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Node with ID '%s' not found"), *NodeIdStr));
    }
    UEdGraphNode* Node = NodePtr->Get();
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> PinArray;
    for (UEdGraphPin* Pin : Node->Pins)
    {
        TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
        PinObj->SetStringField(TEXT("pin_guid"), Pin->PinId.ToString());
        PinObj->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
        PinObj->SetStringField(TEXT("pin_type"), Pin->PinType.PinCategory.ToString());
        if (!Pin->PinType.PinSubCategory.IsNone())
        {
            PinObj->SetStringField(TEXT("pin_sub_category"), Pin->PinType.PinSubCategory.ToString());
        }
        PinObj->SetStringField(TEXT("pin_direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
        if (!Pin->DefaultValue.IsEmpty())
        {
            PinObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
        }
        if (Pin->DefaultObject)
        {
            PinObj->SetStringField(TEXT("default_object"), Pin->DefaultObject->GetPathName());
        }
        PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
    }
    ResultObj->SetArrayField(TEXT("pins"), PinArray);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleConnectBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString SourceNodeId;
    if (!Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_node_id' parameter"));
    }
    FString TargetNodeId;
    if (!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_node_id' parameter"));
    }
    FString SourcePinName;
    if (!Params->TryGetStringField(TEXT("source_pin_name"), SourcePinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_pin_name' parameter"));
    }
    FString TargetPinName;
    if (!Params->TryGetStringField(TEXT("target_pin_name"), TargetPinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_pin_name' parameter"));
    }

    // Find the nodes
    UEdGraphNode* SourceNode = nullptr;
    UEdGraphNode* TargetNode = nullptr;
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        if (Node->NodeGuid.ToString() == SourceNodeId)
        {
            SourceNode = Node;
        }
        else if (Node->NodeGuid.ToString() == TargetNodeId)
        {
            TargetNode = Node;
        }
    }

    if (!SourceNode || !TargetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source or target node not found"));
    }

    // Connect the nodes
    if (FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, SourceNode, SourcePinName, TargetNode, TargetPinName))
    {
        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("source_node_id"), SourceNodeId);
        ResultObj->SetStringField(TEXT("target_node_id"), TargetNodeId);
        return ResultObj;
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to connect nodes"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSetNodePinDefaultValue(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    FString NodeIdStr;
    if (!Params->TryGetStringField(TEXT("node_id"), NodeIdStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_id' parameter"));
    }
    FGuid NodeId;
    if (!FGuid::Parse(NodeIdStr, NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Invalid node ID format: %s"), *NodeIdStr));
    }
    TObjectPtr<UEdGraphNode>* NodePtr = EventGraph->Nodes.FindByPredicate([&NodeId](const UEdGraphNode* N) { return N->NodeGuid == NodeId; });
    if (!NodePtr || !(*NodePtr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Node with ID '%s' not found"), *NodeIdStr));
    }
    UEdGraphNode* Node = NodePtr->Get();
    FString PinName;
    if (!Params->TryGetStringField(TEXT("pin_name"), PinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pin_name' parameter"));
    }
    FString DefaultValue;
    if (!Params->TryGetStringField(TEXT("default_value"), DefaultValue))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'default_value' parameter"));
    }
    UEdGraphPin* Pin = Node->FindPin(PinName);
    if (!Pin)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Pin '%s' not found in node '%s'"), *PinName, *NodeIdStr));
    }
    // Set the default value
    Pin->DefaultValue = DefaultValue;
    Pin->DefaultObject = nullptr; // Clear any default object reference

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
    ResultObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
    return ResultObj;
    
}

bool FUnrealMCPBlueprintNodeCommands::GetEventGraphFromParams(const TSharedPtr<FJsonObject>& Params, UBlueprint*& Blueprint, UEdGraph*& EventGraph, TSharedPtr<FJsonObject>& ErrorResponse)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        ErrorResponse = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
        return false;
    }
    FString GraphName;
    if (!Params->TryGetStringField(TEXT("function_or_graph_name"), GraphName))
    {
        ErrorResponse = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'function_or_graph_name' parameter"));
        return false;
    }

    // Find the blueprint
    Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        ErrorResponse = FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
        return false;
    }
    EventGraph = FUnrealMCPCommonUtils::FindBlueprintGraphByName(Blueprint, GraphName);
    if (!EventGraph)
    {
        ErrorResponse = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
        return false;
    }
    return true;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetAllNodes(const TSharedPtr<FJsonObject>& Params)
{
    UEdGraph* EventGraph;
    UBlueprint* Blueprint;
    TSharedPtr<FJsonObject> ErrorResponse;
    if (!GetEventGraphFromParams(Params, Blueprint, EventGraph, ErrorResponse))
    {
        return ErrorResponse;
    }
    TArray<TObjectPtr<UEdGraphNode>> AllNodes = EventGraph->Nodes;
    TArray<TSharedPtr<FJsonValue>> NodeArray;
    for (TObjectPtr<UEdGraphNode> Node : AllNodes)
    {
        TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
        NodeObj->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
        if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("Event"));
            NodeObj->SetStringField(TEXT("event_name"), EventNode->EventReference.GetMemberName().ToString());
        }
        else if (UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("CallFunction"));
            NodeObj->SetStringField(TEXT("function_name"), CallFunctionNode->FunctionReference.GetMemberName().ToString());
            NodeObj->SetStringField(TEXT("target_class"), CallFunctionNode->FunctionReference.GetMemberParentClass()->GetPathName());
        }
        else if (UK2Node_MacroInstance* FunctionResultNode = Cast<UK2Node_MacroInstance>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("MacroInstance"));
            NodeObj->SetStringField(TEXT("macro_name"), FunctionResultNode->GetMacroGraph()->GetFName().ToString());
        }
        else if (UK2Node_VariableGet* VariableGetNode = Cast<UK2Node_VariableGet>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("VariableGet"));
            NodeObj->SetStringField(TEXT("variable_name"), VariableGetNode->VariableReference.GetMemberName().ToString());
        }
        else if (UK2Node_VariableSet* VariableSetNode = Cast<UK2Node_VariableSet>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("VariableSet"));
            NodeObj->SetStringField(TEXT("variable_name"), VariableSetNode->VariableReference.GetMemberName().ToString());
        }
        else if (UK2Node_IfThenElse* IfThenElseNode = Cast<UK2Node_IfThenElse>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("IfThenElse"));
        }
        else if (UK2Node_ExecutionSequence* SequenceNode = Cast<UK2Node_ExecutionSequence>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("Sequence"));
        }
        else if (UK2Node_Select* SelectNode = Cast<UK2Node_Select>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("Select"));
        }
        else if (UK2Node_SwitchEnum* SwitchEnumNode = Cast<UK2Node_SwitchEnum>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("SwitchEnum"));
            NodeObj->SetStringField(TEXT("enum_path"), SwitchEnumNode->GetEnum()->GetPathName());
        }
        else if (UK2Node_MakeStruct* MakeStructNode = Cast<UK2Node_MakeStruct>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("MakeStruct"));
            NodeObj->SetStringField(TEXT("struct_path"), MakeStructNode->StructType.GetPathName());
        }
        else if (UK2Node_BreakStruct* BreakStructNode = Cast<UK2Node_BreakStruct>(Node))
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("BreakStruct"));
            NodeObj->SetStringField(TEXT("struct_path"), BreakStructNode->StructType.GetPathName());
        }
        else
        {
            NodeObj->SetStringField(TEXT("node_type"), TEXT("Unknown"));
        }

        TArray<UEdGraphPin*> Pins = Node->Pins;
        TArray<TSharedPtr<FJsonValue>> PinArray;
        for (UEdGraphPin* Pin : Pins)
        {
            TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
            PinObj->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
            PinObj->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
            PinObj->SetStringField(TEXT("pin_type"), Pin->PinType.PinCategory.ToString());
            if (!Pin->PinType.PinSubCategory.IsNone())
            {
                PinObj->SetStringField(TEXT("pin_sub_category"), Pin->PinType.PinSubCategory.ToString());
            }
            PinObj->SetStringField(TEXT("pin_direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
            if (!Pin->DefaultValue.IsEmpty())
            {
                PinObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
            }
            if (Pin->DefaultObject)
            {
                PinObj->SetStringField(TEXT("default_object"), Pin->DefaultObject->GetPathName());
            }

            TArray<UEdGraphPin*> Links =  Pin->LinkedTo;            
            TArray<TSharedPtr<FJsonValue>> LinkArray;
            for (UEdGraphPin* Link : Links)
            {
                TSharedPtr<FJsonObject> LinkObj = MakeShared<FJsonObject>();
                LinkObj->SetStringField(TEXT("pin_id"), Link->PinId.ToString());
                LinkArray.Add(MakeShared<FJsonValueObject>(LinkObj));
            }
            PinObj->SetArrayField(TEXT("linked_pins"), LinkArray);

            TArray<UEdGraphPin*> SubPins = Pin->SubPins;
            TArray<TSharedPtr<FJsonValue>> SubPinArray;
            for (UEdGraphPin* SubPin : SubPins)
            {
                TSharedPtr<FJsonObject> SubPinObj = MakeShared<FJsonObject>();
                SubPinObj->SetStringField(TEXT("pin_id"), SubPin->PinId.ToString());
                SubPinObj->SetStringField(TEXT("pin_name"), SubPin->PinName.ToString());
                SubPinObj->SetStringField(TEXT("pin_type"), SubPin->PinType.PinCategory.ToString());
                if (!SubPin->PinType.PinSubCategory.IsNone())
                {
                    SubPinObj->SetStringField(TEXT("pin_sub_category"), SubPin->PinType.PinSubCategory.ToString());
                }
                SubPinObj->SetStringField(TEXT("pin_direction"), SubPin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
                if (!SubPin->DefaultValue.IsEmpty())
                {
                    SubPinObj->SetStringField(TEXT("default_value"), SubPin->DefaultValue);
                }
                if (SubPin->DefaultObject)
                {
                    SubPinObj->SetStringField(TEXT("default_object"), SubPin->DefaultObject->GetPathName());
                }
                TArray<UEdGraphPin*> SubLinks = SubPin->LinkedTo;
                TArray<TSharedPtr<FJsonValue>> SubLinkArray;
                for (UEdGraphPin* SubLink : SubLinks)
                {
                    TSharedPtr<FJsonObject> SubLinkObj = MakeShared<FJsonObject>();
                    SubLinkObj->SetStringField(TEXT("pin_id"), SubLink->PinId.ToString());
                    SubLinkArray.Add(MakeShared<FJsonValueObject>(SubLinkObj));
                }
                SubPinObj->SetArrayField(TEXT("linked_pins"), SubLinkArray);
                // Add sub pin to the array
                SubPinArray.Add(MakeShared<FJsonValueObject>(SubPinObj));
            }// end sub pins
            PinObj->SetArrayField(TEXT("sub_pins"), SubPinArray);
            
            PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
        } // end pins
        NodeObj->SetArrayField(TEXT("pins"), PinArray);
        NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
    } // end nodes
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("nodes"), NodeArray);
    return ResultObj;
}
