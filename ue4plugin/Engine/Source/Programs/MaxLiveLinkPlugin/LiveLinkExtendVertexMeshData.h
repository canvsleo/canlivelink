
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
};


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
};

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
};

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
};

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
};

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
};

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
};

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
};

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
};


USTRUCT()
struct FLiveLinkExtendVertexMeshData
{
	GENERATED_USTRUCT_BODY()

	FLiveLinkExtendVertexMeshData()
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

		return Ar;
	}
};

USTRUCT()
struct FLiveLinkExtendSyncData
{
	GENERATED_USTRUCT_BODY()

	TArray<FLiveLinkExtendVertexMeshData>	MeshList;


	friend FArchive& operator<<( FArchive& Ar, FLiveLinkExtendSyncData*& res )
	{
		return *res << Ar;
	}
	FArchive& operator<<( FArchive& Ar )
	{
		return SerializeTArray( Ar, this->MeshList );
	}

};


