// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Components/PEInventoryComponent.h"
#include "Actors/Interfaces/PEEquipment.h"
#include "Actors/Character/PECharacter.h"
#include "GAS/System/PEAbilitySystemComponent.h"
#include "GAS/System/PEAbilityFunctions.h"
#include "Management/Data/PEGlobalTags.h"
#include <Management/ElementusInventoryFunctions.h>
#include <MFEA_Settings.h>

UPEInventoryComponent::UPEInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	if (const UMFEA_Settings* MF_Settings = GetDefault<UMFEA_Settings>();
		!MF_Settings->InputIDEnumeration.IsNull())
	{
		InputEnumHandle = MF_Settings->InputIDEnumeration.LoadSynchronous();
	}
}

bool UPEInventoryComponent::CanGiveItem(const FElementusItemInfo InItemInfo) const
{
	// We cannot give the item if it is currently equiped
	return Super::CanGiveItem(InItemInfo) && !InItemInfo.Tags.HasTag(FGameplayTag::RequestGameplayTag(GlobalTag_EquipSlot_Base));
}

bool UPEInventoryComponent::EquipItem(const FElementusItemInfo& InItem)
{	
	if (!IsValid(GetOwner()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Invalid owning actor"), *FString(__func__));
		return false;
	}

	if (!ContainsItem(InItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - The item %s is not in the %s's inventory"), *FString(__func__), *InItem.ItemId.ToString(), *GetOwner()->GetName());
		return false;
	}

	if (!GetOwner()->GetClass()->IsChildOf<APECharacter>())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - Owning actor isn't child of PECharacter"), *FString(__func__));
		return false;
	}

	if (const UElementusItemData* const ItemData = UElementusInventoryFunctions::GetSingleItemDataById(InItem.ItemId, { "SoftData" }, false))
	{
		if (UPEEquipment* const EquipedItem = Cast<UPEEquipment>(ItemData->ItemClass.LoadSynchronous()->GetDefaultObject()))
		{
			const FGameplayTagContainer EquipmentSlotTags = EquipedItem->EquipmentSlotTags;
			if (int32 FoundIndex;
				FindFirstItemIndexWithTags(EquipmentSlotTags, FoundIndex))
			{
				// Already equipped
				UE_LOG(LogTemp, Display, TEXT("%s - Actor %s has already unequipped item %s"), *FString(__func__), *GetOwner()->GetName(), *InItem.ItemId.ToString());
				
				UnequipItem(GetItemReferenceAt(FoundIndex));
				return false;
			}

			if (int32 FoundIndex;
				FindFirstItemIndexWithInfo(InItem, FoundIndex))
			{
				ProcessEquipmentAddition_Internal(Cast<APECharacter>(GetOwner()), EquipedItem);

				for (const FGameplayTag& Iterator : EquipmentSlotTags)
				{
					EquipmentMap.Add(Iterator, InItem);
				}				
				GetItemReferenceAt(FoundIndex).Tags.AppendTags(EquipmentSlotTags);

				UE_LOG(LogTemp, Display, TEXT("%s - Actor %s equipped %s"), *FString(__func__), *GetOwner()->GetName(), *InItem.ItemId.ToString());
				OnInventoryUpdate.Broadcast();				

				UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s - Failed to find item %s in %s's inventory"), *FString(__func__), *InItem.ItemId.ToString(), *GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s - Failed to cast item %s to equipment class"), *FString(__func__), *InItem.ItemId.ToString())
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Failed to load item %s"), *FString(__func__), *InItem.ItemId.ToString())
	}

	UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
	return false;
}

bool UPEInventoryComponent::UnequipItem(FElementusItemInfo& InItem)
{
	if (!IsValid(GetOwner()))
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Invalid owning actor"), *FString(__func__));
		return false;
	}

	if (!ContainsItem(InItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - The item %s is not in the %s's inventory"), *FString(__func__), *InItem.ItemId.ToString(), *GetOwner()->GetName());
		return false;
	}

	if (!GetOwner()->GetClass()->IsChildOf<APECharacter>())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - Owning actor isn't child of PECharacter"), *FString(__func__));
		return false;
	}

	if (const UElementusItemData* const ItemData = UElementusInventoryFunctions::GetSingleItemDataById(InItem.ItemId, { "SoftData" }, false))
	{
		if (UPEEquipment* const EquipedItem = Cast<UPEEquipment>(ItemData->ItemClass.LoadSynchronous()->GetDefaultObject()))
		{
			ProcessEquipmentRemoval_Internal(Cast<APECharacter>(GetOwner()), EquipedItem);
			
			const FGameplayTagContainer EquipmentSlotTags = EquipedItem->EquipmentSlotTags;			
			for (const FGameplayTag& Iterator : EquipmentSlotTags)
			{
				EquipmentMap.Remove(Iterator);
			}			
			InItem.Tags.RemoveTags(EquipmentSlotTags);

			UE_LOG(LogTemp, Display, TEXT("%s - Actor %s unequipped %s"), *FString(__func__), *GetOwner()->GetName(), *InItem.ItemId.ToString());
			OnInventoryUpdate.Broadcast();

			UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s - Failed to cast item %s to equipment class"), *FString(__func__), *InItem.ItemId.ToString())
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Failed to load item %s"), *FString(__func__), *InItem.ItemId.ToString())
	}

	UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
	return false;
}

void UPEInventoryComponent::ProcessEquipmentAddition_Internal(APECharacter* OwningCharacter, UPEEquipment* Equipment)
{	
	if (UPEAbilitySystemComponent* const TargetABSC = Cast<UPEAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent()))
	{
		AddEquipmentGASData_Server(TargetABSC, Equipment);		
		TargetABSC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(GlobalTag_WeaponSlot_Base));
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		ProcessEquipmentAttachment_Multicast(OwningCharacter->GetMesh(), Equipment);
	}
	else
	{
		ProcessEquipmentAttachment_Server(OwningCharacter->GetMesh(), Equipment);
	}
}

void UPEInventoryComponent::ProcessEquipmentRemoval_Internal(APECharacter* OwningCharacter, UPEEquipment* Equipment)
{
	if (UPEAbilitySystemComponent* const TargetABSC = Cast<UPEAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent()))
	{
		RemoveEquipmentGASData_Server(TargetABSC, Equipment);		
		TargetABSC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(GlobalTag_WeaponSlot_Base));
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		ProcessEquipmentDettachment_Multicast(Equipment);
	}
	else
	{
		ProcessEquipmentDettachment_Server(Equipment);
	}
}

void UPEInventoryComponent::AddEquipmentGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UPEEquipment* Equipment)
{
	// Add equipment effects
	for (const FGameplayEffectGroupedData& Effect : Equipment->EquipmentEffects)
	{
		TargetABSC->ApplyEffectGroupedDataToSelf(Effect);
	}
	
	// Add equipment abilities
	for (const auto& [InInputID_Name, InAbilityClass] : Equipment->EquipmentAbilities)
	{
		if (InInputID_Name.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s - Invalid InputID"), *FString(__func__));
			continue;
		}
		if (!IsValid(InAbilityClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s - Invalid Ability Class"), *FString(__func__));
			continue;
		}

		UE_LOG(LogTemp, Display, TEXT("%s - Binding ability %s with InputId %s"), *FString(__func__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
		UPEAbilityFunctions::GiveAbility(TargetABSC, InAbilityClass, InInputID_Name, InputEnumHandle.Get(), false, true);
	}
}

void UPEInventoryComponent::RemoveEquipmentGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UPEEquipment* Equipment)
{
	// Remove equipment effects
	for (const FGameplayEffectGroupedData& Effect : Equipment->EquipmentEffects)
	{
		TargetABSC->RemoveEffectGroupedDataFromSelf(Effect, TargetABSC, 1);
	}
	
	// Remove equipment abilities
	for (const auto& [InInputID_Name, InAbilityClass] : Equipment->EquipmentAbilities)
	{
		if (InInputID_Name.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s - Invalid InputID"), *FString(__func__));
			continue;
		}
		if (!IsValid(InAbilityClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s - Invalid Ability Class"), *FString(__func__));
			continue;
		}

		UE_LOG(LogTemp, Display, TEXT("%s - Removing ability %s with InputId %s"), *FString(__func__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
		UPEAbilityFunctions::RemoveAbility(TargetABSC, InAbilityClass);
	}
}

void UPEInventoryComponent::ProcessEquipmentAttachment_Server_Implementation(USkeletalMeshComponent* TargetMesh, UPEEquipment* Equipment)
{
	ProcessEquipmentAttachment_Multicast(TargetMesh, Equipment);
}

void UPEInventoryComponent::ProcessEquipmentAttachment_Multicast_Implementation(USkeletalMeshComponent* TargetMesh, UPEEquipment* Equipment)
{
	USkeletalMeshComponent* const InMesh = NewObject<USkeletalMeshComponent>(GetOwner(), USkeletalMeshComponent::StaticClass(), *Equipment->GetName());
	if (!IsValid(InMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Failed to create skeletal mesh"), *FString(__func__));
		return;
	}

	InMesh->SetIsReplicated(true);
	InMesh->SetSkeletalMesh(Equipment->EquipmentMesh.LoadSynchronous());
	InMesh->ComponentTags.Add(*FString::Printf(TEXT("ElementusEquipment_%s"), *Equipment->GetName()));

	GetOwner()->AddOwnedComponent(InMesh);

	if (!InMesh->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Equipment->SocketToAttach))
	{
		UE_LOG(LogTemp, Error, TEXT("%s - Failed to attach mesh to character"), *FString(__func__));
	}

	GetOwner()->FinishAndRegisterComponent(InMesh);
}

void UPEInventoryComponent::ProcessEquipmentDettachment_Server_Implementation(UPEEquipment* Equipment)
{
	ProcessEquipmentDettachment_Multicast(Equipment);
}

void UPEInventoryComponent::ProcessEquipmentDettachment_Multicast_Implementation(UPEEquipment* Equipment)
{
	const TArray<UActorComponent*> CompArr = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), *FString::Printf(TEXT("ElementusEquipment_%s"), *Equipment->GetName()));

	if (CompArr.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - %s have no equipment attached"), *FString(__func__), *GetOwner()->GetName());
		return;
	}

	for (UActorComponent* const& Iterator : CompArr)
	{
		Iterator->UnregisterComponent();
		Iterator->RemoveFromRoot();
		Iterator->DestroyComponent();
	}
}