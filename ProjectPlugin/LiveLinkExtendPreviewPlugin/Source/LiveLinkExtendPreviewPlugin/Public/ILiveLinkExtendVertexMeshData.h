
#pragma once

#include "Serialization/JsonSerializerMacros.h"



struct ULiveLinkExtendPoint3
	: public FJsonSerializable
{
	ULiveLinkExtendPoint3()
		: X( 0.0f )
		, Y( 0.0f )
		, Z( 0.0f )
	{}

	ULiveLinkExtendPoint3(
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


	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE( "X", X );
		JSON_SERIALIZE( "Y", Y );
		JSON_SERIALIZE( "Z", Z );
	END_JSON_SERIALIZER
};

struct ULiveLinkExtendPoint2
	: public FJsonSerializable
{
	ULiveLinkExtendPoint2()
		: X( 0.0f )
		, Y( 0.0f )
	{}

	ULiveLinkExtendPoint2(
		float	x,
		float	y
	)
		: X( x )
		, Y( y )
	{}


	float	X;
	float	Y;


	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE( "X", X );
		JSON_SERIALIZE( "Y", Y );
	END_JSON_SERIALIZER
};

struct ULiveLinkExtendPoint4
	: public FJsonSerializable
{
	ULiveLinkExtendPoint4()
		: X( 0.0f )
		, Y( 0.0f )
		, Z( 0.0f )
		, W( 0.0f )
	{}

	ULiveLinkExtendPoint4(
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


	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE( "X", X );
		JSON_SERIALIZE( "Y", Y );
		JSON_SERIALIZE( "Z", Z );
		JSON_SERIALIZE( "W", W );
	END_JSON_SERIALIZER
};


struct ULiveLinkSkinLink
	: public FJsonSerializable
{
	ULiveLinkSkinLink()
		: SkinIndex( -1 )
		, SkinWeight( 0.0f )
	{}

	ULiveLinkSkinLink(
		int		skinIndex	,
		float	skinWeight	
	)
		: SkinIndex( skinIndex )
		, SkinWeight( skinWeight )
	{}


	int		SkinIndex;
	float	SkinWeight;


	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE( "SkinIndex", SkinIndex );
		JSON_SERIALIZE( "SkinWeight", SkinWeight );
	END_JSON_SERIALIZER
};

struct ULiveLinkPointSkin
	: public FJsonSerializable
{
	ULiveLinkPointSkin()
	{}

	TArray< ULiveLinkSkinLink >		SkinList;


	BEGIN_JSON_SERIALIZER
		{
			Serializer.StartArray( "SkinList" );
			for( auto& color : this->SkinList )
			{
				color.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}
	END_JSON_SERIALIZER
};

struct ULiveLinkExtendPolygon
	: public FJsonSerializable
{
	ULiveLinkExtendPolygon()
	{}

	uint32				SmoothingGroup;

	TArray<int>			VertexIndexList;
	TArray<ULiveLinkExtendPoint4>	ColorList0;

	TArray<ULiveLinkExtendPoint2>	UVList0;
	TArray<ULiveLinkExtendPoint2>	UVList1;
	TArray<ULiveLinkExtendPoint2>	UVList2;
	TArray<ULiveLinkExtendPoint2>	UVList3;


	BEGIN_JSON_SERIALIZER
	{
		{
			FJsonSerializableArray stringArray;
			for( auto integer : this->VertexIndexList )
			{
				stringArray.Push( FString::FromInt( integer ) );
			}
			JSON_SERIALIZE_ARRAY( "VertexIndexList", stringArray );
		}

		{
			Serializer.StartArray( "ColorList0" );
			for( auto& color : this->ColorList0 )
			{
				color.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

		{
			Serializer.StartArray( "UVList0" );
			for( auto& point : this->UVList0 )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}
		{
			Serializer.StartArray( "UVList1" );
			for( auto& point : this->UVList1 )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}
		{
			Serializer.StartArray( "UVList2" );
			for( auto& point : this->UVList2 )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}
		{
			Serializer.StartArray( "UVList3" );
			for( auto& point : this->UVList3 )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

		JSON_SERIALIZE( "SmoothingGroup", SmoothingGroup );
	}
	END_JSON_SERIALIZER
};


struct ULiveLinkExtendMaterialGroupMeshData
	: public FJsonSerializable
{

	ULiveLinkExtendMaterialGroupMeshData()
	{
	}

	FString		MaterialName;
	int			Triangles;

	TArray<ULiveLinkExtendPolygon>	PolygonList;

	BEGIN_JSON_SERIALIZER

		JSON_SERIALIZE( "MaterialName", MaterialName );
		JSON_SERIALIZE( "Triangles", Triangles );

		{
			Serializer.StartArray( "PolygonList" );
			for( auto& point : this->PolygonList )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

	END_JSON_SERIALIZER
};


struct ULiveLinkExtendVertexMeshData
	: public FJsonSerializable
{

	ULiveLinkExtendVertexMeshData()
	{
	}

	FString		MeshName;
	FDateTime	TimeStamp;

	TArray<ULiveLinkExtendMaterialGroupMeshData>	MaterialGroupList;
	TArray<ULiveLinkExtendPoint3>	PointList;
	TArray<ULiveLinkPointSkin>		PointSkinList;

	TArray<FString>					SkinNameList;


	BEGIN_JSON_SERIALIZER

		JSON_SERIALIZE( "MeshName", MeshName );
		JSON_SERIALIZE( "TimeStamp", TimeStamp );

		{
			Serializer.StartArray( "MaterialGroupList" );
			for( auto& point : this->MaterialGroupList )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

		{
			Serializer.StartArray( "PointList" );
			for( auto& point : this->PointList )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}
		{
			Serializer.StartArray( "PointSkinList" );
			for( auto& point : this->PointSkinList )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

		JSON_SERIALIZE_ARRAY( "SkinNameList", SkinNameList );

	END_JSON_SERIALIZER


};



struct ULiveLinkExtendSyncData
	: public FJsonSerializable
{
	TArray<ULiveLinkExtendVertexMeshData>	MeshList;

	BEGIN_JSON_SERIALIZER

		{
			Serializer.StartArray( "MeshList" );
			for( auto& point : this->MeshList )
			{
				point.Serialize( Serializer, bFlatObject );
			}
			Serializer.EndArray();
		}

	END_JSON_SERIALIZER

};


