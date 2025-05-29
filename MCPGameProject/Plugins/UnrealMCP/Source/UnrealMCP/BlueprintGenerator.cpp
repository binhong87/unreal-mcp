#include "BlueprintGenerator.h"

// 所需的 Unreal Engine 头文件
#include "Engine/Blueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "EdGraphSchema_K2.h"
#include "PackageTools.h"
#include "GameFramework/Actor.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"

UBlueprint* UBlueprintGenerator::CreateHelloPrintBlueprint(const FString& BlueprintName, const FString& BlueprintPath, UClass* ParentClass)
{
    // 如果没有指定父类，默认使用AActor
    if (!ParentClass)
    {
        ParentClass = AActor::StaticClass();
    }

    if (BlueprintName.IsEmpty() || BlueprintPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 提供的参数无效。"));
        return nullptr;
    }

    // 创建完整的包名
    FString FullPackageName = UPackageTools::SanitizePackageName(BlueprintPath + TEXT("/") + BlueprintName);

    // 检查包是否已存在
    if (FPackageName::DoesPackageExist(FullPackageName))
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateHelloPrintBlueprint: 包 '%s' 已存在。"), *FullPackageName);
        // 可以选择加载现有资源或返回失败
    }

    // 创建包
    UPackage* Package = CreatePackage(*FullPackageName);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 创建包 '%s' 失败。"), *FullPackageName);
        return nullptr;
    }
    Package->SetDirtyFlag(true);

    // 创建蓝图对象
    UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
        ParentClass,
        Package,
        *BlueprintName,
        BPTYPE_Normal, // EBlueprintType: 普通蓝图
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass(),
        FName("CreateHelloPrintBlueprint_Transaction") // 事务上下文名称
    );

    if (!NewBP)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 创建蓝图资源失败。"));
        return nullptr;
    }

    // 获取事件图
    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(NewBP);
    if (!EventGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 找不到事件图。"));
        return nullptr;
    }

    // --- 添加 BeginPlay 事件节点 ---
    UK2Node_Event* BeginPlayNode = nullptr;
    // 检查 BeginPlay 是否已存在
    TArray<UK2Node_Event*> EventNodes;
    EventGraph->GetNodesOfClass<UK2Node_Event>(EventNodes);
    for (UK2Node_Event* ExistingEventNode : EventNodes)
    {
        if (ExistingEventNode->EventSignatureName_DEPRECATED == TEXT("BeginPlay") &&
            ExistingEventNode->EventSignatureClass_DEPRECATED == ParentClass) // 确保是正确的BeginPlay
        {
            BeginPlayNode = ExistingEventNode;
            break;
        }
    }

    if (!BeginPlayNode) // 如果不存在，则创建新的BeginPlay事件节点
    {
        BeginPlayNode = NewObject<UK2Node_Event>(EventGraph); // Outer 设置为 EventGraph
        BeginPlayNode->EventSignatureClass_DEPRECATED = ParentClass;
        BeginPlayNode->EventSignatureName_DEPRECATED = TEXT("BeginPlay");
        BeginPlayNode->NodePosX = -200; // 示例节点位置
        BeginPlayNode->NodePosY = 0;
        BeginPlayNode->CreateNewGuid();
        BeginPlayNode->PostPlacedNewNode();
        BeginPlayNode->SetFlags(RF_Transactional);
        EventGraph->AddNode(BeginPlayNode, false, false);
    }
    
    if (!BeginPlayNode)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 查找或创建BeginPlay节点失败。"));
        return nullptr;
    }

    // --- 添加 Print String 节点 ---
    UK2Node_CallFunction* PrintStringNode = NewObject<UK2Node_CallFunction>(EventGraph); // Outer 设置为 EventGraph
    PrintStringNode->CreateNewGuid();
    PrintStringNode->PostPlacedNewNode();
    PrintStringNode->SetFlags(RF_Transactional);
    PrintStringNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintString), UKismetSystemLibrary::StaticClass());
    PrintStringNode->NodePosX = 200; // 示例节点位置
    PrintStringNode->NodePosY = 0;
    EventGraph->AddNode(PrintStringNode, false, false);

    // 设置 PrintString 节点的 "InString" 引脚
    UEdGraphPin* InStringPin = PrintStringNode->FindPin(TEXT("InString"));
    if (InStringPin)
    {
        InStringPin->DefaultValue = TEXT("Hello");
    }

    // --- 连接引脚 ---
    UEdGraphPin* BeginPlayExecPin = BeginPlayNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* PrintStringExecInPin = PrintStringNode->GetExecPin(); // 获取执行输入引脚

    if (BeginPlayExecPin && PrintStringExecInPin)
    {
        BeginPlayExecPin->MakeLinkTo(PrintStringExecInPin);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CreateHelloPrintBlueprint: 获取连接所需的引脚失败。"));
        return nullptr; // 或者进行一些清理
    }

    // 编译蓝图
    FKismetEditorUtilities::CompileBlueprint(NewBP);

    // 通知资源注册表新资源的创建
    FAssetRegistryModule::AssetCreated(NewBP);

    // 标记包为脏，以便编辑器提示保存
    Package->MarkPackageDirty();

    UE_LOG(LogTemp, Log, TEXT("成功创建蓝图 '%s' 于 '%s'"), *BlueprintName, *BlueprintPath);
    return NewBP;
}
