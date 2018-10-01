
/*! @file		CameraStreamedSubject.h
	@brief		カメラ同期用のSubjectを管理する構成
*/


#include "CameraStreamedSubject.h"

#include "../MaxLiveLink.h"


FMaxLiveLinkCameraStreamedSubject::FMaxLiveLinkCameraStreamedSubject(
	FMaxLiveLink* livelinkReference,
	INode* node, const FName& subjectName
)
	: FMaxLiveLinkBaseStreamedSubject(
		livelinkReference,
		node, subjectName
	)
{
	this->BuildSubjectData();
}

FMaxLiveLinkCameraStreamedSubject::FMaxLiveLinkCameraStreamedSubject(
	FMaxLiveLink* livelinkReference,
	const FName& subjectName
)
	: FMaxLiveLinkBaseStreamedSubject(
		livelinkReference,
		subjectName
	)
{
	this->BuildSubjectData();
}

FMaxLiveLinkCameraStreamedSubject::~FMaxLiveLinkCameraStreamedSubject()
{
	this->GetLiveLinkProvider()->ClearSubject( this->subjectName_ );
}


const Matrix3	FMaxLiveLinkCameraStreamedSubject::ToUE4Matrix(
	Point3(  0.0f, -1.0f,  0.0f ),
	Point3(  1.0f,  0.0f,  0.0f ),
	Point3(  0.0f,  0.0f,  1.0f ),
	Point3(  0.0f,  0.0f,  0.0f )
);

const Matrix3	FMaxLiveLinkCameraStreamedSubject::ToFrontAxisXMatrix(
	Point3(  0.0f, -1.0f,  0.0f ),
	Point3(  1.0f,  0.0f,  0.0f ),
	Point3(  0.0f,  0.0f,  1.0f ),
	Point3(  0.0f,  0.0f,  0.0f )
);


/*! @brief		Subjectとしてのデータを構築
*/
void FMaxLiveLinkCameraStreamedSubject::BuildSubjectData()
{
	TArray<FName>	jointNames;
	TArray<int32>	jointParents;
	jointNames.Add( TEXT( "root" ) );
	jointParents.Add( -1 );

	{
		Matrix3 viewAffine;
		TArray<FTransform> transformList =
		{
			ConvertMaxMatrixToTransform( viewAffine )
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


/*! @brief		UE4へのデータ送信
*/
void FMaxLiveLinkCameraStreamedSubject::OnStream( double streamTime, int32 frameNumber )
{
	if( this->nodeList_.Num() == 0 )
	{
		return;
	}

	TArray<FTransform> transformList =
	{
		this->GetSubjectTransformFromSeconds( this->nodeList_[ 0 ].ReferencedNode, streamTime, false, true )
	};
	TArray<FLiveLinkCurveElement> curves;

	this->GetLiveLinkProvider()->UpdateSubjectFrame(
		this->subjectName_,
		transformList,
		curves,
		streamTime
	);
}

/*! @brief		カメラとしてのUE4へのデータ送信
*/
void FMaxLiveLinkCameraStreamedSubject::OnStreamCamera( double streamTime, int frame, ViewExp13* vpt, int flags )
{
	auto fov		= vpt->GetFOV();
	auto degFov		= FMath::RadiansToDegrees( fov );

	auto scaleValue			= FVector::OneVector;
	auto rotateValue		= FQuat::Identity;
	auto translationValue	= FVector::ZeroVector;

	auto frontZMatrix		= FMaxLiveLink::GetInvertFrontZMatrix();
	frontZMatrix.Invert();

	Matrix3 viewAffine;
	vpt->GetAffineTM( viewAffine );
	AffineParts affine;


	auto trans = viewAffine.GetRow( 3 );
	auto scale = 1.0f;
	{
		viewAffine = viewAffine * frontZMatrix;

		if( this->livelinkReference_->UseForceFrontAxisX() )
		{
			viewAffine = ToFrontAxisXMatrix * viewAffine;
		}
		{
			viewAffine.SetRow( 3, Point3( 0.0f, 0.0f, 0.0f ) );
			auto invRotate = viewAffine;
			invRotate.Invert();
			auto parsedTrans = Point3( -trans.x, trans.z, -trans.y );
			parsedTrans = parsedTrans * invRotate;

			auto unitScale = this->livelinkReference_->GetUnitScale();
			{
				parsedTrans *= unitScale;
			}

			auto graphicWindos = vpt->getGW();
			if( graphicWindos )
			{
				float mat[ 4 ][ 4 ];
				Matrix3 invTM;
				int persp;
				float hither;
				float yon;

				graphicWindos->getCameraMatrix(
					mat, &invTM,
					&persp, &hither, &yon
				);

				if( !persp )
				{
					// LiveLinkではパース描画でなければカメラの同期が有効ではないため、
					// 正投影の場合は仮の状態にてシミュレートさせる

					parsedTrans = Point3( -trans.x, 0.0f, -trans.y );
					parsedTrans = parsedTrans * invRotate;

					auto unitScale = this->livelinkReference_->GetUnitScale();
					{
						parsedTrans *= unitScale;
					}

					Point3 offset = Point3( 0.0f, -100.0f * degFov * unitScale, 0.0f );
					scale = degFov;

					offset = offset * invRotate;
					parsedTrans += offset;
					degFov = 4;
				}
			}

			trans = parsedTrans;
		}
		viewAffine = viewAffine * ToUE4Matrix;
	}
	decomp_affine( viewAffine, &affine );

	
	
	{
		{
			rotateValue.X		= affine.q.x;
			rotateValue.Y		=-affine.q.y;
			rotateValue.Z		= affine.q.z;
			rotateValue.W		=-affine.q.w;
		}
		{
			scaleValue.X	= affine.k.x * scale;
			scaleValue.Y	= affine.k.y * scale;
			scaleValue.Z	= affine.k.z * scale;
		}
		{

			translationValue.X = trans.x;
			translationValue.Y =-trans.y;
			translationValue.Z = trans.z;
		}
	}

	TArray<FTransform> transformList =
	{
		FTransform(
			rotateValue,
			translationValue,
			scaleValue
		)
	};

	TArray<FLiveLinkCurveElement> curves;

	FLiveLinkMetaData metaData;
	metaData.SceneTime.Frames		= frame;
	metaData.SceneTime.FrameRate	= FLiveLinkFrameRate( ( uint32 )( 1.0f / vpt->GetFPS() ), 1 );
	metaData.SceneTime.Seconds		= ( int32 )( streamTime );

	metaData.StringMetaData.Add( FName( TEXT( "fov" ) ), FString::SanitizeFloat( degFov ) );

	this->GetLiveLinkProvider()->UpdateSubjectFrame(
		this->subjectName_,
		transformList,
		curves,
		metaData,
		streamTime
	);
}