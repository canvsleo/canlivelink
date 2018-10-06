
/*! @file		LiveLinkExtendVertexMeshData.h
	@brief		LiveLinkExtendPreviewPlugin にて使用する\n
				メッシュ同期用データ群
*/

#pragma once


#include "CoreMinimal.h"
#include "Serialization/JsonSerializerMacros.h"

#include "LiveLinkExtendVertexMeshData.generated.h"



template<typename ElementType>
FArchive& SerializeTArray( FArchive& Ar, TArray< ElementType >& arrayData )
{
	if( Ar.IsLoading() )
	{
		int32 SerializeNum = 0;
		Ar << SerializeNum;

		arrayData.Empty( SerializeNum );

		for( int32 i=0; i<SerializeNum; i++ )
		{
			*::new( arrayData ) ElementType << Ar;
		}
	}
	else
	{
		int32 SerializeNum = arrayData.Num();
		Ar << SerializeNum;

		for( int32 i=0; i< SerializeNum; i++ )
		{
			arrayData[ i ] << Ar;
		}
	}
	return Ar;
}



/*! @struct		FLiveLinkExtendPoint3
	@brief		汎用的な3要素データ
*/
USTRUCT()
struct FLiveLinkExtendPoint3
{
	GENERATED_USTRUCT_BODY()


	FLiveLinkExtendPoint3()
		: X( 0.0f )
		, Y( 0.0f )
		, Z( 0.0f )
	{}

	FLiveLinkExtendPoint3(
		float	x,
		float	y,
		float	z
	)
		: X( x )
		, Y( y )
		, Z( z )
	{}

	float	X;
	float	Y;
	float	Z;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendPoint3*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->X;
		Ar << this->Y;
		Ar << this->Z;

		return Ar;
	}

	bool operator==( const FLiveLinkExtendPoint3& rhs ) const
	{
		return (
			this->X == rhs.X &&
			this->Y == rhs.Y &&
			this->Z == rhs.Z
			);
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendPoint3& rhs ) const
	{
		return !( *this == rhs );
	}

};


/*! @struct		FLiveLinkExtendPoint2
	@brief		汎用的な2要素データ
*/

USTRUCT()
struct FLiveLinkExtendPoint2
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendPoint2()
		: X( 0.0f )
		, Y( 0.0f )
	{}

	FLiveLinkExtendPoint2(
		float	x,
		float	y
	)
		: X( x )
		, Y( y )
	{}


	float	X;
	float	Y;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendPoint2*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->X;
		Ar << this->Y;

		return Ar;
	}

	bool operator==( const FLiveLinkExtendPoint2& rhs ) const
	{
		return (
			this->X == rhs.X &&
			this->Y == rhs.Y
			);
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendPoint2& rhs ) const
	{
		return !( *this == rhs );
	}
};


/*! @struct		FLiveLinkExtendPoint4
	@brief		汎用的な4要素データ
*/
USTRUCT()
struct FLiveLinkExtendPoint4
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendPoint4()
		: X( 0.0f )
		, Y( 0.0f )
		, Z( 0.0f )
		, W( 0.0f )
	{}

	FLiveLinkExtendPoint4(
		float	x,
		float	y,
		float	z,
		float	w
	)
		: X( x )
		, Y( y )
		, Z( z )
		, W( w )
	{}


	float	X;
	float	Y;
	float	Z;
	float	W;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendPoint4*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->X;
		Ar << this->Y;
		Ar << this->Z;
		Ar << this->W;

		return Ar;
	}

	bool operator==( const FLiveLinkExtendPoint4& rhs ) const
	{
		return (
			this->X == rhs.X &&
			this->Y == rhs.Y &&
			this->Z == rhs.Z &&
			this->W == rhs.W
			);
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendPoint4& rhs ) const
	{
		return !( *this == rhs );
	}
};




/*! @struct		FLiveLinkSkinLink
	@brief		1頂点と、ボーンを紐づける重みデータ
*/
USTRUCT()
struct FLiveLinkSkinLink
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkSkinLink()
		: SkinIndex( -1 )
		, SkinWeight( 0.0f )
	{}

	FLiveLinkSkinLink(
		int		skinIndex	,
		float	skinWeight	
	)
		: SkinIndex( skinIndex )
		, SkinWeight( skinWeight )
	{}


	int		SkinIndex;
	float	SkinWeight;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkSkinLink*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->SkinIndex;
		Ar << this->SkinWeight;

		return Ar;
	}


	bool operator==( const FLiveLinkSkinLink& rhs ) const
	{
		if( this->SkinIndex != rhs.SkinIndex )
		{
			return false;
		}

		if( this->SkinWeight != rhs.SkinWeight )
		{
			return false;
		}

		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkSkinLink& rhs ) const
	{
		return !( *this == rhs );
	}
};


/*! @struct		FLiveLinkPointSkin
	@brief		メッシュが保持するボーン影響データ
*/
USTRUCT()
struct FLiveLinkPointSkin
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkPointSkin()
	{}

	TArray< FLiveLinkSkinLink >		SkinList;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkPointSkin*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		SerializeTArray( Ar, this->SkinList );

		return Ar;
	}


	bool operator==( const FLiveLinkPointSkin& rhs ) const
	{
		auto myNumSkin		= this->SkinList.Num();
		auto otherNumSkin		= rhs.SkinList.Num();
		if( myNumSkin != otherNumSkin )
		{
			return false;
		}

		{

			for( int iSkin = 0; iSkin < myNumSkin; ++iSkin )
			{
				if( this->SkinList[ iSkin ] != rhs.SkinList[ iSkin ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkPointSkin& rhs ) const
	{
		return !( *this == rhs );
	}
};


/*! @struct		FLiveLinkExtendPolygon
	@brief		１ポリゴンごとの構成データ
*/
USTRUCT()
struct FLiveLinkExtendPolygon
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendPolygon()
	{}

	uint32				SmoothingGroup;

	TArray<int>			VertexIndexList;
	TArray<FLiveLinkExtendPoint4>	ColorList0;

	TArray<FLiveLinkExtendPoint2>	UVList0;
	TArray<FLiveLinkExtendPoint2>	UVList1;
	TArray<FLiveLinkExtendPoint2>	UVList2;
	TArray<FLiveLinkExtendPoint2>	UVList3;

	TArray<FLiveLinkExtendPoint3>	NormalList;


	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendPolygon*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->SmoothingGroup;

		Ar << this->VertexIndexList;

		SerializeTArray( Ar, this->ColorList0 );

		SerializeTArray( Ar, this->UVList0 );
		SerializeTArray( Ar, this->UVList1 );
		SerializeTArray( Ar, this->UVList2 );
		SerializeTArray( Ar, this->UVList3 );

		SerializeTArray( Ar, this->NormalList );

		return Ar;
	}


	bool operator==( const FLiveLinkExtendPolygon& rhs ) const
	{
		if( this->SmoothingGroup != rhs.SmoothingGroup )
		{
			return false;
		}


		auto myNumVertexIndex		= this->VertexIndexList.Num();
		auto otherNumVertexIndex	= rhs.VertexIndexList.Num();
		if( myNumVertexIndex != otherNumVertexIndex )
		{
			return false;
		}

		auto myNumColor0		= this->ColorList0.Num();
		auto otherNumColor0		= rhs.ColorList0.Num();
		if( myNumColor0 != otherNumColor0 )
		{
			return false;
		}


		auto myNumUV0		= this->UVList0.Num();
		auto otherNumUV0	= rhs.UVList0.Num();
		if( myNumUV0 != otherNumUV0 )
		{
			return false;
		}

		auto myNumUV1		= this->UVList1.Num();
		auto otherNumUV1	= rhs.UVList1.Num();
		if( myNumUV1 != otherNumUV1 )
		{
			return false;
		}

		auto myNumUV2		= this->UVList2.Num();
		auto otherNumUV2	= rhs.UVList2.Num();
		if( myNumUV2 != otherNumUV2 )
		{
			return false;
		}

		auto myNumUV3		= this->UVList3.Num();
		auto otherNumUV3	= rhs.UVList3.Num();
		if( myNumUV3 != otherNumUV3 )
		{
			return false;
		}


		auto myNumNormal	= this->NormalList.Num();
		auto otherNumNormal	= rhs.NormalList.Num();
		if( myNumNormal != otherNumNormal )
		{
			return false;
		}



		{
			for( int iVertexIndex = 0; iVertexIndex < myNumVertexIndex; ++iVertexIndex )
			{
				if( this->VertexIndexList[ iVertexIndex ] != rhs.VertexIndexList[ iVertexIndex ] )
				{
					return false;
				}
			}
		}

		{
			for( int iColor0 = 0; iColor0 < myNumColor0; ++iColor0 )
			{
				if( this->ColorList0[ iColor0 ] != rhs.ColorList0[ iColor0 ] )
				{
					return false;
				}
			}
		}

		{
			for( int iUV0 = 0; iUV0 < myNumUV0; ++iUV0 )
			{
				if( this->UVList0[ iUV0 ] != rhs.UVList0[ iUV0 ] )
				{
					return false;
				}
			}
		}

		{
			for( int iUV1 = 0; iUV1 < myNumUV1; ++iUV1 )
			{
				if( this->UVList1[ iUV1 ] != rhs.UVList1[ iUV1 ] )
				{
					return false;
				}
			}
		}

		{
			for( int iUV2 = 0; iUV2 < myNumUV2; ++iUV2 )
			{
				if( this->UVList2[ iUV2 ] != rhs.UVList2[ iUV2 ] )
				{
					return false;
				}
			}
		}

		{
			for( int iUV3 = 0; iUV3 < myNumUV3; ++iUV3 )
			{
				if( this->UVList3[ iUV3 ] != rhs.UVList3[ iUV3 ] )
				{
					return false;
				}
			}
		}

		{
			for( int iNormal = 0; iNormal < myNumNormal; ++iNormal )
			{
				if( this->NormalList[ iNormal ] != rhs.NormalList[ iNormal ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendPolygon& rhs ) const
	{
		return !( *this == rhs );
	}

};



/*! @struct		FLiveLinkExtendMaterialGroupMeshData
	@brief		メッシュ内のマテリアルグループごとのポリゴン形成データ
*/
USTRUCT()
struct FLiveLinkExtendMaterialGroupMeshData
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendMaterialGroupMeshData()
	{
	}

	FString		MaterialName;
	int			Triangles;

	TArray<FLiveLinkExtendPolygon>	PolygonList;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendMaterialGroupMeshData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->MaterialName;
		Ar << this->Triangles;

		SerializeTArray( Ar, this->PolygonList );

		return Ar;
	}


	bool operator==( const FLiveLinkExtendMaterialGroupMeshData& rhs ) const
	{
		if( this->MaterialName != rhs.MaterialName )
		{
			return false;
		}

		if( this->Triangles != rhs.Triangles )
		{
			return false;
		}


		auto myNumPolygon		= this->PolygonList.Num();
		auto otherNumPolygon	= rhs.PolygonList.Num();
		if( myNumPolygon != otherNumPolygon )
		{
			return false;
		}

		{

			for( int iPolygon = 0; iPolygon < myNumPolygon; ++iPolygon )
			{
				if( this->PolygonList[ iPolygon ] != rhs.PolygonList[ iPolygon ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendMaterialGroupMeshData& rhs ) const
	{
		return !( *this == rhs );
	}
};


/*! @struct		FLiveLinkExtendMeshMorphVertexData
	@brief		モーフターゲットの頂点ごとのデータ
*/
USTRUCT()
struct FLiveLinkExtendMeshMorphVertexData
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendPoint3		DeltaPoint;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendMeshMorphVertexData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		this->DeltaPoint << Ar;

		return Ar;
	}


	bool operator==( const FLiveLinkExtendMeshMorphVertexData& rhs ) const
	{
		return this->DeltaPoint == rhs.DeltaPoint;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendMeshMorphVertexData& rhs ) const
	{
		return !( *this == rhs );
	}
};


/*! @struct		FLiveLinkExtendMeshMorphData
	@brief		モーフターゲットごとのまとめデータ
*/
USTRUCT()
struct FLiveLinkExtendMeshMorphData
{
	GENERATED_USTRUCT_BODY()

	FString		MorphName;
	TArray<FLiveLinkExtendMeshMorphVertexData>	VertexList;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendMeshMorphData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->MorphName;

		SerializeTArray( Ar, this->VertexList );

		return Ar;
	}

	bool operator==( const FLiveLinkExtendMeshMorphData& rhs ) const
	{
		if( this->MorphName != rhs.MorphName )
		{
			return false;
		}

		auto myNumVertex		= this->VertexList.Num();
		auto otherNumVertex		= rhs.VertexList.Num();
		if( myNumVertex != otherNumVertex )
		{
			return false;
		}

		{

			for( int iVertex = 0; iVertex < myNumVertex; ++iVertex )
			{
				if( this->VertexList[ iVertex ] != rhs.VertexList[ iVertex ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendMeshMorphData& rhs ) const
	{
		return !( *this == rhs );
	}
};

/*! @struct		FLiveLinkExtendVertexMeshData
	@brief		メッシュ単位の保持データ
*/
USTRUCT()
struct FLiveLinkExtendVertexMeshData
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendVertexMeshData()
		: NumUseUVs( 1 )
		, NumUseColors( 1 )
	{
	}

	FString		MeshName;
	FDateTime	TimeStamp;

	TArray<FLiveLinkExtendMaterialGroupMeshData>	MaterialGroupList;
	TArray<FLiveLinkExtendPoint3>	PointList;
	TArray<FLiveLinkPointSkin>		PointSkinList;
	TArray<FLiveLinkExtendPoint3>	PointNormalList;

	TArray<FString>							SkinNameList;

	TArray<FLiveLinkExtendMeshMorphData>	MorphList;

	int		NumUseUVs;
	int		NumUseColors;

	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendVertexMeshData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->MeshName;
		Ar << this->TimeStamp;

		SerializeTArray( Ar, this->MaterialGroupList );
		SerializeTArray( Ar, this->PointList );
		SerializeTArray( Ar, this->PointSkinList );
		SerializeTArray( Ar, this->PointNormalList );

		Ar << this->SkinNameList;

		SerializeTArray( Ar, this->MorphList );

		Ar << this->NumUseUVs;
		Ar << this->NumUseColors;

		return Ar;
	}


	bool operator==( const FLiveLinkExtendVertexMeshData& rhs ) const
	{
		if( this->MeshName != rhs.MeshName )
		{
			return false;
		}

		if( this->NumUseUVs != rhs.NumUseUVs )
		{
			return false;
		}

		if( this->NumUseColors != rhs.NumUseColors )
		{
			return false;
		}

		auto myNumMaterialGroup		= this->MaterialGroupList.Num();
		auto otherNumMaterialGroup	= rhs.MaterialGroupList.Num();
		if( myNumMaterialGroup != otherNumMaterialGroup )
		{
			return false;
		}


		auto myNumPoint		= this->PointList.Num();
		auto otherNumPoint	= rhs.PointList.Num();
		if( myNumPoint != otherNumPoint )
		{
			return false;
		}


		auto myNumPointSkin		= this->PointSkinList.Num();
		auto otherNumPointSkin	= rhs.PointSkinList.Num();
		if( myNumPointSkin != otherNumPointSkin )
		{
			return false;
		}


		auto myNumPointNormal		= this->PointNormalList.Num();
		auto otherNumPointNormal	= rhs.PointNormalList.Num();
		if( myNumPointNormal != otherNumPointNormal )
		{
			return false;
		}


		auto myNumSkin		= this->SkinNameList.Num();
		auto otherNumSkin	= rhs.SkinNameList.Num();
		if( myNumSkin != otherNumSkin )
		{
			return false;
		}


		auto myNumMorph		= this->MorphList.Num();
		auto otherNumMorph	= rhs.MorphList.Num();
		if( myNumMorph != otherNumMorph )
		{
			return false;
		}



		{
			for( int iMaterialGroup = 0; iMaterialGroup < myNumMaterialGroup; ++iMaterialGroup )
			{
				if( this->MaterialGroupList[ iMaterialGroup ] != rhs.MaterialGroupList[ iMaterialGroup ] )
				{
					return false;
				}
			}
		}

		{
			for( int iPoint = 0; iPoint < myNumPoint; ++iPoint )
			{
				if( this->PointList[ iPoint ] != rhs.PointList[ iPoint ] )
				{
					return false;
				}
			}
		}

		{
			for( int iPointSkin = 0; iPointSkin < myNumPointSkin; ++iPointSkin )
			{
				if( this->PointSkinList[ iPointSkin ] != rhs.PointSkinList[ iPointSkin ] )
				{
					return false;
				}
			}
		}

		{
			for( int iPointNormal = 0; iPointNormal < myNumPointNormal; ++iPointNormal )
			{
				if( this->PointNormalList[ iPointNormal ] != rhs.PointNormalList[ iPointNormal ] )
				{
					return false;
				}
			}
		}

		{
			for( int iSkin = 0; iSkin < myNumSkin; ++iSkin )
			{
				if( this->SkinNameList[ iSkin ] != rhs.SkinNameList[ iSkin ] )
				{
					return false;
				}
			}
		}

		{
			for( int iMorph = 0; iMorph < myNumMorph; ++iMorph )
			{
				if( this->MorphList[ iMorph ] != rhs.MorphList[ iMorph ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendVertexMeshData& rhs ) const
	{
		return !( *this == rhs );
	}
};

/*! @struct		FLiveLinkExtendBoneBindPose
	@brief		ボーンのバインドポーズデータ
*/
USTRUCT()
struct FLiveLinkExtendBoneBindPose
{
	GENERATED_USTRUCT_BODY()


	FLiveLinkExtendBoneBindPose()
	{}

	FLiveLinkExtendBoneBindPose(
		const FString&		meshName,
		const FMatrix&		bindPose
	)
		: MeshName( meshName )
	{
		this->BindPose.SetFromMatrix( bindPose );
	}

	FLiveLinkExtendBoneBindPose(
		const FString&		meshName,
		const FTransform&	bindPose
	)
		: MeshName( meshName )
		, BindPose( bindPose )
	{}

	FString		MeshName;
	FTransform	BindPose;


	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendBoneBindPose*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		Ar << this->MeshName;
		Ar << this->BindPose;
		return Ar;
	}

	bool operator==( const FLiveLinkExtendBoneBindPose& rhs ) const
	{
		return (
			this->MeshName == rhs.MeshName &&
			this->BindPose.Equals( rhs.BindPose )
		);
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendBoneBindPose& rhs ) const
	{
		return !( *this == rhs );
	}

};



/*! @struct		FLiveLinkExtendSyncData
	@brief		メッシュ同期データのルート
*/
USTRUCT()
struct FLiveLinkExtendSyncData
{
	GENERATED_USTRUCT_BODY()

	TArray<FLiveLinkExtendVertexMeshData>	MeshList;
	TArray<FLiveLinkExtendBoneBindPose>		BoneBindPoseList;


	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendSyncData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		SerializeTArray( Ar, this->MeshList );
		SerializeTArray( Ar, this->BoneBindPoseList );
		return Ar;
	}



	bool operator==( const FLiveLinkExtendSyncData& rhs ) const
	{
		auto myMeshNum		= this->MeshList.Num();
		auto otherMeshNum	= rhs.MeshList.Num();
		if( myMeshNum != otherMeshNum )
		{
			return false;
		}

		auto myBindPoseNum		= this->BoneBindPoseList.Num();
		auto otherBindPoseNum	= rhs.BoneBindPoseList.Num();
		if( myBindPoseNum != otherBindPoseNum )
		{
			return false;
		}

		{
				
			for( int iMesh = 0; iMesh < myMeshNum; ++iMesh )
			{
				if( this->MeshList[ iMesh ] != rhs.MeshList[ iMesh ] )
				{
					return false;
				}
			}
		}

		{

			for( int iBindPose = 0; iBindPose < myBindPoseNum; ++iBindPose )
			{
				if( this->BoneBindPoseList[ iBindPose ] != rhs.BoneBindPoseList[ iBindPose ] )
				{
					return false;
				}
			}
		}
		return true;
	}

	FORCEINLINE bool operator!=( const FLiveLinkExtendSyncData& rhs ) const
	{
		return !( *this == rhs );
	}	
};

