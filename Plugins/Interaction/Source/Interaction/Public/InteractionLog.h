
#pragma once

#include "CoreMinimal.h"



DECLARE_LOG_CATEGORY_EXTERN(LogInteractionSystem, Log, All);


// // 매크로 정의
// #define LOG_WARNING_AND_SCREEN(Message) \
// { \
// FString LogMessage = FString::Printf(TEXT("%s : %s"), *FString(__FUNCTION__), *FString(Message)); \
// UE_LOG(LogInteractionSystem, Warning, TEXT("%s"), *LogMessage); \
// if (GEngine) \
// { \
// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, LogMessage); \
// } \
// }


#define LOG_WARNING_AND_SCREEN(DisplayDuration,Format, ...) \
{ \
FString LogMessage = FString::Printf(TEXT("%s : ") Format, *FString(__FUNCTION__), ##__VA_ARGS__); \
UE_LOG(LogInteractionSystem, Warning, TEXT("%s"), *LogMessage); \
if (GEngine) \
{ \
GEngine->AddOnScreenDebugMessage(-1, DisplayDuration, FColor::Red, LogMessage); \
} \
}