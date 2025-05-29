#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BlueprintGenerator.generated.h"

/**
 * 提供用于生成蓝图的工具函数
 */
UCLASS()
class UNREALMCP_API UBlueprintGenerator : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * 创建一个新的蓝图资源，该蓝图在BeginPlay事件触发时打印 "Hello"。
	 *
	 * @param BlueprintName 蓝图资源的名称 (例如 "BP_HelloPrinter")
	 * @param BlueprintPath 蓝图资源将被创建的包路径 (例如 "/Game/MyBlueprints")
	 * @param ParentClass 蓝图将继承的父类，默认为AActor
	 * @return 新创建的 UBlueprint 指针，如果创建失败则为 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Generator")
	static UBlueprint* CreateHelloPrintBlueprint(const FString& BlueprintName, const FString& BlueprintPath, UClass* ParentClass = nullptr);
};
