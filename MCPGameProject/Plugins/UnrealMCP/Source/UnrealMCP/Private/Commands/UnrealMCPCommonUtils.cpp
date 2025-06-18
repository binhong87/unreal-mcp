#include "Commands/UnrealMCPCommonUtils.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Selection.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_Select.h"
#include "K2Node_SwitchEnum.h"
#include "K2Node_SwitchInteger.h"
#include "K2Node_SwitchString.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet2/KismetEditorUtilities.h"


#define LOCTEXT_NAMESPACE "FUnrealMCPModule"

// JSON Utilities
TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::CreateErrorResponse(const FString& Message)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), false);
    ResponseObject->SetStringField(TEXT("error"), Message);
    return ResponseObject;
}

TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::CreateSuccessResponse(const TSharedPtr<FJsonObject>& Data)
{
    TSharedPtr<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
    ResponseObject->SetBoolField(TEXT("success"), true);
    
    if (Data.IsValid())
    {
        ResponseObject->SetObjectField(TEXT("data"), Data);
    }
    
    return ResponseObject;
}

void FUnrealMCPCommonUtils::GetIntArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<int32>& OutArray)
{
    OutArray.Reset();
    
    if (!JsonObject->HasField(FieldName))
    {
        return;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
        {
            OutArray.Add((int32)Value->AsNumber());
        }
    }
}

void FUnrealMCPCommonUtils::GetFloatArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<float>& OutArray)
{
    OutArray.Reset();
    
    if (!JsonObject->HasField(FieldName))
    {
        return;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
        {
            OutArray.Add((float)Value->AsNumber());
        }
    }
}

FVector2D FUnrealMCPCommonUtils::GetVector2DFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FVector2D Result(0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 2)
    {
        Result.X = (float)(*JsonArray)[0]->AsNumber();
        Result.Y = (float)(*JsonArray)[1]->AsNumber();
    }
    
    return Result;
}

FVector FUnrealMCPCommonUtils::GetVectorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FVector Result(0.0f, 0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 3)
    {
        Result.X = (float)(*JsonArray)[0]->AsNumber();
        Result.Y = (float)(*JsonArray)[1]->AsNumber();
        Result.Z = (float)(*JsonArray)[2]->AsNumber();
    }
    
    return Result;
}

FRotator FUnrealMCPCommonUtils::GetRotatorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    FRotator Result(0.0f, 0.0f, 0.0f);
    
    if (!JsonObject->HasField(FieldName))
    {
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (JsonObject->TryGetArrayField(FieldName, JsonArray) && JsonArray->Num() >= 3)
    {
        Result.Pitch = (float)(*JsonArray)[0]->AsNumber();
        Result.Yaw = (float)(*JsonArray)[1]->AsNumber();
        Result.Roll = (float)(*JsonArray)[2]->AsNumber();
    }
    
    return Result;
}

// Blueprint Utilities
UBlueprint* FUnrealMCPCommonUtils::FindBlueprint(const FString& BlueprintName)
{
    return FindBlueprintByName(BlueprintName);
}

UBlueprint* FUnrealMCPCommonUtils::FindBlueprintByName(const FString& BlueprintName)
{
    FString AssetPath = TEXT("/Game/Blueprints/") + BlueprintName;
    return LoadObject<UBlueprint>(nullptr, *AssetPath);
}

UEdGraph* FUnrealMCPCommonUtils::FindOrCreateEventGraph(UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return nullptr;
    }
    
    // Try to find the event graph
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (Graph->GetName().Contains(TEXT("EventGraph")))
        {
            return Graph;
        }
    }
    
    // Create a new event graph if none exists
    UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, FName(TEXT("EventGraph")), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
    FBlueprintEditorUtils::AddUbergraphPage(Blueprint, NewGraph);
    return NewGraph;
}

UEdGraph* FUnrealMCPCommonUtils::FindBlueprintGraphByName(UBlueprint* Blueprint, const FString& GraphName)
{
    if (!Blueprint)
    {
        return nullptr;
    }    
    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);
    for (UEdGraph* Graph : AllGraphs)
    {
        if (Graph->GetName() == GraphName)
        {
            return Graph;
        }
    }
    return nullptr;
}

// Blueprint node utilities
UK2Node_Event* FUnrealMCPCommonUtils::CreateEventNode(UEdGraph* Graph, const FString& EventName, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
    if (!Blueprint)
    {
        return nullptr;
    }
    
    // Check for existing event node with this exact name
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
        if (EventNode && EventNode->EventReference.GetMemberName() == FName(*EventName))
        {
            UE_LOG(LogTemp, Display, TEXT("Using existing event node with name %s (ID: %s)"), 
                *EventName, *EventNode->NodeGuid.ToString());
            return EventNode;
        }
    }

    // No existing node found, create a new one
    UK2Node_Event* EventNode = nullptr;
    
    // Find the function to create the event
    UClass* BlueprintClass = Blueprint->GeneratedClass;
    UFunction* EventFunction = BlueprintClass->FindFunctionByName(FName(*EventName));
    
    if (EventFunction)
    {
        EventNode = NewObject<UK2Node_Event>(Graph);
        EventNode->EventReference.SetExternalMember(FName(*EventName), BlueprintClass);
        EventNode->NodePosX = Position.X;
        EventNode->NodePosY = Position.Y;
        Graph->AddNode(EventNode, true);
        EventNode->PostPlacedNewNode();
        EventNode->AllocateDefaultPins();
        UE_LOG(LogTemp, Display, TEXT("Created new event node with name %s (ID: %s)"), 
            *EventName, *EventNode->NodeGuid.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find function for event name: %s"), *EventName);
    }
    
    return EventNode;
}

UK2Node_CallFunction* FUnrealMCPCommonUtils::CreateFunctionCallNode(UEdGraph* Graph, UFunction* Function, const FVector2D& Position)
{
    if (!Graph || !Function)
    {
        return nullptr;
    }
    
    UK2Node_CallFunction* FunctionNode = NewObject<UK2Node_CallFunction>(Graph);
    FunctionNode->SetFromFunction(Function);
    FunctionNode->NodePosX = Position.X;
    FunctionNode->NodePosY = Position.Y;
    Graph->AddNode(FunctionNode, true);
    FunctionNode->CreateNewGuid();
    FunctionNode->PostPlacedNewNode();
    FunctionNode->AllocateDefaultPins();
    
    return FunctionNode;
}

UK2Node_VariableGet* FUnrealMCPCommonUtils::CreateVariableGetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position)
{
    if (!Graph || !Blueprint)
    {
        return nullptr;
    }
    
    UK2Node_VariableGet* VariableGetNode = NewObject<UK2Node_VariableGet>(Graph);
    
    FName VarName(*VariableName);
    FProperty* Property = FindFProperty<FProperty>(Blueprint->GeneratedClass, VarName);
    
    if (Property)
    {
        VariableGetNode->VariableReference.SetFromField<FProperty>(Property, false);
        VariableGetNode->NodePosX = Position.X;
        VariableGetNode->NodePosY = Position.Y;
        Graph->AddNode(VariableGetNode, true);
        VariableGetNode->PostPlacedNewNode();
        VariableGetNode->AllocateDefaultPins();
        
        return VariableGetNode;
    }
    
    return nullptr;
}

UK2Node_VariableSet* FUnrealMCPCommonUtils::CreateVariableSetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position)
{
    if (!Graph || !Blueprint)
    {
        return nullptr;
    }
    
    UK2Node_VariableSet* VariableSetNode = NewObject<UK2Node_VariableSet>(Graph);
    
    FName VarName(*VariableName);
    FProperty* Property = FindFProperty<FProperty>(Blueprint->GeneratedClass, VarName);
    
    if (Property)
    {
        VariableSetNode->VariableReference.SetFromField<FProperty>(Property, false);
        VariableSetNode->NodePosX = Position.X;
        VariableSetNode->NodePosY = Position.Y;
        Graph->AddNode(VariableSetNode, true);
        VariableSetNode->PostPlacedNewNode();
        VariableSetNode->AllocateDefaultPins();
        
        return VariableSetNode;
    }
    
    return nullptr;
}

UK2Node_InputAction* FUnrealMCPCommonUtils::CreateInputActionNode(UEdGraph* Graph, const FString& ActionName, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UK2Node_InputAction* InputActionNode = NewObject<UK2Node_InputAction>(Graph);
    InputActionNode->InputActionName = FName(*ActionName);
    InputActionNode->NodePosX = Position.X;
    InputActionNode->NodePosY = Position.Y;
    Graph->AddNode(InputActionNode, true);
    InputActionNode->CreateNewGuid();
    InputActionNode->PostPlacedNewNode();
    InputActionNode->AllocateDefaultPins();
    
    return InputActionNode;
}

UK2Node_Self* FUnrealMCPCommonUtils::CreateSelfReferenceNode(UEdGraph* Graph, const FVector2D& Position)
{
    if (!Graph)
    {
        return nullptr;
    }
    
    UK2Node_Self* SelfNode = NewObject<UK2Node_Self>(Graph);
    SelfNode->NodePosX = Position.X;
    SelfNode->NodePosY = Position.Y;
    Graph->AddNode(SelfNode, true);
    SelfNode->CreateNewGuid();
    SelfNode->PostPlacedNewNode();
    SelfNode->AllocateDefaultPins();
    
    return SelfNode;
}

bool FUnrealMCPCommonUtils::ConnectGraphNodes(UEdGraph* Graph, UEdGraphNode* SourceNode, const FString& SourcePinName, 
                                           UEdGraphNode* TargetNode, const FString& TargetPinName)
{
    if (!Graph || !SourceNode || !TargetNode)
    {
        return false;
    }
    
    UEdGraphPin* SourcePin = FindPin(SourceNode, SourcePinName, EGPD_Output);
    UEdGraphPin* TargetPin = FindPin(TargetNode, TargetPinName, EGPD_Input);
    
    if (SourcePin && TargetPin)
    {
        return Graph->GetSchema()->TryCreateConnection(SourcePin, TargetPin);
    }
    
    return false;
}

UEdGraphPin* FUnrealMCPCommonUtils::FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction)
{
    if (!Node)
    {
        return nullptr;
    }
    
    // Log all pins for debugging
    UE_LOG(LogTemp, Display, TEXT("FindPin: Looking for pin '%s' (Direction: %d) in node '%s'"), 
           *PinName, (int32)Direction, *Node->GetName());
    
    for (UEdGraphPin* Pin : Node->Pins)
    {
        UE_LOG(LogTemp, Display, TEXT("  - Available pin: '%s', Direction: %d, Category: %s"), 
               *Pin->PinName.ToString(), (int32)Pin->Direction, *Pin->PinType.PinCategory.ToString());
    }
    
    // First try exact match
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin->PinName.ToString() == PinName && (Direction == EGPD_MAX || Pin->Direction == Direction))
        {
            UE_LOG(LogTemp, Display, TEXT("  - Found exact matching pin: '%s'"), *Pin->PinName.ToString());
            return Pin;
        }
    }
    
    // If no exact match and we're looking for a component reference, try case-insensitive match
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin->PinName.ToString().Equals(PinName, ESearchCase::IgnoreCase) && 
            (Direction == EGPD_MAX || Pin->Direction == Direction))
        {
            UE_LOG(LogTemp, Display, TEXT("  - Found case-insensitive matching pin: '%s'"), *Pin->PinName.ToString());
            return Pin;
        }
    }
    
    // If we're looking for a component output and didn't find it by name, try to find the first data output pin
    if (Direction == EGPD_Output && Cast<UK2Node_VariableGet>(Node) != nullptr)
    {
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                UE_LOG(LogTemp, Display, TEXT("  - Found fallback data output pin: '%s'"), *Pin->PinName.ToString());
                return Pin;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("  - No matching pin found for '%s'"), *PinName);
    return nullptr;
}

// Actor utilities
TSharedPtr<FJsonValue> FUnrealMCPCommonUtils::ActorToJson(AActor* Actor)
{
    if (!Actor)
    {
        return MakeShared<FJsonValueNull>();
    }
    
    TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
    ActorObject->SetStringField(TEXT("name"), Actor->GetName());
    ActorObject->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
    
    FVector Location = Actor->GetActorLocation();
    TArray<TSharedPtr<FJsonValue>> LocationArray;
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
    ActorObject->SetArrayField(TEXT("location"), LocationArray);
    
    FRotator Rotation = Actor->GetActorRotation();
    TArray<TSharedPtr<FJsonValue>> RotationArray;
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
    ActorObject->SetArrayField(TEXT("rotation"), RotationArray);
    
    FVector Scale = Actor->GetActorScale3D();
    TArray<TSharedPtr<FJsonValue>> ScaleArray;
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
    ActorObject->SetArrayField(TEXT("scale"), ScaleArray);
    
    return MakeShared<FJsonValueObject>(ActorObject);
}

TSharedPtr<FJsonObject> FUnrealMCPCommonUtils::ActorToJsonObject(AActor* Actor, bool bDetailed)
{
    if (!Actor)
    {
        return nullptr;
    }
    
    TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
    ActorObject->SetStringField(TEXT("name"), Actor->GetName());
    ActorObject->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
    
    FVector Location = Actor->GetActorLocation();
    TArray<TSharedPtr<FJsonValue>> LocationArray;
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
    LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
    ActorObject->SetArrayField(TEXT("location"), LocationArray);
    
    FRotator Rotation = Actor->GetActorRotation();
    TArray<TSharedPtr<FJsonValue>> RotationArray;
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
    RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
    ActorObject->SetArrayField(TEXT("rotation"), RotationArray);
    
    FVector Scale = Actor->GetActorScale3D();
    TArray<TSharedPtr<FJsonValue>> ScaleArray;
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
    ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
    ActorObject->SetArrayField(TEXT("scale"), ScaleArray);
    
    return ActorObject;
}

UK2Node_Event* FUnrealMCPCommonUtils::FindExistingEventNode(UEdGraph* Graph, const FString& EventName)
{
    if (!Graph)
    {
        return nullptr;
    }

    // Look for existing event nodes
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
        if (EventNode && EventNode->EventReference.GetMemberName() == FName(*EventName))
        {
            UE_LOG(LogTemp, Display, TEXT("Found existing event node with name: %s"), *EventName);
            return EventNode;
        }
    }

    return nullptr;
}

bool FUnrealMCPCommonUtils::SetObjectProperty(UObject* Object, const FString& PropertyName, 
                                     const TSharedPtr<FJsonValue>& Value, FString& OutErrorMessage)
{
    if (!Object)
    {
        OutErrorMessage = TEXT("Invalid object");
        return false;
    }

    FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
    if (!Property)
    {
        OutErrorMessage = FString::Printf(TEXT("Property not found: %s"), *PropertyName);
        return false;
    }

    void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(Object);
    
    // Handle different property types
    if (Property->IsA<FBoolProperty>())
    {
        ((FBoolProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsBool());
        return true;
    }
    else if (Property->IsA<FIntProperty>())
    {
        int32 IntValue = static_cast<int32>(Value->AsNumber());
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (IntProperty)
        {
            IntProperty->SetPropertyValue_InContainer(Object, IntValue);
            return true;
        }
    }
    else if (Property->IsA<FFloatProperty>())
    {
        ((FFloatProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsNumber());
        return true;
    }
    else if (Property->IsA<FStrProperty>())
    {
        ((FStrProperty*)Property)->SetPropertyValue(PropertyAddr, Value->AsString());
        return true;
    }
    else if (Property->IsA<FByteProperty>())
    {
        FByteProperty* ByteProp = CastField<FByteProperty>(Property);
        UEnum* EnumDef = ByteProp ? ByteProp->GetIntPropertyEnum() : nullptr;
        
        // If this is a TEnumAsByte property (has associated enum)
        if (EnumDef)
        {
            // Handle numeric value
            if (Value->Type == EJson::Number)
            {
                uint8 ByteValue = static_cast<uint8>(Value->AsNumber());
                ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
                
                UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric value: %d"), 
                      *PropertyName, ByteValue);
                return true;
            }
            // Handle string enum value
            else if (Value->Type == EJson::String)
            {
                FString EnumValueName = Value->AsString();
                
                // Try to convert numeric string to number first
                if (EnumValueName.IsNumeric())
                {
                    uint8 ByteValue = FCString::Atoi(*EnumValueName);
                    ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric string value: %s -> %d"), 
                          *PropertyName, *EnumValueName, ByteValue);
                    return true;
                }
                
                // Handle qualified enum names (e.g., "Player0" or "EAutoReceiveInput::Player0")
                if (EnumValueName.Contains(TEXT("::")))
                {
                    EnumValueName.Split(TEXT("::"), nullptr, &EnumValueName);
                }
                
                int64 EnumValue = EnumDef->GetValueByNameString(EnumValueName);
                if (EnumValue == INDEX_NONE)
                {
                    // Try with full name as fallback
                    EnumValue = EnumDef->GetValueByNameString(Value->AsString());
                }
                
                if (EnumValue != INDEX_NONE)
                {
                    ByteProp->SetPropertyValue(PropertyAddr, static_cast<uint8>(EnumValue));
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to name value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                else
                {
                    // Log all possible enum values for debugging
                    UE_LOG(LogTemp, Warning, TEXT("Could not find enum value for '%s'. Available options:"), *EnumValueName);
                    for (int32 i = 0; i < EnumDef->NumEnums(); i++)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - %s (value: %d)"), 
                               *EnumDef->GetNameStringByIndex(i), EnumDef->GetValueByIndex(i));
                    }
                    
                    OutErrorMessage = FString::Printf(TEXT("Could not find enum value for '%s'"), *EnumValueName);
                    return false;
                }
            }
        }
        else
        {
            // Regular byte property
            uint8 ByteValue = static_cast<uint8>(Value->AsNumber());
            ByteProp->SetPropertyValue(PropertyAddr, ByteValue);
            return true;
        }
    }
    else if (Property->IsA<FEnumProperty>())
    {
        FEnumProperty* EnumProp = CastField<FEnumProperty>(Property);
        UEnum* EnumDef = EnumProp ? EnumProp->GetEnum() : nullptr;
        FNumericProperty* UnderlyingNumericProp = EnumProp ? EnumProp->GetUnderlyingProperty() : nullptr;
        
        if (EnumDef && UnderlyingNumericProp)
        {
            // Handle numeric value
            if (Value->Type == EJson::Number)
            {
                int64 EnumValue = static_cast<int64>(Value->AsNumber());
                UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                
                UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric value: %lld"), 
                      *PropertyName, EnumValue);
                return true;
            }
            // Handle string enum value
            else if (Value->Type == EJson::String)
            {
                FString EnumValueName = Value->AsString();
                
                // Try to convert numeric string to number first
                if (EnumValueName.IsNumeric())
                {
                    int64 EnumValue = FCString::Atoi64(*EnumValueName);
                    UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to numeric string value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                
                // Handle qualified enum names
                if (EnumValueName.Contains(TEXT("::")))
                {
                    EnumValueName.Split(TEXT("::"), nullptr, &EnumValueName);
                }
                
                int64 EnumValue = EnumDef->GetValueByNameString(EnumValueName);
                if (EnumValue == INDEX_NONE)
                {
                    // Try with full name as fallback
                    EnumValue = EnumDef->GetValueByNameString(Value->AsString());
                }
                
                if (EnumValue != INDEX_NONE)
                {
                    UnderlyingNumericProp->SetIntPropertyValue(PropertyAddr, EnumValue);
                    
                    UE_LOG(LogTemp, Display, TEXT("Setting enum property %s to name value: %s -> %lld"), 
                          *PropertyName, *EnumValueName, EnumValue);
                    return true;
                }
                else
                {
                    // Log all possible enum values for debugging
                    UE_LOG(LogTemp, Warning, TEXT("Could not find enum value for '%s'. Available options:"), *EnumValueName);
                    for (int32 i = 0; i < EnumDef->NumEnums(); i++)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - %s (value: %d)"), 
                               *EnumDef->GetNameStringByIndex(i), EnumDef->GetValueByIndex(i));
                    }
                    
                    OutErrorMessage = FString::Printf(TEXT("Could not find enum value for '%s'"), *EnumValueName);
                    return false;
                }
            }
        }
    }
    
    OutErrorMessage = FString::Printf(TEXT("Unsupported property type: %s for property %s"), 
                                    *Property->GetClass()->GetName(), *PropertyName);
    return false;
}


bool FUnrealMCPCommonUtils::SpawnFunctionCallNode(UEdGraph* LocalGraph, FName NameOfFunction, UClass* ClassOfFunction, UEdGraphNode*& NewNode)
{
    if (!ClassOfFunction)
        return false;
    if (!LocalGraph)
        return false;

    UFunction* Function = ClassOfFunction->FindFunctionByName(NameOfFunction);
    if (!Function)
        return false;
    NewNode = NewObject<UEdGraphNode>(LocalGraph);
    UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);
    if (FunctionNodeSpawner)
    {
        FunctionNodeSpawner->SetFlags(RF_Transactional);
        NewNode = FunctionNodeSpawner->Invoke(LocalGraph, IBlueprintNodeBinder::FBindingSet(), LocalGraph->GetGoodPlaceForNewNode());
    }
    return true;
}

bool FUnrealMCPCommonUtils::SpawnMathNode(UEdGraph* LocalGraph, EArithmeticOperation Operation, EArithmeticDataType DataType, UEdGraphNode*& NewNode)
{
    UEdGraph* Graph = LocalGraph;
    if (!Graph)
        return false;

    const UEdGraphSchema_K2* Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema());
    if (!Schema)
        return false;

    FVector2D NodePosition = Graph->GetGoodPlaceForNewNode();
    FName FunctionName;
    bool bIsValidOperation = true;

    switch (Operation)
    {
    case EArithmeticOperation::Add:
        switch (DataType)
        {
        case EArithmeticDataType::ArithType_Integer:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_IntInt);
            break;
        case EArithmeticDataType::ArithType_Float:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_DoubleDouble);
            break;
        case EArithmeticDataType::ArithType_Byte:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_ByteByte);
            break;
        case EArithmeticDataType::ArithType_Integer64:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_Int64Int64);
            break;
        case EArithmeticDataType::ArithType_Vector:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_VectorVector);
            break;
        case EArithmeticDataType::ArithType_Vector2D:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_Vector2DVector2D);
            break;
        case EArithmeticDataType::ArithType_Vector4:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_Vector4Vector4);
            break;
        default:
            bIsValidOperation = false;
        }
        break;
    case EArithmeticOperation::Subtract:
        switch (DataType)
        {
        case EArithmeticDataType::ArithType_Integer:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_IntInt);
            break;
        case EArithmeticDataType::ArithType_Float:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_DoubleDouble);
            break;
        case EArithmeticDataType::ArithType_Byte:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_ByteByte);
            break;
        case EArithmeticDataType::ArithType_Integer64:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_Int64Int64);
            break;
        case EArithmeticDataType::ArithType_Vector:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_VectorVector);
            break;
        case EArithmeticDataType::ArithType_Vector2D:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_Vector2DVector2D);
            break;
        case EArithmeticDataType::ArithType_Vector4:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Subtract_Vector4Vector4);
            break;
        default:
            bIsValidOperation = false;
        }
        break;
    case EArithmeticOperation::Multiply:
        switch (DataType)
        {
        case EArithmeticDataType::ArithType_Integer:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_IntInt);
            break;
        case EArithmeticDataType::ArithType_Float:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_DoubleDouble);
            break;
        case EArithmeticDataType::ArithType_Byte:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_ByteByte);
            break;
        case EArithmeticDataType::ArithType_Integer64:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_Int64Int64);
            break;
        case EArithmeticDataType::ArithType_Vector:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_VectorVector);
            break;
        case EArithmeticDataType::ArithType_Vector2D:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_Vector2DVector2D);
            break;
        case EArithmeticDataType::ArithType_Vector4:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Multiply_Vector4Vector4);
            break;
        default:
            bIsValidOperation = false;
        }
        break;
    case EArithmeticOperation::Divide:
        switch (DataType)
        {
        case EArithmeticDataType::ArithType_Integer:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_IntInt);
            break;
        case EArithmeticDataType::ArithType_Float:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_DoubleDouble);
            break;
        case EArithmeticDataType::ArithType_Byte:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_ByteByte);
            break;
        case EArithmeticDataType::ArithType_Integer64:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_Int64Int64);
            break;
        case EArithmeticDataType::ArithType_Vector:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_VectorVector);
            break;
        case EArithmeticDataType::ArithType_Vector2D:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_Vector2DVector2D);
            break;
        case EArithmeticDataType::ArithType_Vector4:
            FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Divide_Vector4Vector4);
            break;
        default:
            bIsValidOperation = false;
        }
        break;
    default:
        bIsValidOperation = false;
    }

    if (!bIsValidOperation)
        return false;

    UK2Node_CallFunction* FunctionNode = NewObject<UK2Node_CallFunction>(Graph);
    if (FunctionNode)
    {
        FunctionNode->CreateNewGuid();
        FunctionNode->PostPlacedNewNode();
        FunctionNode->SetFromFunction(UKismetMathLibrary::StaticClass()->FindFunctionByName(FunctionName));
        FunctionNode->SetFlags(RF_Transactional);
        FunctionNode->AllocateDefaultPins();

        Graph->AddNode(FunctionNode, true, false);
        FunctionNode->NodePosX = NodePosition.X;
        FunctionNode->NodePosY = NodePosition.Y;
        FunctionNode->SnapToGrid(16);

        NewNode = FunctionNode;
        return true;
    }

    return false;
}

bool FUnrealMCPCommonUtils::SpawnSequenceNode(UEdGraph* LocalGraph, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;

    FVector2D NodePosition = LocalGraph->GetGoodPlaceForNewNode();
    UK2Node_ExecutionSequence* NewSequenceNode = NewObject<UK2Node_ExecutionSequence>(LocalGraph);
    if (!NewSequenceNode)
        return false;

    NewSequenceNode->SetFlags(RF_Transactional);

    LocalGraph->AddNode(NewSequenceNode, true, false);
    NewSequenceNode->CreateNewGuid();
    NewSequenceNode->PostPlacedNewNode();
    NewSequenceNode->AllocateDefaultPins();
    NewSequenceNode->NodePosX = NodePosition.X;
    NewSequenceNode->NodePosY = NodePosition.Y;

    NewNode = NewSequenceNode;

    return true;
}

bool FUnrealMCPCommonUtils::SpawnNodeByType(UEdGraph* LocalGraph, EK2NodeType NodeType, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;
    FVector2D NodePosition = LocalGraph->GetGoodPlaceForNewNode();
    UK2Node* NewCreatedNode = nullptr;
    
    switch (NodeType) {
    case EK2NodeType::K2NodeType_If:
        NewCreatedNode = NewObject<UK2Node_IfThenElse>(LocalGraph);
        break;
    case EK2NodeType::K2NodeType_For:
        SpawnStandardMacrosNode(LocalGraph, "ForLoop", NewNode);
        break;
    case EK2NodeType::K2NodeType_Foreach:
        SpawnStandardMacrosNode(LocalGraph, "ForEachLoop", NewNode);
        break;
    case EK2NodeType::K2NodeType_While:
        SpawnStandardMacrosNode(LocalGraph, "WhileLoop", NewNode);
        break;
    case EK2NodeType::K2NodeType_Do:
        return false;
        break;
    case EK2NodeType::K2NodeType_SwitchString:
        NewCreatedNode = NewObject<UK2Node_SwitchString>(LocalGraph);
        break;
    case K2NodeType_SwitchInt:
        NewCreatedNode = NewObject<UK2Node_SwitchInteger>(LocalGraph);
        break;
    case K2NodeType_SwitchEnum:
        NewCreatedNode = NewObject<UK2Node_SwitchEnum>(LocalGraph);
        break;
    }
    
    if (!NewCreatedNode)
    {    
        NewNode = nullptr;
        return false;
    }
    
    NewCreatedNode->SetFlags(RF_Transactional);
    LocalGraph->AddNode(NewCreatedNode, true, false);
    NewCreatedNode->CreateNewGuid();
    NewCreatedNode->PostPlacedNewNode();
    NewCreatedNode->AllocateDefaultPins();
    NewCreatedNode->NodePosX = NodePosition.X;
    NewCreatedNode->NodePosY = NodePosition.Y;
    NewNode = NewCreatedNode;
    return true;
}

bool FUnrealMCPCommonUtils::SpawnSelectNode(UEdGraph* LocalGraph, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;

    FVector2D NodePosition = LocalGraph->GetGoodPlaceForNewNode();
    UK2Node_Select* SelectNode = NewObject<UK2Node_Select>(LocalGraph);
    if (!SelectNode)
        return false;

    SelectNode->SetFlags(RF_Transactional);
    LocalGraph->AddNode(SelectNode, true, false);
    SelectNode->CreateNewGuid();
    SelectNode->PostPlacedNewNode();
    SelectNode->AllocateDefaultPins();

    SelectNode->NodePosX = NodePosition.X;
    SelectNode->NodePosY = NodePosition.Y;

    UEdGraphPin* IndexPin = SelectNode->GetIndexPin();
    if (IndexPin)
    {
        IndexPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        IndexPin->PinType.PinSubCategory = NAME_None;
        IndexPin->PinType.PinSubCategoryObject = nullptr;

        SelectNode->PinTypeChanged(IndexPin);
    }
    NewNode = SelectNode;
    return true;
}

bool FUnrealMCPCommonUtils::SpawnSelectNode2(UEdGraph* LocalGraph, FKB_PinTypeInformations PinTypeInfo, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;

    if (!SpawnSelectNode(LocalGraph,NewNode))
    {
        return false;
    }

    UK2Node_Select* SelectNode = Cast<UK2Node_Select>(NewNode);
    if (!SelectNode)
    {
        return false;
    }

    UEdGraphPin* ReturnPin = SelectNode->GetReturnValuePin();
    if (!ReturnPin)
    {
        return false;
    }

    ReturnPin->PinType.PinCategory = PinTypeInfo.Category;
    ReturnPin->PinType.PinSubCategory = PinTypeInfo.SubCategory;
    // CallOnDebug(FString::Printf(TEXT("lidkhjoiwduiowadjioawd %s"), *PinTypeInfo.SubCategory.ToString()));

    // TODO
    // if (PinList.IsValidIndex(PinTypeInfo.Index))
    // {
    //     UEdGraphPin* SourcePin = PinList[PinTypeInfo.Index];
    //     if (SourcePin && SourcePin->PinType.PinSubCategoryObject.IsValid())
    //     {
    //         ReturnPin->PinType.PinSubCategoryObject = SourcePin->PinType.PinSubCategoryObject;
    //     }
    // }

    SelectNode->PinTypeChanged(ReturnPin);

    return true;
}

bool FUnrealMCPCommonUtils::SpawnEnumSwitch(UEdGraph* LocalGraph, FString EnumPath, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;

    UEnum* EnumAsset = Cast<UEnum>(StaticLoadObject(UEnum::StaticClass(), nullptr, *EnumPath));
    if (!EnumAsset)
        return false;

    FVector2D NodePosition = LocalGraph->GetGoodPlaceForNewNode();
    UK2Node_SwitchEnum* NewSwitchEnumNode = NewObject<UK2Node_SwitchEnum>(LocalGraph);
    if (!NewSwitchEnumNode)
        return false;

    NewSwitchEnumNode->Enum = EnumAsset;
    NewSwitchEnumNode->SetFlags(RF_Transactional);
    LocalGraph->AddNode(NewSwitchEnumNode, true, false);
    NewSwitchEnumNode->CreateNewGuid();
    NewSwitchEnumNode->PostPlacedNewNode();
    NewSwitchEnumNode->AllocateDefaultPins();
    NewSwitchEnumNode->NodePosX = NodePosition.X;
    NewSwitchEnumNode->NodePosY = NodePosition.Y;

    NewNode = NewSwitchEnumNode;
    return true;
}

bool FUnrealMCPCommonUtils::SpawnStructNode(UEdGraph* LocalGraph, UScriptStruct* StructType, bool bMakeStruct, UEdGraphNode*& NewNode)
{
    if (!LocalGraph)
        return false;

    FVector2D MyPosition = LocalGraph->GetGoodPlaceForNewNode();

    if (bMakeStruct)
    {
        UK2Node_MakeStruct* MakeStructNode = NewObject<UK2Node_MakeStruct>(LocalGraph);
        if (MakeStructNode)
        {
            MakeStructNode->StructType = StructType;
            MakeStructNode->SetFlags(RF_Transactional);
            MakeStructNode->CreateNewGuid();
            MakeStructNode->PostPlacedNewNode();
            MakeStructNode->NodePosX = MyPosition.X;
            MakeStructNode->NodePosY = MyPosition.Y;
            MakeStructNode->AllocateDefaultPins();
            LocalGraph->AddNode(MakeStructNode);
            NewNode = MakeStructNode;
            return true;
        }
    }
    else
    {
        UK2Node_BreakStruct* BreakStructNode = NewObject<UK2Node_BreakStruct>(LocalGraph);
        if (BreakStructNode)
        {
            BreakStructNode->StructType = StructType;
            BreakStructNode->SetFlags(RF_Transactional);
            BreakStructNode->CreateNewGuid();
            BreakStructNode->PostPlacedNewNode();
            BreakStructNode->NodePosX = MyPosition.X;
            BreakStructNode->NodePosY = MyPosition.Y;
            BreakStructNode->AllocateDefaultPins();
            LocalGraph->AddNode(BreakStructNode);
            NewNode = BreakStructNode;
            return true;
        }
    }
    return false;
}

bool FUnrealMCPCommonUtils::CreateBlueprintFunction(UBlueprint* BlueprintRef, FString FunctionName,
    TArray<FKB_FunctionPinInformations> InPins, TArray<FKB_FunctionPinInformations> OutPins, UEdGraph*& FunctionGraph)
{
    UBlueprint* LocalBlueprint = BlueprintRef;
    if (!LocalBlueprint || !IsValid(LocalBlueprint))
        return false;
    FText DocumentNameText = LOCTEXT("NewDocFuncName", "NewFunction");
    FName DocumentName = FName(*DocumentNameText.ToString());
    if (!SafeFindUniqueKismetName(LocalBlueprint, FunctionName, DocumentName)) {
        return false;
    }
    const FScopedTransaction Transaction(LOCTEXT("AddNewFunction", "Add New Function"));
    LocalBlueprint->Modify();

    UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(LocalBlueprint, DocumentName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
    FBlueprintEditorUtils::AddFunctionGraph<UClass>(LocalBlueprint, NewGraph, true, nullptr);
    if (!NewGraph)
        return false;

    FunctionGraph = NewGraph;
    
    TWeakObjectPtr<class UK2Node_EditablePinBase> FunctionEntryNodePtr;

    if (UEdGraph* NewTargetGraph = NewGraph)
    {
        TArray<UK2Node_FunctionEntry*> EntryNodes;
        NewTargetGraph->GetNodesOfClass(EntryNodes);
        if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable())
        {
            FunctionEntryNodePtr = EntryNodes[0];
        }
    }
    UK2Node_EditablePinBase* FunctionEntryNode = FunctionEntryNodePtr.Get();
    if (FunctionEntryNode)
    {
        for (int y = 0; y < InPins.Num(); y++) {
            EVariablePinType Banana = InPins[y].VarType;
            EPinContainerType LocalContainerType = EPinContainerType::None;
            switch (Banana) {
            case EVariablePinType::Single:
                LocalContainerType = EPinContainerType::None;
                break;
            case EVariablePinType::Array:
                LocalContainerType = EPinContainerType::Array;
                break;
            case EVariablePinType::Set:
                LocalContainerType = EPinContainerType::Set;
                break;
            case EVariablePinType::Map:
                LocalContainerType = EPinContainerType::Map;
                break;
            }
            FScopedTransaction Transaction2(LOCTEXT("AddInParam", "Add In Parameter"));
            FunctionEntryNode->Modify();
            FName LocalPinCategory1 = "None";
            FName LocalPinSubCategory1 = "None";
            TWeakObjectPtr<UObject> LocalPinSubObject1 = nullptr;

            if (InPins[y].UseCustomVarType)
            {
                if (!InPins[y].CustomVarTypePath.IsEmpty() && !InPins[y].CustomVarTypeName.IsEmpty())
                {
                    UObject* CustomType = LoadObject<UObject>(nullptr, *InPins[y].CustomVarTypePath);
                    if (CustomType)
                    {
                        if (UEnum* CustomEnum = Cast<UEnum>(CustomType))
                        {
                            LocalPinCategory1 = "byte";
                            LocalPinSubCategory1 = "None";
                            LocalPinSubObject1 = CustomEnum;
                        }
                        else if (UScriptStruct* CustomStruct = Cast<UScriptStruct>(CustomType))
                        {
                            LocalPinCategory1 = "struct";
                            LocalPinSubCategory1 = "None";
                            LocalPinSubObject1 = CustomStruct;
                        }
                    }
                }
            }
            else
            {
                PinVarConversionLocal(InPins[y].Type, LocalPinCategory1, LocalPinSubCategory1, LocalPinSubObject1);
            }

            FEdGraphPinType MyPinType;
            MyPinType.bIsConst = false;
            MyPinType.bIsReference = false;
            MyPinType.bIsUObjectWrapper = false;
            MyPinType.bIsWeakPointer = false;
            MyPinType.ContainerType = LocalContainerType;
            MyPinType.PinCategory = LocalPinCategory1;
            MyPinType.PinSubCategory = LocalPinSubCategory1;
            MyPinType.PinSubCategoryObject = LocalPinSubObject1;
            MyPinType.PinSubCategoryMemberReference.MemberName = "None";
            MyPinType.PinValueType.bTerminalIsConst = false;
            MyPinType.PinValueType.bTerminalIsUObjectWrapper = false;
            MyPinType.PinValueType.bTerminalIsWeakPointer = false;
            MyPinType.PinValueType.TerminalCategory = "None";
            MyPinType.PinValueType.TerminalSubCategory = "None";

            if (LocalContainerType == EPinContainerType::Map) {
                FName KeyPinCategory = "int";
                FName ValuePinCategory = "string";

                MyPinType.PinCategory = KeyPinCategory;
                MyPinType.PinValueType.TerminalCategory = ValuePinCategory;
                MyPinType.PinValueType.TerminalSubCategory = "None";
                MyPinType.PinValueType.TerminalSubCategoryObject = nullptr;

                MyPinType.PinSubCategory = "None";
                MyPinType.PinSubCategoryObject = nullptr;
            }

            const FName NewPinName = InPins[y].Name;
            if (!FunctionEntryNode->CreateUserDefinedPin(NewPinName, MyPinType, EGPD_Output))
            {
                Transaction2.Cancel();
            }
        }
    }
    TWeakObjectPtr<class UK2Node_EditablePinBase> FunctionResultNodePtr;
    FScopedTransaction Transaction3(LOCTEXT("AddOutParam", "Add Out Parameter"));

    LocalBlueprint->Modify();
    NewGraph->Modify();
    UK2Node_EditablePinBase* EntryPin = FunctionEntryNodePtr.Get();
    EntryPin->Modify();
    for (int32 iPin = 0; iPin < EntryPin->Pins.Num(); iPin++)
    {
        EntryPin->Pins[iPin]->Modify();
    }

    if (!FunctionResultNodePtr.IsValid())
    {
        FunctionResultNodePtr = FBlueprintEditorUtils::FindOrCreateFunctionResultNode(FunctionEntryNodePtr.Get());
    }

    UK2Node_EditablePinBase* FunctionResultNode = FunctionResultNodePtr.Get();
    if (FunctionResultNode)
    {
        for (int i = 0; i < OutPins.Num(); i++) {
            EVariablePinType Banana2 = OutPins[i].VarType;
            EPinContainerType LocalContainerType2 = EPinContainerType::None;
            switch (Banana2) {
            case EVariablePinType::Single:
                LocalContainerType2 = EPinContainerType::None;
                break;
            case EVariablePinType::Array:
                LocalContainerType2 = EPinContainerType::Array;
                break;
            case EVariablePinType::Set:
                LocalContainerType2 = EPinContainerType::Set;
                break;
            case EVariablePinType::Map:
                LocalContainerType2 = EPinContainerType::Map;
                break;
            }
            FName LocalPinCategory = "None";
            FName LocalPinSubCategory = "None";
            TWeakObjectPtr<UObject> LocalPinSubObject = nullptr;

            if (OutPins[i].UseCustomVarType)
            {
                if (!OutPins[i].CustomVarTypePath.IsEmpty() && !OutPins[i].CustomVarTypeName.IsEmpty())
                {
                    UObject* CustomType = LoadObject<UObject>(nullptr, *OutPins[i].CustomVarTypePath);
                    if (CustomType)
                    {
                        if (UEnum* CustomEnum = Cast<UEnum>(CustomType))
                        {
                            LocalPinCategory = "byte";
                            LocalPinSubCategory = "None";
                            LocalPinSubObject = CustomEnum;
                        }
                        else if (UScriptStruct* CustomStruct = Cast<UScriptStruct>(CustomType))
                        {
                            LocalPinCategory = "struct";
                            LocalPinSubCategory = "None";
                            LocalPinSubObject = CustomStruct;
                        }
                    }
                }
            }
            else
            {
                PinVarConversionLocal(OutPins[i].Type, LocalPinCategory, LocalPinSubCategory, LocalPinSubObject);
            }

            FEdGraphPinType MyPinType2;
            MyPinType2.bIsConst = false;
            MyPinType2.bIsReference = false;
            MyPinType2.bIsUObjectWrapper = false;
            MyPinType2.bIsWeakPointer = false;
            MyPinType2.ContainerType = LocalContainerType2;
            MyPinType2.PinCategory = LocalPinCategory; 
            MyPinType2.PinSubCategory = LocalPinSubCategory;
            MyPinType2.PinSubCategoryObject = LocalPinSubObject;    
            MyPinType2.PinSubCategoryMemberReference.MemberName = "None";
            MyPinType2.PinValueType.bTerminalIsConst = false;
            MyPinType2.PinValueType.bTerminalIsUObjectWrapper = false;
            MyPinType2.PinValueType.bTerminalIsWeakPointer = false;
            MyPinType2.PinValueType.TerminalCategory = "None";
            MyPinType2.PinValueType.TerminalSubCategory = "None";
            MyPinType2.bIsReference = false;

            const FName NewPinName = FunctionResultNode->CreateUniquePinName(OutPins[i].Name);

            TArray<UK2Node_EditablePinBase*> TargetNodes;
            if (UK2Node_FunctionResult* ResultNode = Cast<UK2Node_FunctionResult>(FunctionResultNode))
            {
                TargetNodes = (TArray<UK2Node_EditablePinBase*>)ResultNode->GetAllResultNodes();
            }
            else if (FunctionResultNode) {
                TargetNodes.Add(FunctionResultNode);
            }
            bool bAllChanged = TargetNodes.Num() > 0;
            for (UK2Node_EditablePinBase* Node : TargetNodes)
            {
                Node->Modify();
                UEdGraphPin* NewPin = Node->CreateUserDefinedPin(NewPinName, MyPinType2, EGPD_Input, false);
                bAllChanged &= (nullptr != NewPin);
                if (!bAllChanged)
                {
                    break;
                }
            }
            if (!bAllChanged)
            {
                Transaction3.Cancel();
            }
        }
    }
    else
    {
        Transaction3.Cancel();
    }
    return true;
}

bool FUnrealMCPCommonUtils::SafeFindUniqueKismetName(UBlueprint* InBlueprint, const FString& InBaseName, FName& ReturnName)
{
    if (!IsValid(InBlueprint))
    {
        ReturnName = FName(*InBaseName);
        return false;
    }

    bool ShouldFail = false;

    FString BaseName = InBaseName.IsEmpty() ? TEXT("NewFunction") : InBaseName;
    FString KismetName = BaseName;

    if (InBlueprint->GeneratedClass)
    {
        int32 Count = 0;
        bool bNameExists = true;

        while (bNameExists && Count < 1000)      
        {
            bNameExists = false;

            for (TFieldIterator<UFunction> FuncIt(InBlueprint->GeneratedClass); FuncIt; ++FuncIt)
            {
                if (FuncIt->GetFName().ToString() == KismetName)
                {
                    bNameExists = true;
                    KismetName = FString::Printf(TEXT("%s_%d"), *BaseName, ++Count);
                    break;
                }
            }

            for (UEdGraph* Graph : InBlueprint->UbergraphPages)
            {
                if (Graph && Graph->GetFName().ToString() == KismetName)
                {
                    bNameExists = true;
                    KismetName = FString::Printf(TEXT("%s_%d"), *BaseName, ++Count);
                    break;
                }
            }

            if (!bNameExists)
            {
                for (UEdGraph* Graph : InBlueprint->FunctionGraphs)
                {
                    if (Graph && Graph->GetFName().ToString() == KismetName)
                    {
                        bNameExists = true;
                        KismetName = FString::Printf(TEXT("%s_%d"), *BaseName, ++Count);
                        break;
                    }
                }
            }
        }
    }
    else {
        ShouldFail = true;
    }
    if (ShouldFail) {
        ReturnName = FName(*InBaseName);
        return false;
    }
    else {
        ReturnName = FName(*KismetName);
        return true;
    }
}


void FUnrealMCPCommonUtils::PinVarConversionLocal(EVariableType VarType, FName& PinCategory, FName& PinSubCategory, TWeakObjectPtr<UObject>& PinSubObject)
{
    FName LocalPinCategory = "None";
    FName LocalPinSubCategory = "None";
    TWeakObjectPtr<UObject> LocalPinSubObject = nullptr;
    switch (VarType) {
    case EVariableType::VarType_Boolean:
        LocalPinCategory = "bool";
        break;
    case EVariableType::VarType_Byte:
        LocalPinCategory = "byte";
        break;
    case EVariableType::VarType_Integer:
        LocalPinCategory = "int";
        break;
    case EVariableType::VarType_Integer64:
        LocalPinCategory = "int64";
        break;
    case EVariableType::VarType_Float:
        LocalPinCategory = "real";
        LocalPinSubCategory = "double";
        break;
    case EVariableType::VarType_Name:
        LocalPinCategory = "name";
        break;
    case EVariableType::VarType_String:
        LocalPinCategory = "string";
        break;
    case EVariableType::VarType_Text:
        LocalPinCategory = "text";
        break;
    case EVariableType::VarType_Vector:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FVector>::Get();
        break;
    case EVariableType::VarType_Rotator:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FRotator>::Get();
        break;
    case EVariableType::VarType_Transform:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FTransform>::Get();
        break;
    case EVariableType::VarType_Object:
        LocalPinCategory = "object";
        LocalPinSubObject = UObject::StaticClass();
        break;
    case EVariableType::VarType_Texture2D:
        LocalPinCategory = "object";
        LocalPinSubObject = UTexture2D::StaticClass();
        break;
    case EVariableType::VarType_Color:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FColor>::Get();
        break;
    case EVariableType::VarType_Actor:
        LocalPinCategory = "object";
        LocalPinSubObject = AActor::StaticClass();
        break;
    case EVariableType::VarType_Vector2D:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FVector2D>::Get();
        break;
    case EVariableType::VarType_LinearColor:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FLinearColor>::Get();
        break;
    case EVariableType::VarType_SlateColor:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FSlateColor>::Get();
        break;
    case EVariableType::VarType_IntPoint:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FIntPoint>::Get();
        break;
    case EVariableType::VarType_DateTime:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FDateTime>::Get();
        break;
    case EVariableType::VarType_Timespan:
        LocalPinCategory = "struct";
        break;
    case EVariableType::VarType_IntVector:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FIntVector>::Get();
        break;
    case EVariableType::VarType_IntVector4:
        LocalPinCategory = "struct";
        LocalPinSubObject = TBaseStructure<FIntVector4>::Get();
        break;
    case EVariableType::VarType_SaveGame:
        LocalPinCategory = "object";
        LocalPinSubObject = USaveGame::StaticClass();
        break;
    }

    PinCategory = LocalPinCategory;
    PinSubCategory = LocalPinSubCategory;
    PinSubObject = LocalPinSubObject;
}

TEnumAsByte<EVariableType> FUnrealMCPCommonUtils::GetVariableTypeFromString(const FString& TypeString)
{
    if (TypeString == "Boolean")
        return EVariableType::VarType_Boolean;
    else if (TypeString == "Byte")
        return EVariableType::VarType_Byte;
    else if (TypeString == "Integer")
        return EVariableType::VarType_Integer;
    else if (TypeString == "Integer64")
        return EVariableType::VarType_Integer64;
    else if (TypeString == "Float")
        return EVariableType::VarType_Float;
    else if (TypeString == "Name")
        return EVariableType::VarType_Name;
    else if (TypeString == "String")
        return EVariableType::VarType_String;
    else if (TypeString == "Text")
        return EVariableType::VarType_Text;
    else if (TypeString == "Vector")
        return EVariableType::VarType_Vector;
    else if (TypeString == "Rotator")
        return EVariableType::VarType_Rotator;
    else if (TypeString == "Transform")
        return EVariableType::VarType_Transform;
    else if (TypeString == "Object")
        return EVariableType::VarType_Object;
    else if (TypeString == "Texture2D")
        return EVariableType::VarType_Texture2D;
    else if (TypeString == "Color")
        return EVariableType::VarType_Color;
    else if (TypeString == "Actor")
        return EVariableType::VarType_Actor;
    else if (TypeString == "Vector2D")
        return EVariableType::VarType_Vector2D;
    else if (TypeString == "LinearColor")
        return EVariableType::VarType_LinearColor;
    else if (TypeString == "SlateColor")
        return EVariableType::VarType_SlateColor;
    else if (TypeString == "IntPoint")
        return EVariableType::VarType_IntPoint;
    else if (TypeString == "DateTime")
        return EVariableType::VarType_DateTime;
    else if (TypeString == "Timespan")
        return EVariableType::VarType_Timespan;
    else if (TypeString == "IntVector")
        return EVariableType::VarType_IntVector;
    else if (TypeString == "IntVector4")
        return EVariableType::VarType_IntVector4;
    else if (TypeString == "SaveGame")
        return EVariableType::VarType_SaveGame;

    else
        return EVariableType::VarType_Boolean; // Default case
}

TEnumAsByte<EVariablePinType> FUnrealMCPCommonUtils::GetVariablePinTypeFromString(const FString& TypeString)
{
    if (TypeString == "Single")
        return EVariablePinType::Single;
    else if (TypeString == "Array")
        return EVariablePinType::Array;
    else if (TypeString == "Set")
        return EVariablePinType::Set;
    else if (TypeString == "Map")
        return EVariablePinType::Map;
    else
        return EVariablePinType::Single; // Default case

}

EArithmeticOperation FUnrealMCPCommonUtils::GetArithmeticOperationFromString(const FString& OperationString)
{
    if (OperationString == "Add")
        return EArithmeticOperation::Add;
    else if (OperationString == "Subtract")
        return EArithmeticOperation::Subtract;
    else if (OperationString == "Multiply")
        return EArithmeticOperation::Multiply;
    else if (OperationString == "Divide")
        return EArithmeticOperation::Divide;
    else
        return EArithmeticOperation::Add; // Default case
}

EArithmeticDataType FUnrealMCPCommonUtils::GetArithmeticDataTypeFromString(const FString& DataTypeString)
{
    if (DataTypeString == "Integer")
        return EArithmeticDataType::ArithType_Integer;
    else if (DataTypeString == "Float")
        return EArithmeticDataType::ArithType_Float;
    else if (DataTypeString == "Byte")
        return EArithmeticDataType::ArithType_Byte;
    else if (DataTypeString == "Integer64")
        return EArithmeticDataType::ArithType_Integer64;
    else if (DataTypeString == "Vector")
        return EArithmeticDataType::ArithType_Vector;
    else if (DataTypeString == "Vector2D")
        return EArithmeticDataType::ArithType_Vector2D;
    else if (DataTypeString == "Vector4")
        return EArithmeticDataType::ArithType_Vector4;
    else if (DataTypeString == "Rotator")
        return EArithmeticDataType::ArithType_Rotator;
    else if (DataTypeString == "Transform")
        return EArithmeticDataType::ArithType_Transform;
    else if (DataTypeString == "LinearColor")
        return EArithmeticDataType::ArithType_LinearColor;
    else if (DataTypeString == "IntPoint")
        return EArithmeticDataType::ArithType_IntPoint;
    else if (DataTypeString == "DateTime")
        return EArithmeticDataType::ArithType_DateTime;
    else if (DataTypeString == "Timespan")
        return EArithmeticDataType::ArithType_Timespan;
    else if (DataTypeString == "IntVector")
        return EArithmeticDataType::ArithType_IntVector;
    else if (DataTypeString == "Plane")
        return EArithmeticDataType::ArithType_Plane;
    else if (DataTypeString == "Quaternion")
        return EArithmeticDataType::ArithType_Quat;
    else
        return EArithmeticDataType::ArithType_Integer; // Default case
}

EK2NodeType FUnrealMCPCommonUtils::GetK2NodeTypeFromString(const FString& NodeTypeString)
{
    if (NodeTypeString == "If")
        return EK2NodeType::K2NodeType_If;
    else if (NodeTypeString == "For")
        return EK2NodeType::K2NodeType_For;
    else if (NodeTypeString == "Foreach")
        return EK2NodeType::K2NodeType_Foreach;
    else if (NodeTypeString == "While")
        return EK2NodeType::K2NodeType_While;
    else if (NodeTypeString == "Do")
        return EK2NodeType::K2NodeType_Do;
    else if (NodeTypeString == "SwitchInt")
        return EK2NodeType::K2NodeType_SwitchInt;
    else if (NodeTypeString == "SwitchString")
        return EK2NodeType::K2NodeType_SwitchString;
    else if (NodeTypeString == "SwitchEnum")
        return EK2NodeType::K2NodeType_SwitchEnum;
    else
        return EK2NodeType::K2NodeType_If; // Default case
}

bool FUnrealMCPCommonUtils::SpawnStandardMacrosNode(UEdGraph* LocalGraph, FName MacroName, UEdGraphNode*& NewNode)
{
    UBlueprint* StandardMacros = LoadObject<UBlueprint>(nullptr, TEXT("/Engine/EditorBlueprintResources/StandardMacros.StandardMacros"));
    if (!StandardMacros)
    {
        return false;
    }
    UEdGraph* ForEachMacroGraph = FindMacroGraphByName(StandardMacros, MacroName);
    if (!ForEachMacroGraph)
    {
        return false;
    }

    if (!SpawnMacroInstanceNode(LocalGraph, ForEachMacroGraph, NewNode))
    {
        return false;
    }
    return true;
}

UEdGraph* FUnrealMCPCommonUtils::FindMacroGraphByName(UBlueprint* Blueprint, const FName MacroName)
{
    for (UEdGraph* Graph : Blueprint->MacroGraphs)
    {
        if (Graph && Graph->GetFName() == MacroName)
        {
            return Graph;
        }
    }
    return nullptr;
}

bool FUnrealMCPCommonUtils::SpawnMacroInstanceNode(UEdGraph* LocalGraph, UEdGraph* MacroGraph, UEdGraphNode*& NewNode)
{
    UK2Node_MacroInstance* NewMacroNode = NewObject<UK2Node_MacroInstance>(LocalGraph);
    if (!NewMacroNode)
        return false;

    FVector2D NodePosition = LocalGraph->GetGoodPlaceForNewNode();

    NewMacroNode->SetFlags(RF_Transactional);

    NewMacroNode->SetMacroGraph(MacroGraph);    

    LocalGraph->AddNode(NewMacroNode, true, false);
    NewMacroNode->CreateNewGuid();
    NewMacroNode->PostPlacedNewNode();
    NewMacroNode->AllocateDefaultPins();
    NewMacroNode->NodePosX = NodePosition.X;
    NewMacroNode->NodePosY = NodePosition.Y;

    NewNode = NewMacroNode;

    return true;
}

bool FUnrealMCPCommonUtils::KBL_SpawnK2VarNode(UEdGraph* LocalGraph, FName VarName, bool AutoDetectIfLocalOrGlobal, EVariableScopeType VarBoundaries, EVariableOperateType NodeType, UEdGraphNode*& NewNode)
{
    UEdGraph* Graph = LocalGraph;
    if (!Graph)
        return false;
    EVariableScopeType LocalVariableType = VarBoundaries;
    bool LocalBool;
    EVariableType LocalVarType;
    if (!KBL_DoesVariableExist(LocalGraph, VarName, LocalBool, LocalVarType))
        return false;
    if (AutoDetectIfLocalOrGlobal) {
        if (LocalBool) {
            LocalVariableType = EVariableScopeType::Local;
        }
        else {
            LocalVariableType = EVariableScopeType::Global;
        }
    }
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
    if (!Schema)
        return false;
    FVector2D NodePosition = Graph->GetGoodPlaceForNewNode();
    UK2Node_VariableSet* SetNode = nullptr;
    UK2Node_VariableGet* GetNode = nullptr;
    UK2Node_VariableSet* LocalSetNode = nullptr;
    UK2Node_VariableGet* LocalGetNode = nullptr;
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
    switch (LocalVariableType) {
    case EVariableScopeType::Global:
        UStruct* LocalStruct;
        KBL_NomalVarConversion(LocalVarType, LocalStruct);
        switch (NodeType) {
        case EVariableOperateType::SetValue:
            LocalSetNode = Schema->SpawnVariableSetNode(NodePosition, Graph, VarName, LocalStruct);        
            LocalSetNode->CreateNewGuid();
            NewNode = LocalSetNode;
            // NodeIndex = NodeList.Add(LocalSetNode);
            break;
        case EVariableOperateType::GetValue:
            LocalGetNode = Schema->SpawnVariableGetNode(NodePosition, Graph, VarName, LocalStruct);        
            LocalGetNode->CreateNewGuid();
            NewNode = LocalGetNode;
            // NodeIndex = NodeList.Add(LocalGetNode);
            break;
        }
        break;
    case EVariableScopeType::Local:
        if (!Blueprint)
            return false;
        switch (NodeType) {
        case EVariableOperateType::SetValue:
            SetNode = NewObject<UK2Node_VariableSet>(Graph);
            SetNode->VariableReference.SetLocalMember(VarName, Graph->GetName(), FGuid());
            SetNode->CreateNewGuid();
            if (!SetNode)
                return false;
            SetNode->AllocateDefaultPins();
            Graph->AddNode(SetNode, true, false);
            SetNode->NodePosX = NodePosition.X;
            SetNode->NodePosY = NodePosition.Y;
            NewNode = SetNode;
            // NodeIndex = NodeList.Add(SetNode);
            break;
        case EVariableOperateType::GetValue:
            GetNode = NewObject<UK2Node_VariableGet>(Graph);
            GetNode->VariableReference.SetLocalMember(VarName, Graph->GetName(), FGuid());
            GetNode->CreateNewGuid();
            if (!GetNode)
                return false;
            GetNode->AllocateDefaultPins();
            Graph->AddNode(GetNode, true, false);
            GetNode->NodePosX = NodePosition.X;
            GetNode->NodePosY = NodePosition.Y;
            NewNode = GetNode;
            // NodeIndex = NodeList.Add(GetNode);
            break;
        }
        break;
    }
    return true;
}

bool FUnrealMCPCommonUtils::KBL_DoesVariableExist(UEdGraph* LocalGraph, FName VariableName, bool& IsLocalVar, EVariableType& VarType)
{
    UEdGraph* Graph = LocalGraph;
    if (!Graph)
        return false;
    for (FBPVariableDescription LocalVar : GetLocalVariablesForGraph(Graph)) {
        if (LocalVar.VarName == VariableName) {
            IsLocalVar = true;
            VarType = KBL_RevertedPinVarConversion(LocalVar.VarType.PinCategory, LocalVar.VarType.PinSubCategory, LocalVar.VarType.PinSubCategoryObject);
            return true;
        }
    }
    for (FBPVariableDescription GlobalVar : GetGlobalVariablesForBlueprint(FBlueprintEditorUtils::FindBlueprintForGraph(Graph))) {
        if (GlobalVar.VarName == VariableName) {
            IsLocalVar = false;
            VarType = KBL_RevertedPinVarConversion(GlobalVar.VarType.PinCategory, GlobalVar.VarType.PinSubCategory, GlobalVar.VarType.PinSubCategoryObject);
            return true;
        }
    }
    return false;
}

void FUnrealMCPCommonUtils::KBL_NomalVarConversion(EVariableType VarType, UStruct*& ReturnStruct)
{
    UStruct* LocalReturnStruct = nullptr;
    switch (VarType) {
    case EVariableType::VarType_Object:
        LocalReturnStruct = UObject::StaticClass();
        break;
    case EVariableType::VarType_Texture2D:
        LocalReturnStruct = UTexture2D::StaticClass();
        break;
    case EVariableType::VarType_Actor:
        LocalReturnStruct = AActor::StaticClass();
        break;
    }
    ReturnStruct = LocalReturnStruct;
}

TArray<FBPVariableDescription> FUnrealMCPCommonUtils::GetLocalVariablesForGraph(const UEdGraph* Graph)
{
    TArray<FBPVariableDescription> LocalVariables;

    if (!Graph)
    {
        return LocalVariables;
    }

    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UK2Node_FunctionEntry* FunctionEntryNode = Cast<UK2Node_FunctionEntry>(Node))
        {
            LocalVariables.Append(FunctionEntryNode->LocalVariables);
        }
    }

    return LocalVariables;
}

TArray<FBPVariableDescription> FUnrealMCPCommonUtils::GetGlobalVariablesForBlueprint(const UBlueprint* Blueprint)
{
    TArray<FBPVariableDescription> GlobalVariables;

    if (!Blueprint)
    {
        return GlobalVariables;
    }

    GlobalVariables = Blueprint->NewVariables;

    return GlobalVariables;
}

EVariableType FUnrealMCPCommonUtils::KBL_RevertedPinVarConversion(FName PinCategory, FName PinSubCategory, TWeakObjectPtr<UObject> PinSubObject)
{
        if (PinCategory == "struct") {
            if (PinSubObject == TBaseStructure<FVector>::Get()) {
                return EVariableType::VarType_Vector;
            }
            else if (PinSubObject == TBaseStructure<FRotator>::Get()) {
                return EVariableType::VarType_Rotator;
            }
            else if (PinSubObject == TBaseStructure<FTransform>::Get()) {
                return EVariableType::VarType_Transform;
            }
            else if (PinSubObject == TBaseStructure<FColor>::Get()) {
                return EVariableType::VarType_Color;
            }
            else if (PinSubObject == TBaseStructure<FVector2D>::Get()) {
                return EVariableType::VarType_Vector2D;
            }
            else if (PinSubObject == TBaseStructure<FLinearColor>::Get()) {
                return EVariableType::VarType_LinearColor;
            }
            else if (PinSubObject == TBaseStructure<FSlateColor>::Get()) {
                return EVariableType::VarType_SlateColor;
            }
            else if (PinSubObject == TBaseStructure<FIntPoint>::Get()) {
                return EVariableType::VarType_IntPoint;
            }
            else if (PinSubObject == TBaseStructure<FDateTime>::Get()) {
                return EVariableType::VarType_DateTime;
            }
            else if (PinSubObject == TBaseStructure<FIntVector>::Get()) {
                return EVariableType::VarType_IntVector;
            }
            else if (PinSubObject == TBaseStructure<FIntVector4>::Get()) {
                return EVariableType::VarType_IntVector4;
            }

        }
        else if (PinCategory == "object") {
            if (PinSubObject == UObject::StaticClass()) {
                return EVariableType::VarType_Object;
            }
            else if (PinSubObject == UTexture2D::StaticClass()) {
                return EVariableType::VarType_Texture2D;
            }
            else if (PinSubObject == AActor::StaticClass()) {
                return EVariableType::VarType_Actor;
            }
        }
        else if (PinCategory == "bool") {
            return EVariableType::VarType_Boolean;
        }
        else if (PinCategory == "byte") {
            return EVariableType::VarType_Byte;
        }
        else if (PinCategory == "int") {
            return EVariableType::VarType_Integer;
        }
        else if (PinCategory == "int64") {
            return EVariableType::VarType_Integer64;
        }
        else if (PinCategory == "real") {
            if (PinSubCategory == "double")
                return EVariableType::VarType_Float;
        }
        else if (PinCategory == "name") {
            return EVariableType::VarType_Name;
        }
        else if (PinCategory == "string") {
            return EVariableType::VarType_String;
        }
        else if (PinCategory == "text") {
            return EVariableType::VarType_Text;
        }
        return EVariableType::VarType_Integer;
}

bool FUnrealMCPCommonUtils::CreateLocalVariable(UBlueprint* Blueprint, UEdGraph* LocalGraph, FKB_FunctionPinInformations Var)
{
    UBlueprint* LocalBlueprint = Blueprint;
    if (!LocalBlueprint)
        return false;
    if (!LocalGraph)
        return false;

    EVariablePinType Banana = Var.VarType;
    EPinContainerType LocalContainerType = EPinContainerType::None;
    switch (Banana) {
    case EVariablePinType::Single:
        LocalContainerType = EPinContainerType::None;
        break;
    case EVariablePinType::Array:
        LocalContainerType = EPinContainerType::Array;
        break;
    case EVariablePinType::Set:
        LocalContainerType = EPinContainerType::Set;
        break;
    case EVariablePinType::Map:
        LocalContainerType = EPinContainerType::Map;
        break;
    }

    FName LocalPinCategory = "None";
    FName LocalPinSubCategory = "None";
    TWeakObjectPtr<UObject> LocalPinSubObject = nullptr;

    if (Var.UseCustomVarType)
    {
        if (!Var.CustomVarTypePath.IsEmpty() && !Var.CustomVarTypeName.IsEmpty())
        {
            UObject* CustomType = LoadObject<UObject>(nullptr, *Var.CustomVarTypePath);
            if (CustomType)
            {
                if (UEnum* CustomEnum = Cast<UEnum>(CustomType))
                {
                    LocalPinCategory = "byte";
                    LocalPinSubCategory = "None";
                    LocalPinSubObject = CustomEnum;
                }
                else if (UScriptStruct* CustomStruct = Cast<UScriptStruct>(CustomType))
                {
                    LocalPinCategory = "struct";
                    LocalPinSubCategory = "None";
                    LocalPinSubObject = CustomStruct;
                }
            }
        }
    }
    else
    {
        PinVarConversionLocal(Var.Type, LocalPinCategory, LocalPinSubCategory, LocalPinSubObject);
    }

    FEdGraphPinType MyPinType;
    MyPinType.bIsConst = false;
    MyPinType.bIsReference = false;
    MyPinType.bIsUObjectWrapper = false;
    MyPinType.bIsWeakPointer = false;
    MyPinType.ContainerType = LocalContainerType;
    MyPinType.PinCategory = LocalPinCategory;
    MyPinType.PinSubCategory = LocalPinSubCategory;
    MyPinType.PinSubCategoryObject = LocalPinSubObject;
    MyPinType.PinSubCategoryMemberReference.MemberName = "None";
    MyPinType.PinValueType.bTerminalIsConst = false;
    MyPinType.PinValueType.bTerminalIsUObjectWrapper = false;
    MyPinType.PinValueType.bTerminalIsWeakPointer = false;
    MyPinType.PinValueType.TerminalCategory = "None";
    MyPinType.PinValueType.TerminalSubCategory = "None";

    if (LocalContainerType == EPinContainerType::Map) {
        MyPinType.PinCategory = "int";    
        MyPinType.PinValueType.TerminalCategory = "string";    
        MyPinType.PinValueType.TerminalSubCategory = "None";
        MyPinType.PinValueType.TerminalSubCategoryObject = nullptr;
        MyPinType.PinSubCategory = "None";
        MyPinType.PinSubCategoryObject = nullptr;
    }

    FBlueprintEditorUtils::AddLocalVariable(LocalBlueprint, LocalGraph, Var.Name, MyPinType, FString());
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(LocalBlueprint);
    FKismetEditorUtilities::CompileBlueprint(LocalBlueprint);
    return true;
}

#undef LOCTEXT_NAMESPACE
