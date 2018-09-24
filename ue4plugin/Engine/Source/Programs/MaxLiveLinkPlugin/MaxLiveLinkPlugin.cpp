


#include "RequiredProgramMainCPPInclude.h"

#include "Modules/ModuleManager.h"
//#include "UObject/Object.h"
#include "Misc/ConfigCacheIni.h"

#include "LiveLinkExtendVertexMeshData.h"

#include "Misc/CommandLine.h"
#include "Async/TaskGraphInterfaces.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"

#include "LiveLinkProvider.h"
#include "LiveLinkRefSkeleton.h"
#include "LiveLinkTypes.h"
#include "Misc/OutputDevice.h"


#include "resource.h"

#include "notify.h"

#include "Windows/WindowsSystemIncludes.h"



#include "Base64.h"


DEFINE_LOG_CATEGORY_STATIC( LogBlankMaxPlugin, Log, All);

IMPLEMENT_APPLICATION( MaxLiveLinkPlugin, "MaxLiveLinkPlugin");



bool GIsConsoleExecutable;

// Max includes

#define TRUE 1
#define FALSE 0

#define Rect Box

#undef message

#include "maxscript/maxscript.h"
#include "maxheap.h"
#include "export.h"
#include "meshadj.h"
#include "maxscript/maxwrapper/mxsobjects.h"
#include "ISceneEventManager.h"
#include "maxscript/macros/define_instantiation_functions.h"

#include "max.h"
#include "utilapi.h"
#include "decomp.h"
#include "modstack.h"
#include "iskin.h"
#include "Tab.h"
#include "Animatable.h"
#include "MeshNormalSpec.h"
#include "IGame/IGameModifier.h"

#include "MorphR3.h"

using NodeArray = TArray<INode*>;

#undef DWORD


class FMaxLiveLink;

class NullView 
	: public View
{
public:
	Point2 ViewToScreen( Point3 p )
	{
		return Point2( p.x, p.y );
	}
	NullView() {
		worldToView.IdentityMatrix();
		screenW=640.0f; screenH = 480.0f;
	}
};


applyable_class_debug_ok ( NodeEventCallbackValue )
class NodeEventCallbackValue 
	: public INodeEventCallback, ViewportDisplayCallback
{
public:
	NodeEventCallbackValue(
		FMaxLiveLink* maxLink,
		Interface* ip
	);
	virtual ~NodeEventCallbackValue();

private:
	FMaxLiveLink*		maxLink_;
	Interface*			interface_;

	NodeEventNamespace::CallbackKey nodeEventKey_;

public:
	virtual void ModelStructured( NodeKeyTab& nodes );
	virtual void GeometryChanged( NodeKeyTab& nodes );
	virtual void MappingChanged( NodeKeyTab& nodes );
	virtual void ExtentionChannelChanged( NodeKeyTab& nodes );
	virtual void ModelOtherEvent( NodeKeyTab& nodes );

	virtual void NameChanged( NodeKeyTab& nodes );
	virtual void Added( NodeKeyTab& nodes );
	virtual void Deleted( NodeKeyTab& nodes );

	virtual void Display( TimeValue t, ViewExp *vpt, int flags );
	virtual void GetViewportRect( TimeValue t, ViewExp* vpt, Rect* rect );
	virtual BOOL Foreground();
};




class FMaxLiveLinkJointStreamedSubject;
class FMaxLiveLinkCameraStreamedSubject;

class FMaxLiveLink
	: public UtilityObj
{
	friend class CCUtilClassDesc;

public:
	FMaxLiveLink(
		HINSTANCE instanceHandle
	);
	~FMaxLiveLink();

	void BeginEditParams( Interface *ip, IUtil *iu );
	void EndEditParams( Interface *ip, IUtil *iu );
	void DeleteThis() {}

	void Init( HWND hWnd );
	void Destroy( HWND hWnd );




public:
	typedef enum _CollectOptionBits_
	{
		CollectOptionBits_None				= 0,
		CollectOptionBits_OnlySelected		= 0x1 << 0,

		CollectOptionBits_Mesh				= 0x1 << 8,
		CollectOptionBits_Bone				= 0x1 << 9,
		CollectOptionBits_Camera			= 0x1 << 10,
		CollectOptionBits_Helper			= 0x1 << 11,
		CollectOptionBits_Dummy				= 0x1 << 12,

		CollectOptionBits_SkinedMesh		= 0x1 << 20,
		CollectOptionBits_MorphMesh			= 0x1 << 21,

		CollectOptionBits_AllTypes =
		CollectOptionBits_Mesh
		| CollectOptionBits_Bone
		| CollectOptionBits_Camera
		| CollectOptionBits_Helper
		| CollectOptionBits_Dummy
		,

	}CollectOptionBits;

private:

	HINSTANCE	instanceHandle_;
	HWND		controlPageHandle_;
	NodeEventCallbackValue* nodeEvent_;

	Interface*	interface_;


private:
	void	CollectSubjects_Recursive(
		NodeArray& outArray, INode* currentNode,
		uint32 collectOption
	);

public:
	NodeArray	CollectSubjects(
		uint32 collectOption = CollectOptionBits_AllTypes
	);
	void	CollectSubjects(
		NodeArray& outArray,
		uint32 collectOption = CollectOptionBits_AllTypes
	);


private:
	TSharedPtr<ILiveLinkProvider>	liveLinkProvider_;
	FDelegateHandle					connectionStatusChangedHandle_;

public:
	ILiveLinkProvider*	GetLiveLinkProvider() const { return this->liveLinkProvider_.Get(); }


private:
	HWND				windowHandle_;

	ICustButton*		buttonAddSubject_;
	ICustButton*		buttonRemoveSubject_;

	ICustEdit*			textSubjectName_;
	HWND				listSubject_;

	ICustButton*		buttonAddCameraSubject_;
	ICustButton*		buttonSendMesh_;



	FMaxLiveLinkCameraStreamedSubject*			editViewCameraSubject_;

	bool useForceFrontX_Cache_;
	bool useSendMeshAutomatically_Cache_;


	TMap< FString, FMaxLiveLinkJointStreamedSubject* >	skinNodeSubjectList_;
	TArray<FMaxLiveLinkCameraStreamedSubject*>			cameraSubjectList_;

public:
	void	InitializeDialog( HWND hWnd );
	void	FinalizeDialog();

	bool	UseForceFrontAxisX() const;
	bool	UseAutomaticMeshUpdate() const;

	void	OnReceiveDialogCallbackCommand( WPARAM wParam );
	void	OnReceiveEnterEditCallbackCommand( WPARAM wParam );

	void	AddEditedSubject();
	void	RemoveSelectedSubject();

	void	DeleteAllSkinNodeSubject();

	void	ReceiveChangeGeometoryNodes( const TArray<INode*>& nodes );

	void	ApplyCurrentFrameSubjects( bool forceUpdateMesh = false );

	TimeValue	GetBindPoseTime() const;

private:
	TimeValue	lastUpdateTime_;

public:
	void	ModelOtherEvent();
	void	OnDisplay( TimeValue t, ViewExp *vpt, int flags );


	FMatrix	GetPointConvertMatrix() const
	{
		FMatrix matrix;

		if( this->UseForceFrontAxisX() )
		{
			matrix.M[ 0 ][ 0 ] = 0.0f;
			matrix.M[ 0 ][ 1 ] = 1.0f;
			matrix.M[ 0 ][ 2 ] = 0.0f;
			matrix.M[ 0 ][ 3 ] = 0.0f;

			matrix.M[ 1 ][ 0 ] = 0.0f;
			matrix.M[ 1 ][ 1 ] = 0.0f;
			matrix.M[ 1 ][ 2 ] = 1.0f;
			matrix.M[ 1 ][ 3 ] = 0.0f;

			matrix.M[ 2 ][ 0 ] = 1.0f;
			matrix.M[ 2 ][ 1 ] = 0.0f;
			matrix.M[ 2 ][ 2 ] = 0.0f;
			matrix.M[ 2 ][ 3 ] = 0.0f;

			matrix.M[ 3 ][ 0 ] = 0.0f;
			matrix.M[ 3 ][ 1 ] = 0.0f;
			matrix.M[ 3 ][ 2 ] = 0.0f;
			matrix.M[ 3 ][ 3 ] = 1.0f;
		}
		else
		{
			matrix.M[ 0 ][ 0 ] = 1.0f;
			matrix.M[ 0 ][ 1 ] = 0.0f;
			matrix.M[ 0 ][ 2 ] = 0.0f;
			matrix.M[ 0 ][ 3 ] = 0.0f;

			matrix.M[ 1 ][ 0 ] = 0.0f;
			matrix.M[ 1 ][ 1 ] = 0.0f;
			matrix.M[ 1 ][ 2 ] = 1.0f;
			matrix.M[ 1 ][ 3 ] = 0.0f;

			matrix.M[ 2 ][ 0 ] = 0.0f;
			matrix.M[ 2 ][ 1 ] =-1.0f;
			matrix.M[ 2 ][ 2 ] = 0.0f;
			matrix.M[ 2 ][ 3 ] = 0.0f;

			matrix.M[ 3 ][ 0 ] = 0.0f;
			matrix.M[ 3 ][ 1 ] = 0.0f;
			matrix.M[ 3 ][ 2 ] = 0.0f;
			matrix.M[ 3 ][ 3 ] = 1.0f;
		}

		return matrix;
	}

	static const Matrix3& GetFrontXMatrix()
	{
		static const Matrix3 frontXMatrix(
			Point3( 0.0f, 1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);
		return frontXMatrix;
	}

	static const Matrix3& GetInvertFrontZMatrix()
	{
		static const Matrix3 invertFrontZMatrix(
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 0.0f, -1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);

		/*
		FrontZMatrix(
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 0.0f, -1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);
		*/

		return invertFrontZMatrix;
	}

	static const Matrix3& GetFrontXRootMatrix()
	{
		static const Matrix3 frontXRootMatrix(
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);
		return frontXRootMatrix;
	}
};



NodeEventCallbackValue::NodeEventCallbackValue(
	FMaxLiveLink* maxLink,
	Interface* ip
)
	: maxLink_( maxLink )
	, interface_( ip )
{
	this->nodeEventKey_	=
		GetISceneEventManager()->RegisterCallback(
			this,
			FALSE, 50,
			TRUE
		);
	this->interface_->RegisterViewportDisplayCallback( TRUE, this );
}

/*

//Declare the callback function  
static void NodeDeleteNotify( void *param, NotifyInfo *info )
{
	// Get the nodes being deleted
	Tab<INode *> *tNode = ( Tab<INode*> )( info->callParam );

	// Do custom processing
	...
}

// Register the callback
RegisterNotification( NodeDeleteNotify, this, NOTIFY_SCENE_PRE_DELETED_NODE );

...

// When done, unregister the callback
UnRegisterNotification( NodeDeleteNotify, this, NOTIFY_SCENE_PRE_DELETED_NODE );

*/
NodeEventCallbackValue::~NodeEventCallbackValue()
{
	this->interface_->UnRegisterViewportDisplayCallback( TRUE, this );
	GetISceneEventManager()->UnRegisterCallback( this->nodeEventKey_ );

}


void	NodeEventCallbackValue::ModelStructured( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "ModelStructured" ) );
	printf( "ModelStructured" );
}
void	NodeEventCallbackValue::GeometryChanged( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "GeometryChanged" ) );
	printf( "GeometryChanged" );
}
void	NodeEventCallbackValue::MappingChanged( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "MappingChanged" ) );
	printf( "MappingChanged" );
	
	TArray<INode*> nodeList;
	for( int iNode = 0; iNode < nodes.Count(); ++iNode )
	{
		INode* node = NodeEventNamespace::GetNodeByKey( nodes[ iNode ] );
		if( node ) 
		{
			if( node->Selected() )
			{
				nodeList.Add( node ); 
			}
		}
	}
	if( nodeList.Num() > 0 )
	{
		this->maxLink_->ReceiveChangeGeometoryNodes( nodeList );
	}
}

void	NodeEventCallbackValue::ExtentionChannelChanged( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "ExtentionChannelChanged" ) );
	printf( "ExtentionChannelChanged" );
}

void	NodeEventCallbackValue::ModelOtherEvent( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "ModelOtherEvent" ) );
	printf( "ModelOtherEvent" );
	this->maxLink_->ModelOtherEvent();
}

void	NodeEventCallbackValue::NameChanged( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "NameChanged" ) );
	printf( "NameChanged" );
	this->maxLink_->DeleteAllSkinNodeSubject();
}
void	NodeEventCallbackValue::Added( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "Added" ) );
	printf( "Added" );
	this->maxLink_->DeleteAllSkinNodeSubject();
}
void	NodeEventCallbackValue::Deleted( NodeKeyTab& nodes )
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "Deleted" ) );
	printf( "Deleted" );
	this->maxLink_->DeleteAllSkinNodeSubject();
}


void	NodeEventCallbackValue::Display( TimeValue t, ViewExp *vpt, int flags )
{
	this->maxLink_->OnDisplay( t, vpt, flags );
}


void	NodeEventCallbackValue::GetViewportRect( TimeValue t, ViewExp* vpt, Rect* rect )
{
}

BOOL	NodeEventCallbackValue::Foreground()
{
	return TRUE;
}



class FMaxLiveLinkBaseStreamedSubject
{

public:
	FMaxLiveLinkBaseStreamedSubject(
		FMaxLiveLink* livelinkReference,
		INode* node, const FName& subjectName
	)
		: livelinkReference_( livelinkReference )
		, subjectName_( subjectName )
	{
		this->nodeList_.Add( node );
	}

	FMaxLiveLinkBaseStreamedSubject(
		FMaxLiveLink* livelinkReference,
		const NodeArray& nodeList, const FName& subjectName
	)
		: livelinkReference_( livelinkReference )
		, subjectName_( subjectName )
	{
		for( auto node : nodeList )
		{
			this->nodeList_.Add( node );
		}
	}

	FMaxLiveLinkBaseStreamedSubject(
		FMaxLiveLink* livelinkReference,
		const FName& subjectName
	)
		: livelinkReference_( livelinkReference )
		, subjectName_( subjectName )
	{}

	virtual ~FMaxLiveLinkBaseStreamedSubject()
	{
	}

protected:
	FMaxLiveLink*	livelinkReference_;

	static const int ParentIndex_None = -1;
	struct NodeReference
	{
		NodeReference()
			: ReferencedNode( nullptr )
			, ShouldUpdateGeometry( false )
			, ParentIndexCache( ParentIndex_None )
		{}
		NodeReference(
			INode*		node,
			bool		shouldUpdateGeometry = true
		)
			: ReferencedNode( node )
			, ShouldUpdateGeometry( shouldUpdateGeometry )
			, ParentIndexCache( ParentIndex_None )
		{}

		INode*		ReferencedNode;
		bool		ShouldUpdateGeometry;
		int			ParentIndexCache;
	};


	TArray<NodeReference>	nodeList_;
	FName					subjectName_;


	ILiveLinkProvider* GetLiveLinkProvider() const
	{
		return this->livelinkReference_->GetLiveLinkProvider();
	}

public:
	virtual bool	ValidateSubject() const = 0;
	virtual void	RebuildSubjectData() = 0;

	static FTransform	ConvertAffineToTransform( const Matrix3& mtx )
	{
		AffineParts affine;
		decomp_affine( mtx, &affine );

		FQuat	rotateValue			= FQuat::Identity;
		FVector scaleValue			= FVector::OneVector;
		FVector translationValue	= FVector::ZeroVector;

		{
			{
				rotateValue.X	= affine.q.x;
				rotateValue.Y	=-affine.q.y;
				rotateValue.Z	= affine.q.z;
				rotateValue.W	= affine.q.w;
			}
			{
				scaleValue.X	= affine.k.x;
				scaleValue.Y	= affine.k.y;
				scaleValue.Z	= affine.k.z;
			}
			{
				translationValue.X		= affine.t.x;
				translationValue.Y		=-affine.t.y;
				translationValue.Z		= affine.t.z;
			}
		}
		return FTransform(
			rotateValue,
			translationValue,
			scaleValue
		);
	}

	static FTransform	GetSubjectTransformFromSeconds(
		INode* node,
		double streamTime,
		bool frontAxisX, bool forRootNode
	)
	{
		return GetSubjectTransform(
			node,
			SecToTicks( streamTime ),
			frontAxisX, forRootNode
		);
	}
	static FTransform	GetSubjectTransform(
		INode* node,
		TimeValue timeValue,
		bool frontAxisX, bool forRootNode
	)
	{
		auto nodeName = node->GetName();


		const auto& frontXMatrix = FMaxLiveLink::GetFrontXMatrix();

		Matrix3 vm(
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 0.0f,-1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);

		vm.Invert();

		const auto& frontXRootMatrix = FMaxLiveLink::GetFrontXRootMatrix();


		Matrix3 parentMtx;
		auto transformMatrix = node->GetNodeTM( timeValue, nullptr );

		if( frontAxisX )
		{
			transformMatrix = ( transformMatrix * vm ) * frontXMatrix;
			transformMatrix = frontXRootMatrix * transformMatrix;
		}

		Interface14* interFace = GetCOREInterface14();

		bool isRootNode = false;
		auto parentNode = node->GetParentNode();

		if( parentNode )
		{
			if( interFace->GetRootNode() == parentNode )
			{
				isRootNode = true;
			}
			else
			{
				auto objRef		= node->GetObjectRef();
				if( objRef )
				{
					if( objRef->ClassID() != BONE_OBJ_CLASSID )
					{
						int numChild = node->NumChildren();
						for( int iChild = 0; iChild < numChild; ++iChild )
						{
							auto child = node->GetChildNode( iChild );
							if( child )
							{
								auto childObject		= child->GetObjectRef();
								if( 
									childObject &&
									childObject->ClassID() == BONE_OBJ_CLASSID 
								)
								{
									isRootNode = true;
									break;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			isRootNode = true;
		}

		

		if( !isRootNode )
		{
			parentMtx = parentNode->GetNodeTM( timeValue );

			if( frontAxisX )
			{
				parentMtx = frontXRootMatrix * ( ( parentMtx * vm ) * frontXMatrix );
			}

			parentMtx.Invert();
			transformMatrix = transformMatrix * parentMtx;
		}
		FQuat	rotateValue			= FQuat::Identity;
		FVector scaleValue			= FVector::OneVector;
		FVector translationValue	= FVector::ZeroVector;

		Interval interval( timeValue, timeValue );
		
		auto unitScale = GetMasterScale( GetUSDefaultUnit());
		AffineParts affine;
		decomp_affine( transformMatrix, &affine );


		auto tmController = node->GetTMController();
		if( tmController && 0 )
		{
			// use animation contlloer
			AffineParts affine2 = affine;
			{
				auto scaleController = tmController->GetScaleController();
				if( scaleController )
				{
					auto controllerX = scaleController->GetXController();
					if( controllerX )
					{
						controllerX->GetValue(
							timeValue, &affine2.k.x,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerY = scaleController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &affine2.k.y,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerZ = scaleController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &affine2.k.z,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}
				}
			}
			{
				auto positionController = tmController->GetPositionController();
				if( positionController )
				{
					auto controllerX = positionController->GetXController();
					if( controllerX )
					{
						controllerX->GetValue(
							timeValue, &affine2.t.x,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerY = positionController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &affine2.t.y,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerZ = positionController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &affine2.t.z,
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}
				}
			}
			{
				auto rotationController = tmController->GetRotationController();
				if( rotationController )
				{
					float eularXYZ[3];
					affine2.q.GetEuler( &eularXYZ[ 0 ], &eularXYZ[ 1 ], &eularXYZ[ 2 ] );

					float eularXYZ2[ 3 ] = { eularXYZ[0], eularXYZ[ 1 ], eularXYZ[ 2 ] };

					auto controllerX = rotationController->GetXController();
					if( controllerX )
					{
						controllerX->GetValue(
							timeValue, &eularXYZ[ 0 ],
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerY = rotationController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &eularXYZ[ 1 ],
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					auto controllerZ = rotationController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &eularXYZ[ 2 ],
							interval, GetSetMethod::CTRL_ABSOLUTE
						);
					}

					affine2.q.SetEuler( eularXYZ[ 0 ], eularXYZ[ 1 ], eularXYZ[ 2 ] );

					Matrix3 srtm, rtm, ptm, stm, ftm;
					ptm.IdentityMatrix();
					ptm.SetTrans( affine2.t );
					affine2.q.MakeMatrix( rtm );
					affine2.u.MakeMatrix( srtm );
					stm = ScaleMatrix( affine2.k );
					ftm = ScaleMatrix( Point3( affine2.f, affine2.f, affine2.f ) );
					Matrix3 resultLocalMtx = Inverse( srtm ) * stm * srtm * rtm * ftm * ptm;

					if( isRootNode )
					{
						resultLocalMtx *= node->GetParentTM( timeValue );
					}
					decomp_affine( resultLocalMtx, &affine2 );
					affine = affine2;
				}
			}
		}
		{
			{
				rotateValue.X	= affine.q.x;
				rotateValue.Y	=-affine.q.y;
				rotateValue.Z	= affine.q.z;
				rotateValue.W	= affine.q.w;
			}
			{
				scaleValue.X	= affine.k.x;
				scaleValue.Y	= affine.k.y;
				scaleValue.Z	= affine.k.z;
			}
			{
				translationValue.X		= affine.t.x;
				translationValue.Y		=-affine.t.y;
				translationValue.Z		= affine.t.z;
			}
		}
		return FTransform(
			rotateValue,
			translationValue,
			scaleValue
		);
	}

	virtual void OnStream( double streamTime, int32 frameNumber ) = 0;
};



class FMaxLiveLinkCameraStreamedSubject
	: public FMaxLiveLinkBaseStreamedSubject
{
public:
	FMaxLiveLinkCameraStreamedSubject(
		FMaxLiveLink* livelinkReference,
		INode* node, const FName& subjectName
	)
		: FMaxLiveLinkBaseStreamedSubject(
			livelinkReference,
			node, subjectName
		)
	{
		this->RebuildSubjectData();
	}

	FMaxLiveLinkCameraStreamedSubject(
		FMaxLiveLink* livelinkReference,
		const FName& subjectName
	)
		: FMaxLiveLinkBaseStreamedSubject(
			livelinkReference,
			subjectName
		)
	{
		this->RebuildSubjectData();
	}

	virtual ~FMaxLiveLinkCameraStreamedSubject()
	{
		this->GetLiveLinkProvider()->ClearSubject( this->subjectName_ );
	}

public:


	virtual bool ValidateSubject() const
	{
		return true;
	}

	virtual void RebuildSubjectData()
	{

		TArray<FName>	jointNames;
		TArray<int32>	jointParents;
		jointNames.Add( TEXT( "root" ) );
		jointParents.Add( -1 );

		{
			Matrix3 viewAffine;
			TArray<FTransform> transformList =
			{
				ConvertAffineToTransform( viewAffine )
			};
			TArray<FLiveLinkCurveElement> curves;

			this->GetLiveLinkProvider()->UpdateSubjectFrame(
				this->subjectName_,
				transformList,
				curves,
				0.0
			);
		}

		this->GetLiveLinkProvider()->UpdateSubject(
			this->subjectName_,
			jointNames,
			jointParents
		);
	}

	virtual void OnStream( double streamTime, int32 frameNumber )
	{
		if( this->nodeList_.Num() == 0 )
		{
			return;
		}

		TArray<FTransform> transformList =
		{
			this->GetSubjectTransformFromSeconds( this->nodeList_[0].ReferencedNode, streamTime, false, true )
		};
		TArray<FLiveLinkCurveElement> curves;

		this->GetLiveLinkProvider()->UpdateSubjectFrame(
			this->subjectName_,
			transformList,
			curves,
			streamTime
		);
	}

	void OnStreamCamera( double streamTime, int frame, ViewExp13* vpt, int flags )
	{
		auto unit		= GetUSDefaultUnit();
		auto unitScale	= 1.0 / ( GetMasterScale( unit ) / 10.0 );
		unitScale = 1.0;
		//unitScale = 2.311;

		Matrix3 viewAffine;
		vpt->GetAffineTM( viewAffine );

		auto distance	= vpt->GetFocalDist();
		auto scale		= vpt->NonScalingObjectSize();

		auto trans = viewAffine.GetRow( 3 );

		FQuat quat = FQuat::Identity;

		auto viewType	= vpt->GetViewType();
		auto fov		= vpt->GetFOV();
		auto zoom		= vpt->GetZoom();

		Point3 screenScale( 1,1,1 );
		auto screenScaleFactor = vpt->GetScreenScaleFactor( screenScale );

		auto cameraScale = FVector::OneVector;

		FLiveLinkMetaData metaData;
		metaData.SceneTime.Frames		= frame;
		metaData.SceneTime.FrameRate	= FLiveLinkFrameRate( (uint32)(1.0f / vpt->GetFPS()), 1 );
		metaData.SceneTime.Seconds		= (int32)( streamTime );

		auto graphicWindow = vpt->getGW();
		if( graphicWindow )
		{
			auto transform = graphicWindow->getTransform();

			Matrix3 invTm;
			float mtx[4][4];
			int persp;
			float hither;
			float yon;

			graphicWindow->getCameraMatrix(
				mtx,
				&invTm,
				&persp,
				&hither, &yon
			);

			{
				cameraScale.X = zoom;
				cameraScale.Y = zoom;
				cameraScale.Z = zoom;
			}

			auto fovRad = FMath::RadiansToDegrees( fov );

			metaData.StringMetaData.Add( FName( TEXT("fov") ), FString::SanitizeFloat( fovRad ));
			
			/*
			if( vpt->GetViewCamera() )
			{
				invTm = vpt->GetViewCamera()->GetNodeTM( SecToTicks( streamTime ) );

				trans = invTm.GetTrans();
				invTm.SetRow( 3, Point3( 0.0, 0.0, 0.0 ) );
				trans = trans * invTm;

				if( vpt->GetViewSpot() )
				{
					auto viewSpotTM = vpt->GetViewSpot()->GetNodeTM( SecToTicks( streamTime ) );

					auto viewSpotPos = viewSpotTM.GetTrans();

					distance = ( viewSpotPos - trans ).Length();

				}
			}
			*/

			Matrix3 rotateMtx;
			rotateMtx.SetRow( 0, Point3( 0.0, 0.0,-1.0 ) );
			rotateMtx.SetRow( 1, Point3( 0.0, 1.0, 0.0 ) );
			rotateMtx.SetRow( 2, Point3(-1.0, 0.0, 0.0 ) );
			rotateMtx.SetRow( 3, Point3( 0.0, 0.0, 0.0 ) );

			Matrix3 convMtx;
			convMtx.SetRow( 0, Point3( 1.0, 0.0, 0.0 ) );
			convMtx.SetRow( 1, Point3( 0.0, 0.0, 1.0 ) );
			convMtx.SetRow( 2, Point3( 0.0, -1.0, 0.0 ) );
			convMtx.SetRow( 3, Point3( 0.0, 0.0, 0.0 ) );
			convMtx.Invert();

			viewAffine = ( ( invTm ) * convMtx );

			viewAffine *= rotateMtx;
			

			viewAffine.SetRow( 3, Point3( 0.0, 0.0, 0.0 ) );

			float yaw;
			float pitch;
			float roll;
			viewAffine.GetYawPitchRoll( &yaw, &pitch, &roll );

			quat = 
				FQuat( FRotator( 
					FMath::RadiansToDegrees( pitch ), 
					FMath::RadiansToDegrees( yaw ),
					FMath::RadiansToDegrees( roll )
				) );

			auto pos = Point3( 0.0f, 0.0f, distance );
			pos = pos * viewAffine;

			invTm.SetRow( 3, Point3( 0.0, 0.0, 0.0 ) );
			trans = trans * viewAffine;
			trans = Point3( -trans.x, trans.y, -trans.z );
			trans += Point3( pos.x, -pos.y, pos.z );
		}

		TArray<FTransform> transformList =
		{
			FTransform(
				quat,
				FVector( -trans.z * unitScale, -trans.x * unitScale, -trans.y * unitScale ),
				cameraScale
			)
		};

		

		TArray<FLiveLinkCurveElement> curves;

		this->GetLiveLinkProvider()->UpdateSubjectFrame(
			this->subjectName_,
			transformList,
			curves,
			metaData,
			streamTime
		);
	}
};



class FMaxLiveLinkJointStreamedSubject
	: public FMaxLiveLinkBaseStreamedSubject
{
public:
	FMaxLiveLinkJointStreamedSubject(
		FMaxLiveLink* livelinkReference,
		INode* node, const FName& subjectName
	)
		: FMaxLiveLinkBaseStreamedSubject(
			livelinkReference,
			node, subjectName
		)
	{
		this->RebuildSubjectData();
	}

	FMaxLiveLinkJointStreamedSubject(
		FMaxLiveLink* livelinkReference,
		const NodeArray& nodeList, const FName& subjectName
	)
		: FMaxLiveLinkBaseStreamedSubject(
			livelinkReference,
			nodeList, subjectName
		)
	{
		this->RebuildSubjectData();
	}


	virtual ~FMaxLiveLinkJointStreamedSubject()
	{
		this->GetLiveLinkProvider()->ClearSubject( this->subjectName_ );
	}

private:

	NodeArray	jointNodeList_;

public:


	bool OnlyJoint() const { return false; }

	TArray<ISkin*>		GetSkinList() const
	{
		TArray<ISkin*> ret;

		for( auto node : this->nodeList_ )
		{
			auto derivedObject	= reinterpret_cast<IDerivedObject*>( node.ReferencedNode->GetObjectRef() );
			if( !derivedObject )
			{
				continue;
			}
			int numModifier = derivedObject->NumModifiers();
			for( int iModifier = 0; iModifier < numModifier; ++iModifier )
			{
				auto modifier = derivedObject->GetModifier( iModifier );
				if( modifier )
				{
					auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
					if( skin )
					{
						ret.Add( skin );
					}
				}
			}
		}
		return ret;
	}


	virtual bool ValidateSubject() const
	{
		if( this->GetSkinList().Num() <= 0 )
		{
			return false;
		}
		return true;
	}

	virtual void RebuildSubjectData()
	{
		
		
		TArray<FName>	jointNames;
		TArray<int32>	jointParents;

		if( OnlyJoint() )
		{
			this->nodeList_.Empty();
			this->jointNodeList_.Empty();

			auto skinList = this->GetSkinList();
			for( auto skin : skinList )
			{
				int numBones = skin->GetNumBones();
				for( int iBone = 0; iBone < numBones; ++iBone )
				{
					auto bone = skin->GetBone( iBone );
					if( bone )
					{
						this->jointNodeList_.AddUnique( bone );
					}
				}
			}

			for( int iBone = 0; iBone < this->jointNodeList_.Num(); ++iBone )
			{
				auto node = this->jointNodeList_[ iBone ];

				int parentIndex = -1;
				for( int jBone = 0; jBone < this->jointNodeList_.Num(); ++jBone )
				{
					if( node->GetParentNode() == this->jointNodeList_[ jBone ] )
					{
						parentIndex = jBone;
						break;
					}
				}
				jointNames.Add( node->GetName() );
				jointParents.Add( parentIndex );
			}
		}
		else
		{
			for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
			{
				auto node = this->nodeList_[ iNode ].ReferencedNode;
				const auto nodeName = node->GetName();

				this->nodeList_[ iNode ].ParentIndexCache = ParentIndex_None;
				for( int jNode = 0; jNode < this->nodeList_.Num(); ++jNode )
				{
					if( node->GetParentNode() == this->nodeList_[ jNode ].ReferencedNode )
					{
						this->nodeList_[ iNode ].ParentIndexCache = jNode;
						break;
					}
				}
				jointNames.Add( node->GetName() );
				jointParents.Add( this->nodeList_[ iNode ].ParentIndexCache );
			}
		}

		this->GetLiveLinkProvider()->UpdateSubject(
			this->subjectName_,
			jointNames,
			jointParents
		);
	}



	void ReceiveUpdateGeometryOfNodes( const TArray<INode*>& nodes )
	{
		for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
		{
			for( int iUpdateNodes = 0; iUpdateNodes < nodes.Num(); ++iUpdateNodes )
			{
				if( this->nodeList_[ iNode ].ReferencedNode == nodes[ iUpdateNodes ] )
				{
					this->nodeList_[ iNode ].ShouldUpdateGeometry = true;
					break;
				}
			}
		}
	}
	void	SettingUpdateGeometry( bool doUpdate )
	{
		for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
		{
			this->nodeList_[ iNode ].ShouldUpdateGeometry = doUpdate;
		}
	}

	virtual void OnStream( double streamTime, int32 frameNumber )
	{
		TArray<FTransform> transformList;
		TArray<FLiveLinkCurveElement> curves;

		bool hasMeshData = false;

		FLiveLinkMetaData metaData;
		FLiveLinkExtendSyncData syncMeshData;

		FDateTime currentTime = FDateTime::Now();


		FMatrix matrix = this->livelinkReference_->GetPointConvertMatrix();
		if( OnlyJoint() )
		{
			for( int iJoint = 0; iJoint < this->jointNodeList_.Num(); ++iJoint )
			{
				auto joint = this->jointNodeList_[ iJoint ];
				auto transform = 
					this->GetSubjectTransformFromSeconds( 
						joint, streamTime, 
						this->livelinkReference_->UseForceFrontAxisX(), false
					);

				transformList.Add( transform );
			}
		}
		else
		{
			for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
			{
				auto node = this->nodeList_[ iNode ].ReferencedNode;
				const auto nodeName = node->GetName();

				auto transform = 
					this->GetSubjectTransformFromSeconds( 
						node, streamTime, 
						this->livelinkReference_->UseForceFrontAxisX(),
						this->nodeList_[ iNode ].ParentIndexCache == ParentIndex_None
					);

				transformList.Add( transform );

				if( !this->nodeList_[ iNode ].ShouldUpdateGeometry )
				{
					continue;
				}
				this->nodeList_[ iNode ].ShouldUpdateGeometry = false;

				auto obj	= node->GetObjectRef();
				if( !obj )
				{ 
					continue; 
				}

				auto material = node->GetMtl();

				ISkin*		skinObject	= nullptr;
				MorphR3*	morphObject	= nullptr;
				{
					auto deriverdObj	= reinterpret_cast<IDerivedObject*>( obj );
					int numModifier = deriverdObj->NumModifiers();
					for( int iModifier = 0; iModifier < numModifier; ++iModifier )
					{
						auto modifier = deriverdObj->GetModifier( iModifier );
						if( modifier )
						{

							auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
							if( skin )
							{
								skinObject = skin;
								ObjectState state = obj->Eval( this->livelinkReference_->GetBindPoseTime() );
								obj = state.obj;
								break;
							}

							if(
								modifier->SuperClassID() == OSM_CLASS_ID &&
								modifier->ClassID() == MORPHER_CLASS_ID
								)
							{
								morphObject = static_cast<MorphR3*>( modifier );

								ObjectState state = obj->Eval( this->livelinkReference_->GetBindPoseTime() );
								obj = state.obj;

								break;
							}
						}
					}
				}


				FLiveLinkExtendVertexMeshData meshData;

				auto id		= obj->ClassID();
				auto sid	= obj->SuperClassID();

				TriObject* triObject	= nullptr;
				PolyObject* polyObject	= nullptr;
				if( obj->IsSubClassOf( triObjectClassID ) )
				{
					triObject = ( TriObject* )obj;
				}
				else if( 
					sid == GEOMOBJECT_CLASS_ID && 
					( id == EPOLYOBJ_CLASS_ID || id.PartA() == POLYOBJ_CLASS_ID ) 
				)
				{
					polyObject = ( PolyObject* )obj;
				}
				else if( 
					sid == GEOMOBJECT_CLASS_ID && 
					( id.PartA() == EDITTRIOBJ_CLASS_ID || id.PartA() == TRIOBJ_CLASS_ID ) 
				)
				{
					triObject = ( TriObject* )obj;
				}
				else if( obj->CanConvertToType( Class_ID( POLYOBJ_CLASS_ID, 0 ) ) )
				{
					polyObject = ( PolyObject* )obj;
				}
				else if( obj->CanConvertToType( Class_ID( TRIOBJ_CLASS_ID, 0 ) ) )
				{
					triObject = ( TriObject* )obj;
				}


				bool hasNodeMeshData = false;

				if( triObject )
				{
					int numVertexies	= triObject->mesh.numVerts;
					int numFaces		= triObject->mesh.numFaces;
					if( numVertexies > 0 && numFaces > 0 )
					{
						meshData.MeshName	= nodeName;
						meshData.TimeStamp	= currentTime;

						
						{
							for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
							{
								auto vertexPos = triObject->mesh.getVertPtr( iVertex );

								auto resultPos = 
									matrix.TransformFVector4( FVector4(
										vertexPos->x,
										vertexPos->y,
										vertexPos->z
									) );

								resultPos.Y = -resultPos.Y;

								meshData.PointList.Add(
									FLiveLinkExtendPoint3(
										resultPos.X,
										resultPos.Y,
										resultPos.Z
									)
								);
							}
						}

						const int NotHasNormalSpec		= 0;
						const int HasVertexNormalSpec	= 1;
						const int HasFacesNormalSpec	= 2;

						int hasNormalSpec = NotHasNormalSpec;

						MeshNormalSpec* normalSpec = nullptr;
						normalSpec = triObject->mesh.GetSpecifiedNormals();
						if( normalSpec )
						{
							if( numVertexies == normalSpec->GetNumNormals() )
							{
								hasNormalSpec = HasVertexNormalSpec;
							}
							else if( numFaces == normalSpec->GetNumFaces() )
							{
								hasNormalSpec = HasFacesNormalSpec;
							}
						}

						if( hasNormalSpec == HasVertexNormalSpec )
						{
							for( int iNormal = 0; iNormal < numVertexies; ++iNormal )
							{
								const auto& normal = normalSpec->Normal( iNormal );

								auto resultVec =
									matrix.TransformFVector4( FVector4(
										normal.x,
										normal.y,
										normal.z
									) );

								resultVec.Y = -resultVec.Y;
								meshData.PointNormalList.Add(
									FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
								);

							}
						}
						else if( hasNormalSpec == NotHasNormalSpec )
						{
							for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
							{
								auto normal = triObject->mesh.getNormal( iVertex );
								auto resultVec =
									matrix.TransformFVector4( FVector4(
										normal.x,
										normal.y,
										normal.z
									) );

								resultVec.Y = -resultVec.Y;
								meshData.PointNormalList.Add(
									FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
								);
							}

						}


						int maxTextureCoord = triObject->mesh.getNumMaps();
						TMap<MtlID, TArray<int> > materialFaceMap;
						{
							for( int iFace = 0; iFace < numFaces; ++iFace )
							{
								auto materialID		= triObject->mesh.getFaceMtlIndex( iFace );
								auto findMaterial	= materialFaceMap.Find( materialID );
								if( findMaterial )
								{
									findMaterial->Add( iFace );
								}
								else
								{
									TArray<int> faceArray;
									faceArray.Push( iFace );
									materialFaceMap.Add( materialID , faceArray );
								}
							}
						}


						for( const auto& facePair : materialFaceMap )
						{
							if( facePair.Key >= material->NumSubMtls() )
							{
								continue;
							}

							auto groupMaterial = material->GetSubMtl( facePair.Key );

							FLiveLinkExtendMaterialGroupMeshData groupData;

							groupData.Triangles		= 0;
							if( materialFaceMap.Num() == 1 )
							{
								groupData.MaterialName	= material->GetName();
							}
							else
							{
								groupData.MaterialName	= groupMaterial->GetName();
							}

							const auto& faceList = facePair.Value;

							const int numTriangle = 3;
							int numFaces = facePair.Value.Num();
							for( int iFace = 0; iFace < numFaces; ++iFace )
							{
								auto faceIndex		= faceList[ iFace ];
								const auto& face	= triObject->mesh.faces[ faceIndex ];

								FLiveLinkExtendPolygon polygon;
								polygon.SmoothingGroup = face.smGroup;

								for( int iVertexIndex = 0; iVertexIndex < 3; ++iVertexIndex )
								{
									auto vertexIndex = face.v[ iVertexIndex ];
									polygon.VertexIndexList.Push( vertexIndex );
								}

								if( hasNormalSpec == HasFacesNormalSpec )
								{
									for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
									{
										const auto& normal = normalSpec->GetNormal( iFace, iTriangle );
										auto resultVec =
											matrix.TransformFVector4( FVector4(
												normal.x,
												normal.y,
												normal.z
											) );

										resultVec.Y = -resultVec.Y;
										polygon.NormalList.Add(
											FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
										);
									}

								}

								polygon.ColorList0.Init( FLiveLinkExtendPoint4( 1.0f, 1.0f, 1.0f, 1.0f ), numTriangle );

								for( int iMapChannel = -NUM_HIDDENMAPS; iMapChannel < triObject->mesh.numMaps; ++iMapChannel )
								{
									auto& mapChannel = triObject->mesh.Map( iMapChannel );
									if( !mapChannel.GetFlag( MN_DEAD ) && iFace < mapChannel.fnum )
									{
										const auto& mapFace = mapChannel.tf[ iFace ];
										switch( iMapChannel )
										{
										case MAP_ALPHA:
											{
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& color = mapChannel.tv[ vIndex ];
													polygon.ColorList0[ iTriangle ].W = color.x;
												}
											}break;
										case 0:
											{
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& color = mapChannel.tv[ vIndex ];
													polygon.ColorList0[ iTriangle ].X = color.x;
													polygon.ColorList0[ iTriangle ].Y = color.y;
													polygon.ColorList0[ iTriangle ].Z = color.z;
												}
											}break;

										case 1:
											{
												polygon.UVList0.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList0[ iTriangle ].X = uv.x;
													polygon.UVList0[ iTriangle ].Y = uv.y;
												}
											}break;
										case 2:
											{
												polygon.UVList1.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList1[ iTriangle ].X = uv.x;
													polygon.UVList1[ iTriangle ].Y = uv.y;
												}
											}break;
										case 3:
											{
												polygon.UVList2.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList2[ iTriangle ].X = uv.x;
													polygon.UVList2[ iTriangle ].Y = uv.y;
												}
											}break;
										case 4:
											{
												polygon.UVList3.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList3[ iTriangle ].X = uv.x;
													polygon.UVList3[ iTriangle ].Y = uv.y;
												}
											}break;

										default:
											break;

										}
									}
								}

								groupData.PolygonList.Push( polygon );
							}
							meshData.MaterialGroupList.Push( groupData );
						}

						hasNodeMeshData = true;
					}
				}
				else if( polyObject )
				{
					meshData.MeshName	= nodeName;
					meshData.TimeStamp	= currentTime;

					int numVertexies	= polyObject->mm.numv;
					int numFaces		= polyObject->mm.numf;
					if( numVertexies > 0 && numFaces > 0 )
					{
						for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
						{
							MNVert* vertex = polyObject->mm.V( iVertex );
							auto resultPos =
								matrix.TransformFVector4( FVector4(
									vertex->p.x,
									vertex->p.y,
									vertex->p.z
								) );

							resultPos.Y = -resultPos.Y;

							meshData.PointList.Add(
								FLiveLinkExtendPoint3(
									resultPos.X,
									resultPos.Y,
									resultPos.Z
								)
							);
						}

						const int NotHasNormalSpec		= 0;
						const int HasVertexNormalSpec	= 1;
						const int HasFacesNormalSpec	= 2;

						int hasNormalSpec = NotHasNormalSpec;

						MNNormalSpec* normalSpec = nullptr;
						normalSpec = polyObject->mm.GetSpecifiedNormals();
						if( normalSpec )
						{
							if( numVertexies == normalSpec->GetNumNormals() )
							{
								hasNormalSpec = HasVertexNormalSpec;
							}
							else if( numFaces == normalSpec->GetNumFaces() )
							{
								hasNormalSpec = HasFacesNormalSpec;
							}
						}

						if( hasNormalSpec == HasVertexNormalSpec )
						{
							for( int iNormal = 0; iNormal < numVertexies; ++iNormal )
							{
								const auto& normal = normalSpec->Normal( iNormal );
								auto resultVec =
									matrix.TransformFVector4( FVector4(
										normal.x,
										normal.y,
										normal.z
									) );

								resultVec.Y = -resultVec.Y;
								meshData.PointNormalList.Add(
									FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
								);
							}
						}
						else if( hasNormalSpec == NotHasNormalSpec )
						{
							for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
							{
								auto normal = polyObject->mm.GetVertexNormal( iVertex );
								auto resultVec =
									matrix.TransformFVector4( FVector4(
										normal.x,
										normal.y,
										normal.z
									) );

								resultVec.Y = -resultVec.Y;
								meshData.PointNormalList.Add(
									FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
								);
							}
							
						}

					
						TMap<MtlID, TArray<int> > materialFaceMap;
						{
							
							for( int iFace = 0; iFace < numFaces; ++iFace )
							{
								auto face = polyObject->mm.F( iFace );
								if( face )
								{
									auto findMaterial = materialFaceMap.Find( face->material );
									if( findMaterial )
									{
										findMaterial->Add( iFace );
									}
									else
									{
										TArray<int> faceArray;
										faceArray.Push( iFace );
										materialFaceMap.Add( face->material, faceArray );
									}
								}
							}
						}

						for( const auto& facePair : materialFaceMap )
						{
							if( facePair.Key >= material->NumSubMtls() )
							{
								continue;
							}

							auto groupMaterial = material->GetSubMtl( facePair.Key );

							FLiveLinkExtendMaterialGroupMeshData groupData;
							groupData.Triangles	= 0;
							if( materialFaceMap.Num() == 1 )
							{
								groupData.MaterialName	= material->GetName();
							}
							else
							{
								groupData.MaterialName	= groupMaterial->GetName();
							}

							

							int numFace		= polyObject->mm.numf;
							for( int iFace = 0; iFace < numFace; ++iFace )
							{
								auto face = polyObject->mm.F( iFace );
								if( face )
								{
									Tab<int> triangleList;

									if( 0 >= face->TriNum() )
									{
										continue;
									}

									face->GetTriangles( triangleList );
									FLiveLinkExtendPolygon polygon;
									polygon.SmoothingGroup = face->smGroup;

									groupData.Triangles += face->TriNum();

									int numTriangle = triangleList.Count();
									for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
									{
										int vertexIndex = face->vtx[ triangleList[ iTriangle ] ];
										polygon.VertexIndexList.Push( vertexIndex );
									}

									if( hasNormalSpec == HasFacesNormalSpec )
									{
										for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
										{
											const auto& normal = normalSpec->GetNormal( iFace, triangleList[ iTriangle ] );
											auto resultVec =
												matrix.TransformFVector4( FVector4(
													normal.x,
													normal.y,
													normal.z
												) );

											resultVec.Y = -resultVec.Y;
											polygon.NormalList.Add(
												FLiveLinkExtendPoint3( resultVec.X, resultVec.Y, resultVec.Z )
											);
										}
										
									}

									polygon.ColorList0.Init( FLiveLinkExtendPoint4( 1.0f, 1.0f, 1.0f, 1.0f ), numTriangle );

									for( int iMapChannel = -NUM_HIDDENMAPS; iMapChannel < polyObject->mm.numm; ++iMapChannel )
									{
										const auto mapChannel = polyObject->mm.M( iMapChannel );
										if( mapChannel )
										{
											if( iFace >= mapChannel->numf )
											{
												continue;
											}
										
											if( !mapChannel->GetFlag( MN_DEAD ) && iFace < mapChannel->numf )
											{
												const auto& mapFace = mapChannel->f[ iFace ];
												switch( iMapChannel )
												{
												case MAP_ALPHA:
													{
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& color = mapChannel->v[ vIndex ];
															polygon.ColorList0[ iTriangle ].W = color.x;
														}
													}break;
												case 0:
													{
													
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& color = mapChannel->v[ vIndex ];
															polygon.ColorList0[ iTriangle ].X = color.x;
															polygon.ColorList0[ iTriangle ].Y = color.y;
															polygon.ColorList0[ iTriangle ].Z = color.z;
														}
													}break;
												case 1:
													{
														polygon.UVList0.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList0[ iTriangle ].X = uv.x;
															polygon.UVList0[ iTriangle ].Y = uv.y;
														}
													}break;
												case 2:
													{
														polygon.UVList1.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList1[ iTriangle ].X = uv.x;
															polygon.UVList1[ iTriangle ].Y = uv.y;
														}
													}break;
												case 3:
													{
														polygon.UVList2.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList2[ iTriangle ].X = uv.x;
															polygon.UVList2[ iTriangle ].Y = uv.y;
														}
													}break;
												case 4:
													{
														polygon.UVList3.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList3[ iTriangle ].X = uv.x;
															polygon.UVList3[ iTriangle ].Y = uv.y;
														}
													}break;

												default:
													{
													}break;
												}
											}
										}
									}
									groupData.PolygonList.Push( polygon );
								}
							}
							meshData.MaterialGroupList.Push( groupData );
						}
						hasNodeMeshData = true;
					}
				}

				if( hasNodeMeshData )
				{
					if( skinObject )
					{
						auto skinContext = skinObject->GetContextInterface( node );
						if( skinContext )
						{
							auto numBones = skinObject->GetNumBones();

							auto numVertex = meshData.PointList.Num();
							for( int iBone = 0; iBone < numBones; ++iBone )
							{
								auto bone = skinObject->GetBone( iBone );
								meshData.SkinNameList.Add( bone->GetName() );
							}

							for( int iVertex = 0; iVertex < numVertex; ++iVertex )
							{
								int assignedBones = skinContext->GetNumAssignedBones( iVertex );
								FLiveLinkPointSkin pointSkin;

								for( int iAssignedBone = 0; iAssignedBone < assignedBones; ++iAssignedBone )
								{
									auto weight = skinContext->GetBoneWeight( iVertex, iAssignedBone );
									if( weight < 0.1f )
									{
										continue;
									}

									FLiveLinkSkinLink link;
									link.SkinWeight	= weight;
									link.SkinIndex	= skinContext->GetAssignedBone( iVertex, iAssignedBone );
									pointSkin.SkinList.Add( link );
								}
								meshData.PointSkinList.Add( pointSkin );
							}
						}
					}

					if( morphObject )
					{
						if( morphObject->chanBank.size() > 0 )
						{
							for( const auto& channel : morphObject->chanBank )
							{

								auto name = channel.mName;

								auto numDeltas	= channel.mDeltas.size();

								if( 
									name &&
									numDeltas > 0 &&
									wcslen( name ) > 0
								)
								{

									FLiveLinkExtendMeshMorphData morphData;
									morphData.MorphName = name;
									for( int iPoint = 0; iPoint < numDeltas; ++iPoint )
									{
										FLiveLinkExtendMeshMorphVertexData vertex;

										const auto& point	= channel.mDeltas[ iPoint ];
										auto resultPos =
											matrix.TransformFVector4( FVector4(
												point.x, point.y, point.z
											) );
										resultPos.Y = -resultPos.Y;

										resultPos *= channel.mTargetPercent;

										vertex.DeltaPoint =
											FLiveLinkExtendPoint3(
												resultPos.X, resultPos.Y, resultPos.Z
											);

										morphData.VertexList.Add( vertex );
									}
									meshData.MorphList.Add( morphData );
								}
							}
						}
					}

					syncMeshData.MeshList.Add( meshData );
					hasMeshData = true;
				}
				
			}
		}

		if( hasMeshData )
		{
			TArray<uint8> buffer;
			FMemoryWriter writer( buffer );

			syncMeshData << writer;

			
			metaData.StringMetaData.Add( TEXT( "mesh_sync_data" ), FBase64::Encode( buffer ) );
			metaData.StringMetaData.Add( TEXT( "send_time" ), currentTime.ToIso8601() );
		}

		this->GetLiveLinkProvider()->UpdateSubjectFrame(
			this->subjectName_,
			transformList,
			curves,
			metaData,
			streamTime
		);
	}

};







class FMaxOutputDevice : public FOutputDevice
{
public:
	FMaxOutputDevice() : bAllowLogVerbosity( false ) {}

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category ) override
	{
		if( ( bAllowLogVerbosity && Verbosity <= ELogVerbosity::Log ) || ( Verbosity <= ELogVerbosity::Display ) )
		{
			_tprintf( V );
		}
	}

private:

	bool bAllowLogVerbosity;

};



void OnConnectionStatusChanged()
{
	UE_LOG( LogBlankMaxPlugin, Display, TEXT( "OnConnectionStatusChanged" ) );
	printf( "OnConnectionStatusChanged" );
}





FMaxLiveLink::FMaxLiveLink(
	HINSTANCE instanceHandle
)
	: instanceHandle_( instanceHandle )
	, controlPageHandle_( nullptr )
	, nodeEvent_( nullptr )
	, interface_( nullptr )
	, buttonAddSubject_( nullptr )
	, buttonRemoveSubject_( nullptr )
	, textSubjectName_( nullptr )
	, listSubject_( NULL )
	, useForceFrontX_Cache_( false )
	, useSendMeshAutomatically_Cache_( false )
{
}

FMaxLiveLink::~FMaxLiveLink()
{
}


INT_PTR CALLBACK CustCtrlDlgProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam 
) 
{
	FMaxLiveLink *u = DLGetWindowLongPtr<FMaxLiveLink *>( hWnd );
	if( !u && message != WM_INITDIALOG ) 
	{
		return FALSE;
	}

	switch( message )
	{
	case WM_INITDIALOG: 
		{
			u = ( FMaxLiveLink * )lParam;
			DLSetWindowLongPtr( hWnd, u );
			u->InitializeDialog( hWnd );
			return TRUE;
		}
	case WM_DESTROY: 
		{
			u->FinalizeDialog();
		}break;
	case WM_COMMAND: 
		{
			u->OnReceiveDialogCallbackCommand( wParam );
		}break;
	case WM_CUSTEDIT_ENTER:
		{
			u->OnReceiveEnterEditCallbackCommand( wParam );
		}break;
	default:
		return FALSE;
	}
	return TRUE;

}

void	FMaxLiveLink::BeginEditParams( Interface *ip, IUtil *iu )
{
	if( !this->nodeEvent_ )
	{
		Sleep( 0 );
		GEngineLoop.PreInit( TEXT( "MaxLiveLinkPlugin -Messaging" ) );
		ProcessNewlyLoadedUObjects();

		FModuleManager::Get().StartProcessingNewlyLoadedObjects();

		FModuleManager::Get().LoadModule( TEXT( "UdpMessaging" ) );

		this->nodeEvent_ = new NodeEventCallbackValue( this, ip );

		GLog->TearDown(); //clean up existing output devices
		GLog->AddOutputDevice( new FMaxOutputDevice() ); //Add Maya output device

		UE_LOG( LogBlankMaxPlugin, Display, TEXT( "MaxLiveLinkPlugin initialized" ) );

		this->liveLinkProvider_ = ILiveLinkProvider::CreateLiveLinkProvider( TEXT( "Max Live Link" ) );
		this->connectionStatusChangedHandle_ =
			this->liveLinkProvider_->RegisterConnStatusChangedHandle(
				FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic( &OnConnectionStatusChanged )
			);
		this->interface_		= ip;

		this->editViewCameraSubject_ = 
			new FMaxLiveLinkCameraStreamedSubject(
				this,
				TEXT( "EditorActiveCamera" )
			);
		
	}

	this->controlPageHandle_ = 
		ip->AddRollupPage(
			this->instanceHandle_,
			MAKEINTRESOURCE( IDD_LIVELINK_PAGE ),
			CustCtrlDlgProc,
			_T( "LiveLink Controls" ),
			( LPARAM )this 
		);


	
}

void	FMaxLiveLink::EndEditParams( Interface *ip, IUtil *iu )
{
	ip->DeleteRollupPage( this->controlPageHandle_ );
	this->controlPageHandle_ = nullptr;
}

void	FMaxLiveLink::Init( HWND hPanel )
{
}

void	FMaxLiveLink::Destroy( HWND hWnd )
{
}




void	FMaxLiveLink::CollectSubjects_Recursive(
	NodeArray& outArray, INode* currentNode,
	uint32 collectOption
)
{
	auto nodeName = currentNode->GetName();

	auto cID	= currentNode->ClassID();
	auto scID	= currentNode->SuperClassID();

	auto derivedObject	= reinterpret_cast<IDerivedObject*>( currentNode->GetObjectRef() );
	if( 
		derivedObject &&
		( !( collectOption & CollectOptionBits_OnlySelected ) ) ||
		( ( collectOption & CollectOptionBits_OnlySelected ) && currentNode->Selected() )
	)
	{
		bool isTargetType = false;
		auto superClassID	= derivedObject->SuperClassID();
		auto dcID			= derivedObject->ClassID();

		if( CollectOptionBits_SkinedMesh & collectOption )
		{
			int numModifier = derivedObject->NumModifiers();
			for( int iModifier = 0; iModifier < numModifier; ++iModifier )
			{
				auto modifier = derivedObject->GetModifier( iModifier );
				if( modifier )
				{

					auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
					if( skin )
					{
						isTargetType = true;
					}
				}
			}
		}
		
		if( CollectOptionBits_MorphMesh & collectOption )
		{
			int numModifier = derivedObject->NumModifiers();
			for( int iModifier = 0; iModifier < numModifier; ++iModifier )
			{
				auto modifier = derivedObject->GetModifier( iModifier );
				if( modifier )
				{
					if(
						modifier->SuperClassID() == OSM_CLASS_ID &&
						modifier->ClassID() == MORPHER_CLASS_ID
					)
					{
					
						isTargetType = true;
					}
				}
			}
		}
		
		if( CollectOptionBits_Mesh & collectOption ) 
		{
			if(
				   superClassID == GEOMOBJECT_CLASS_ID
				|| superClassID == TRIOBJ_CLASS_ID
				|| superClassID == POLYOBJ_CLASS_ID
			)
			{
				isTargetType = true;
			}
		}
		if( CollectOptionBits_Bone & collectOption )
		{
			if( superClassID == BONE_CLASS_ID )
			{
				isTargetType = true;
			}
		}
		if( CollectOptionBits_Camera & collectOption )
		{
			if( superClassID == CAMERA_CLASS_ID )
			{
				isTargetType = true;
			}
		}

		if( CollectOptionBits_Helper & collectOption )
		{
			if( superClassID == HELPER_CLASS_ID )
			{
				isTargetType = true;
			}
		}

		if( CollectOptionBits_Dummy & collectOption )
		{
			if( superClassID == DUMMY_CLASS_ID )
			{
				isTargetType = true;
			}
		}
		

		if( isTargetType )
		{
			outArray.AddUnique( currentNode );
		}
	}


	int numChildren = currentNode->NumberOfChildren();
	for( int iChild = 0; iChild < numChildren; ++iChild )
	{
		INode* child = currentNode->GetChildNode( iChild );
		if( child )
		{
			this->CollectSubjects_Recursive(
				outArray, child,
				collectOption
			);
		}
	}
}

NodeArray	FMaxLiveLink::CollectSubjects(
	uint32 collectOption
)
{
	NodeArray ret;
	this->CollectSubjects(
		ret, 
		collectOption
	);

	return ret;
}

void	FMaxLiveLink::CollectSubjects(
	NodeArray& outArray,
	uint32 collectOption
)
{
	outArray.Empty();

	Interface14* interFace = GetCOREInterface14();
	if( !interFace )
	{
		return;
	}

	INode* rootNode = interFace->GetRootNode();

	this->CollectSubjects_Recursive(
		outArray, rootNode,
		collectOption
	);
}

void	FMaxLiveLink::InitializeDialog( HWND hWnd )
{
	this->windowHandle_		= hWnd;

	this->buttonAddSubject_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_ADD_SUBJECT ) );
	if( this->buttonAddSubject_ )
	{
		this->buttonAddSubject_->SetType( CBT_PUSH );
		this->buttonAddSubject_->SetRightClickNotify( TRUE );
		this->buttonAddSubject_->SetButtonDownNotify( TRUE );
		this->buttonAddSubject_->SetText( TEXT( "Add" ) );
	}

	this->buttonRemoveSubject_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_REMOVE_SUBJECT ) );
	if( this->buttonRemoveSubject_ )
	{
		this->buttonRemoveSubject_->SetType( CBT_PUSH );
		this->buttonRemoveSubject_->SetRightClickNotify( TRUE );
		this->buttonRemoveSubject_->SetButtonDownNotify( TRUE );
		this->buttonRemoveSubject_->SetText( TEXT( "Remove" ) );
	}

	this->textSubjectName_	= GetICustEdit( GetDlgItem( this->windowHandle_, IDC_SUBJECTNAME ) );
	this->listSubject_		= GetDlgItem( this->windowHandle_, IDC_SUBJECT_LIST );
	//this->listSubject_		= GetDlgItem( hWnd, IDC_SUBJECT_LIST );

	this->buttonAddCameraSubject_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_SUBJECT_CAMERA ) );
	if( this->buttonAddCameraSubject_ )
	{
		this->buttonAddCameraSubject_->SetType( CBT_PUSH );
		this->buttonAddCameraSubject_->SetRightClickNotify( TRUE );
		this->buttonAddCameraSubject_->SetButtonDownNotify( TRUE );
		this->buttonAddCameraSubject_->SetText( TEXT( "Add Camera Subject" ) );
	}
	
	this->buttonSendMesh_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_SendMeshForce ) );
	if( this->buttonSendMesh_ )
	{
		this->buttonSendMesh_->SetType( CBT_PUSH );
		this->buttonSendMesh_->SetRightClickNotify( TRUE );
		this->buttonSendMesh_->SetButtonDownNotify( TRUE );
		this->buttonSendMesh_->SetText( TEXT( "Send Mesh" ) );
	}

	CheckDlgButton( 
		this->windowHandle_, IDC_CHECK_ForceFrontX, 
		this->useForceFrontX_Cache_ ? BST_CHECKED: BST_UNCHECKED 
	);

	CheckDlgButton(
		this->windowHandle_, IDC_CHECK_SendMeshAutomatically,
		this->useSendMeshAutomatically_Cache_ ? BST_CHECKED: BST_UNCHECKED
	);

	{
		SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
		for( const auto& skinPair : this->skinNodeSubjectList_ )
		{
			auto skinName = *skinPair.Key;
			SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
		}
	}
}

void	FMaxLiveLink::FinalizeDialog()
{
	this->useForceFrontX_Cache_				= this->UseForceFrontAxisX();
	this->useSendMeshAutomatically_Cache_	= this->UseAutomaticMeshUpdate();


	if( this->buttonAddCameraSubject_ )
	{
		ReleaseICustButton( this->buttonAddCameraSubject_ );
	}
	this->buttonAddCameraSubject_ = nullptr;
	this->listSubject_ = NULL;

	if( this->textSubjectName_ )
	{
		ReleaseICustEdit( this->textSubjectName_ );
	}
	this->textSubjectName_ = nullptr;

	if( this->buttonRemoveSubject_ )
	{
		ReleaseICustButton( this->buttonRemoveSubject_ );
	}
	this->buttonRemoveSubject_ = nullptr;

	if( this->buttonAddSubject_ )
	{
		ReleaseICustButton( this->buttonAddSubject_ );
	}
	this->buttonAddSubject_ = nullptr;
	this->windowHandle_ = nullptr;
}

bool	FMaxLiveLink::UseForceFrontAxisX() const
{
	if( !this->windowHandle_ )
	{
		return false;
	}
	return IsDlgButtonChecked( this->windowHandle_, IDC_CHECK_ForceFrontX ) == TRUE;
}

bool	FMaxLiveLink::UseAutomaticMeshUpdate() const
{
	if( !this->windowHandle_ )
	{
		return false;
	}
	return IsDlgButtonChecked( this->windowHandle_, IDC_CHECK_SendMeshAutomatically ) == TRUE;
}




void	FMaxLiveLink::OnReceiveDialogCallbackCommand( WPARAM wParam )
{
	switch( LOWORD( wParam ) )
	{
	case IDC_BUTTON_ADD_SUBJECT:
		{
			if( HIWORD( wParam ) == BN_BUTTONUP )
			{
				this->AddEditedSubject();
			}
		}break;

	case IDC_BUTTON_REMOVE_SUBJECT:
		{
			if( HIWORD( wParam ) == BN_BUTTONUP )
			{
				this->RemoveSelectedSubject();
			}
		}break;

	case IDC_BUTTON_SUBJECT_CAMERA:
		{
			if( HIWORD( wParam ) == BN_BUTTONUP )
			{
				for(
					int iCamera = 0;
					iCamera < this->cameraSubjectList_.Num();
					++iCamera
				)
				{
					auto camera = this->cameraSubjectList_[ iCamera ];
					delete camera;
				}
				this->cameraSubjectList_.Empty();

				auto cameraNodeList =
					this->CollectSubjects(
						CollectOptionBits_Camera
					);
				for( INode* cameraNode : cameraNodeList )
				{
					auto stream =
						new FMaxLiveLinkCameraStreamedSubject(
							this,
							cameraNode, cameraNode->GetName()
						);
					this->cameraSubjectList_.Add( stream );
				}
				this->lastUpdateTime_ = TIME_PosInfinity;
			}
		}break;

	case IDC_BUTTON_SendMeshForce:
		{
			if( HIWORD( wParam ) == BN_BUTTONUP )
			{
				this->ApplyCurrentFrameSubjects( true );
			}
		}break;

	default:
		break;
	}
}


void	FMaxLiveLink::OnReceiveEnterEditCallbackCommand( WPARAM wParam )
{
	switch( wParam )
	{
	case IDC_SUBJECTNAME:
		{
			this->AddEditedSubject();
		}break;
	default:
		break;
	}
}


void	FMaxLiveLink::AddEditedSubject()
{
	TCHAR subjectName[ 256 ];
	int sizeOfText = this->textSubjectName_->GetTextLength();
	if( sizeOfText > 0 )
	{
		this->textSubjectName_->GetText( subjectName, 256 );

		auto oldSubject = this->skinNodeSubjectList_.Find( subjectName );
		if( oldSubject )
		{
			this->skinNodeSubjectList_.Remove( subjectName );
			delete ( *oldSubject );
		}

		NodeArray skinNodeList;
		{
			NodeArray selectedNodeList = 
				this->CollectSubjects(
					CollectOptionBits_OnlySelected |
					CollectOptionBits_SkinedMesh |
					CollectOptionBits_MorphMesh |
					CollectOptionBits_Mesh |
					CollectOptionBits_Helper |
					CollectOptionBits_Dummy
				);
			if( selectedNodeList.Num() == 0 )
			{
				skinNodeList =
					this->CollectSubjects(
						CollectOptionBits_SkinedMesh |
						CollectOptionBits_MorphMesh |
						CollectOptionBits_Mesh |
						CollectOptionBits_Helper |
						CollectOptionBits_Dummy
					);
			}
			else
			{
				skinNodeList = selectedNodeList;
			}
		}

		
		{
			auto stream =
				new FMaxLiveLinkJointStreamedSubject(
					this,
					skinNodeList, subjectName
				);
			this->skinNodeSubjectList_.Add( subjectName, stream );
		}

		{
			SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
			for( const auto& skinPair : this->skinNodeSubjectList_ )
			{
				auto skinName = *skinPair.Key;
				SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
			}
		}
		this->lastUpdateTime_ = TIME_PosInfinity;
	}
}

void	FMaxLiveLink::RemoveSelectedSubject()
{
	int selectedIndexList[ 256 ];
	auto numSelected = SendMessageW( this->listSubject_, LB_GETSELCOUNT, 256, ( LPARAM )selectedIndexList );
	SendMessageW( this->listSubject_, LB_GETSELITEMS, 256, ( LPARAM )selectedIndexList );


	for( int iSelect = 0; iSelect < numSelected; ++iSelect )
	{
		TCHAR subjectName[ 256 ];

		auto textLen	= SendMessageW( this->listSubject_, LB_GETTEXTLEN, selectedIndexList[ iSelect ], 0 );
		auto getResult	= SendMessageW( this->listSubject_, LB_GETTEXT, selectedIndexList[ iSelect ], ( LPARAM )subjectName );

		auto oldSubject = this->skinNodeSubjectList_.Find( subjectName );
		if( oldSubject )
		{
			this->skinNodeSubjectList_.Remove( subjectName );
			delete ( *oldSubject );
		}
	}

	{
		SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
		for( const auto& skinPair : this->skinNodeSubjectList_ )
		{
			auto skinName = *skinPair.Key;
			SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
		}
	}
}
void	FMaxLiveLink::DeleteAllSkinNodeSubject()
{
	SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );

	for( const auto& subjectpair : this->skinNodeSubjectList_ )
	{
		auto subjectObject = subjectpair.Value;
		delete ( subjectObject );
	}
	this->skinNodeSubjectList_.Empty();
}

void	FMaxLiveLink::ReceiveChangeGeometoryNodes( const TArray<INode*>& nodes )
{
	for( const auto& subjectpair : this->skinNodeSubjectList_ )
	{
		subjectpair.Value->ReceiveUpdateGeometryOfNodes( nodes );
	}
}



TimeValue	FMaxLiveLink::GetBindPoseTime() const
{
	auto range = this->interface_->GetAnimRange();
	return range.Start();
}


void	FMaxLiveLink::ModelOtherEvent()
{
	return;
	double	frameSeconds	= 0.0;
	int		frame			= 0;
	if( this->interface_ )
	{
		frameSeconds	= TicksToSec( this->interface_->GetTime() );
		frame			= this->interface_->GetTime() / GetFrameRate() / GetTicksPerFrame();
	}

	{
		for( auto stream : this->skinNodeSubjectList_ )
		{
			stream.Value->OnStream( frameSeconds, frame );
		}
	}
	{
		for( auto stream : this->cameraSubjectList_ )
		{
			//stream->OnStream( frameSeconds, frame );
		}
	}
}

void	FMaxLiveLink::OnDisplay( TimeValue t, ViewExp* vpt, int flags )
{
	if( this->lastUpdateTime_ == t )
	{
		return;
	}
	this->lastUpdateTime_ = t;

	double	frameSeconds	= TicksToSec( t );
	int		frame			= t / GetTicksPerFrame();

	{

		if( !this->UseAutomaticMeshUpdate() )
		{
			for( auto stream : this->skinNodeSubjectList_ )
			{
				stream.Value->SettingUpdateGeometry( false );
			}
		}

		for( auto stream : this->skinNodeSubjectList_ )
		{
			stream.Value->OnStream( frameSeconds, frame );
		}
	}
	auto vp13 = reinterpret_cast<ViewExp13*>( vpt->Execute( ViewExp::kEXECUTE_GET_VIEWEXP_13 ) );
	if( vp13 )
	{
		{
			for( auto stream : this->cameraSubjectList_ )
			{
				stream->OnStreamCamera( frameSeconds, frame, vp13, flags );
			}
		}

		if( this->editViewCameraSubject_ )
		{
			this->editViewCameraSubject_->OnStreamCamera( frameSeconds, frame, vp13, flags );
		}
	}
}


void	FMaxLiveLink::ApplyCurrentFrameSubjects( bool forceUpdateMesh )
{
	this->lastUpdateTime_ = this->interface_->GetTime();


	double	frameSeconds	= TicksToSec( this->lastUpdateTime_ );
	int		frame			= this->lastUpdateTime_ / GetTicksPerFrame();

	{
		for( auto stream : this->skinNodeSubjectList_ )
		{
			if( forceUpdateMesh )
			{
				stream.Value->SettingUpdateGeometry( true );
			}
			stream.Value->OnStream( frameSeconds, frame );
		}
	}
}



HINSTANCE g_hInst = NULL;

#define LIVELINK_CLASS_ID	Class_ID(0x36d125ab, 0x6d2c806e)


FMaxLiveLink* liveLinkInstance_ = nullptr;

class LiveLinkClassDesc :
	public ClassDesc 
{
public:
	virtual int 			IsPublic() { return TRUE; }
	virtual void*			Create( BOOL loading = FALSE )
	{
		return liveLinkInstance_;
	}
	const TCHAR *	ClassName() { return L"MaxLiveLink"; }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return LIVELINK_CLASS_ID; }
	const TCHAR* 	Category() { return L"MaxLiveLink"; }
	void			ResetClassParams ( BOOL fileReset );
};


static LiveLinkClassDesc LiveLinkDesc;
ClassDesc* GetLiveLinkDesc()
{
	return &LiveLinkDesc;
}

void LiveLinkClassDesc::ResetClassParams ( BOOL fileReset )
{
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	static BOOL controlsInit = FALSE;
	switch( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		{
			MaxSDK::Util::UseLanguagePackLocale();
			g_hInst = hModule;

			if( !liveLinkInstance_ )
			{
				liveLinkInstance_ = new FMaxLiveLink( g_hInst );
			}

			DisableThreadLibraryCalls( hModule );
		}break;

	case DLL_THREAD_ATTACH:
		{

		}break;
	case DLL_THREAD_DETACH:
		{
		}break;
	}
	return( TRUE );
}



#ifdef __cplusplus  
extern "C" {
#endif 

	__declspec( dllexport ) int LibNumberClasses()
	{
		return 1;
	}


	__declspec( dllexport ) ClassDesc* LibClassDesc( int i )
	{
		switch( i ) {
		case 0: return GetLiveLinkDesc();
		default: return 0;
		}
	}


	__declspec( dllexport ) const TCHAR* LibDescription()
	{
		static const TCHAR* libDescription = L"Max Live Link";
		return libDescription;
	}


	__declspec( dllexport ) ULONG LibVersion()
	{
		return VERSION_3DSMAX;
	}

#ifdef __cplusplus  
}


#endif 
