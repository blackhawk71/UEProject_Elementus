// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "GAS/System/PEEffectData.h"
#include "PEEquipment.generated.h"

class UGameplayAbility;

/**
 *
 */
UCLASS(Abstract, Blueprintable, Category = "Project Elementus | Classes")
class UPEEquipment : public UObject
{
	GENERATED_BODY()

public:
	explicit UPEEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties", Meta = (TitleProperty = "{EffectClass}"))
	TArray<FGameplayEffectGroupedData> EquipmentEffects;

	UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
	FGameplayTagContainer EquipmentSlotTags;

	UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
	TMap<FName, TSubclassOf<UGameplayAbility>> EquipmentAbilities;

	UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
	TSoftObjectPtr<USkeletalMesh> EquipmentMesh;

	UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
	FName SocketToAttach = NAME_None;
};
