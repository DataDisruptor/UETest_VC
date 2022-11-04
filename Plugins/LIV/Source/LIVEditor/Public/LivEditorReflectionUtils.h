// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

namespace LivEditorReflectionUtils
{
	template<typename ContainerClassBaseType, typename PropertyClassType>
	TArray<FObjectProperty*> GetObjectPropertiesOfTypeInClass(const TSubclassOf<ContainerClassBaseType> ContainerClass, EFieldIteratorFlags::SuperClassFlags SuperClassFlags)
	{
		const UClass* PropertyClass = PropertyClassType::StaticClass();
		
		TArray<FObjectProperty*> ObjectProperties;
		
		for (TFieldIterator<FProperty> PropIt(ContainerClass, SuperClassFlags); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
			if (ObjectProperty)
			{
				const UClass* ObjectPropertyClass = ObjectProperty->PropertyClass;

				if (ObjectPropertyClass->IsChildOf(PropertyClass))
				{
					ObjectProperties.Add(ObjectProperty);
				}
			}
		}
		
		return ObjectProperties;
	}

	template<typename ObjectType>
	ObjectType* GetObjectPropertyValue(const FObjectProperty& ObjectProperty, UObject const* Container)
	{
		const void* ObjectContainer = ObjectProperty.ContainerPtrToValuePtr<void>(Container);
		return Cast<ObjectType>(ObjectProperty.GetObjectPropertyValue(ObjectContainer));
	}
};
