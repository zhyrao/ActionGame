// Fill out your copyright notice in the Description page of Project Settings.

#include "PunchThrowAnimNotifyState.h"
#include "ActionGameCharacter.h"
#include "Engine.h"

void UPunchThrowAnimNotifyState::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, __FUNCTION__);
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AActionGameCharacter* player = Cast<AActionGameCharacter>(MeshComp->GetOwner());
		if (player != NULL && !player->PunchThrowAudioComponent->IsPlaying())
		{
			player->PunchThrowAudioComponent->Play(0.0f);
		}
	}
}

void UPunchThrowAnimNotifyState::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, __FUNCTION__);
}

void UPunchThrowAnimNotifyState::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, __FUNCTION__);
}
