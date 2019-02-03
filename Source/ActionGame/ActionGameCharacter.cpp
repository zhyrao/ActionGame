// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ActionGameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Engine.h"
#include "UnrealMathUtility.h"

//////////////////////////////////////////////////////////////////////////
// AActionGameCharacter

AActionGameCharacter::AActionGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeFistAttackMontageObject(TEXT("AnimMontage'/Game/Resources/Animations/MeleeFistAttackMontage.MeleeFistAttackMontage'"));
	if (MeleeFistAttackMontageObject.Succeeded())
	{
		MeleeFistAttackMontage = MeleeFistAttackMontageObject.Object;
	}

	LeftCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftCollisionBox"));
	LeftCollisionBox->SetupAttachment(RootComponent);
	LeftCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);

	LeftCollisionBox->SetWorldScale3D(FVector(0.18f));
	LeftCollisionBox->SetHiddenInGame(false);
	LeftCollisionBox->SetNotifyRigidBodyCollision(false); // collision simulation generates hit events


	RightCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightCollisionBox"));
	RightCollisionBox->SetupAttachment(RootComponent);
	RightCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);

	RightCollisionBox->SetWorldScale3D(FVector(0.18f));
	RightCollisionBox->SetHiddenInGame(false);
	RightCollisionBox->SetNotifyRigidBodyCollision(false); // collision simulation generates hit events


	// sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> AttackPunchSoundCueObject(TEXT("SoundCue'/Game/Resources/Audio/AttackPunchCue.AttackPunchCue'"));
	if (AttackPunchSoundCueObject.Succeeded())
	{
		AttackPunchSoundCue = AttackPunchSoundCueObject.Object;

		PunchAudioComponent = CreateDefaultSubobject<UAudioComponent>("PunchAudioComponent");
		PunchAudioComponent->SetupAttachment(RootComponent);		
	}
}

void AActionGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// attach collision to sockets based on transformation definitions
	const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	LeftCollisionBox->AttachToComponent(GetMesh(), AttachmentTransformRules, "fist_l_collision");
	RightCollisionBox->AttachToComponent(GetMesh(), AttachmentTransformRules, "fist_r_collision");

	LeftCollisionBox->OnComponentHit.AddDynamic(this, &AActionGameCharacter::OnAttackHit);
	RightCollisionBox->OnComponentHit.AddDynamic(this, &AActionGameCharacter::OnAttackHit);

	//LeftCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AActionGameCharacter::OnAttackOverlapBegin);
	//RightCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AActionGameCharacter::OnAttackOverlapBegin);
	//
	//LeftCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AActionGameCharacter::OnAttackOverlapEnd);
	//RightCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AActionGameCharacter::OnAttackOverlapEnd);

	if (PunchAudioComponent != NULL && AttackPunchSoundCue != NULL)
	{
		PunchAudioComponent->SetSound(AttackPunchSoundCue);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AActionGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Punch", IE_Pressed, this, &AActionGameCharacter::PunchInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &AActionGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AActionGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AActionGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AActionGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AActionGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AActionGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AActionGameCharacter::OnResetVR);
}

void AActionGameCharacter::PunchInput()
{
	Log(ELogLevel::INFO, __FUNCTION__);

	// generate random index 1-3
	int AnimSectionIndex = rand() % 3 + 1;
	// combine to section name
	FString AnimSectionName = "start_" + FString::FromInt(AnimSectionIndex);

	PlayAnimMontage(MeleeFistAttackMontage, 1.0f, FName(*AnimSectionName));
}

void AActionGameCharacter::AttackNotifyStart()
{
	Log(ELogLevel::INFO, __FUNCTION__);

	LeftCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Enabled);
	LeftCollisionBox->SetNotifyRigidBodyCollision(true);
	//LeftCollisionBox->SetGenerateOverlapEvents(true);

	RightCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Enabled);
	RightCollisionBox->SetNotifyRigidBodyCollision(true);
	//RightCollisionBox->SetGenerateOverlapEvents(true);
}

void AActionGameCharacter::AttackNotifyEnd()
{
	Log(ELogLevel::INFO, __FUNCTION__);

	LeftCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);
	LeftCollisionBox->SetNotifyRigidBodyCollision(false);
	//LeftCollisionBox->SetGenerateOverlapEvents(false);

	RightCollisionBox->SetCollisionProfileName(MeleeCollisionProfile.Disabled);
	RightCollisionBox->SetNotifyRigidBodyCollision(false);
	//RightCollisionBox->SetGenerateOverlapEvents(false);
}


void AActionGameCharacter::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Log(ELogLevel::WARNING, __FUNCTION__);
	Log(ELogLevel::INFO, Hit.Actor->GetName());
	if (PunchAudioComponent != NULL && !PunchAudioComponent->IsPlaying())
	{
		// default pitch value 1.0f
		PunchAudioComponent->SetPitchMultiplier(FMath::RandRange(1.f, 1.3f));
		PunchAudioComponent->Play(0.f);
	}
}

void AActionGameCharacter::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Log(ELogLevel::WARNING, __FUNCTION__);
}

void AActionGameCharacter::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Log(ELogLevel::WARNING, __FUNCTION__);
}

void AActionGameCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AActionGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AActionGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AActionGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AActionGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AActionGameCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AActionGameCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


//========= LOG ON SCREEN =========//
void AActionGameCharacter::Log(ELogLevel LogLevel, FString Message)
{
	Log(LogLevel, Message, ELogOutput::ALL);
}

void AActionGameCharacter::Log(ELogLevel LogLevel, FString Message, ELogOutput LogOutput)
{
	// only print when screen is selected and the GEngine is availiable
	if ((LogOutput == ELogOutput::ALL || LogOutput == ELogOutput::SCREEN) && GEngine)
	{
		// default color
		FColor LogColor = FColor::Cyan;
		// change color based on the type
		switch (LogLevel)
		{
		case ELogLevel::TRACE:
			LogColor = FColor::Green;
			break;
		case ELogLevel::DEBUG:
			LogColor = FColor::Cyan;
			break;
		case ELogLevel::INFO:
			LogColor = FColor::White;
			break;
		case ELogLevel::WARNING:
			LogColor = FColor::Yellow;
			break;
		case ELogLevel::ERROR:
			LogColor = FColor::Red;
			break;
		default:
			break;
		}

		// print message on the screen for duration time
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, LogColor, Message);
	}
	if (LogOutput == ELogOutput::ALL || LogOutput == ELogOutput::SCREEN)
	{
		switch (LogLevel)
		{
		case ELogLevel::TRACE:
			UE_LOG(LogTemp, VeryVerbose, TEXT("%s"), *Message);
			break;
		case ELogLevel::DEBUG:
			UE_LOG(LogTemp, Verbose, TEXT("%s"), *Message);
			break;
		case ELogLevel::INFO:
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
			break;
		case ELogLevel::WARNING:
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
			break;
		case ELogLevel::ERROR:
			UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
			break;
		default:
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
			break;
		}
	}
}


