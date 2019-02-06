// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"

#include "ActionGameCharacter.generated.h"


USTRUCT(BlueprintType)
struct FPlayAttackMontage : public FTableRowBase 
{
	GENERATED_BODY()

	/** Animation Montage **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage;

	/** Montage Section Cound **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AnimationSectionCount;

	/** Discription **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;
};


USTRUCT(BlueprintType)
struct FMeleeCollisionProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Enabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Disabled;

	// default constructor
	FMeleeCollisionProfile()
	{
		Enabled = FName(TEXT("Weapon"));
		Disabled = FName(TEXT("NoCollision"));
	}
};


UENUM(BlueprintType)
enum class ELogLevel : uint8 {
	TRACE		UMETA(DisplayName = "Trace"),
	DEBUG		UMETA(DisplayName = "Debug"),
	INFO		UMETA(DisplayName = "Info"),
	WARNING		UMETA(DisplayName = "Warning"),
	ERROR		UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ELogOutput : uint8 {
	ALL			UMETA(DisplayName = "All levels"),
	OUTPUT_LOG	UMETA(DisplayName = "Output log"),
	SCREEN		UMETA(DisplayName = "Screen")
};

UENUM(BlueprintType)
enum class EAttackType: uint8 {
	MELEE_FIST	UMETA(DisplayName = "Melee - Fist")
};

UCLASS(config=Game)
class AActionGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


	/** Melee fist attack montage **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* MeleeFistAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UDataTable* MeleeAttackDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	class USoundCue* AttackPunchSoundCue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	class USoundCue* PunchThrowSoundCue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightCollisionBox;

public:
	AActionGameCharacter();

	// called when the game starts or when the player spawned
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);


	/** Call for Punch input */
	void PunchInput();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UAudioComponent* PunchThrowAudioComponent;

	/** Notifiy start**/
	UFUNCTION()
	void AttackNotifyStart();
	
	/** Notifiy end**/
	UFUNCTION()
	void AttackNotifyEnd();
	
	// triggered when the collision hit event fires between enemy
	UFUNCTION()
	void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// triggered when the collider overlaps another component
	UFUNCTION()
	void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// triggered when the collider stops overlaps another component
	UFUNCTION()
	void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
private:
	UAudioComponent* PunchAudioComponent;
	

	FMeleeCollisionProfile MeleeCollisionProfile;
	/** Log - prints message to log outputs **/
	void Log(ELogLevel LogLevel, FString Message);

	/** Log - prints message on the screen **/
	void Log(ELogLevel LogLevel, FString Message, ELogOutput LogOutput);
};

