#include "UserInterface/UW_InteractionTarget.h"

#include "InputAction.h"
#include "InteractionLog.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/GridPanel.h"
#include "Components/InteractionTarget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetTextLibrary.h"


#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UUW_InteractionTarget::UUW_InteractionTarget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), ContentSwitcher(nullptr), InteractionInfoBox(nullptr),
                                                                                            InteractionContainer(nullptr), LootKey(nullptr),
                                                                                            InfoText(nullptr),
                                                                                            POI_Container(nullptr), Icon(nullptr),
                                                                                            POI_Icon(nullptr),
                                                                                            Foreground(nullptr), Triangle(nullptr), Background(nullptr),
                                                                                            TriangleBox(nullptr),
                                                                                            SlideUp(nullptr),
                                                                                            LootText(nullptr),
                                                                                            InteractionRepeatUpdate(nullptr),
                                                                                            InteractionCompleted(nullptr), Flash(nullptr),
                                                                                            InteractionTextAnim(nullptr), InteractionCanceled(nullptr),
                                                                                            Rotate(nullptr), T_Circle(nullptr),
                                                                                            bDisplayProgress(false), bClearTarget(false),
                                                                                            bIsInteractionState(false),
                                                                                            CustomPercent(0),
                                                                                            CurrentPercent(0),
                                                                                            ScreenRadiusPercent(0)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> T_CircleObject(TEXT("/Script/Engine.Texture2D'/Interaction/UserInterface/Textures/T_Circle.T_Circle'"));
	if (T_CircleObject.Succeeded())
	{
		T_Circle = T_CircleObject.Object;
	}
}

void UUW_InteractionTarget::NativeConstruct()
{
	Super::NativeConstruct();

	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));

	SetPositionInViewport(FVector2D(0));

	SetInteractionPercent(0.0f);

	UpdateInteractionTarget(nullptr);


	FWidgetAnimationDynamicEvent StartedDelegate;
	StartedDelegate.BindDynamic(this, &UUW_InteractionTarget::OnSlideUpAnimStarted);
	BindToAnimationStarted(SlideUp, StartedDelegate);

	FWidgetAnimationDynamicEvent FinishedDelegate;
	FinishedDelegate.BindDynamic(this, &UUW_InteractionTarget::OnSlideUpAnimFinished);
	BindToAnimationFinished(SlideUp, FinishedDelegate);
}

void UUW_InteractionTarget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePositionInViewport();
}

void UUW_InteractionTarget::SetInteractionKeyText(const FKey& InKey)
{
	if (!IsValid(WidgetInteractionTarget))
	{
		LOG_WARNING_AND_SCREEN(5.0f, TEXT("WidgetInteractionTarget가 설정되어 있지 않습니다"))
		return;
	}

	if (WidgetInteractionTarget->bDisplayInteractionKey)
	{
		LootKey->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (!UKismetInputLibrary::Key_IsKeyboardKey(InKey))
			return;

		FString KeyUpperName = InKey.GetDisplayName(true).ToString().ToUpper();
		LootKey->SetText(UKismetTextLibrary::Conv_StringToText(KeyUpperName));
	}
	else
	{
		LootKey->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUW_InteractionTarget::ClearWidgetData()
{
	bClearTarget = false;
	bIsInteractionState = false;
	WidgetPositionErrorFactor = FVector2D::Zero();
	WidgetInteractionTarget = nullptr;
	CurrentPercent = 0.0f;
	bCustomClockwise = true;
	SetAlignmentInViewport(FVector2D(0.5, 0.5));
}

void UUW_InteractionTarget::ToggleLootText(bool bIsVisible)
{
	StopAnimation(LootText);
	EUMGSequencePlayMode::Type UMGSequencePlayMode = bIsVisible ? EUMGSequencePlayMode::Reverse : EUMGSequencePlayMode::Forward;
	PlayAnimation(LootText, GetAnimationCurrentTime(LootText), 1, UMGSequencePlayMode);
}

void UUW_InteractionTarget::OnWidgetNewTarget(bool bIsNewTarget)
{
	if (bIsNewTarget)
	{
		ToggleLootText(true);
		if (!bIsInteractionState)
		{
			bIsInteractionState = true;
			PlayAnimationForward(SlideUp);
		}
	
	}
	else
	{
		if (!IsValid(WidgetInteractionTarget))
			return;

		SetInteractionPercent(0.0f);
		ToggleLootText(true);

		UObject* BrushImage = Icon->Background.GetResourceObject();
		if (!IsValid(BrushImage))
			UpdateContentState(false);
	}
}

void UUW_InteractionTarget::SetInteractionPercent(float NewPercent)
{
	// 현재 진행률을 업데이트합니다.
	CurrentPercent = NewPercent;

	// 진행률 표시가 활성화되어 있는 경우
	if (bDisplayProgress)
	{
		// Foreground 머티리얼의 동적 인스턴스를 가져옵니다.
		UMaterialInstanceDynamic* ForegroundDynamicMat = Foreground->GetDynamicMaterial();

		// ForegroundDynamicMat이 유효하다면, 퍼센트 값을 설정합니다.
		if (IsValid(ForegroundDynamicMat))
			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("Percent")), NewPercent);

		// 진행률이 0이면 Foreground를 숨깁니다.
		if (CurrentPercent == 0.0f)
		{
			if (Foreground->IsVisible())
				Foreground->SetVisibility(ESlateVisibility::Collapsed);
		}
		// 그렇지 않으면 Foreground를 표시합니다.
		else
		{
			if (!Foreground->IsVisible())
				Foreground->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else // 진행률 표시가 비활성화되어 있는 경우
	{
		UMaterialInstanceDynamic* ForegroundDynamicMat = Foreground->GetDynamicMaterial();
		if (!IsValid(ForegroundDynamicMat))
			return;

		// NewPercent가 0.75 이하인 경우
		if (NewPercent <= 0.75f)
		{
			float DeltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(this) / 1.0f;
			double ClockWiseTrueValue = FMath::Min(CustomPercent + DeltaSeconds, 1.0f);
			double ClockWiseFalseValue = FMath::Max(CustomPercent - DeltaSeconds, 0.0f);
			CustomPercent = bCustomClockwise ? ClockWiseTrueValue : ClockWiseFalseValue;

			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("Percent")), CustomPercent);
			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("Is Clockwise")), 1.0f);
			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("StartPoint")), 1.0f - CustomPercent);

			// 진행률이 최대값에 도달했을 때, 방향을 반전시킵니다.
			if (CustomPercent == 1.0f)
			{
				bCustomClockwise = false;
			}
			// 진행률이 최소값에 도달했을 때, 방향을 원래대로 돌립니다.
			else if (CustomPercent == 0.0f)
			{
				bCustomClockwise = true;
			}
		}
		else
		{
			float DeltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(this) / (WidgetInteractionTarget->HoldSeconds / 10.0f);
			CustomPercent = FMath::FInterpConstantTo(CustomPercent, CurrentPercent, DeltaSeconds, 0.75f);
			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("Percent")), CustomPercent);
			ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("StartPoint")), 1.0f - CustomPercent);
		}
	}
}

void UUW_InteractionTarget::ResetProgress()
{
	// 상호작용 진행 퍼센트를 초기값으로 설정합니다.
	CustomPercent = 0.0f;

	// 진행 방향을 시계 방향으로 초기화합니다.
	bCustomClockwise = true;

	// Foreground 컴포넌트에서 동적 머티리얼 인스턴스를 가져옵니다.
	UMaterialInstanceDynamic* ForegroundDynamicMat = Foreground->GetDynamicMaterial();

	// 머티리얼이 유효한지 확인합니다.
	if (!IsValid(ForegroundDynamicMat))
		return;

	// 머티리얼의 "Percent" 파라미터를 0으로 설정하여 진행 상태를 초기화합니다.
	ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("Percent")), 0.0f);

	// 머티리얼의 "StartPoint" 파라미터를 0으로 설정하여 시작 지점을 초기화합니다.
	ForegroundDynamicMat->SetScalarParameterValue(FName(TEXT("StartPoint")), 0.0f);
}

void UUW_InteractionTarget::PlayInteractionUpdateAnimation()
{
	PlayAnimation(InteractionRepeatUpdate, 0, 1, EUMGSequencePlayMode::PingPong);
}

void UUW_InteractionTarget::PlayInteractionCompletedAnimation(EInteractionResult InteractionResult)
{
	// 상호작용이 완료된 경우
	if (InteractionResult == EInteractionResult::Completed)
	{
		// 상호작용 텍스트를 "Done"으로 업데이트합니다.
		UpdateInteractionText(true, EInteractionState::Done);
		// SlideUp 애니메이션을 중지합니다.
		StopAnimation(SlideUp);
		// 상호작용 타겟 초기화
		bClearTarget = false;
		WidgetInteractionTarget = nullptr;
		// 상호작용 완료 애니메이션을 재생합니다.
		PlayAnimation(InteractionCompleted);
	}
	// 상호작용이 취소된 경우
	else if (InteractionResult == EInteractionResult::Canceled)
	{
		// 상호작용 취소 애니메이션을 재생합니다.
		PlayAnimation(InteractionCanceled);

		// 상호작용 타입이 Tap 또는 Hold인 경우
		if (WidgetInteractionTarget->InteractionType == EInteractionType::Tap || WidgetInteractionTarget->InteractionType == EInteractionType::Hold)
		{
			CurrentPercent = 0.0f;
			bCustomClockwise = true;
			ResetProgress();

			// 상호작용 텍스트를 "Waiting"으로 업데이트합니다.
			UpdateInteractionText(false, EInteractionState::Waiting);
		}
	}
}

void UUW_InteractionTarget::UpdateContentState(bool bIsInteraction)
{
	// 상호작용 진행 상태를 초기화합니다.
	ResetProgress();

	// 상호작용이 시작된 경우의 처리
	if (bIsInteraction)
	{
		// 아이콘 이미지 리소스를 가져옵니다.
		UObject* BrushImage = Icon->Background.GetResourceObject();;

		// 아이콘 이미지가 유효하지 않으면 함수 실행을 중지합니다.
		if (!IsValid(BrushImage))
			return;

		// 상호작용 상태를 활성화 상태로 설정합니다.
		bIsInteractionState = true;

		// 상호작용 시작 애니메이션을 실행합니다.
		PlayAnimationForward(SlideUp, 1.0f);

		// 상호작용 타겟 아이콘의 깜박임 애니메이션을 실행합니다.
		PlayFlashAnimation(WidgetInteractionTarget->bFlashTargetIcon);
	}
	else
	{
		UpdateInteractionText(false, EInteractionState::Waiting);
		if (!bIsInteractionState)
			return;

		bIsInteractionState = false;
		PlayAnimationForward(SlideUp);
		PlayFlashAnimation(WidgetInteractionTarget->bFlashPOIIcon);
	}
}

void UUW_InteractionTarget::SetInteractionText()
{
	// 현재 상호작용 텍스트가 비어 있는지 확인합니다.
	if (CurrentInteractionText.IsEmpty())
	{
		// 상호작용 텍스트가 비어 있으면, InfoText 위젯을 숨깁니다.
		InfoText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		// 상호작용 텍스트가 비어 있지 않으면, InfoText 위젯에 텍스트를 설정하고 가시성을 설정합니다.
		InfoText->SetText(CurrentInteractionText);
		InfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UUW_InteractionTarget::UpdateInteractionText(bool bIsImmediately, EInteractionState InteractionState)
{
	// 주어진 상호작용 상태에 대한 텍스트가 존재하는지 확인합니다.
	if (!WidgetInteractionTarget->InteractionText.Contains(InteractionState))
		return;

	// 해당 상호작용 상태에 맞는 텍스트를 가져옵니다.
	FText NewInteractionTargetText = *WidgetInteractionTarget->InteractionText.Find(InteractionState);

	// 현재 상호작용 텍스트와 비교하여 다르지 않으면 반환합니다.
	if (!UKismetTextLibrary::NotEqual_TextText(CurrentInteractionText, NewInteractionTargetText))
		return;

	// 상호작용 텍스트를 업데이트합니다.
	CurrentInteractionText = NewInteractionTargetText;

	// 즉시 업데이트가 필요한 경우 텍스트를 설정하고, 그렇지 않으면 애니메이션을 재생합니다.
	if (bIsImmediately)
		SetInteractionText();
	else
		PlayAnimation(InteractionTextAnim);
}

void UUW_InteractionTarget::SetControllerScreenSettings(UInteractionTarget* InteractionTarget)
{
	// 주어진 상호작용 타겟의 화면 반경 비율을 가져와 ScreenRadiusPercent에 설정합니다.
	ScreenRadiusPercent = InteractionTarget->ScreenRadiusPercent;
	// 주어진 상호작용 타겟의 화면 마진을 가져와 ScreenMargin에 설정합니다.
	ScreenMargin = InteractionTarget->WidgetMargin;
}

void UUW_InteractionTarget::SetBrushFromTarget()
{
	// 상호작용 대상이 유효하지 않으면 함수를 종료합니다.
	if (!IsValid(WidgetInteractionTarget))
		return;

	// POI(관심 지점) 아이콘 설정
	const FIconSelector& POI_IconSelector = WidgetInteractionTarget->POI_Icon;
	if (POI_IconSelector.bUseMaterialAsTexture)
	{
		if (IsValid(POI_IconSelector.Material))
			POI_Icon->SetBrushFromMaterial(POI_IconSelector.Material);
		else
			POI_Icon->SetBrushFromTexture(T_Circle);
	}
	else
	{
		if (IsValid(POI_IconSelector.Texture2D))
			POI_Icon->SetBrushFromTexture(POI_IconSelector.Texture2D);
		else
			POI_Icon->SetBrushFromTexture(T_Circle);
	}
	POI_Icon->SetBrushColor(WidgetInteractionTarget->POI_IconColor);

	// 타겟 아이콘 설정
	const FIconSelector& Target_IconSelector = WidgetInteractionTarget->Target_Icon;
	if (Target_IconSelector.bUseMaterialAsTexture)
	{
		if (IsValid(Target_IconSelector.Material))
		{
			Icon->SetBrushFromMaterial(Target_IconSelector.Material);
			Icon->SetBrushColor(WidgetInteractionTarget->Target_IconColor);
			Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Icon->SetBrushFromTexture(nullptr);
			Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (IsValid(Target_IconSelector.Texture2D))
		{
			Icon->SetBrushFromTexture(Target_IconSelector.Texture2D);
			Icon->SetBrushColor(WidgetInteractionTarget->Target_IconColor);
			Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Icon->SetBrushFromTexture(nullptr);
			Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 배경 설정
	const FIconSelector& BackGround_IconSelector = WidgetInteractionTarget->Interaction_Icon_Background;
	if (BackGround_IconSelector.bUseMaterialAsTexture)
	{
		if (IsValid(BackGround_IconSelector.Material))
			Background->SetBrushFromMaterial(BackGround_IconSelector.Material);
		else
			Background->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (IsValid(BackGround_IconSelector.Texture2D))
			Background->SetBrushFromTexture(BackGround_IconSelector.Texture2D);
		else
			Background->SetVisibility(ESlateVisibility::Collapsed);
	}
	Background->SetBrushColor(WidgetInteractionTarget->Interaction_Icon_Background_Color);
	Background->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 전경 설정
	const FIconSelector& Foreground_IconSelector = WidgetInteractionTarget->Interaction_Icon_Foreground;
	if (Foreground_IconSelector.bUseMaterialAsTexture)
	{
		if (IsValid(Foreground_IconSelector.Material))
			Foreground->SetBrushFromMaterial(Foreground_IconSelector.Material);
		else
			Foreground->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (IsValid(Foreground_IconSelector.Texture2D))
			Foreground->SetBrushFromTexture(Foreground_IconSelector.Texture2D);
		else
			Foreground->SetVisibility(ESlateVisibility::Collapsed);
	}
	Foreground->SetBrushColor(WidgetInteractionTarget->Interaction_Icon_Foreground_Color);
	Foreground->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 삼각형 키(TriangleKey)의 가시성을 설정합니다.
	Triangle->SetVisibility(ESlateVisibility::HitTestInvisible);
	TriangleBox->SetVisibility(WidgetInteractionTarget->bDisplayTriangle ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// 진행 표시 여부를 설정합니다.
	bDisplayProgress = WidgetInteractionTarget->bDisplayProgress;
}

void UUW_InteractionTarget::UpdatePositionInViewport()
{
	// 화면 크기와 화면 중앙 좌표를 계산
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	FVector2D ViewportCenter = ViewportSize * 0.5f;

	// 카메라 위치와 월드 방향을 계산합니다.
	FVector CameraLocation;
	FVector CameraDirection;
	UGameplayStatics::DeprojectScreenToWorld(GetOwningPlayer(), ViewportCenter, CameraLocation, CameraDirection);

	// 위젯이 타겟과 연결되어 있는지 확인하고, 마커 위치를 계산합니다.
	if (IsValid(WidgetInteractionTarget))
	{
		USceneComponent* MarkerComponent = WidgetInteractionTarget->MarkerTargetComponent;
		// 마커 컴포넌트의 회전 및 오프셋을 적용한 위치를 계산
		FVector MarkerOffset = UKismetMathLibrary::GreaterGreater_VectorRotator(WidgetInteractionTarget->MarkerOffset, MarkerComponent->GetComponentRotation());
		LastWorldLocation = MarkerComponent->GetComponentLocation() + MarkerOffset;
	}

	// 카메라 회전과 타겟의 회전을 비교하여 타겟 방향을 계산합니다.
	FRotator CameraRotation = GetOwningPlayer()->PlayerCameraManager->GetCameraRotation();
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, LastWorldLocation);
	FVector TargetDirection = UKismetMathLibrary::GetForwardVector(TargetRotation);

	// 카메라 방향과 타겟 방향의 도트 곱을 구하여 타겟이 화면 안에 얼마나 있는지 확인
	double CameraToTargetDot = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(CameraRotation), TargetDirection);

	// 도트 곱의 결과를 바탕으로 화면 상에서의 보정 오프셋을 계산합니다.
	float OffsetFactor = UKismetMathLibrary::MapRangeClamped(CameraToTargetDot, 0.0f, -1.0f, 0.0f, -1.0f);

	// 타겟의 최종 월드 위치를 계산
	FVector AdjustedWorldLocation = TargetDirection + CameraLocation + (UKismetMathLibrary::GetUpVector(TargetRotation) * OffsetFactor) + UKismetMathLibrary::GetRightVector(TargetRotation);

	// 타겟의 월드 좌표를 화면 좌표로 변환
	FVector2D ScreenLocation;
	GetOwningPlayer()->ProjectWorldLocationToScreen(AdjustedWorldLocation, ScreenLocation, true);

	// 화면 중앙에서 타겟의 위치를 기준으로 정규화된 방향을 계산
	FVector2D NormalizedScreenDirection = UKismetMathLibrary::Normal2D(FVector2D(ScreenLocation.X - ViewportCenter.X, ScreenLocation.Y - ViewportCenter.Y));

	// 타겟의 위치에 따른 각도(코사인)를 계산하여 화면 상 X좌표를 계산
	float AngleCos = UKismetMathLibrary::DegCos(UKismetMathLibrary::DegAcos(NormalizedScreenDirection.X));
	float ScreenRadius = UKismetMathLibrary::MapRangeClamped(ScreenRadiusPercent, 0.0f, 1.0f, 0.0f, ViewportSize.Y);
	float ScreenPosX = (AngleCos * ScreenRadius) + ViewportCenter.X;

	// 타겟의 위치에 따른 각도(사인)를 계산하여 화면 상 Y좌표를 계산
	float AngleSin = UKismetMathLibrary::DegSin(UKismetMathLibrary::DegAcos(NormalizedScreenDirection.X));
	float ScreenPosY = AngleSin * ScreenRadius * (NormalizedScreenDirection.Y <= 0 ? -1.0f : 1.0f) + ViewportCenter.Y;

	// 계산된 화면 좌표를 클램핑하여 화면 안에 들어오도록 제한합니다.
	FVector2D ClampedScreenPos = FVector2D(ScreenPosX, ScreenPosY);

	FVector2D WidgetSize = GetDesiredSize();

	// X 좌표 클램핑
	float XMin = WidgetSize.X + FMath::Abs(ScreenMargin.Left);
	float XMax = ViewportSize.X - WidgetSize.X - FMath::Abs(ScreenMargin.Top);
	float ClampedX = FMath::Clamp(ClampedScreenPos.X, XMin, XMax);

	// Y 좌표 클램핑
	float YMin = WidgetSize.Y + FMath::Abs(ScreenMargin.Right);
	float YMax = ViewportSize.Y - WidgetSize.Y - FMath::Abs(ScreenMargin.Bottom);
	float ClampedY = FMath::Clamp(ClampedScreenPos.Y, YMin, YMax);

	// 타겟이 화면 안에 있는지 확인하고, 위치를 결정
	FVector2D FinalScreenPos = FVector2D(ClampedX, ClampedY);

	// 화면 내에서 타겟의 위치를 결정
	bool bIsOnScreen = IsOnScreen(ScreenMargin);

	FVector2D ProjectedScreenPos;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), LastWorldLocation, ProjectedScreenPos, true);
	FVector2D FinalWidgetPos = bIsOnScreen ? ProjectedScreenPos : FinalScreenPos;

	// 계산된 위치에서 위젯 오차 보정 값을 적용
	FVector2D LocalWidgetPos = FinalWidgetPos - WidgetPositionErrorFactor;

	// 위젯의 로컬 좌표로 변환하여 위치 설정
	FGeometry ViewportGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this);
	FVector2D LocalCoordinate;
	USlateBlueprintLibrary::ScreenToWidgetLocal(this, ViewportGeometry, LocalWidgetPos, LocalCoordinate);

	SetRenderTranslation(LocalCoordinate);
}

// void UUW_InteractionTarget::UpdatePositionInViewport()
// {
// 	// 화면 크기와 중앙 좌표 계산
// 	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
// 	FVector2D ViewportCenter = ViewportSize * 0.5f;
//
// 	// 카메라 위치와 월드 방향 계산
// 	FVector CameraLocation;
// 	FVector CameraDirection;
// 	UGameplayStatics::DeprojectScreenToWorld(GetOwningPlayer(), ViewportCenter, CameraLocation, CameraDirection);
//
// 	if (IsValid(WidgetInteractionTarget))
// 	{
// 		USceneComponent* MarkerComponent = WidgetInteractionTarget->MarkerTargetComponent;
// 		FVector MarkerOffset = UKismetMathLibrary::GreaterGreater_VectorRotator(WidgetInteractionTarget->MarkerOffset, MarkerComponent->GetComponentRotation());
// 		LastWorldLocation = MarkerComponent->GetComponentLocation() + MarkerOffset;
// 	}
//
// 	// 카메라 회전 및 방향 계산
// 	FRotator CameraRotation = GetOwningPlayer()->PlayerCameraManager->GetCameraRotation();
// 	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, LastWorldLocation);
// 	FVector TargetDirection = UKismetMathLibrary::GetForwardVector(TargetRotation);
//
// 	double CameraToTargetDot = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(CameraRotation), TargetDirection);
// 	float OffsetFactor = UKismetMathLibrary::MapRangeClamped(CameraToTargetDot, 0.0f, -1.0f, 0.0f, -1.0f);
//
// 	FVector AdjustedWorldLocation = TargetDirection + CameraLocation +
// 		(UKismetMathLibrary::GetUpVector(TargetRotation) * OffsetFactor) +
// 		UKismetMathLibrary::GetRightVector(TargetRotation);
//
// 	// 월드 좌표를 화면 좌표로 변환
// 	FVector2D ScreenLocation;
// 	GetOwningPlayer()->ProjectWorldLocationToScreen(AdjustedWorldLocation, ScreenLocation, true);
//
// 	FVector2D ScreenDirection = UKismetMathLibrary::Normal2D(FVector2D(ScreenLocation.X - ViewportCenter.X, ScreenLocation.Y - ViewportCenter.Y));
//
// 	float DegAos = UKismetMathLibrary::DegAcos(ScreenDirection.X);
// 	float DegCos = UKismetMathLibrary::DegCos(DegAos);
//
// 	float RangeScreenRadiusPercent = UKismetMathLibrary::MapRangeClamped(ScreenRadiusPercent, 0.0f, 1.0f, 0.0f, ViewportSize.Y);
// 	float Vector2DX = (DegCos * RangeScreenRadiusPercent) + ViewportCenter.X;
//
//
// 	float DegSin = UKismetMathLibrary::DegSin(DegAos);
// 	float CalculateY = DegSin * RangeScreenRadiusPercent * (ScreenDirection.Y <= 0 ? -1.0f : 1.0f);
// 	float Vector2DY = CalculateY + ViewportCenter.Y;
//
// 	FVector2D NewMakeVector2D = FVector2D(Vector2DX, Vector2DY);
//
// 	FVector2D DesiredSize = GetDesiredSize();
//
// 	float XMin = DesiredSize.X + FMath::Abs(ScreenMargin.Left);
// 	float XMax = ViewportSize.X - DesiredSize.X - FMath::Abs(ScreenMargin.Top);
//
// 	float NewX = FMath::Clamp(NewMakeVector2D.X, XMin, XMax);
//
// 	float YMin = DesiredSize.Y + FMath::Abs(ScreenMargin.Right);
// 	float YMax = DesiredSize.Y - DesiredSize.Y - FMath::Abs(ScreenMargin.Bottom);
// 	float NewY = FMath::Clamp(NewMakeVector2D.Y, YMin, YMax);
//
// 	FVector2D FalseVector2D = FVector2D(NewX, NewY);
//
// 	bool bIsOnScreen = IsOnScreen(ScreenMargin);
//
// 	FVector2D ProScreenPosition;
// 	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), LastWorldLocation, ProScreenPosition, true);
// 	FVector2D SelectVector2D = bIsOnScreen ? ProScreenPosition : FalseVector2D;
//
// 	FVector2D InScreenPosition = SelectVector2D - WidgetPositionErrorFactor;
//
// 	FGeometry Geometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this);
// 	FVector2D LocalCoordinate;
// 	USlateBlueprintLibrary::ScreenToWidgetLocal(this, Geometry, InScreenPosition, LocalCoordinate);
//
// 	SetRenderTranslation(LocalCoordinate);
// }

bool UUW_InteractionTarget::IsOnScreen(FMargin InMargin)
{
	// 상호작용 타겟의 월드 위치를 화면 위치로 변환합니다.
	FVector2D ScreenPosition;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), LastWorldLocation, ScreenPosition, true);

	// 화면의 크기와 화면 중심 좌표를 계산합니다.
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	FVector2D ViewportCenter = ViewportSize * 0.5f;

	// 화면의 좌우 및 상하 위치에 따른 마진 값을 설정합니다.
	float VerticalMargin = ScreenPosition.X < ViewportCenter.X ? InMargin.Top : InMargin.Bottom;
	float HorizontalMargin = ScreenPosition.Y < ViewportCenter.Y ? InMargin.Left : InMargin.Right;

	// 상호작용 타겟이 화면 안에 위치하는지 확인하여 결과를 반환합니다.
	return UKismetMathLibrary::InRange_FloatFloat(ScreenPosition.X, VerticalMargin, ViewportSize.X - VerticalMargin) &&
		UKismetMathLibrary::InRange_FloatFloat(ScreenPosition.Y, HorizontalMargin, ViewportSize.Y - HorizontalMargin);
}

void UUW_InteractionTarget::UpdateInteractionTarget(UInteractionTarget* InteractionTarget)
{
	// 새로운 상호작용 타겟이 유효할 경우
	if (IsValid(InteractionTarget))
	{
		// 위젯의 데이터를 초기화하고 새로운 타겟을 설정합니다.
		ClearWidgetData();
		WidgetInteractionTarget = InteractionTarget;
		SetControllerScreenSettings(WidgetInteractionTarget);
		SetBrushFromTarget();

		// 상호작용 정보 박스를 초기화하고, 초기 애니메이션을 설정합니다.
		InteractionInfoBox->SetRenderOpacity(0.0f);
		PlayFlashAnimation(WidgetInteractionTarget->bFlashPOIIcon);
		SetInteractionPercent(0.0f);
		UpdatePositionInViewport();
		UpdateInteractionText(true, EInteractionState::Waiting);

		// 상호작용 상태에 따라 적절한 컨텐츠를 표시합니다.
		if (bIsInteractionState)
			ContentSwitcher->SetActiveWidget(InteractionContainer);
		else
			ContentSwitcher->SetActiveWidget(POI_Container);

		// 슬라이드 업 애니메이션을 재생합니다.
		PlayAnimation(SlideUp, 0, 1, EUMGSequencePlayMode::Reverse);

		// 상호작용 아이콘의 회전 애니메이션을 처리합니다.
		if (WidgetInteractionTarget->bRotateInteractionIcon)
		{
			PlayAnimation(Rotate, 0, 0);
			return;
		}

		// 이미 회전 애니메이션이 재생 중일 경우, 이를 역방향으로 재생합니다.
		if (IsAnimationPlaying(Rotate))
			PlayAnimation(Rotate, 0, 1, EUMGSequencePlayMode::Reverse, 2.0f);
	}
	// 새로운 상호작용 타겟이 유효하지 않을 경우, 현재 타겟을 제거합니다.
	else
	{
		bClearTarget = true;
		PlayAnimationForward(SlideUp);
	}
}

void UUW_InteractionTarget::PlayFlashAnimation(bool bIsCondition)
{
	if (bIsCondition)
		PlayAnimation(Flash, 0, 1, EUMGSequencePlayMode::PingPong);
	else
		PlayAnimation(Flash, 0, 1, EUMGSequencePlayMode::Reverse);
}

void UUW_InteractionTarget::OnSlideUpAnimStarted()
{
}

void UUW_InteractionTarget::OnSlideUpAnimFinished()
{
	if (IsAnimationPlayingForward(SlideUp))
	{
		if (bClearTarget)
		{
			ClearWidgetData();
		}
		else
		{
			if (bIsInteractionState)
			{
				ContentSwitcher->SetActiveWidget(InteractionContainer);
				SetAlignmentInViewport(FVector2D(0, 0.5f));
				UObject* BrushImage = Icon->Background.GetResourceObject();
				WidgetPositionErrorFactor = IsValid(BrushImage) ? FVector2D(20, -20) : FVector2D(15, -5);
			}
			else
			{
				ContentSwitcher->SetActiveWidget(POI_Container);
				SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
				WidgetPositionErrorFactor = FVector2D::Zero();
			}
			PlayAnimation(SlideUp);
		}
	}
	else
	{
		if (bIsInteractionState)
		{
			ContentSwitcher->SetActiveWidget(InteractionContainer);
			PlayAnimationForward(SlideUp);
		}
	}
}
