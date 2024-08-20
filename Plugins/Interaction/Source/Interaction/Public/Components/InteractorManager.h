#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.h"
#include "Components/ActorComponent.h"
#include "InteractorManager.generated.h"


class UInteractionTarget;
class UUW_InteractionTarget;
class UPostProcessComponent;
class UInputAction;

UCLASS(Blueprintable, BlueprintType, ClassGroup=("Interaction Plugin"), meta=(BlueprintSpawnableComponent))
class INTERACTION_API UInteractionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionManager();

protected:
	virtual void BeginPlay() override;

	/**
     * @brief 포스트 프로세스 컴포넌트를 생성하고 설정합니다.
     * 
     * 이 함수는 플레이어의 폰에 포스트 프로세스 컴포넌트를 동적으로 추가하고, 해당 컴포넌트에 사용할 동적 머티리얼 인스턴스를 생성한 후, 이를 포스트 프로세스 설정에 반영합니다.
     */
	void ConstructPostProcessComponent();

	/**
     * @brief 플레이어와 관련된 필수 요소를 초기화합니다.
     * 
     * 이 함수는 플레이어 컨트롤러를 캐시하고, 포스트 프로세스 컴포넌트를 생성하며,상호 작용 키를 업데이트하기 위한 타이머를 설정합니다.
     */
	void ConstructPlayerEssentials();

	/**
     * @brief 상호작용 타겟 위젯을 풀링하여 초기화합니다.
     * 
     * 이 함수는 플레이어 컨트롤러가 로컬 컨트롤러일 때만 호출됩니다. 
     * 상호작용 타겟 위젯 클래스를 기반으로 위젯 풀을 생성하고, 이를 화면에 추가합니다.
     */
	void ConstructPooledMarkerWidgets();

	/*상호 작용 키를 찾고 설정하는 곳입니다.
	 *이 시스템은 기본적으로 향상된 입력을 사용하지만 필요에 따라 이 기능에서 키를 설정할 수 있습니다.*/
	UFUNCTION()
	void UpdateInteractionKeys();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawDebugInteractor();


	/*========================================================================================
	*	Interaction
	=========================================================================================*/

	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Interaction")
	void ApplyFinishMethod(UInteractionTarget* InteractionTarget, EInteractionResult InteractionResult);

	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Interaction")
	void ReceiveAnyKey(FKey InKey);


	/*========================================================================================
	*	Interactable
	=========================================================================================*/
public:
	/**
     * @brief 상호작용 대상 업데이트 함수
     * 현재 상호작용 대상(BestInteractionTarget)과 새로운 상호작용 대상(NewTarget)을 비교하여 변경 사항이 있으면 해당 상호작용 대상을 업데이트합니다.
     * @param NewTarget 새로운 상호작용 대상 포인터. 유효하지 않으면 기존 상호작용 대상이 제거됩니다.
     */
	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Interactable")
	void UpdateBestInteractable(UInteractionTarget* NewTarget);

	/**
     * @brief 상호작용 대상이 상호작용 가능한지 여부를 판단하는 함수입니다.
     *
     * 이 함수는 상호작용 대상이 현재 상호작용 가능한 상태인지 확인합니다.
     * 상호작용이 가능하려면 상호작용 대상이 활성화되어 있어야 하며, 대기 목록(PendingTargets)이나 비활성화된 목록(DeactivatedTargets)에 포함되지 않아야 합니다.
     *
     * @param ItemToFind 확인할 상호작용 대상입니다.
     * @return 상호작용 가능하면 true, 그렇지 않으면 false를 반환합니다.
     */
	UFUNCTION(BlueprintPure, Category="Interactor Manager|Interactable")
	bool IsInteractable(UInteractionTarget* ItemToFind);

	/**
     * @brief 상호작용 키를 가져오는 함수
     *
     * 이 함수는 현재 설정된 상호작용 대상(BestInteractionTarget)이 유효한지 확인하고, 유효한 경우 대상에 설정된 사용자 정의 키를 반환합니다. 그렇지 않으면 기본 상호작용 키를 반환합니다.
     * 반환되는 키가 유효한지 여부도 함께 반환됩니다.
     *
     * @param OutInteractionKeys 반환할 상호작용 키 배열입니다.
     * @return bool 반환된 상호작용 키 배열에 유효한 키가 포함되어 있는지 여부를 반환합니다.
     */
	UFUNCTION(BlueprintPure, Category="Interactor Manager|Interactable")
	bool GetInteractionKeys(TArray<FKey>& OutInteractionKeys);

	/**
     * @brief 주어진 상호작용 대상과 연관된 위젯을 찾는 함수
     *
     * 이 함수는 주어진 상호작용 대상(InteractionTarget)과 연결된 상호작용 위젯을 위젯 풀(WidgetPool)에서 검색하여 반환합니다.
     * 만약 해당 대상과 연관된 위젯이 없다면, nullptr을 반환합니다.
     *
     * @param InteractionTarget 검색할 상호작용 대상입니다.
     * @return UUW_InteractionTarget* 상호작용 대상과 연관된 위젯입니다. 없을 경우 nullptr을 반환합니다.
     */
	UFUNCTION(BlueprintPure, Category="Interactor Manager|Interactable")
	UUW_InteractionTarget* FindWidgetByInteractionTarget(UInteractionTarget* InteractionTarget);

	/**
     * @brief 빈 상호작용 위젯을 찾는 함수
     *
     * 이 함수는 위젯 풀(WidgetPool)을 순회하여, 상호작용 대상이 설정되지 않은 빈 위젯을 찾아 반환합니다.
     * 만약 모든 위젯이 이미 사용 중이라면, nullptr을 반환합니다.
     *
     * @return UUW_InteractionTarget* 빈 상호작용 위젯입니다. 없을 경우 nullptr을 반환합니다.
     */
	UFUNCTION(BlueprintPure, Category="Interactor Manager|Interactable")
	UUW_InteractionTarget* FindEmptyWidget();

	/**
     * @brief 상호작용 가능한 타겟 중에서 가장 적합한 타겟을 찾습니다.
     * 이 함수는 DOT Product(내적) 값을 기반으로 플레이어의 카메라 방향과 상호작용 타겟 사이의 각도를 계산하여 가장 적합한 타겟을 선택합니다.
     * @return UManagerInteractionTarget* 가장 적합한 상호작용 타겟을 반환합니다. 적합한 타겟이 없으면 nullptr을 반환합니다.
     */
	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Interactable")
	UInteractionTarget* FindBestInteractable();


	/*========================================================================================
	 *	Target
	 =========================================================================================*/
public:
	/**
	 * @brief 상호작용 대상의 강조 표시를 설정하는 함수
	 *
	 * 이 함수는 주어진 상호작용 대상(InteractionTarget)을 강조 표시할지 여부를 설정합니다.
	 * 강조 표시가 활성화되면, 대상의 하이라이트 색상을 설정하고, 대상이 상호작용 가능하다는 것을 시각적으로 나타냅니다.
	 *
	 * @param InteractionTarget 강조 표시를 설정할 상호작용 대상입니다.
	 * @param bIsHighlighted 강조 표시를 활성화(true)하거나 비활성화(false)합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Target")
	void SetTargetHighlighted(UInteractionTarget* InteractionTarget, bool bIsHighlighted);

	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Target")
	void AddToDeactivatedTargets(UInteractionTarget* InteractionTarget);

	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Target")
	void RemoveFromDeactivatedTargets(UInteractionTarget* InteractionTarget);
	
	void OnNewTargetSelectedClientSide(UInteractionTarget* NewTarget, bool bIsSelected);

protected:
	UFUNCTION(BlueprintCallable, Category="Interactor Manager|Target")
	void OnInteractionTargetDestroyed(AActor* DestroyedActor);


	/*========================================================================================
	*	Network Event
	=========================================================================================*/
public:
	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerOnInteractionUpdated(UInteractionTarget* InteractionTarget, float InAlpha, int32 InRepeated, APawn* InteractorPawn);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerOnInteractionFinished(UInteractionTarget* InteractionTarget, EInteractionResult InteractionResult);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerOnInteractionBegin(UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerRequestAssignInteractor(bool bIsAdd, UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerUpdateInteractionTargets(bool bIsAdd, UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Server, Unreliable, Category="Manager Interactor|Network Event")
	void ServerUpdatePointOfInterests(bool bIsAdd, UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Manager Interactor|Network Event")
	void ClientUpdateInteractionTargets(bool bIsAdd, UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Manager Interactor|Network Event")
	void ClientCheckPressedKey();


	UFUNCTION(Blueprintable, Client, Unreliable, Category="Manager Interactor|Network Event")
	void ClientUpdatePointOfInterests(bool bIsAdd, UInteractionTarget* InteractionTarget);

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Manager Interactor|Network Event")
	void ClientSetNewTarget(UInteractionTarget* NewTarget, bool bIsSelected);

	UFUNCTION(Blueprintable, Client, Reliable, Category="Manager Interactor|Network Event")
	void ClientOnInteractionTargetDestroyed(UInteractionTarget* ActorInteractionTarget);

	UFUNCTION(Blueprintable, Client, Unreliable, Category="Manager Interactor|Network Event")
	void ClientResetData();

public:
	/**
     * @brief 서버에서 상호작용 타겟 리스트를 업데이트하는 내부 로직을 처리하는 함수입니다.
     *        상호작용 타겟을 서버에서 추가 또는 제거하고, 클라이언트에 그 변경 사항을 전달합니다.
     * 
     * @param bIsAdd 상호작용 타겟을 추가할지 여부 (true: 추가, false: 제거)
     * @param InteractionTarget 상호작용 타겟으로 지정된 UInteractionTarget 객체
     */
	void OnInteractionTargetUpdatedServerSide(bool bIsAdd, UInteractionTarget* InteractionTarget);

	/**
     * @brief 클라이언트에서 상호작용 대상의 상태를 업데이트하는 함수입니다.
     *        이 함수는 상호작용 대상에 연결된 위젯을 찾아, 해당 위젯의 상태를 업데이트합니다.
     * 
     * @param bIsAdd 상호작용 대상이 추가(true)인지 제거(false)인지를 나타내는 bool 값.
     * @param InteractionTarget 업데이트할 상호작용 대상의 포인터.
     */
	void OnInteractionTargetUpdatedClientSide(bool bIsAdd, UInteractionTarget* InteractionTarget);

	/**
     * @brief 서버에서 관심 지점(Point of Interest)을 업데이트하는 함수입니다.
     * 
     * 이 함수는 특정 상호작용 대상(InteractionTarget)을 관심 지점 목록에 추가하거나 제거합니다.
     * 추가 시에는 해당 상호작용 대상이 상호작용 가능한지(IsInteractable) 여부를 확인하여, 가능하면 클라이언트에 업데이트를 전송하고,상호작용 대상의 소멸 이벤트(OnDestroyed)에 대한 바인딩을 설정합니다.
     * 상호작용 불가능한 경우에는 재활성화가 가능한지(IsReactivationEnabled) 확인하고, 가능하면 대기 목록에 추가합니다.
     * 제거 시에는 유효한 상호작용 대상만을 관심 지점 목록에서 제거하고, 클라이언트에 업데이트를 전송합니다.
     * 
     * @param bIsAdd 관심 지점을 추가(true)할지 제거(false)할지 여부를 나타내는 플래그입니다.
     * @param InteractionTarget 업데이트할 대상 상호작용 컴포넌트입니다.
     */
	void OnPointOfInterestUpdatedServerSide(bool bIsAdd, UInteractionTarget* InteractionTarget);

	/**
     * @brief 클라이언트에서 관심 지점(Point of Interest, POI) 업데이트를 처리하는 함수입니다.
     *
     * POI가 추가될 경우, 해당 상호작용 대상을 위한 위젯을 화면에 표시합니다.
     * 기존 위젯을 재사용하거나, 필요하면 새 위젯을 생성합니다.
     * POI가 제거될 경우, 해당 위젯의 상호작용 대상 정보를 초기화하여 위젯을 비웁니다.
     *
     * @param bIsAdd POI를 추가할지 여부를 나타냅니다. true면 추가, false면 제거를 의미합니다.
     * @param InteractionTarget 업데이트할 상호작용 대상입니다.
     */
	void OnPointOfInterestUpdatedClientSide(bool bIsAdd, UInteractionTarget* InteractionTarget);

	/**
     * @brief 상호작용이 진행 중일 때 호출되어, 상호작용 상태를 업데이트하고 서버와 동기화합니다.
     * 
     * 이 함수는 상호작용 타겟에 대한 업데이트 이벤트를 브로드캐스트하고,멀티플레이어 환경에서는 서버와 상호작용 상태를 동기화하기 위해 호출됩니다.
     * 
     * @param InteractionTarget 현재 상호작용 중인 타겟.
     * @param InAlpha 상호작용 진행 상태를 나타내는 비율 값 (0.0f ~ 1.0f).
     * @param InRepeated 상호작용이 반복된 횟수.
     */
	void OnInteractionUpdated(UInteractionTarget* InteractionTarget, float InAlpha, int32 InRepeated);


	/*========================================================================================
	*	Key Event
	=========================================================================================*/
public:
	bool IsHoldingKey();

	/*========================================================================================
	*	Field Members
	=========================================================================================*/

public:
	// 소유자 컨트롤러를 참조합니다. 이 매니저 인터랙터가 속한 플레이어의 컨트롤러입니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor|Reference")
	APlayerController* OwnerController;

	// 포스트 프로세스 컴포넌트를 참조합니다. 후처리 효과를 위한 컴포넌트입니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor|Reference")
	UPostProcessComponent* PostProcessComponent;


	/*========================================================================================
	 *	Config
	 =========================================================================================*/
public:
	// 보류 중인 타겟을 확인하는 간격입니다. 0.1 ~ 1.0 사이의 값으로 조정할 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config", meta=(ClampMin="0.1", ClampMax="1.0", UIMin="0.1", UIMax="1.0"))
	float PendingTargetCheckInterval = 0.35f;

	// 기본적으로 생성되는 위젯 풀의 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config")
	int32 DefaultWidgetPoolSize = 3;

	// 상호 작용에 사용되는 입력 액션을 참조합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config")
	UInputAction* InteractionInputAction;

	// 현재 사용 중인 머티리얼을 참조합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config", meta=(AllowPrivateAccess=true))
	UMaterialInterface* CurrentMaterial;

	// 상호 작용 타겟 위젯 클래스의 참조입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config", meta=(AllowPrivateAccess=true))
	TSubclassOf<UUW_InteractionTarget> InteractionTargetWidgetClass;

	// 디버그 모드를 활성화하거나 비활성화합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config|Debug")
	bool bDebug = false;


	/*========================================================================================
	*	Marker Settings
	=========================================================================================*/
public:
	// 위젯의 화면 마진 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config|Marker Settings", meta=(AllowPrivateAccess=true))
	FMargin WidgetScreenMargin;

	// 화면 반지름의 비율을 설정합니다. 0 ~ 1.0 사이의 값으로 조정할 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Manager Interactor|Config|Marker Settings", meta=(ClampMin="0", ClampMax="1.0", UIMin="0", UIMax="1.0"))
	float ScreenRadiusPercent = 0.5f;


	/*========================================================================================
	*	Manger Interactor Data
	=========================================================================================*/
public:
	// 현재 사용 중인 입력 장치가 게임패드인지 여부를 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	bool bIsGamepad;

	// 상호 작용이 진행 중인지 여부를 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	bool bIsInteracting;

	// 키가 방금 눌렸는지 여부를 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	bool bKeyJustPressed;

	// 반복 상호 작용의 쿨다운 시간을 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	float RepeatCooldown;

	// 마지막으로 눌린 키를 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	FKey LastPressedKey;

	// 현재 키를 누르고 있는 시간을 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	float CurrentHoldTime;

	// 상호 작용이 반복된 횟수를 나타냅니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	int32 Repeated;

	// 보류 중인 타겟을 확인하기 위한 타이머 핸들입니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor")
	FTimerHandle PendingTarget_TimerHandle;

	// 현재 상호 작용 중인 마커를 참조합니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor")
	UUW_InteractionTarget* CurrentInteractionMarker;

	// 상호 작용 키 배열을 참조합니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor")
	TArray<FKey> InteractionKeys;

	// 윤곽선 머티리얼 인스턴스의 동적 머티리얼을 참조합니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor")
	UMaterialInstanceDynamic* Outline_DynamicMaterial;

	// 현재 가장 좋은 상호 작용 타겟을 참조합니다.
	UPROPERTY(BlueprintReadWrite, Category="Manager Interactor")
	UInteractionTarget* BestInteractionTarget;

	// 위젯 풀을 참조합니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	TArray<UUW_InteractionTarget*> WidgetPool;

	// 상호 작용 타겟 배열을 참조합니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	TArray<UInteractionTarget*> InteractionTargets;

	// 관심 지점 배열을 참조합니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	TArray<UInteractionTarget*> PointOfInterests;

	// 보류 중인 타겟 배열을 참조합니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	TArray<UInteractionTarget*> PendingTargets;

	// 비활성화된 타겟 배열을 참조합니다.
	UPROPERTY(BlueprintReadOnly, Category="Manager Interactor")
	TArray<UInteractionTarget*> DeactivatedTargets;

protected:
	// 상호 작용 키 업데이트를 위한 타이머 핸들입니다.
	FTimerHandle BeginUpdateKeys_TimerHandle;
};
