// Fill out your copyright notice in the Description page of Project Settings.

#include "PunchThrowAnimNotify.h"
#include "ActionGameCharacter.h"
#include "Engine.h"

void UPunchThrowAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	/*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, __FUNCTION__);
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		AActionGameCharacter* player = Cast<AActionGameCharacter>(MeshComp->GetOwner());
		if (player != NULL && !player->PunchThrowAudioComponent->IsPlaying())
		{
			player->PunchThrowAudioComponent->Play(0.0f);
		}
	}*/
}
