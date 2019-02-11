// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackAnimNotifyState.h"
#include "ActionGameCharacter.h"
#include "Engine.h"


void UAttackAnimNotifyState::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, __FUNCTION__);

	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AActionGameCharacter* player = Cast<AActionGameCharacter>(MeshComp->GetOwner());
		if (player != NULL)
		{
			player->AttackNotifyStart();
		}
	}
}

void UAttackAnimNotifyState::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AActionGameCharacter* player = Cast<AActionGameCharacter>(MeshComp->GetOwner());
		if (player != NULL)
		{
			if (player->GetCurrentAttackType() == EAttackType::MELEE_KICK)
			{
				player->SetIsKeyboardEnabled(false);
			}
		}
	}
}

void UAttackAnimNotifyState::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, __FUNCTION__);
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AActionGameCharacter* player = Cast<AActionGameCharacter>(MeshComp->GetOwner());
		if (player != NULL)
		{
			player->AttackNotifyEnd();		
			player->SetIsKeyboardEnabled(true);
		}
	}
}
