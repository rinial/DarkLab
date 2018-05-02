// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicInformativeObject.h"

// Returns object's name
FText ABasicInformativeObject::GetName_Implementation()
{
	return Name;
}
// Returns basic infomation about the object
FText ABasicInformativeObject::GetBasicInfo_Implementation()
{
	return BasicInfo;
}

// Sets default values
ABasicInformativeObject::ABasicInformativeObject()
{

}