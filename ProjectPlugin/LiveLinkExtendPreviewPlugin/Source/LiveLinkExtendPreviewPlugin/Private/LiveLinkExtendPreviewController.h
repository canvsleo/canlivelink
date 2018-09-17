// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once




#include "PersonaPreviewSceneController.h"
#include "SubclassOf.h"


#include "AnimInstanceProxy.h"
#include "Animation/AnimInstance.h"
#include "AnimNode_LiveLinkPose.h"

#include "LiveLinkInstance.h"
#include "LiveLinkClientReference.h"


#include "ILiveLinkExtendVertexMeshData.h"

#include "LiveLinkExtendPreviewController.generated.h"

class ULiveLinkRetargetAsset;
class UDebugSkelMeshComponent;


UCLASS()
class ULiveLinkExtendPreviewController
	: public UPersonaPreviewSceneController
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Live Link Extend")
	FName SubjectName;

	UPROPERTY(EditAnywhere, Category = "Live Link Extend")
	bool bEnableCameraSync;

	UPROPERTY(EditAnywhere, NoClear, Category = "Live Link Extend")
	TSubclassOf<ULiveLinkRetargetAsset> RetargetAsset;

	virtual void InitializeView(UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene) const;
	virtual void UninitializeView(UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene) const;

};

USTRUCT()
struct FLiveLinkExtendInstanceProxy :
	public FLiveLinkInstanceProxy
{
public:
	GENERATED_BODY()

	FLiveLinkExtendInstanceProxy()
		: PreviewMeshComponent( nullptr )
	{
		this->currentVertexiesID_		= 0;
	}

	FLiveLinkExtendInstanceProxy( UAnimInstance* InAnimInstance )
		: FLiveLinkInstanceProxy( InAnimInstance )
		, PreviewMeshComponent( nullptr )
	{
		this->currentVertexiesID_		= 0;
	}

	~FLiveLinkExtendInstanceProxy()
	{}

	virtual void Initialize( UAnimInstance* InAnimInstance ) override;
	virtual bool Evaluate( FPoseContext& Output ) override;
	virtual void UpdateAnimationNode( float DeltaSeconds ) override;

private:
	uint64		currentVertexiesID_;


	FLiveLinkClientReference ClientRef;

	TMap< FString, FDateTime >	lastMeshUpdateTimeMap_;
	FDateTime					lastReceiveTime_;

public:
	UDebugSkelMeshComponent*	PreviewMeshComponent;

};

UCLASS( transient, NotBlueprintable )
class ULiveLinkExtendInstance
	: public ULiveLinkInstance
{
	GENERATED_UCLASS_BODY()

public:
	void SetSubject( FName SubjectName )
	{
		GetProxyOnGameThread<FLiveLinkExtendInstanceProxy>().PoseNode.SubjectName = SubjectName;
	}

	void SetRetargetAsset( TSubclassOf<ULiveLinkRetargetAsset> RetargetAsset )
	{
		GetProxyOnGameThread<FLiveLinkExtendInstanceProxy>().PoseNode.RetargetAsset = RetargetAsset;
	}

	void SetPreviewMeshComponent( UDebugSkelMeshComponent* previewMeshComponent )
	{
		GetProxyOnGameThread<FLiveLinkExtendInstanceProxy>().PreviewMeshComponent = previewMeshComponent;
	}

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};


