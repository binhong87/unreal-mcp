#pragma once

#include "CoreMinimal.h"
#include "Json.h"

// Forward declarations
class AActor;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;
class UK2Node_Event;
class UK2Node_CallFunction;
class UK2Node_VariableGet;
class UK2Node_VariableSet;
class UK2Node_InputAction;
class UK2Node_Self;
class UFunction;

	
enum EVariablePinType
{
	Single,
	Array,
	Set,
	Map
};

enum EVariableType
{
	VarType_Boolean,
	VarType_Byte,
	VarType_Integer,
	VarType_Integer64,
	VarType_Float,
	VarType_Name,
	VarType_String,
	VarType_Text,
	VarType_Vector,
	VarType_Rotator,
	VarType_Transform,
	VarType_Object,
	VarType_Texture2D,
	VarType_Color,
	VarType_Actor,
	VarType_Vector2D,
	VarType_LinearColor,
	VarType_SlateColor,
	VarType_IntPoint,
	VarType_DateTime,
	VarType_Timespan,    
	VarType_IntVector,
	VarType_IntVector4,
	VarType_SaveGame,
};

enum EPinDirection
{
	In,
	Out,
};

enum EK2NodeType
{
	K2NodeType_If,                        
	K2NodeType_For,
	K2NodeType_Foreach,
	K2NodeType_While,
	K2NodeType_Do,
	K2NodeType_SwitchString,
	K2NodeType_SwitchInt,
	K2NodeType_SwitchEnum,
};

enum EVariableScopeType
{
	Global,
	Local
};

enum EVariableOperateType
{
	GetValue,
	SetValue
};

enum EArithmeticOperation
{
	Add,
	Subtract,
	Multiply,
	Divide
};

enum EArithmeticDataType               
{
	ArithType_Integer,
	ArithType_Float,
	ArithType_Byte,
	ArithType_Vector,
	ArithType_Vector2D,
	ArithType_IntPoint,
	ArithType_Rotator,
	ArithType_Quat,
	ArithType_Transform,
	ArithType_LinearColor,
	ArithType_DateTime,
	ArithType_Timespan,
	ArithType_Plane,
	ArithType_IntVector,
	ArithType_Integer64,
	ArithType_Vector4,
};

struct FAllInputAndOutputVariable          
{
	FString Name = FString();
	FString Type = FString();
	EVariablePinType VarType = EVariablePinType::Single;
};
struct FCodeSection
{
	FString Type = FString();
	FString Content = FString();
};
struct FEnumInfo
{
	FString Name;
	TArray<FString> Members;
	TArray<FString> UMetas;            
	FString UEnumDeclaration;        
};
struct FStructInfo
{
	FString Name;
	TArray<FString> Members;
	TArray<FString> UProperties;                 
	FString UStructDeclaration;              
};
struct FGlobalAddtitionalEnum            
{
	FString InCodeName;          
	FString RealName;        
	FString AssetPath;        
	TArray<FString> Members;      
};
struct FGlobalAdditionalStruct            
{
	FString InCodeName;          
	FString RealName;        
	FString AssetPath;        
	TArray<FString> Members;          
};
struct FKB_FunctionPinInformations   
{
	FName Name = FName();
	TEnumAsByte<EVariableType> Type = EVariableType::VarType_Boolean;
	TEnumAsByte<EVariablePinType> VarType = EVariablePinType::Single;
	bool UseCustomVarType = false;
	FString CustomVarTypeName = FString();
	FString CustomVarTypePath = FString();
};

struct FKB_PinTypeInformations   
{
	int Index = 0;
	FName Name = FName();
	FName Category = FName();
	FName SubCategory = FName();
	FString SubCategoryObject = FString();
	EPinDirection PinDirection = EPinDirection::In;  
	bool IsHidden = false;
};

struct FKB_TempVarsUntilNow       
{
	TArray<FString> TempVarNames = {};
	TArray<FKB_PinTypeInformations> TempVarPins = {};
	TArray<FString> TempVarTypes = {};
};

struct FKB_SGlobalWhileSections
{
	TArray<FString> GlobalCodes = {};
};

struct FCustomEnumOperandInfo
{
	bool bIsCustomEnum = false;
	UEnum* EnumAsset = nullptr;
	FString EnumAssetPath;
	FString EnumInCodeName;   
	FString EnumValueIfLiteral;        
	bool bIsLiteral = false;
	FString OriginalOperandString;

	FCustomEnumOperandInfo() {}
};

/**
 * Common utilities for UnrealMCP commands
 */
class UNREALMCP_API FUnrealMCPCommonUtils
{
public:
    // JSON utilities
    static TSharedPtr<FJsonObject> CreateErrorResponse(const FString& Message);
    static TSharedPtr<FJsonObject> CreateSuccessResponse(const TSharedPtr<FJsonObject>& Data = nullptr);
    static void GetIntArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<int32>& OutArray);
    static void GetFloatArrayFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<float>& OutArray);
    static FVector2D GetVector2DFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);
    static FVector GetVectorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);
    static FRotator GetRotatorFromJson(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);
    
    // Actor utilities
    static TSharedPtr<FJsonValue> ActorToJson(AActor* Actor);
    static TSharedPtr<FJsonObject> ActorToJsonObject(AActor* Actor, bool bDetailed = false);
    
    // Blueprint utilities
    static UBlueprint* FindBlueprint(const FString& BlueprintName);
    static UBlueprint* FindBlueprintByName(const FString& BlueprintName);
    static UEdGraph* FindOrCreateEventGraph(UBlueprint* Blueprint);
	static UEdGraph* FindBlueprintGraphByName(UBlueprint* Blueprint, const FString& GraphName);
    
    // Blueprint node utilities
    static UK2Node_Event* CreateEventNode(UEdGraph* Graph, const FString& EventName, const FVector2D& Position);
    static UK2Node_CallFunction* CreateFunctionCallNode(UEdGraph* Graph, UFunction* Function, const FVector2D& Position);
    static UK2Node_VariableGet* CreateVariableGetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position);
    static UK2Node_VariableSet* CreateVariableSetNode(UEdGraph* Graph, UBlueprint* Blueprint, const FString& VariableName, const FVector2D& Position);
    static UK2Node_InputAction* CreateInputActionNode(UEdGraph* Graph, const FString& ActionName, const FVector2D& Position);
    static UK2Node_Self* CreateSelfReferenceNode(UEdGraph* Graph, const FVector2D& Position);
    static bool ConnectGraphNodes(UEdGraph* Graph, UEdGraphNode* SourceNode, const FString& SourcePinName, 
                                UEdGraphNode* TargetNode, const FString& TargetPinName);
    static UEdGraphPin* FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction = EGPD_MAX);
    static UK2Node_Event* FindExistingEventNode(UEdGraph* Graph, const FString& EventName);

    // Property utilities
    static bool SetObjectProperty(UObject* Object, const FString& PropertyName, const TSharedPtr<FJsonValue>& Value, FString& OutErrorMessage);


	static bool SpawnFunctionCallNode(UEdGraph* LocalGraph, FName NameOfFunction, UClass* ClassOfFunction, UEdGraphNode*& NewNode);

	static bool SpawnMathNode(UEdGraph* LocalGraph, EArithmeticOperation Operation, EArithmeticDataType DataType, UEdGraphNode*& NewNode);

	static bool SpawnSequenceNode(UEdGraph* LocalGraph, UEdGraphNode*& NewNode);

	static bool SpawnNodeByType(UEdGraph* LocalGraph, EK2NodeType NodeType, UEdGraphNode*& NewNode);

	static bool SpawnSelectNode(UEdGraph* LocalGraph, UEdGraphNode*& NewNode);

	static bool SpawnSelectNode2(UEdGraph* LocalGraph, FKB_PinTypeInformations PinTypeInfo, UEdGraphNode*& NewNode);

	static bool SpawnEnumSwitch(UEdGraph* LocalGraph, FString EnumPath, UEdGraphNode*& NewNode);

	static bool SpawnStructNode(UEdGraph* LocalGraph, UScriptStruct* StructType, bool bMakeStruct, UEdGraphNode*& NewNode);
	
	static bool CreateBlueprintFunction(UBlueprint* BlueprintRef, FString FunctionName, TArray<FKB_FunctionPinInformations> InPins, TArray<FKB_FunctionPinInformations> OutPins,  UEdGraph*& FunctionGraph);

	static bool SafeFindUniqueKismetName(UBlueprint* InBlueprint, const FString& InBaseName, FName& ReturnName);

	static void PinVarConversionLocal(EVariableType VarType, FName& PinCategory, FName& PinSubCategory, TWeakObjectPtr<UObject>& PinSubObject);

	static TEnumAsByte<EVariableType> GetVariableTypeFromString(const FString& TypeString);

	static TEnumAsByte<EVariablePinType> GetVariablePinTypeFromString(const FString& TypeString);

	static EArithmeticOperation GetArithmeticOperationFromString(const FString& OperationString);

	static EArithmeticDataType GetArithmeticDataTypeFromString(const FString& DataTypeString);

	static EK2NodeType GetK2NodeTypeFromString(const FString& NodeTypeString);

	// static FKB_PinTypeInformations GetPinTypeFromString(const FString& PinTypeString);

};