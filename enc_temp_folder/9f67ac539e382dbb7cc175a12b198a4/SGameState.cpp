// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}
