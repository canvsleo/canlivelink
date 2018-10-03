
/*! @file		BaseStreamedSubject.cpp
	@brief		送信するSubjectを管理する基礎構成
*/

#include "BaseStreamedSubject.h"

#include "../MaxLiveLink.h"



FMaxLiveLinkBaseStreamedSubject::FMaxLiveLinkBaseStreamedSubject(
	FMaxLiveLink* livelinkReference,
	INode* node, const FName& subjectName
)
	: livelinkReference_( livelinkReference )
	, subjectName_( subjectName )
{
	this->nodeList_.Add( node );
}

FMaxLiveLinkBaseStreamedSubject::FMaxLiveLinkBaseStreamedSubject(
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

FMaxLiveLinkBaseStreamedSubject::FMaxLiveLinkBaseStreamedSubject(
	FMaxLiveLink* livelinkReference,
	const FName& subjectName
)
	: livelinkReference_( livelinkReference )
	, subjectName_( subjectName )
{}

FMaxLiveLinkBaseStreamedSubject::~FMaxLiveLinkBaseStreamedSubject()
{
}

ILiveLinkProvider* FMaxLiveLinkBaseStreamedSubject::GetLiveLinkProvider() const
{
	return this->livelinkReference_->GetLiveLinkProvider();
}


/*! @brief		ノードリストの内容が存在しているかを確認して参照状態を更新
*/
void	FMaxLiveLinkBaseStreamedSubject::UpdateNodeListReference()
{
	auto allNodes = this->livelinkReference_->CollectSubjectNodes( FMaxLiveLink::CollectOptionBits_AllNode );
	for( auto& nodeReference : this->nodeList_ )
	{
		if( !nodeReference.ReferencedNode )
		{
			continue;
		}

		bool hasNode = false;
		for( auto node : allNodes )
		{
			if( nodeReference.ReferencedNode == node )
			{
				hasNode = true;
				break;
			}
		}

		if( !hasNode )
		{
			nodeReference.ReferencedNode = nullptr;
		}
	}

}

/*! @brief		ノード参照が1つでも存在するか
*/
bool	FMaxLiveLinkBaseStreamedSubject::HasAnyNodeReferences() const
{
	for( const auto& nodeReference : this->nodeList_ )
	{
		if( nodeReference.ReferencedNode )
		{
			return true;
		}
	}
	return false;
}

/*! @brief		ノードが保持されているか
*/
bool	FMaxLiveLinkBaseStreamedSubject::IsContainNodes( const TArray<INode*>& nodes ) const
{
	for( auto& nodeReference : this->nodeList_ )
	{
		if( !nodeReference.ReferencedNode )
		{
			continue;
		}

		bool hasNode = false;
		for( auto node : nodes )
		{
			if( nodeReference.ReferencedNode == node )
			{
				return true;
			}
		}
	}
	return false;
}



/*------- ↓ トランスフォームの変換 ↓ ------- {{{ */



/*! @brief		3DSMaxのMatrixをトランスフォームに変換
*/
FTransform	FMaxLiveLinkBaseStreamedSubject::ConvertMaxMatrixToTransform( const Matrix3& mtx )
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


const Matrix3 FMaxLiveLinkBaseStreamedSubject::MaxIdentityMatrix(
	Point3( 1, 0, 0 ),
	Point3( 0, 1, 0 ),
	Point3( 0, 0, 1 ),
	Point3( 0, 0, 0 )
);


/*! @brief		ノードからトランスフォームを取得
*/
FTransform	FMaxLiveLinkBaseStreamedSubject::GetSubjectTransform(
	INode* node,
	TimeValue timeValue,
	bool frontAxisX, bool forRootNode
)
{
	const auto& frontXMatrix		= FMaxLiveLink::GetFrontXMatrix();
	const auto& invFrontZMatrix		= FMaxLiveLink::GetInvertFrontZMatrix();
	const auto& frontXRootMatrix	= FMaxLiveLink::GetFrontXRootMatrix();

	Interface14* interFace = GetCOREInterface14();

	bool isRootNode = false;

	if( node )
	{
		auto parentNode = node->GetParentNode();
		if( parentNode )
		{
			// UE4上での親となるノードを検出する
			// 単純な RootNode ではない
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
	}
	FQuat	rotateValue			= FQuat::Identity;
	FVector scaleValue			= FVector::OneVector;
	FVector translationValue	= FVector::ZeroVector;

	Interval interval( timeValue, timeValue );
	
	AffineParts affine;
	affine.t = Point3( 0, 0, 0 );
	affine.k = Point3( 1, 1, 1 );
	affine.q = Quat( 0.0f, 0.0f, 0.0f, 1.0f );
	affine.u = Quat( 0.0f, 0.0f, 0.0f, 1.0f );
	affine.f = 1.0f;

	if( node )
	{
		auto tmController = node->GetTMController();
		if( tmController )
		{
			// アニメーションコントローラ（タイムライン）を使用する場合の例
			{
				auto scaleController = tmController->GetScaleController();
				if( scaleController )
				{
					auto controllerX = scaleController->GetXController();
					if( controllerX )
					{
						controllerX->GetValue(
							timeValue, &affine.k.x,
							interval
						);
					}

					auto controllerY = scaleController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &affine.k.y,
							interval
						);
					}

					auto controllerZ = scaleController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &affine.k.z,
							interval
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
							timeValue, &affine.t.x,
							interval
						);
					}

					auto controllerY = positionController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &affine.t.y,
							interval
						);
					}

					auto controllerZ = positionController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &affine.t.z,
							interval
						);
					}
				}
			}
			{
				auto rotationController = tmController->GetRotationController();
				if( rotationController )
				{
					float eularXYZ[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
					auto controllerX = rotationController->GetXController();
					if( controllerX )
					{
						controllerX->GetValue(
							timeValue, &eularXYZ[ 0 ],
							interval
						);
					}

					auto controllerY = rotationController->GetYController();
					if( controllerY )
					{
						controllerY->GetValue(
							timeValue, &eularXYZ[ 1 ],
							interval
						);
					}

					auto controllerZ = rotationController->GetZController();
					if( controllerZ )
					{
						controllerZ->GetValue(
							timeValue, &eularXYZ[ 2 ],
							interval
						);
					}

					int rotateEulerType = EULERTYPE_XYZ;
					{
						auto eulerControl = static_cast<IEulerControl*>( rotationController->GetInterface( I_EULERCTRL ) );
						if( eulerControl )
						{
							rotateEulerType = eulerControl->GetOrder();
						}
					}
					switch( rotateEulerType )
					{
					default:
					case EULERTYPE_XYZ:
						{
							Quat q;
							q.SetEuler( eularXYZ[ 0 ], eularXYZ[ 1 ], eularXYZ[ 2 ] );
							affine.q = q;
						}break;
					case EULERTYPE_ZXY:
						{
							Quat q;
							q.SetEuler( eularXYZ[ 1 ], eularXYZ[ 0 ], eularXYZ[ 2 ] );
							affine.q.x = q.y;
							affine.q.y = q.x;
							affine.q.z = q.z;
							affine.q.w = q.w;
						}break;
					}
				}
			}

			if(
				isRootNode ||
				frontAxisX
			)
			{
				Matrix3 parentMtx		= MaxIdentityMatrix;

				Matrix3 srtm, rtm, ptm, stm, ftm;
				ptm.IdentityMatrix();
				ptm.SetTrans( affine.t );
				affine.q.MakeMatrix( rtm );
				affine.u.MakeMatrix( srtm );
				stm = ScaleMatrix( affine.k );
				ftm = ScaleMatrix( Point3( affine.f, affine.f, affine.f ) );
				Matrix3 resultLocalMtx = Inverse( srtm ) * stm * srtm * rtm * ftm * ptm;

				if( node->GetParentNode() )
				{
					parentMtx = node->GetParentNode()->GetObjTMAfterWSM( timeValue );
				}
				if( isRootNode )
				{
					if( frontAxisX )
					{
						parentMtx = frontXRootMatrix * ( ( parentMtx * invFrontZMatrix ) * frontXMatrix );
					}
					resultLocalMtx *= parentMtx;
				}
				else
				{
					if( frontAxisX )
					{
						resultLocalMtx	= resultLocalMtx * parentMtx;


						resultLocalMtx = ( resultLocalMtx * invFrontZMatrix ) * frontXMatrix;
						resultLocalMtx = frontXRootMatrix * resultLocalMtx;

						parentMtx		= frontXRootMatrix * ( ( parentMtx * invFrontZMatrix ) * frontXMatrix );
						parentMtx.Invert();

						resultLocalMtx = resultLocalMtx * parentMtx;
					}
				}

				decomp_affine( resultLocalMtx, &affine );
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
		}
	}
	
	return FTransform(
		rotateValue,
		translationValue,
		scaleValue
	);
}

/*------- ↑ トランスフォームの変換 ↑ ------- }}} */