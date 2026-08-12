#pragma once

UENUM(BlueprintType)
enum class ESpiderSurfaceState : uint8
{
    Grounded    UMETA(DisplayName = "Grounded"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Ceiling     UMETA(DisplayName = "Ceiling")
};