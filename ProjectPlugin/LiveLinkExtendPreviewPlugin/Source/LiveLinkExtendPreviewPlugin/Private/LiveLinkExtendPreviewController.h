
#pragma once


/*! @file		LiveLinkExtendPreviewController.hs
	@brief		LiveLinkExtendを使用した際のメッシュプレビュー中の挙動 
	@details	LiveLinkPreviewController.h をもとにした実装となっており、\n
				カメラの画角とメッシュ更新が拡張されております。
*/



#include "PersonaPreviewSceneController.h"
#include "SubclassOf.h"


#include "AnimInstanceProxy.h"
#include "Animation/AnimInstance.h"
#include "AnimNode_LiveLinkPose.h"

#include "LiveLinkInstance.h"
#include "LiveLinkClientReference.h"


#include "LiveLinkExtendVertexMeshData.h"

#include "LiveLinkExtendPreviewController.generated.h"

class ULiveLinkRetargetAsset;
class UDebugSkelMeshComponent;




/*! @struct		FLiveLinkExtendInstanceProxy
	@brief		Subjectごとに受け取ったポーズとメタデータの適応方法
*/
USTRUCT()
struct FLiveLinkExtendInstanceProxy :
	public FLiveLinkInstanceProxy
{
public:
	GENERATED_BODY()

	FLiveLinkExtendInstanceProxy();
	FLiveLinkExtendInstanceProxy( UAnimInstance* InAnimInstance );
	~FLiveLinkExtendInstanceProxy();

	virtual void Initialize( UAnimInstance* InAnimInstance ) override;
	virtual bool Evaluate( FPoseContext& Output ) override;
	virtual void UpdateAnimationNode( float DeltaSeconds ) override;

private:

	FLiveLinkClientReference	clientRef_;

	//! 最後にメッシュ処理したメタデータのタイムスタンプにて同一データの多重処理を防ぐ
	FDateTime					lastReceiveTime_;
	TMap< FString, FDateTime >	lastMeshUpdateTimeMap_;

public:

	/*! 
		プレビューに表示されているスケルタルメッシュ\n
		メッシュ更新に使用される。
	*/
	UDebugSkelMeshComponent*	PreviewMeshComponent;

};




/*! @class		ULiveLinkExtendInstance
	@brief		FLiveLinkExtendInstanceProxy と ULiveLinkExtendPreviewController の\n
				動作中継を行う。
*/
UCLASS( transient, NotBlueprintable )
class ULiveLinkExtendInstance
	: public ULiveLinkInstance
{
public:
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







/*! @class		ULiveLinkExtendInstance
	@brief		LiveLinkExtendを使用した際のメッシュプレビュー操作
*/
UCLASS()
class ULiveLinkExtendPreviewController
	: public UPersonaPreviewSceneController
{
public:
	GENERATED_BODY()

		UPROPERTY( EditAnywhere, Category = "Live Link Extend" )
		FName SubjectName;

	UPROPERTY( EditAnywhere, Category = "Live Link Extend" )
		bool bEnableCameraSync;

	UPROPERTY( EditAnywhere, NoClear, Category = "Live Link Extend" )
		TSubclassOf<ULiveLinkRetargetAsset> RetargetAsset;

	virtual void InitializeView( UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene ) const;
	virtual void UninitializeView( UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene ) const;

};
