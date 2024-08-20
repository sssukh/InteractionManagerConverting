#pragma once

#include "CoreMinimal.h"
#include "InteractionEnumStruct.h"
#include "Blueprint/UserWidget.h"
#include "UW_InteractionTarget.generated.h"

class UHorizontalBox;
class UOverlay;
class UTextBlock;
class UBorder;
class USizeBox;
class UGridPanel;
class UWidgetSwitcher;
class UInteractionTarget;
/**
 * 
 */
UCLASS()
class INTERACTION_API UUW_InteractionTarget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_InteractionTarget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void SetInteractionKeyText(const FKey& InKey);

	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void ClearWidgetData();

	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void ToggleLootText(bool bIsVisible);

	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void OnWidgetNewTarget(bool bIsNewTarget);

	/**
     * @brief 상호작용 진행률을 설정하고, 해당 진행률에 따라 위젯의 시각적 상태를 업데이트하는 함수입니다.
     * 
     * 이 함수는 상호작용 진행률을 나타내는 퍼센트를 설정하며, 위젯의 시각적 피드백을 업데이트합니다.
     * 진행률에 따라 동적으로 머티리얼의 파라미터를 변경하고, 진행률이 0일 때에는 Foreground 컴포넌트를 숨깁니다.
     * 
     * @param NewPercent 새롭게 설정할 상호작용 진행률입니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void SetInteractionPercent(float NewPercent);

	/**
     * @brief 상호작용 진행 상태를 초기화하는 함수입니다.
     * 
     * 이 함수는 상호작용 위젯의 프로그레스와 관련된 변수를 초기화하고,
     * 위젯의 진행 상황을 나타내는 머티리얼의 파라미터를 초기값으로 설정합니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void ResetProgress();

	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void PlayInteractionUpdateAnimation();

	/**
     * @brief 상호작용이 완료되었을 때 호출되는 애니메이션을 재생하는 함수입니다.
     * 
     * 이 함수는 상호작용이 완료되거나 취소된 경우에 호출됩니다. 
     * 상호작용이 완료되었을 때는 해당 상호작용에 대한 텍스트를 "Done"으로 업데이트하고,애니메이션을 재생하며, 상호작용 타겟을 초기화합니다.
     * 상호작용이 취소되었을 때는 애니메이션을 재생하고, 상호작용 타입이 Tap 또는 Hold인 경우 프로그레스와 텍스트를 초기화합니다.
     * 
     * @param InteractionResult 상호작용의 결과를 나타내는 열거형 값입니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void PlayInteractionCompletedAnimation(EInteractionResult InteractionResult);

	/**
     * @brief 상호작용 상태에 따라 위젯의 콘텐츠 상태를 업데이트합니다.
     * 이 함수는 상호작용 상태에 따라 위젯의 프로그래스 및 애니메이션을 업데이트하고, 상호작용이 시작되었는지 또는 종료되었는지에 따라 적절한 애니메이션 및 텍스트 업데이트를 수행합니다.
     * @param bIsInteraction 상호작용 상태 여부를 나타내는 bool 값입니다. true이면 상호작용이 시작된 상태를 나타내고, false이면 종료된 상태를 나타냅니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void UpdateContentState(bool bIsInteraction);

	/**
     * @brief 현재 상호작용 텍스트를 위젯에 설정하는 함수입니다.
     * 이 함수는 현재 상호작용 상태를 나타내는 텍스트인 `CurrentInteractionText`를 위젯의 `InfoText`에 설정하고, 텍스트의 존재 여부에 따라 텍스트의 가시성을 조정합니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void SetInteractionText();

	/**
     * @brief 상호작용 텍스트를 업데이트하는 함수입니다.
     * 
     * 이 함수는 상호작용 상태(InteractionState)에 따라 상호작용 텍스트를 업데이트합니다.
     * 주어진 상호작용 상태에 해당하는 텍스트가 존재하고, 현재 텍스트와 다를 경우에만 업데이트가 수행됩니다.
     * 업데이트가 즉시 적용되어야 하는 경우 바로 텍스트를 설정하며, 그렇지 않은 경우 애니메이션을 통해 텍스트를 전환합니다.
     * 
     * @param bIsImmediately 텍스트 업데이트를 즉시 적용할지 여부를 결정합니다.
     * @param InteractionState 업데이트할 상호작용 상태를 나타냅니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void UpdateInteractionText(bool bIsImmediately, EInteractionState InteractionState);

	/**
     * @brief 상호작용 타겟의 화면 설정을 초기화하는 함수입니다.
     * 
     * 이 함수는 주어진 상호작용 타겟(InteractionTarget)의 화면 설정을 적용합니다. 
     * 여기에는 위젯이 화면에 표시될 위치를 결정하는 ScreenRadiusPercent와 
     * 화면 마진(ScreenMargin)을 포함합니다. 이 설정을 통해 위젯이 올바른 위치에 
     * 정확하게 표시되도록 합니다.
     * 
     * @param InteractionTarget 화면 설정을 적용할 상호작용 타겟입니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void SetControllerScreenSettings(UInteractionTarget* InteractionTarget);

	/**
     * @brief 상호작용 대상(WidgetInteractionTarget)의 아이콘, 배경 및 기타 UI 요소에 대한 브러시를 설정합니다.
     * 
     * 이 함수는 상호작용 대상의 다양한 시각적 속성(아이콘, 배경 등)을 UI 위젯의 브러시로 설정합니다.
     * 아이콘은 재질(Material) 또는 텍스처(Texture2D)로 설정할 수 있으며, 설정된 아이콘에 따라 UI 요소의 색상 및 가시성을 조정합니다.
     * 또한 삼각형 키(TriangleKey)의 표시 여부와 진행 표시 여부를 설정합니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void SetBrushFromTarget();

	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void UpdatePositionInViewport();
	
	/**
     * @brief 상호작용 타겟이 화면 내에 있는지 여부를 확인하는 함수입니다.
     * 
     * 이 함수는 상호작용 타겟(InteractionTarget)의 월드 위치를 화면 위치로 변환한 후, 해당 위치가 화면 안에 있는지 확인합니다.
     * 화면 가장자리의 마진 값을 고려하여, 화면 안에 위치하는지 여부를 반환합니다.
     * 
     * @param InMargin 화면 경계에 사용할 마진 값입니다.
     * @return 화면 내에 위치하면 true, 화면 밖에 위치하면 false를 반환합니다.
     */
	UFUNCTION(BlueprintPure, Category="UW Interaction Target Event")
	bool IsOnScreen(FMargin InMargin);

	/**
     * @brief 상호작용 타겟을 업데이트하고 위젯의 상태를 설정하는 함수입니다.
     *
     * 이 함수는 새로 설정된 상호작용 타겟을 위젯에 적용하고, 관련된 UI 요소들을 업데이트합니다. 
     * 상호작용 타겟이 유효할 경우, 위젯의 데이터를 초기화하고 타겟의 화면 설정과 브러쉬(아이콘)를 설정한 후, 
     * 다양한 애니메이션과 상태를 적용하여 UI를 업데이트합니다. 
     * 만약 상호작용 타겟이 유효하지 않으면, 현재 타겟을 제거하고 UI를 초기화합니다.
     *
     * @param InteractionTarget 새롭게 설정할 상호작용 타겟입니다. nullptr일 경우, 기존 타겟이 제거됩니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void UpdateInteractionTarget(UInteractionTarget* InteractionTarget);

	/**
     * @brief 상호작용 위젯에서 플래시 애니메이션을 재생하는 함수입니다.
     * 이 함수는 조건에 따라 플래시 애니메이션을 정방향 또는 역방향으로 재생합니다.
     * 특정 상호작용 상태에 따라 위젯의 시각적 피드백을 제공하기 위해 사용됩니다.
     * @param bIsCondition 조건에 따라 애니메이션을 정방향(PingPong)으로 재생할지, 역방향(Reverse)으로 재생할지를 결정합니다.
     */
	UFUNCTION(BlueprintCallable, Category="UW Interaction Target Event")
	void PlayFlashAnimation(bool bIsCondition);

protected:
	UFUNCTION()
	void OnSlideUpAnimStarted();

	UFUNCTION()
	void OnSlideUpAnimFinished();


	/*========================================================================================
	 *	Designer
	 =========================================================================================*/
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UWidgetSwitcher* ContentSwitcher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UHorizontalBox* InteractionInfoBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UGridPanel* InteractionContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UTextBlock* LootKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UTextBlock* InfoText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	USizeBox* POI_Container;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UBorder* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UBorder* POI_Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UBorder* Foreground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UBorder* Triangle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget))
	UBorder* Background;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidget), Transient)
	UOverlay* TriangleBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* SlideUp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* LootText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* InteractionRepeatUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* InteractionCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* Flash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* InteractionTextAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* InteractionCanceled;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UW Interaction Target|Designer", meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* Rotate;


	/*========================================================================================
	*	UW Interaction
	=========================================================================================*/
public:
	// T_Circle 텍스처, UI에서 원형 이미지를 사용하기 위한 텍스처입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UW Interaction Target|Config")
	UTexture2D* T_Circle;

	// 진행률을 화면에 표시할지 여부를 결정하는 플래그입니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	bool bDisplayProgress;

	// 진행률 애니메이션이 시계 방향으로 진행될지 여부를 설정하는 플래그입니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	bool bCustomClockwise = true;

	// 상호작용 타겟이 제거될 때, 위젯을 초기화할지 여부를 나타내는 플래그입니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	bool bClearTarget;

	// 현재 상호작용 상태인지 여부를 나타내는 플래그입니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	bool bIsInteractionState;

	// 사용자 정의 진행률을 나타내는 변수입니다. 진행률 애니메이션에 사용됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	float CustomPercent;

	// 현재 진행률을 나타내는 변수입니다. 0에서 1 사이의 값으로 표현됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	float CurrentPercent;

	// 화면에서의 반지름 비율을 나타내는 변수입니다. UI 요소의 크기와 위치를 조절하는 데 사용됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	float ScreenRadiusPercent;

	// 현재 상호작용 상태를 나타내는 텍스트입니다. UI에 표시됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	FText CurrentInteractionText;

	// 위젯의 위치 오차를 보정하기 위한 벡터입니다. 화면상의 위치를 조절하는 데 사용됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	FVector2D WidgetPositionErrorFactor;

	// 마지막으로 상호작용 타겟의 월드 위치를 나타내는 벡터입니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	FVector LastWorldLocation;

	// 화면의 여백을 나타내는 변수입니다. UI 요소가 화면에서 벗어나지 않도록 위치를 제한하는 데 사용됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	FMargin ScreenMargin;

	// 현재 상호작용 중인 타겟의 참조입니다. UI와 상호작용 로직에서 사용됩니다.
	UPROPERTY(BlueprintReadWrite, Category="UW Interaction")
	UInteractionTarget* WidgetInteractionTarget = nullptr;
	
};
