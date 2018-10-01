
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


	Matrix3 parentMtx		= MaxIdentityMatrix;
	Matrix3 transformMatrix	= MaxIdentityMatrix;

	Interface14* interFace = GetCOREInterface14();

	bool isRootNode = false;

	if( node )
	{
		transformMatrix = node->GetObjTMAfterWSM( timeValue, nullptr );

		auto name = node->GetName();
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
			parentMtx = parentNode->GetObjTMAfterWSM( timeValue );
		}
		else
		{
			isRootNode = true;
		}
	}

	if( frontAxisX )
	{
		transformMatrix = ( transformMatrix * invFrontZMatrix ) * frontXMatrix;
		transformMatrix = frontXRootMatrix * transformMatrix;
	}

	if( !isRootNode )
	{
		// ルートに配置されたノードでなければ、親行列の影響を外す
		if( frontAxisX )
		{
			parentMtx = frontXRootMatrix * ( ( parentMtx * invFrontZMatrix ) * frontXMatrix );
		}

		parentMtx.Invert();
		transformMatrix = transformMatrix * parentMtx;
	}
	FQuat	rotateValue			= FQuat::Identity;
	FVector scaleValue			= FVector::OneVector;
	FVector translationValue	= FVector::ZeroVector;

	Interval interval( timeValue, timeValue );

	auto unitScale = GetMasterScale( GetUSDefaultUnit() );
	
	
	AffineParts affine;
	decomp_affine( transformMatrix, &affine );

	if( node )
	{
		auto tmController = node->GetTMController();
		if( tmController && 0 )
		{
			// アニメーションコントローラ（タイムライン）を使用する場合の例

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

/*------- ↑ トランスフォームの変換 ↑ ------- }}} */