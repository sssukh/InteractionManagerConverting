#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.h"
#include "Components/ActorComponent.h"
#include "InteractionTarget.generated.h"

class AInteractableActor;
class UInteractionFinish;
class UInteractionManager;
class USphereComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionBegin, APawn*, InInteractorPawn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionEnd, EInteractionResult, InInteractionResult, APawn*, InInteractorPawn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractionUpdated, float, InAlpha, int32, InRepeated, APawn*, InInteractorPawn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionReactivated, APawn*, ForPawn);


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionDeactivated);

UCLASS(Blueprintable, BlueprintType, ClassGroup=("Interaction Plugin"), meta=(BlueprintSpawnableComponent))
class INTERACTION_API UInteractionTarget : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionTarget();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	/*========================================================================================
	*	SetUp
	=========================================================================================*/
protected:
	void ConstructOwnerEssentials();

	/**
     * @brief 상호작용 대상의 충돌 영역(InnerZone, OuterZone)을 생성 및 설정하는 함수입니다.
     * 이 함수는 상호작용 대상 컴포넌트의 충돌 영역을 동적으로 생성하고, 해당 영역에 대해 필요한 속성들을 초기화합니다.
     * InnerZone과 OuterZone은 각각 내부와 외부의 충돌 영역을 나타내며, 각각의 겹침 이벤트(OnComponentBeginOverlap, OnComponentEndOverlap)에 대해 바인딩합니다.
     */
	void ConstructOverlapZones();

	/**
     * @brief 상호작용 대상의 하이라이트할 컴포넌트를 설정하는 함수입니다.
     * 이 함수는 소유자의 루트 컴포넌트 및 자식 컴포넌트들 중에서 특정 이름과 일치하는 PrimitiveComponent를 찾아,하이라이트 리스트(HighlightedComponents)에 추가합니다.
     */
	void ConstructHighlightedComponents();

	/**
     * @brief 상호작용 대상의 마커 컴포넌트를 선택하는 함수입니다.
     * 이 함수는 지정된 이름(MarkerComponentName)과 일치하는 자식 컴포넌트를 찾아
     * 마커 타겟 컴포넌트(MarkerTargetComponent)로 설정합니다.
     * 만약 해당 이름을 가진 컴포넌트를 찾지 못한 경우, 루트 컴포넌트를 마커 타겟으로 설정합니다.
     */
	void SelectMarkerComponent();

public:
	void AssignInteractor(bool bIsAdd, APlayerController* AssignedController);

	void OnDeactivated();

	UFUNCTION(BlueprintCallable, Category="Interaction Target|Interaction")
	void ApplyFinishMethod(UInteractionManager* InteractingManager, EInteractionResult InteractionResult);

	UFUNCTION(BlueprintCallable, Category="Interaction Target|Interaction")
	void InteractionFinishExecute(UInteractionManager* InteractingManager, EInteractionResult InteractionResult);

	/*========================================================================================
	*	Overlap Events
	=========================================================================================*/
protected:
	/**
     * @brief InnerZone 영역에 다른 액터가 들어올 때 호출되는 함수입니다. 상호작용 가능한 대상이 들어왔을 때, InteractorManager에 상호작용 타겟을 업데이트 요청합니다.
     * 
     * @param OverlappedComponent 충돌이 발생한 컴포넌트
     * @param OtherActor 충돌한 다른 액터
     * @param OtherComp 충돌한 다른 컴포넌트
     * @param OtherBodyIndex 충돌한 몸체 인덱스
     * @param bFromSweep 스위프에 의한 충돌 여부
     * @param SweepResult 스위프 결과 정보
     */
	UFUNCTION(Category="Interaction Target|On Onverlap Events")
	void OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
     * @brief InnerZone 영역에서 다른 액터가 나갈 때 호출되는 함수입니다. 상호작용 가능한 대상이 나갔을 때, InteractorManager에 상호작용 타겟 제거를 요청합니다.
     * 
     * @param OverlappedComponent 충돌이 발생한 컴포넌트
     * @param OtherActor 충돌한 다른 액터
     * @param OtherComp 충돌한 다른 컴포넌트
     * @param OtherBodyIndex 충돌한 몸체 인덱스
     */
	UFUNCTION(Category="Interaction Target|On Onverlap Events")
	void OnInnerZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
     * @brief OuterZone 영역에 다른 액터가 들어올 때 호출되는 함수입니다. 상호작용 가능한 대상이 들어왔을 때, InteractorManager에 관심 지점 및 상호작용자 할당을 요청합니다.
     * 
     * @param OverlappedComponent 충돌이 발생한 컴포넌트
     * @param OtherActor 충돌한 다른 액터
     * @param OtherComp 충돌한 다른 컴포넌트
     * @param OtherBodyIndex 충돌한 몸체 인덱스
     * @param bFromSweep 스위프에 의한 충돌 여부
     * @param SweepResult 스위프 결과 정보
     */
	UFUNCTION(Category="Interaction Target|On Onverlap Events")
	void OnOuterZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
     * @brief OuterZone 영역에서 다른 액터가 나갈 때 호출되는 함수입니다. 상호작용 가능한 대상이 나갔을 때, InteractorManager에 관심 지점 및 상호작용자 해제를 요청합니다.
     * 
     * @param OverlappedComponent 충돌이 발생한 컴포넌트
     * @param OtherActor 충돌한 다른 액터
     * @param OtherComp 충돌한 다른 컴포넌트
     * @param OtherBodyIndex 충돌한 몸체 인덱스
     */
	UFUNCTION(Category="Interaction Target|On Onverlap Events")
	void OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
     * @brief 액터에서 상호작용 관리자 컴포넌트를 찾는 함수입니다. 액터가 유효한 상호작용자(Pawn)인지 확인하고, 해당 상호작용자의 관리자를 반환합니다.
     * 
     * @param OtherActor 확인할 액터
     * @param OutManagerInteractor 반환할 상호작용 관리자
     * @return true 상호작용 관리자 컴포넌트를 성공적으로 찾았을 때
     * @return false 상호작용 관리자 컴포넌트를 찾지 못했을 때
     */
	bool TryGetInteractorComponents(AActor* OtherActor, UInteractionManager*& OutManagerInteractor);


	/*========================================================================================
	*	On Interaction Events
	=========================================================================================*/
public:

	UFUNCTION(BlueprintCallable, Category="Interaction Target|On Interaction Events")
	void OnInteractionBeginEvent(APawn* InInteractorPawn);

	
	UFUNCTION(Category="Interaction Target|On Interaction Events")
	void OnInteractionEndEvent(EInteractionResult InInteractionResult, APawn* InInteractorPawn);


	/*========================================================================================
	*	Check
	=========================================================================================*/
public:
	UFUNCTION(BlueprintPure, Category="Interaction Target|Check")
	bool IsInteractionEnabled();

	UFUNCTION(BlueprintPure, Category="Interaction Target|Check")
	bool IsReactivationEnabled();

	UFUNCTION(BlueprintPure, Category="Interaction Target|Check")
	bool CancelOnRelease();


	/*========================================================================================
	*	Visual
	=========================================================================================*/
public:
	UFUNCTION(BlueprintCallable, Category="Interaction Target|Visual")
	void SetHighlight(bool bIsHighlighted);

	UFUNCTION(BlueprintCallable, Category="Interaction Target|Visual")
	void UpdateWidgetInfo(FMargin InWidgetMargin, float InScreenRadiusPercent);


	/*========================================================================================
	*	Field Members
	=========================================================================================*/
public:
	// 소유자 액터의 레퍼런스
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target|Reference")
	AActor* OwnerReference;

	// 상호작용의 내부 영역을 정의하는 구체 컴포넌트
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target|Components")
	USphereComponent* InnerZone;

	// 상호작용의 외부 영역을 정의하는 구체 컴포넌트
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target|Components")
	USphereComponent* OuterZone;


	/*========================================================================================
	*	Interaction Settings
	=========================================================================================*/
public:
	// 쿨다운 기능 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	bool bCoolDownEnabled = true;

	// 상호작용 구역을 붙일 컴포넌트의 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	FString InteractionZoneComponentToAttach;

	// 내부 구체 영역의 반지름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	float InnerZoneRadius = 200.0f;

	// 외부 구체 영역의 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	float OuterZoneExtent = 200.0f;

	// 상호작용 타입 설정 (Tap, Hold 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	EInteractionType InteractionType = EInteractionType::Tap;

	// 상호작용 반복 시 쿨다운 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings",
		meta=(EditCondition="InteractionType == EInteractionType::Repeat", EditConditionHides="InteractionType == EInteractionType::Repeat"))
	float RepeatCoolDown = 1.0f;

	// 상호작용 반복 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings",
		meta=(EditCondition="InteractionType == EInteractionType::Repeat", EditConditionHides="InteractionType == EInteractionType::Repeat"))
	int32 RepeatCount = 2;

	// 홀드 상호작용의 유지 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings",
		meta=(EditCondition="InteractionType == EInteractionType::Hold", EditConditionHides="InteractionType == EInteractionType::Hold"))
	float HoldSeconds = 2.0f;

	// 네트워크에서 상호작용 처리 방법 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	EInteractionNetworkHandleMethod NetworkHandleMethod = EInteractionNetworkHandleMethod::KeepEnabled;

	// 상호작용 상태에 따른 텍스트를 저장하는 맵
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Interaction Settings")
	TMap<EInteractionState, FText> InteractionText;


	/*========================================================================================
	 *	Highlight Setting
	 =========================================================================================*/
public:
	/*사용자 정의 깊이로 강조 표시할 구성 요소 이름을 추가합니다.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Highlight Settings")
	TArray<FString> ComponentsToHighlight;

	/*강조 표시 색상*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Highlight Settings")
	FLinearColor HighlightColor;

	/*========================================================================================
	 *	Marker Settings
	 =========================================================================================*/
public:
	/*타겟 아이콘 깜박임 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	bool bFlashTargetIcon = false;

	/*관심 지점(POI) 아이콘 깜박임 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	bool bFlashPOIIcon = false;

	/* 삼각형 키 표시 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	bool bDisplayTriangle = true;

	/* 상호작용 아이콘 회전 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	bool bRotateInteractionIcon = false;

	/*상호작용 진행 상태 표시 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	bool bDisplayProgress = true;

	/*마커 컴포넌트 이름*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FString MarkerComponentName;

	/*마커 오프셋*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FVector MarkerOffset;

	/*관심 지점(POI) 아이콘 선택자*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FIconSelector POI_Icon;

	/*관심 지점(POI) 아이콘 색상*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FLinearColor POI_IconColor;

	/*타겟 아이콘 선택자*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FIconSelector Target_Icon;

	/*타겟 아이콘 색상*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FLinearColor Target_IconColor;

	/*상호작용 아이콘 배경 선택자*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FIconSelector Interaction_Icon_Background;

	/*상호작용 아이콘 배경 색상*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FLinearColor Interaction_Icon_Background_Color;

	/*상호작용 아이콘 전경 선택자*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FIconSelector Interaction_Icon_Foreground;

	/*상호작용 아이콘 전경 색상*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Marker Settings")
	FLinearColor Interaction_Icon_Foreground_Color;

	/*========================================================================================
	 *	Key Settings
	 =========================================================================================*/
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Key Settings")
	bool bDisplayInteractionKey = true;

	/*사용자 정의 키 사용 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Key Settings")
	bool bUseCustomKeys = false;

	/*사용자 정의 키 리스트*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Config|Key Settings")
	TArray<FKey> CustomKeys;

	/*========================================================================================
	*	On Finished
	=========================================================================================*/

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Interaction Target|Config|On Finished")
	UInteractionFinish* InteractionFinishInstance;


	/*========================================================================================
	 *	Interaction Target
	 =========================================================================================*/
public:
	/*현재 상호작용중인가?*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	bool bIsInteracting = false;

	/*상호작용 활성화 여부*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	bool bInteractionEnabled = true;

	/*화면 반지름 비율*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	float ScreenRadiusPercent = 0;

	/*위젯 여백 설정*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	FMargin WidgetMargin;

	/*강조 표시된 컴포넌트 리스트*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	TArray<UPrimitiveComponent*> HighlightedComponents;

	/* 마커 타겟 컴포넌트*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	USceneComponent* MarkerTargetComponent;

	/*할당된 상호작용자 리스트*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	TArray<AController*> AssignedControllers;

	/*마지막 상호작용 시간*/
	UPROPERTY(BlueprintReadWrite, Category="Interaction Target")
	float LastInteractedTime = 2.0f;

	/*디버그 모드 활성화 여부*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Target|Debug")
	bool bEnableDebug = true;
	

	/*========================================================================================
	*	Delegate
	=========================================================================================*/

	// 상호작용 시작 델리게이트
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	FOnInteractionBegin OnInteractionBegin;

	// 상호작용 종료 델리게이트
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	FOnInteractionEnd OnInteractionEnd;

	// 상호작용 업데이트 델리게이트
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	FOnInteractionUpdated OnInteractionUpdated;

	// 상호작용 재활성화 델리게이트
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	FOnInteractionReactivated OnInteractionReactivated;

	// 상호작용 비활성화 델리게이트
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Manager Interaction Target|Delegate")
	FOnInteractionDeactivated OnInteractionDeactivated;

	
};
