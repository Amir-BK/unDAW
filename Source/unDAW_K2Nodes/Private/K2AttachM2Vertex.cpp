// Fill out your copyright notice in the Description page of Project Settings.


#include "K2AttachM2Vertex.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "EdGraph/EdGraphNode.h"

void UK2AttachM2Vertex::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        check(NodeSpawner);

        ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
    }
}

FText UK2AttachM2Vertex::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return INVTEXT("Attach M2 Vertex");
}

FText UK2AttachM2Vertex::GetTooltipText() const
{
    return INVTEXT("Allows attaching a metasound patch implementing the unDAW interface to a mixer input.");
}
